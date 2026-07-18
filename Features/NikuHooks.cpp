#include "NikuHooks.h"
#include <android/log.h>
#include "../Il2CppHelper.h"

#define LOG_TAG "NikuHooks"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Helper to resolve method pointer by name
static void* Il2CppGetMethodOffset(const char* assembly, const char* namespaze, const char* className, const char* methodName, int argsCount) {
    void* klass = Il2Cpp::GetClass(assembly, namespaze, className);
    if (!klass) return nullptr;
    void* method = Il2Cpp::GetMethodRecursively(klass, methodName, argsCount);
    return method;
}

// Initialize static members
bool NikuHooksFeature::speedRunEnabled = false;
bool NikuHooksFeature::goldBodyEnabled = false;
bool NikuHooksFeature::doubleGunEnabled = false;
bool NikuHooksFeature::freeMoveEnabled = false;
bool NikuHooksFeature::mediRunEnabled = false;
bool NikuHooksFeature::movementShootEnabled = false;
bool NikuHooksFeature::noRecoilEnabled = false;
bool NikuHooksFeature::norecoil0odEnabled = false;
int NikuHooksFeature::speedRunLevel = 5;

// Original function pointers
float (*NikuHooksFeature::orig_GetDashSpeedScale)(void*) = nullptr;
bool (*NikuHooksFeature::orig_get_IsGod)(void*) = nullptr;
bool (*NikuHooksFeature::orig_GetCombineType)(void*) = nullptr;
bool (*NikuHooksFeature::orig_IsFoldWingGliding)(void*) = nullptr;
bool (*NikuHooksFeature::orig_IsFreeMove)(void*) = nullptr;
bool (*NikuHooksFeature::orig_GetScatterRate)(void*, int) = nullptr;
bool (*NikuHooksFeature::orig_GenerateRandomDirection)(void*) = nullptr;
bool (*NikuHooksFeature::orig_OnPreparationCancel)(void*) = nullptr;
bool (*NikuHooksFeature::orig_IsIgnoreHighFalling)(void*) = nullptr;
bool (*NikuHooksFeature::orig_get_IsPoseFallingHigh)(void*) = nullptr;
bool (*NikuHooksFeature::orig_get_InFallingState)(void*) = nullptr;
bool (*NikuHooksFeature::orig_GetPhysXStateType)(void*) = nullptr;
bool (*NikuHooksFeature::orig_GetPhysXState)(void*) = nullptr;
bool (*NikuHooksFeature::orig_get_HasTriggerGrenadeOnHand)(void*) = nullptr;

// Hook implementations -------------------------------------------------

// SpeedRun - GetDashSpeedScale
float NikuHooksFeature::Hook_GetDashSpeedScale(void* instance) {
    if (speedRunEnabled && instance) {
        switch (speedRunLevel) {
            case 1: return 2.4f;
            case 2: return 2.5f;
            case 3: return 2.6f;
            case 4: return 2.7f;
            case 5: return 2.8f;
            case 6: return 2.9f;
            case 7: return 3.0f;
            case 8: return 3.1f;
            case 9: return 3.2f;
            default: return orig_GetDashSpeedScale ? orig_GetDashSpeedScale(instance) : 1.0f;
        }
    }
    return orig_GetDashSpeedScale ? orig_GetDashSpeedScale(instance) : 1.0f;
}

// GoldBody - get_IsGod
bool NikuHooksFeature::Hook_get_IsGod(void* thiz) {
    if (goldBodyEnabled && thiz) return true;
    return orig_get_IsGod ? orig_get_IsGod(thiz) : false;
}

// DoubleGun - GetCombineType
bool NikuHooksFeature::Hook_GetCombineType(void* thiz) {
    if (doubleGunEnabled && thiz) return true;
    return orig_GetCombineType ? orig_GetCombineType(thiz) : false;
}

// FreeMove - IsFoldWingGliding (SpeedFix)
bool NikuHooksFeature::Hook_IsFoldWingGliding(void* thiz) {
    if (speedRunEnabled && thiz) return true;
    return orig_IsFoldWingGliding ? orig_IsFoldWingGliding(thiz) : false;
}

// FreeMove - IsFreeMove
bool NikuHooksFeature::Hook_IsFreeMove(void* thiz) {
    if (movementShootEnabled && thiz) return true;
    return orig_IsFreeMove ? orig_IsFreeMove(thiz) : false;
}

// NoRecoil - GetScatterRate
bool NikuHooksFeature::Hook_GetScatterRate(void* thiz, int value) {
    if (noRecoilEnabled && thiz) return false; // 0 = no scatter
    return orig_GetScatterRate ? orig_GetScatterRate(thiz, value) : false;
}

// NoRecoil - GenerateRandomDirection (WeaponFireComponentLongRangeBase)
bool NikuHooksFeature::Hook_GenerateRandomDirection(void* thiz) {
    if (norecoil0odEnabled && thiz) return false;
    return orig_GenerateRandomDirection ? orig_GenerateRandomDirection(thiz) : false;
}

// MediRun - OnPreparationCancel
bool NikuHooksFeature::Hook_OnPreparationCancel(void* thiz) {
    if (mediRunEnabled && thiz) return true;
    return orig_OnPreparationCancel ? orig_OnPreparationCancel(thiz) : false;
}

// IsIgnoreHighFalling
bool NikuHooksFeature::Hook_IsIgnoreHighFalling(void* thiz) {
    if (speedRunEnabled && thiz) return true;
    return orig_IsIgnoreHighFalling ? orig_IsIgnoreHighFalling(thiz) : false;
}

// get_IsPoseFallingHigh
bool NikuHooksFeature::Hook_get_IsPoseFallingHigh(void* thiz) {
    if (speedRunEnabled && thiz) return true;
    return orig_get_IsPoseFallingHigh ? orig_get_IsPoseFallingHigh(thiz) : false;
}

// get_InFallingState
bool NikuHooksFeature::Hook_get_InFallingState(void* thiz) {
    if (speedRunEnabled && thiz) return false; // Prevent falling state
    return orig_get_InFallingState ? orig_get_InFallingState(thiz) : false;
}

// GetPhysXStateType
bool NikuHooksFeature::Hook_GetPhysXStateType(void* thiz) {
    if (goldBodyEnabled && thiz) return true; // Return special state for god mode
    return orig_GetPhysXStateType ? orig_GetPhysXStateType(thiz) : false;
}

// GetPhysXState
bool NikuHooksFeature::Hook_GetPhysXState(void* thiz) {
    if (goldBodyEnabled && thiz) return true;
    return orig_GetPhysXState ? orig_GetPhysXState(thiz) : false;
}

// get_HasTriggerGrenadeOnHand
bool NikuHooksFeature::Hook_get_HasTriggerGrenadeOnHand(void* thiz) {
    return orig_get_HasTriggerGrenadeOnHand ? orig_get_HasTriggerGrenadeOnHand(thiz) : false;
}

// Install all hooks
void NikuHooksFeature::InstallHooks() {
    LOGI("[NikuHooks] Installing all hooks...");

    // SpeedRun - GetDashSpeedScale (PlayerAttributes)
    void* p1 = (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "COW.GamePlay", "PlayerAttributes", "GetDashSpeedScale", 0);
    if (p1) {
        A64HookFunction(p1, (void*)Hook_GetDashSpeedScale, (void**)&orig_GetDashSpeedScale);
        LOGI("[NikuHooks] SpeedRun (GetDashSpeedScale) hook installed");
    }

    // GoldBody - get_IsGod (Player)
    void* p2 = (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "COW.GamePlay", "Player", "get_IsGod", 0);
    if (p2) {
        A64HookFunction(p2, (void*)Hook_get_IsGod, (void**)&orig_get_IsGod);
        LOGI("[NikuHooks] GoldBody (get_IsGod) hook installed");
    }

    // DoubleGun - GetCombineType (Weapon)
    void* p3 = (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "COW.GamePlay", "Weapon", "GetCombineType", 0);
    if (p3) {
        A64HookFunction(p3, (void*)Hook_GetCombineType, (void**)&orig_GetCombineType);
        LOGI("[NikuHooks] DoubleGun (GetCombineType) hook installed");
    }

    // FreeMove - IsFoldWingGliding (Player) - SpeedFix
    void* p4 = (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "COW.GamePlay", "Player", "IsFoldWingGliding", 0);
    if (p4) {
        A64HookFunction(p4, (void*)Hook_IsFoldWingGliding, (void**)&orig_IsFoldWingGliding);
        LOGI("[NikuHooks] SpeedFix (IsFoldWingGliding) hook installed");
    }

    // FreeMove - IsFreeMove (Player)
    void* p5 = (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "COW.GamePlay", "Player", "IsFreeMove", 0);
    if (p5) {
        A64HookFunction(p5, (void*)Hook_IsFreeMove, (void**)&orig_IsFreeMove);
        LOGI("[NikuHooks] FreeMove (IsFreeMove) hook installed");
    }

    // NoRecoil - GetScatterRate (PlayerAttributes)
    void* p6 = (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "COW.GamePlay", "PlayerAttributes", "GetScatterRate", 0);
    if (p6) {
        A64HookFunction(p6, (void*)Hook_GetScatterRate, (void**)&orig_GetScatterRate);
        LOGI("[NikuHooks] NoRecoil (GetScatterRate) hook installed");
    }

    // NoRecoil - GenerateRandomDirection (WeaponFireComponentLongRangeBase)
    void* p7 = (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "COW.GamePlay", "WeaponFireComponentLongRangeBase", "GenerateRandomDirection", 0);
    if (p7) {
        A64HookFunction(p7, (void*)Hook_GenerateRandomDirection, (void**)&orig_GenerateRandomDirection);
        LOGI("[NikuHooks] NoRecoil (GenerateRandomDirection) hook installed");
    }

    // MediRun - OnPreparationCancel (PlayerSkillEatFaster)
    void* p8 = (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "COW.GamePlay", "PlayerSkillEatFaster", "OnPreparationCancel", 0);
    if (p8) {
        A64HookFunction(p8, (void*)Hook_OnPreparationCancel, (void**)&orig_OnPreparationCancel);
        LOGI("[NikuHooks] MediRun (OnPreparationCancel) hook installed");
    }

    // IsIgnoreHighFalling
    void* p9 = (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "COW.GamePlay", "Player", "IsIgnoreHighFalling", 0);
    if (p9) {
        A64HookFunction(p9, (void*)Hook_IsIgnoreHighFalling, (void**)&orig_IsIgnoreHighFalling);
        LOGI("[NikuHooks] IsIgnoreHighFalling hook installed");
    }

    // get_IsPoseFallingHigh
    void* p10 = (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "COW.GamePlay", "Player", "get_IsPoseFallingHigh", 0);
    if (p10) {
        A64HookFunction(p10, (void*)Hook_get_IsPoseFallingHigh, (void**)&orig_get_IsPoseFallingHigh);
        LOGI("[NikuHooks] get_IsPoseFallingHigh hook installed");
    }

    // get_InFallingState
    void* p11 = (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "COW.GamePlay", "Player", "get_InFallingState", 0);
    if (p11) {
        A64HookFunction(p11, (void*)Hook_get_InFallingState, (void**)&orig_get_InFallingState);
        LOGI("[NikuHooks] get_InFallingState hook installed");
    }

    // GetPhysXStateType (GoldBody)
    void* p12 = (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "COW.GamePlay", "PhysXState", "GetPhysXStateType", 0);
    if (p12) {
        A64HookFunction(p12, (void*)Hook_GetPhysXStateType, (void**)&orig_GetPhysXStateType);
        LOGI("[NikuHooks] GetPhysXStateType hook installed");
    }

    // GetPhysXState
    void* p13 = (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "COW.GamePlay", "Player", "GetPhysXState", 0);
    if (p13) {
        A64HookFunction(p13, (void*)Hook_GetPhysXState, (void**)&orig_GetPhysXState);
        LOGI("[NikuHooks] GetPhysXState hook installed");
    }

    // get_HasTriggerGrenadeOnHand
    void* p14 = (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "COW.GamePlay", "Player", "get_HasTriggerGrenadeOnHand", 0);
    if (p14) {
        A64HookFunction(p14, (void*)Hook_get_HasTriggerGrenadeOnHand, (void**)&orig_get_HasTriggerGrenadeOnHand);
        LOGI("[NikuHooks] get_HasTriggerGrenadeOnHand hook installed");
    }

    LOGI("[NikuHooks] All hooks installed successfully!");
}

NikuHooksFeature::NikuHooksFeature() {
    InstallHooks();
}

NikuHooksFeature::~NikuHooksFeature() {
    // Hooks remain active while module is loaded
}

void NikuHooksFeature::Start() {
    // Hooks already installed in constructor
}

void NikuHooksFeature::Stop() {
    // No cleanup needed for inline hooks
}

void NikuHooksFeature::RenderUI() {}

void NikuHooksFeature::SetSpeedRun(bool v) {
    speedRunEnabled = v;
    if (v) speedRunLevel = 5; // Default level
}

void NikuHooksFeature::SetSpeedRunLevel(int level) {
    if (level >= 0 && level <= 9) speedRunLevel = level;
}

void NikuHooksFeature::SetGoldBody(bool v) { goldBodyEnabled = v; }
void NikuHooksFeature::SetDoubleGun(bool v) { doubleGunEnabled = v; }
void NikuHooksFeature::SetFreeMove(bool v) { 
    freeMoveEnabled = v;
    movementShootEnabled = v; // Enable both
}

void NikuHooksFeature::SetMediRun(bool v) { mediRunEnabled = v; }
void NikuHooksFeature::SetNoRecoil(bool v) { 
    noRecoilEnabled = v;
    norecoil0odEnabled = v;
}

void NikuHooksFeature::SetMovementShoot(bool v) { movementShootEnabled = v; }