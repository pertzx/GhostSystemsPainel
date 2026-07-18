#pragma once
#include "IFeature.h"
#include <thread>
#include <atomic>
#include "../Il2CppHelper.h"
#include "../Entity.h"

class GhostHackFeature : public IFeature {
public:
    GhostHackFeature();
    ~GhostHackFeature() override;
    void Start() override;
    void Stop() override;
    void RenderUI() override;
    const char* Name() const override { return "Ghost Hack"; }
    bool enabled = false;
private:
    void Worker();
    void RestorePosition();
    std::atomic<bool> running{false};
    std::thread thread;
};