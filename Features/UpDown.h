#pragma once
#include "IFeature.h"
#include <thread>
#include <atomic>
#include <chrono>

class UpDownFeature : public IFeature {
public:
    UpDownFeature();
    ~UpDownFeature() override;
    void Start() override;
    void Stop() override;
    void RenderUI() override;
    const char* Name() const override { return "Up/Down Player"; }
    
    void EnableUp(float speed = 0.5f);
    void EnableDown(float speed = 0.5f);
    void Disable();
    
    bool enableUp = false;
    bool enableDown = false;
    float upSpeed = 0.5f;
    float downSpeed = 0.5f;

private:
    void Worker();
    
    std::atomic<bool> running{false};
    std::thread thread;
    
    static bool GetRootMatrixAddr(uint64_t node, uint64_t& outAddr);
};