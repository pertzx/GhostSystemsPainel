#pragma once
#include "IFeature.h"
#include <thread>
#include <atomic>
#include "../Il2CppHelper.h"

class AimMagnetFeature : public IFeature {
public:
    AimMagnetFeature();
    ~AimMagnetFeature() override;
    void Start() override;
    void Stop() override;
    void RenderUI() override;
    const char* Name() const override { return "Aim Magnet"; }
    bool enabled = false;
private:
    void Worker();
    std::atomic<bool> running{false};
    std::thread thread;
};
