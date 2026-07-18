#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <mutex>
#include <atomic>
#include <deque>
#include "Il2CppHelper.h"

#define NETPROXY_RING_SIZE 1000
#define NETPROXY_MAX_PAYLOAD_BYTES 8192

namespace GhostSystems {

enum class PacketDirection : uint8_t {
    DIR_OUTGOING  = 0,
    DIR_INCOMING  = 1,
    DIR_PLAINTEXT = 2,
};

struct CapturedPacket {
    uint64_t id;
    uint32_t cmd;
    uint8_t  region;
    PacketDirection direction;
    uint64_t timestamp_ms;
    uint32_t threadId;
    uint32_t size;
    uint8_t  payload[NETPROXY_MAX_PAYLOAD_BYTES];
    bool     isEncrypted;
    char     note[64];
};

class NetworkProxy {
public:
    static bool Start();
    static void Stop();

    static void RenderImGuiTab();

    static void OnPacketCaptured(uint32_t cmd, uint8_t region,
                                PacketDirection dir,
                                const uint8_t* data, uint32_t size,
                                bool isEncrypted,
                                const char* note = nullptr);

    static size_t GetPacketCount();
    static bool GetPacket(size_t idx, CapturedPacket& out);
    static void ClearPackets();

    static bool   HasAesKey();
    static const char* GetAesKeyHex();
    static const char* GetAesIvHex();
    static uint32_t GetKts();
    static uint64_t GetAccountId();

    static void CaptureAesKeyFromObject(void* aesManaged);

private:
    static bool ResolveClasses();
    static bool HookAll();
};

} // namespace GhostSystems
