#pragma once
#include "IFeature.h"
#include "../And64InlineHook.hpp"

class NikuHooksFeature : public IFeature {
public:
    NikuHooksFeature();
    ~NikuHooksFeature() override;
    void Start() override;
    void Stop() override;
    void RenderUI() override;
    const char* Name() const override { return "Niku Hooks"; }

    // Setters called from UI
    void SetSpeedRun(bool v);
    void SetSpeedRunLevel(int level);
    void SetGoldBody(bool v);
    void SetDoubleGun(bool v);
    void SetFreeMove(bool v);
    void SetMediRun(bool v);
    void SetNoRecoil(bool v);
    void SetMovementShoot(bool v);

private:
    // Feature enable flags
    static bool speedRunEnabled;
    static bool goldBodyEnabled;
    static bool doubleGunEnabled;
    static bool freeMoveEnabled;
    static bool mediRunEnabled;
    static bool movementShootEnabled;
    static bool noRecoilEnabled;
    static bool norecoil0odEnabled;
    static int speedRunLevel;

    // Original function pointers
    static float (*orig_GetDashSpeedScale)(void*);
    static bool  (*orig_get_IsGod)(void*);
    static bool  (*orig_GetCombineType)(void*);
    static bool  (*orig_IsFoldWingGliding)(void*);
    static bool  (*orig_IsFreeMove)(void*);
    static bool  (*orig_GetScatterRate)(void*, int);
    static bool  (*orig_GenerateRandomDirection)(void*);
    static bool  (*orig_OnPreparationCancel)(void*);
    static bool  (*orig_IsIgnoreHighFalling)(void*);
    static bool  (*orig_get_IsPoseFallingHigh)(void*);
    static bool  (*orig_get_InFallingState)(void*);
    static bool  (*orig_GetPhysXStateType)(void*);
    static bool  (*orig_GetPhysXState)(void*);
    static bool  (*orig_get_HasTriggerGrenadeOnHand)(void*);

    // Hook implementations
    static float Hook_GetDashSpeedScale(void* instance);
    static bool  Hook_get_IsGod(void* thiz);
    static bool  Hook_GetCombineType(void* thiz);
    static bool  Hook_IsFoldWingGliding(void* thiz);
    static bool  Hook_IsFreeMove(void* thiz);
    static bool  Hook_GetScatterRate(void* thiz, int value);
    static bool  Hook_GenerateRandomDirection(void* thiz);
    static bool  Hook_OnPreparationCancel(void* thiz);
    static bool  Hook_IsIgnoreHighFalling(void* thiz);
    static bool  Hook_get_IsPoseFallingHigh(void* thiz);
    static bool  Hook_get_InFallingState(void* thiz);
    static bool  Hook_GetPhysXStateType(void* thiz);
    static bool  Hook_GetPhysXState(void* thiz);
    static bool  Hook_get_HasTriggerGrenadeOnHand(void* thiz);

    void InstallHooks();
};