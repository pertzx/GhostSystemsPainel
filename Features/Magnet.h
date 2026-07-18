#pragma once
#include "IFeature.h"
#include <thread>
#include <atomic>
#include "../Il2CppHelper.h"
#include "../Entity.h"

class MagnetFeature : public IFeature {
public:
    MagnetFeature();
    ~MagnetFeature() override;
    void Start() override;
    void Stop() override;
    void RenderUI() override;
    const char* Name() const override { return "Magnet"; }
    bool enabled = false;
    void SetEnabled(bool enabled, uint64_t target, bool isFiring);
private:
    void Worker();
    std::atomic<bool> running{false};
    std::thread thread;
};