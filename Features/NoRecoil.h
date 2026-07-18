#pragma once
#include "IFeature.h"
#include <thread>
#include <atomic>

class NoRecoilFeature : public IFeature {
public:
    NoRecoilFeature();
    ~NoRecoilFeature() override;
    void Start() override; // Not used – controle via flag noRecoilEnabled
    void Stop() override; // Not used
    void RenderUI() override {}
    const char* Name() const override { return "No Recoil"; }
    void Update();
    bool enabled = false;
};
