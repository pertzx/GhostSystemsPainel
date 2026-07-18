#include "NetworkProxy.h"
#include "And64InlineHook.hpp"
#include <imgui.h>
#include <android/log.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>
#include <stdio.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO,  "NetProxy", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN,  "NetProxy", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "NetProxy", __VA_ARGS__))

namespace GhostSystems {

// ============================================================
// Estado global interno
// ============================================================
static std::mutex g_ringMutex;
static std::deque<CapturedPacket> g_ring;
static std::atomic<uint64_t> g_nextId{1};

static std::atomic<bool> g_started{false};
static std::atomic<bool> g_enabled{true};

// Classes Il2Cpp resolvidas
static void* g_klassTCPSession2 = nullptr;
static void* g_klassTCPMsgPacket = nullptr;
static void* g_klassAesManaged = nullptr;
static void* g_klassBinaryWriter = nullptr;

// Metodos
static void* g_methodSend = nullptr;        // TCPSession2.Send
static void* g_methodEncrypt = nullptr;      // TCPSession2.Encrypt
static void* g_methodSerialize = nullptr;    // TCPMsgPacket.Serialize
static void* g_methodOnRecv = nullptr;       // TCPSession2.OnRecvDataThread
static void* g_methodGetKey = nullptr;       // AesManaged.get_Key
static void* g_methodGetIV  = nullptr;       // AesManaged.get_IV

// Originais apos hook (trampolins)
static void* g_origSend = nullptr;
static void* g_origEncrypt = nullptr;
static void* g_origSerialize = nullptr;
static void* g_origOnRecv = nullptr;

// Chave AES capturada
static uint8_t  g_aesKey[32] = {0};
static uint8_t  g_aesIv[16]  = {0};
static bool     g_haveAesKey = false;
static uint32_t g_kts = 0;
static uint64_t g_accountId = 0;
static char     g_aesKeyHex[65] = {0};
static char     g_aesIvHex[33] = {0};

// Configuracao UI
static bool g_uiCaptureOutgoing = true;
static bool g_uiCaptureIncoming = true;
static bool g_uiCapturePlaintext = true;
static int  g_uiSelectedIdx = -1;
static char g_uiCmdFilter[16] = "";
static bool g_uiAutoScroll = true;
static int  g_uiMaxDisplay = 200;

// ============================================================
// Helpers
// ============================================================
static uint64_t NowMs() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
}

static uint32_t GetTid() {
    return (uint32_t)syscall(SYS_gettid);
}

static void BytesToHex(const uint8_t* data, size_t len, char* out) {
    static const char* hex = "0123456789ABCDEF";
    for (size_t i = 0; i < len; i++) {
        out[i*2]   = hex[(data[i] >> 4) & 0xF];
        out[i*2+1] = hex[data[i] & 0xF];
    }
    out[len*2] = 0;
}

// Le um Byte[] Il2Cpp (objeto array com header)
// Layout tipico do Il2Cpp array: 0x10 = bounds, 0x18 = length, 0x20 = primeiro elemento
struct Il2CppArray {
    void* klass;        // 0x00
    void* monitor;      // 0x08
    void* bounds;       // 0x10
    size_t length;      // 0x18
    uint8_t data[1];    // 0x20
};

static bool ReadByteArray(void* arrayOfByte, uint8_t* outBuf, size_t maxOut, size_t* outLen) {
    if (!arrayOfByte) return false;
    Il2CppArray* arr = (Il2CppArray*)arrayOfByte;
    if (arr->length == 0) { if (outLen) *outLen = 0; return true; }
    size_t copyLen = arr->length > maxOut ? maxOut : arr->length;
    memcpy(outBuf, arr->data, copyLen);
    if (outLen) *outLen = arr->length;
    return true;
}

// ============================================================
// Captura de pacote (API publica)
// ============================================================
void NetworkProxy::OnPacketCaptured(uint32_t cmd, uint8_t region,
                                    PacketDirection dir,
                                    const uint8_t* data, uint32_t size,
                                    bool isEncrypted,
                                    const char* note) {
    if (!g_enabled.load()) return;

    CapturedPacket pkt;
    pkt.id           = g_nextId.fetch_add(1);
    pkt.cmd          = cmd;
    pkt.region       = region;
    pkt.direction    = dir;
    pkt.timestamp_ms = NowMs();
    pkt.threadId     = GetTid();
    pkt.size         = size;
    pkt.isEncrypted  = isEncrypted;
    memset(pkt.note, 0, sizeof(pkt.note));
    if (note) {
        strncpy(pkt.note, note, sizeof(pkt.note) - 1);
    }

    uint32_t copyLen = size > NETPROXY_MAX_PAYLOAD_BYTES ? NETPROXY_MAX_PAYLOAD_BYTES : size;
    if (copyLen && data) {
        memcpy(pkt.payload, data, copyLen);
    } else {
        copyLen = 0;
    }

    {
        std::lock_guard<std::mutex> lock(g_ringMutex);
        g_ring.push_back(std::move(pkt));
        while (g_ring.size() > NETPROXY_RING_SIZE) g_ring.pop_front();
    }
}

size_t NetworkProxy::GetPacketCount() {
    std::lock_guard<std::mutex> lock(g_ringMutex);
    return g_ring.size();
}

bool NetworkProxy::GetPacket(size_t idx, CapturedPacket& out) {
    std::lock_guard<std::mutex> lock(g_ringMutex);
    if (idx >= g_ring.size()) return false;
    out = g_ring[idx];
    return true;
}

void NetworkProxy::ClearPackets() {
    std::lock_guard<std::mutex> lock(g_ringMutex);
    g_ring.clear();
}

// ============================================================
// Captura da chave AES (Hook #2)
// ============================================================
void NetworkProxy::CaptureAesKeyFromObject(void* aesManaged) {
    if (!aesManaged || !g_methodGetKey || !g_methodGetIV) return;
    if (g_haveAesKey) return;

    void* exc = nullptr;
    void* keyObj = Il2Cpp::runtime_invoke(g_methodGetKey, aesManaged, nullptr, &exc);
    if (!keyObj || exc) return;
    void* ivObj = Il2Cpp::runtime_invoke(g_methodGetIV, aesManaged, nullptr, &exc);
    if (!ivObj || exc) return;

    size_t keyLen = 0, ivLen = 0;
    if (!ReadByteArray(keyObj, g_aesKey, sizeof(g_aesKey), &keyLen)) return;
    if (!ReadByteArray(ivObj, g_aesIv, sizeof(g_aesIv), &ivLen)) return;

    if (keyLen == 0) return;
    g_haveAesKey = true;
    BytesToHex(g_aesKey, keyLen, g_aesKeyHex);
    BytesToHex(g_aesIv, ivLen, g_aesIvHex);
    LOGI("[NetProxy] AES Key capturada (keyLen=%zu, ivLen=%zu): KEY=%s IV=%s",
         keyLen, ivLen, g_aesKeyHex, g_aesIvHex);
}

bool   NetworkProxy::HasAesKey()    { return g_haveAesKey; }
const char* NetworkProxy::GetAesKeyHex() { return g_aesKeyHex; }
const char* NetworkProxy::GetAesIvHex()  { return g_aesIvHex; }
uint32_t NetworkProxy::GetKts()        { return g_kts; }
uint64_t NetworkProxy::GetAccountId()  { return g_accountId; }

// ============================================================
// Hook #1: TCPSession2.Send(uint cmd, Byte[] data, int count, Byte region)
// Signature: void Send(this, uint32 cmd, void* data, int count, uint8 region)
// ============================================================
typedef void (*Send_t)(void* self, uint32_t cmd, void* data, int32_t count, uint8_t region);

static Send_t g_fnSend = nullptr;

static void hook_TCPSession2_Send(void* self, uint32_t cmd, void* data, int32_t count, uint8_t region) {
    if (g_uiCaptureOutgoing && data && count > 0) {
        uint8_t buf[NETPROXY_MAX_PAYLOAD_BYTES];
        uint32_t copyLen = count > (int32_t)NETPROXY_MAX_PAYLOAD_BYTES ? NETPROXY_MAX_PAYLOAD_BYTES : count;
        memcpy(buf, ((Il2CppArray*)data)->data, copyLen);
        NetworkProxy::OnPacketCaptured(cmd, region, PacketDirection::DIR_OUTGOING,
                                       buf, count, true, "TCP.Send");
    }

    // Captura chave AES do objeto TCPSession2.m_Aes (offset 0xb8)
    if (!g_haveAesKey && self) {
        void* aesField = *(void**)((uint8_t*)self + 0xb8);
        if (aesField) {
            NetworkProxy::CaptureAesKeyFromObject(aesField);
        }
        // AccessValidInfo m_AccessValidInfo (offset 0xe8)
        void* avi = *(void**)((uint8_t*)self + 0xe8);
        if (avi) {
            g_kts        = *(uint32_t*)((uint8_t*)avi + 0x10);
            void* ak     = *(void**)((uint8_t*)avi + 0x18);
            void* aiv    = *(void**)((uint8_t*)avi + 0x20);
            g_accountId  = *(uint64_t*)((uint8_t*)avi + 0x28);
            if (ak && aiv && !g_haveAesKey) {
                size_t kl=0, il=0;
                if (ReadByteArray(ak, g_aesKey, sizeof(g_aesKey), &kl) &&
                    ReadByteArray(aiv, g_aesIv, sizeof(g_aesIv), &il) && kl > 0) {
                    g_haveAesKey = true;
                    BytesToHex(g_aesKey, kl, g_aesKeyHex);
                    BytesToHex(g_aesIv, il, g_aesIvHex);
                    LOGI("[NetProxy] AES via AccessValidInfo: KEY=%s IV=%s kts=%u acc=%llu",
                         g_aesKeyHex, g_aesIvHex, g_kts, (unsigned long long)g_accountId);
                }
            }
        }
    }

    if (g_fnSend) g_fnSend(self, cmd, data, count, region);
    else if (g_origSend) ((Send_t)g_origSend)(self, cmd, data, count, region);
}

// ============================================================
// Hook #2: TCPSession2.Encrypt(AesManaged aes, Byte[] data) -> Byte[]
// Signature: void* Encrypt(this, void* aes, void* data)
// ============================================================
typedef void* (*Encrypt_t)(void* self, void* aes, void* data);

static Encrypt_t g_fnEncrypt = nullptr;

static void* hook_TCPSession2_Encrypt(void* self, void* aes, void* data) {
    if (aes && !g_haveAesKey) {
        NetworkProxy::CaptureAesKeyFromObject(aes);
    }
    if (g_uiCaptureOutgoing && data) {
        Il2CppArray* arr = (Il2CppArray*)data;
        if (arr->length > 0) {
            uint8_t buf[NETPROXY_MAX_PAYLOAD_BYTES];
            uint32_t copyLen = arr->length > NETPROXY_MAX_PAYLOAD_BYTES
                               ? NETPROXY_MAX_PAYLOAD_BYTES : (uint32_t)arr->length;
            memcpy(buf, arr->data, copyLen);
            NetworkProxy::OnPacketCaptured(0, 0, PacketDirection::DIR_PLAINTEXT,
                                           buf, (uint32_t)arr->length, false, "Encrypt-in");
        }
    }

    void* result = nullptr;
    if (g_fnEncrypt) result = g_fnEncrypt(self, aes, data);
    else if (g_origEncrypt) result = ((Encrypt_t)g_origEncrypt)(self, aes, data);

    if (g_uiCaptureOutgoing && result) {
        Il2CppArray* arr = (Il2CppArray*)result;
        if (arr->length > 0) {
            uint8_t buf[NETPROXY_MAX_PAYLOAD_BYTES];
            uint32_t copyLen = arr->length > NETPROXY_MAX_PAYLOAD_BYTES
                               ? NETPROXY_MAX_PAYLOAD_BYTES : (uint32_t)arr->length;
            memcpy(buf, arr->data, copyLen);
            NetworkProxy::OnPacketCaptured(0, 0, PacketDirection::DIR_OUTGOING,
                                           buf, (uint32_t)arr->length, true, "Encrypt-out");
        }
    }
    return result;
}

// ============================================================
// Hook #3: TCPMsgPacket.Serialize(BinaryWriter writer)
// Signature: void Serialize(this, void* writer)
// ============================================================
typedef void (*Serialize_t)(void* self, void* writer);

static Serialize_t g_fnSerialize = nullptr;

static void hook_TCPMsgPacket_Serialize(void* self, void* writer) {
    if (g_uiCapturePlaintext && self) {
        // TCPMsgPacket fields:
        //   Cmd               @ 0x10 (uint8)
        //   Region            @ 0x11 (uint8)
        //   Length            @ 0x14 (int32)
        //   Data              @ 0x18 (Byte[])
        //   accessValidInfo   @ 0x28
        uint8_t  cmd    = *(uint8_t*) ((uint8_t*)self + 0x10);
        uint8_t  region = *(uint8_t*) ((uint8_t*)self + 0x11);
        int32_t  length = *(int32_t*) ((uint8_t*)self + 0x14);
        void*   dataArr = *(void**)  ((uint8_t*)self + 0x18);

        if (dataArr && length > 0) {
            Il2CppArray* arr = (Il2CppArray*)dataArr;
            uint32_t realLen = (uint32_t)arr->length;
            uint32_t copyLen = realLen > NETPROXY_MAX_PAYLOAD_BYTES
                               ? NETPROXY_MAX_PAYLOAD_BYTES : realLen;
            uint8_t buf[NETPROXY_MAX_PAYLOAD_BYTES];
            memcpy(buf, arr->data, copyLen);
            NetworkProxy::OnPacketCaptured(cmd, region, PacketDirection::DIR_PLAINTEXT,
                                           buf, realLen, false, "Serialize");
        } else if (length == 0) {
            NetworkProxy::OnPacketCaptured(cmd, region, PacketDirection::DIR_PLAINTEXT,
                                           nullptr, 0, false, "Serialize-empty");
        }
    }

    if (g_fnSerialize) g_fnSerialize(self, writer);
    else if (g_origSerialize) ((Serialize_t)g_origSerialize)(self, writer);
}

// ============================================================
// Hook #4: TCPSession2.OnRecvDataThread()
// Signature: void OnRecvDataThread(this)
// ============================================================
typedef void (*OnRecv_t)(void* self);

static OnRecv_t g_fnOnRecv = nullptr;

static void hook_TCPSession2_OnRecvDataThread(void* self) {
    if (g_uiCaptureIncoming && self) {
        // NetworkByteStream m_RecvBuffer @ 0x40
        // Apenas registra inicio/fim do recv-loop para diagnostico
        NetworkProxy::OnPacketCaptured(0, 0, PacketDirection::DIR_INCOMING,
                                       nullptr, 0, true, "Recv-loop-start");
    }
    if (g_fnOnRecv) g_fnOnRecv(self);
    else if (g_origOnRecv) ((OnRecv_t)g_origOnRecv)(self);
}

// ============================================================
// Resolucao de classes IL2CPP
// ============================================================
bool NetworkProxy::ResolveClasses() {
    // Assembly-CSharp.dll
    g_klassTCPSession2  = Il2Cpp::GetClass("Assembly-CSharp.dll", "COW", "TCPSession2");
    g_klassTCPMsgPacket = Il2Cpp::GetClass("Assembly-CSharp.dll", "GCommon", "TCPMsgPacket");
    //mscorlib
    g_klassAesManaged   = Il2Cpp::GetClass("mscorlib.dll", "System.Security.Cryptography", "AesManaged");
    g_klassBinaryWriter = Il2Cpp::GetClass("mscorlib.dll", "System.IO", "BinaryWriter");

    LOGI("[NetProxy] Resolved: TCPSession2=%p TCPMsgPacket=%p AesManaged=%p BinaryWriter=%p",
         g_klassTCPSession2, g_klassTCPMsgPacket, g_klassAesManaged, g_klassBinaryWriter);

    if (!g_klassTCPSession2 || !g_klassTCPMsgPacket || !g_klassAesManaged) {
        LOGE("[NetProxy] Falha ao resolver classes Il2Cpp");
        return false;
    }

    g_methodSend      = Il2Cpp::GetMethodRecursively(g_klassTCPSession2,  "Send", 4);
    g_methodEncrypt   = Il2Cpp::GetMethodRecursively(g_klassTCPSession2,  "Encrypt", 2);
    g_methodOnRecv    = Il2Cpp::GetMethodRecursively(g_klassTCPSession2,  "OnRecvDataThread", 0);
    g_methodSerialize = Il2Cpp::GetMethodRecursively(g_klassTCPMsgPacket, "Serialize", 1);
    g_methodGetKey    = Il2Cpp::GetMethodRecursively(g_klassAesManaged,   "get_Key", 0);
    g_methodGetIV     = Il2Cpp::GetMethodRecursively(g_klassAesManaged,   "get_IV", 0);

    LOGI("[NetProxy] Methods: Send=%p Encrypt=%p OnRecv=%p Serialize=%p getKey=%p getIV=%p",
         g_methodSend, g_methodEncrypt, g_methodOnRecv, g_methodSerialize, g_methodGetKey, g_methodGetIV);

    if (!g_methodSend || !g_methodEncrypt || !g_methodOnRecv || !g_methodSerialize) {
        LOGE("[NetProxy] Falha ao resolver metodos");
        return false;
    }
    if (!g_methodGetKey || !g_methodGetIV) {
        LOGW("[NetProxy] get_Key/get_IV nao encontrados (captura AES via objeto)");
    }
    return true;
}

// ============================================================
// Instalacao dos hooks
// ============================================================
bool NetworkProxy::HookAll() {
    // O ponteiro da funcao nativa fica no campo methodPtr (offset 0x0 do MethodInfo)
    void* pSend      = *(void**)g_methodSend;
    void* pEncrypt   = *(void**)g_methodEncrypt;
    void* pOnRecv    = *(void**)g_methodOnRecv;
    void* pSerialize = *(void**)g_methodSerialize;

    LOGI("[NetProxy] Hook targets: Send=%p Encrypt=%p OnRecv=%p Serialize=%p",
         pSend, pEncrypt, pOnRecv, pSerialize);

    A64HookFunction(pSend,      (void*)hook_TCPSession2_Send,         (void**)&g_origSend);
    A64HookFunction(pEncrypt,   (void*)hook_TCPSession2_Encrypt,      (void**)&g_origEncrypt);
    A64HookFunction(pOnRecv,    (void*)hook_TCPSession2_OnRecvDataThread, (void**)&g_origOnRecv);
    A64HookFunction(pSerialize, (void*)hook_TCPMsgPacket_Serialize,   (void**)&g_origSerialize);

    LOGI("[NetProxy] Hooks instalados. orig: Send=%p Encrypt=%p OnRecv=%p Serialize=%p",
         g_origSend, g_origEncrypt, g_origOnRecv, g_origSerialize);
    return true;
}

bool NetworkProxy::Start() {
    if (g_started.load()) return true;
    if (!ResolveClasses()) return false;
    if (!HookAll()) return false;
    g_started.store(true);
    g_enabled.store(true);
    LOGI("[NetProxy] Iniciado com sucesso");
    return true;
}

void NetworkProxy::Stop() {
    g_enabled.store(false);
    LOGI("[NetProxy] Parado (A64Hook permanece instalado)");
}

// ============================================================
// UI ImGui
// ============================================================
static void DrawHexDump(const uint8_t* data, uint32_t size) {
    if (size == 0) {
        ImGui::TextDisabled("(vazio)");
        return;
    }
    // Mostra em chunks de 16 bytes
    uint32_t shown = size > 2048 ? 2048 : size;
    for (uint32_t off = 0; off < shown; off += 16) {
        ImGui::Text("%04X: ", off);
        ImGui::SameLine();
        char line[80] = {0};
        char* p = line;
        for (uint32_t i = 0; i < 16 && (off + i) < shown; i++) {
            p += snprintf(p, sizeof(line) - (p - line), "%02X ", data[off + i]);
        }
        ImGui::TextUnformatted(line);
    }
    if (size > 2048) {
        ImGui::TextDisabled("... (%u bytes truncados)", size - 2048);
    }
}

void NetworkProxy::RenderImGuiTab() {
    if (!ImGui::CollapsingHeader("Network Monitor")) return;

    ImGui::Checkbox("Capturar TX", &g_uiCaptureOutgoing); ImGui::SameLine();
    ImGui::Checkbox("Capturar RX", &g_uiCaptureIncoming); ImGui::SameLine();
    ImGui::Checkbox("Plaintext", &g_uiCapturePlaintext);

    ImGui::Separator();

    if (ImGui::Button("Iniciar Hooks")) { Start(); }
    ImGui::SameLine();
    if (ImGui::Button("Parar Captura")) { Stop(); }
    ImGui::SameLine();
    if (ImGui::Button("Limpar")) { ClearPackets(); }

    ImGui::Separator();

    // Estado AES
    if (HasAesKey()) {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "AES Key capturada");
        ImGui::Text("Key: %s", GetAesKeyHex());
        ImGui::Text("IV:  %s", GetAesIvHex());
        ImGui::Text("kts: %u   accountID: %llu", GetKts(), (unsigned long long)GetAccountId());
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.2f, 1.0f),
            "AES Key ainda nao capturada (aguardando envio de pacote)...");
    }

    ImGui::Separator();

    // Filtro por cmd
    ImGui::Text("Filtro cmd:");
    ImGui::SameLine();
    ImGui::PushItemWidth(80);
    ImGui::InputText("##cmdfilter", g_uiCmdFilter, sizeof(g_uiCmdFilter));
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &g_uiAutoScroll);
    ImGui::SameLine();
    ImGui::Text("Total: %zu   Exibindo: %d", GetPacketCount(), g_uiMaxDisplay);

    // Tabela de pacotes
    size_t total = GetPacketCount();
    size_t start = total > (size_t)g_uiMaxDisplay ? total - g_uiMaxDisplay : 0;
    size_t shown = total - start;

    if (ImGui::BeginChild("##packetlist", ImVec2(0, 300), true)) {
        ImGui::Columns(7, "packets");
        ImGui::TextDisabled("id");            ImGui::NextColumn();
        ImGui::TextDisabled("dir");           ImGui::NextColumn();
        ImGui::TextDisabled("cmd");           ImGui::NextColumn();
        ImGui::TextDisabled("region");        ImGui::NextColumn();
        ImGui::TextDisabled("size");          ImGui::NextColumn();
        ImGui::TextDisabled("enc");           ImGui::NextColumn();
        ImGui::TextDisabled("note");          ImGui::NextColumn();
        ImGui::Separator();

        long filterCmd = -1;
        if (g_uiCmdFilter[0]) filterCmd = atol(g_uiCmdFilter);

        for (size_t i = start; i < total; i++) {
            CapturedPacket pkt;
            if (!GetPacket(i, pkt)) continue;
            if (filterCmd >= 0 && pkt.cmd != (uint32_t)filterCmd) continue;

            ImGui::Text("%llu", (unsigned long long)pkt.id); ImGui::NextColumn();
            const char* d = (pkt.direction == PacketDirection::DIR_OUTGOING)  ? "TX" :
                            (pkt.direction == PacketDirection::DIR_INCOMING)  ? "RX" : "PL";
            ImGui::TextUnformatted(d); ImGui::NextColumn();
            ImGui::Text("%u", pkt.cmd); ImGui::NextColumn();
            ImGui::Text("%u", pkt.region); ImGui::NextColumn();
            ImGui::Text("%u", pkt.size); ImGui::NextColumn();
            ImGui::Text("%s", pkt.isEncrypted ? "AES" : "clr"); ImGui::NextColumn();
            ImGui::TextUnformatted(pkt.note); ImGui::NextColumn();

            if (ImGui::IsItemClicked()) {
                g_uiSelectedIdx = (int)i;
            }
        }
        ImGui::Columns(1);
        if (g_uiAutoScroll) {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();

    // Detalhes do pacote selecionado
    if (g_uiSelectedIdx >= 0) {
        CapturedPacket pkt;
        if (GetPacket((size_t)g_uiSelectedIdx, pkt)) {
            ImGui::Separator();
            ImGui::Text("Pacote #%llu  cmd=%u  region=%u  dir=%s  size=%u  enc=%s",
                        (unsigned long long)pkt.id, pkt.cmd, pkt.region,
                        (pkt.direction == PacketDirection::DIR_OUTGOING) ? "TX" :
                        (pkt.direction == PacketDirection::DIR_INCOMING) ? "RX" : "PL",
                        pkt.size, pkt.isEncrypted ? "true" : "false");
            ImGui::Spacing();
            DrawHexDump(pkt.payload, pkt.size > NETPROXY_MAX_PAYLOAD_BYTES
                                        ? NETPROXY_MAX_PAYLOAD_BYTES : pkt.size);
        }
    }
}

} // namespace GhostSystems
