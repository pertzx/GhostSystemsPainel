#include "NoRecoil.h"
#include <unistd.h>
#include <chrono>
#include <atomic>
#include "SharedAccess.h"
#include "../OffsetResolver.h"

using namespace GhostSystems;

NoRecoilFeature::NoRecoilFeature() {}
NoRecoilFeature::~NoRecoilFeature() {}

void NoRecoilFeature::Start() {
    // NoRecoil e controlado via flag noRecoilEnabled no Menu
}

void NoRecoilFeature::Stop() {
    // Nada a fazer - controlado por flag
}

// Esta funcao e chamada do OnMainThreadTick do Menu
void NoRecoilFeature::Update() {
    if (!enabled) return;
    
    if (!g_GameState) return;
    void* localPlayerObj = g_GameState->localPlayerObj;
    if (!localPlayerObj) return;
    
    uint64_t localPlayer = (uint64_t)localPlayerObj;
    
    // Resolve weapon offset once
    static size_t weaponOffset = 0;
    if (!weaponOffset) {
        weaponOffset = g_OffsetResolver().GetPlayerFieldOffset("weapon") ?:
                       g_OffsetResolver().GetPlayerFieldOffset("currentWeapon") ?:
                       g_OffsetResolver().GetPlayerFieldOffset("equippedWeapon") ?:
                       g_OffsetResolver().GetPlayerFieldOffset("ActiveUISightingWeapon");
    }
    
    // Obter arma atual
    uint64_t weapon = 0;
    if (weaponOffset) {
        weapon = *(uint64_t*)(localPlayer + weaponOffset);
    } else {
        // Fallback offsets
        static const size_t fallbackWeaponOffsets[] = {0x5A8, 0x5B0, 0x5C0, 0x598, 0x580};
        for (size_t off : fallbackWeaponOffsets) {
            uint64_t test = *(uint64_t*)(localPlayer + off);
            if (test && test > 0x100000000) {
                weapon = test;
                weaponOffset = off;
                break;
            }
        }
    }
    
    if (!weapon || weapon < 0x100000000) return;
    
    // Resolve recoil offsets
    static size_t recoilHOffset = 0, recoilVOffset = 0, spreadOffset = 0, recoveryOffset = 0, patternOffset = 0;
    static bool recoilOffsetsResolved = false;
    
    if (!recoilOffsetsResolved) {
        recoilHOffset = g_OffsetResolver().GetWeaponFieldOffset("recoil") ?:
                        g_OffsetResolver().GetWeaponFieldOffset("recoilH") ?:
                        g_OffsetResolver().GetWeaponFieldOffset("horizontalRecoil");
        recoilVOffset = g_OffsetResolver().GetWeaponFieldOffset("recoilV") ?:
                        g_OffsetResolver().GetWeaponFieldOffset("verticalRecoil") ?:
                        (recoilHOffset ? recoilHOffset + 4 : 0);
        spreadOffset = g_OffsetResolver().GetWeaponFieldOffset("spread") ?:
                       g_OffsetResolver().GetWeaponFieldOffset("weaponSpread");
        recoveryOffset = g_OffsetResolver().GetWeaponFieldOffset("recovery") ?:
                         g_OffsetResolver().GetWeaponFieldOffset("recoilRecovery");
        patternOffset = g_OffsetResolver().GetWeaponFieldOffset("pattern") ?:
                        g_OffsetResolver().GetWeaponFieldOffset("recoilPattern");
        recoilOffsetsResolved = true;
    }
    
    // Write no recoil values using resolved offsets or fallbacks
    if (recoilHOffset) {
        *(float*)(weapon + recoilHOffset) = 0.0f;
    } else {
        // Fallback offsets
        *(float*)(weapon + 0x118) = 0.0f;   // recoil horizontal
        *(float*)(weapon + 0x11C) = 999.0f; // recoil vertical max
        *(float*)(weapon + 0x120) = 0.0f;   // recoil spread
        *(float*)(weapon + 0x12C) = 0.0f;   // recoil recovery
        *(float*)(weapon + 0x134) = 0.0f;   // recoil pattern
    }
    
    if (recoilVOffset && recoilVOffset != recoilHOffset) {
        *(float*)(weapon + recoilVOffset) = 999.0f;
    }
    if (spreadOffset) {
        *(float*)(weapon + spreadOffset) = 0.0f;
    }
    if (recoveryOffset) {
        *(float*)(weapon + recoveryOffset) = 0.0f;
    }
    if (patternOffset) {
        *(float*)(weapon + patternOffset) = 0.0f;
    }
}