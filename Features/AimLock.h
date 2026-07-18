#pragma once
#include "IFeature.h"
#include <thread>
#include <atomic>
#include "../Il2CppHelper.h"

class AimLockFeature : public IFeature {
public:
    AimLockFeature();
    ~AimLockFeature() override;
    void Start() override;
    void Stop() override;
    void RenderUI() override;
    const char* Name() const override { return "Aim Lock"; }
    bool enabled = false;
private:
    void Worker();
    std::atomic<bool> running{false};
    std::thread thread;
};
