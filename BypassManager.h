#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <functional>
#include <jni.h>
#include <android/log.h>

#define LOG_TAG "GhostSystems"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace GhostSystems {

// ==================== BYPASS MANAGER ====================
// Sistema de bypass modular e seguro - NÃO usa hooks diretos no anticheat

enum class BypassStatus {
    NOT_APPLIED,
    APPLYING,
    APPLIED,
    FAILED
};

enum class BypassCategory {
    MEMORY_PROTECTION,
    ANTI_DEBUG,
    SIGNATURE_HIDE,
    DETECTION_DELAY,
    NETWORK_PROTECTION
};

struct BypassEntry {
    std::string name;
    std::string description;
    BypassCategory category;
    BypassStatus status;
    std::function<bool()> applyFunc;
    std::function<bool()> revertFunc;
    bool isApplied = false;
};

class BypassManager {
public:
    BypassManager();
    ~BypassManager();

    void Initialize();
    void ApplyAll();
    void RevertAll();
    void ApplyCategory(BypassCategory cat);
    
    std::vector<BypassEntry>& GetEntries() { return entries; }
    size_t GetAppliedCount() const;
    bool IsFullyProtected() const;

private:
    std::vector<BypassEntry> entries;
    bool initialized = false;

    // ===== TÉCNICAS DE BYPASS =====
    
    // 1. Proteção de Memória
    bool ApplyMemoryProtection();
    bool RevertMemoryProtection();
    
    // 2. Anti-Debug Stealth
    bool ApplyAntiDebugStealth();
    bool RevertAntiDebugStealth();
    
    // 3. Ocultação de Assinatura
    bool ApplySignatureHide();
    bool RevertSignatureHide();
    
    // 4. Delay de Detecção
    bool ApplyDetectionDelay();
    bool RevertDetectionDelay();
    
    // 5. Proteção de Rede
    bool ApplyNetworkProtection();
    bool RevertNetworkProtection();
};

// Instância global
extern BypassManager* g_BypassManager;

} // namespace GhostSystems
