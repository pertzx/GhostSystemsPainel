//============================================================================
// GhostSystems - BypassLoginSDK Header v2.1
// Com monitoramento visual em tempo real
//============================================================================
#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <imgui.h>

extern "C" void A64HookFunction(void* const symbol, void* const replace, void** result);

namespace GhostSystems {

//============================================================================
// CONFIGURACAO
//============================================================================
struct LoginSDKConfig {
    bool enableBypass = true;
    bool forceGuestLogin = true;
    bool spoofDeviceID = true;
    bool blockTelemetry = true;  // Filtra apenas dados do cheat
    bool logAllCalls = true;     // Logs no logcat E painel
};

//============================================================================
// INFO DE HOOK
//============================================================================
struct HookInfo {
    std::string name;
    std::string description;
    void* targetAddress;
    void* hookFunction;
    void** originalTrampoline;
    bool isApplied;
    bool isCritical;
    std::string lastError;
};

//============================================================================
// LOG ENTRY PARA VISUALIZACAO
//============================================================================
struct BypassLogEntry {
    std::string timestamp;
    std::string hookName;
    std::string action;      // PERMITIDO, BLOQUEADO, FILTRADO
    std::string details;
    ImVec4 color;
    uint64_t timestamp_ms;
};

//============================================================================
// BYPASS LOGIN SDK
//============================================================================
class BypassLoginSDK {
public:
    BypassLoginSDK();
    ~BypassLoginSDK();

    bool Initialize(const LoginSDKConfig& config = LoginSDKConfig{});
    bool ApplyHooks();
    void RemoveHooks();
    void UpdateConfig(const LoginSDKConfig& newConfig);

    bool AreHooksActive() const { return hooksActive; }
    const std::vector<HookInfo>& GetHookList() const { return hookList; }
    bool HasAnyHookApplied() const;

    // Monitoramento visual
    static const std::deque<BypassLogEntry>& GetLogs();
    static void ClearLogs();
    static std::mutex& GetLogsMutex();

    static BypassLoginSDK& Instance();

public:
    // Configuração (pública para acesso dos hooks)
    LoginSDKConfig config;

private:
    // Hooks
    static void*  Hook_LoginInit(void* instance, int loginType);
    static bool   Hook_VerifySession(void* instance);
    static const char* Hook_GetDeviceID();
    static bool   Hook_CheckRoot();
    static void   Hook_SendTelemetry(void* instance, void* data);
    static void   Hook_AntiCheatInit(void* instance);
    static bool   Hook_ValidateToken(void* instance, const char* token);
    static int    Hook_GetLoginType();
    static void   Hook_IntegrityCheck(void* instance);

    // Helpers
    bool SetupHook(const std::string& name, const std::string& desc,
                   void* target, void* hook, void** trampoline, bool critical);
    void* ResolveAddress(uintptr_t rva) const;

    // Membros
    std::vector<HookInfo> hookList;
    bool hooksActive = false;
    bool initialized = false;
    void* baseAddress = nullptr;

    void* origLoginInit = nullptr;
    void* origVerifySession = nullptr;
    void* origGetDeviceID = nullptr;
    void* origCheckRoot = nullptr;
    void* origSendTelemetry = nullptr;
    void* origAntiCheatInit = nullptr;
    void* origValidateToken = nullptr;
    void* origGetLoginType = nullptr;

    mutable std::string spoofedDeviceID;
    mutable bool deviceIDGenerated = false;
};

} // namespace GhostSystems