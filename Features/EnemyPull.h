#pragma once
#include "IFeature.h"
#include <thread>
#include <atomic>
#include "../Il2CppHelper.h"
#include "../Entity.h"

class EnemyPullFeature : public IFeature {
public:
    EnemyPullFeature();
    ~EnemyPullFeature() override;
    void Start() override;
    void Stop() override;
    void RenderUI() override;
    const char* Name() const override { return "Enemy Pull"; }
    bool enabled = false;
private:
    void Worker();
    void RestoreEnemyPosition();
    std::atomic<bool> running{false};
    std::thread thread;
};
