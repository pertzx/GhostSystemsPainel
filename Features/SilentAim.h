#pragma once

#include "IFeature.h"
#include <thread>
#include <atomic>
#include <mutex>
#include "../Il2CppHelper.h"
#include "../Entity.h"

// Feature Silent Aim baseada em RealSilentAim.h
// Captura a informação de direção alvo e escreve no offset de fim de trajetória.

class SilentAimFeature : public IFeature {
public:
    SilentAimFeature();
    ~SilentAimFeature() override;

    void Start() override;
    void Stop() override;
    void RenderUI() override;
    const char* Name() const override { return "Silent Aim"; }

    // Flag ativada pela UI
    bool enabled = false;

    // Inicia thread de aplicação (chamar uma vez no init do cheat)
    static void StartApplyThread();

    // Static members for thread communication
    static std::mutex silentLock;
    static void* g_HitObjInfo;
    static GhostSystems::Vector3 g_TargetDir;
    static bool g_HasData;

private:
    void Worker(); // loop que aplica a direção corrigida

    std::atomic<bool> running{false};
    std::thread workerThread;
};
