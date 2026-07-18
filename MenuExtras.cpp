#include "Menu.h"
using namespace GhostSystems;

// Inicializa hooks e threads das novas features
void Menu::InitExtras() {
    // Instala hooks que nÃ£o dependem da UI (SpeedRun, GoldBody, DoubleGun, FreeMove, MediRun)
    // O construtor de NikuHooksFeature jÃ¡ registra todos os hooks necessarios
    
    // Iniciar thread de aplicaÃ§Ã£o do Silent Aim (roda em background)
    SilentAimFeature::StartApplyThread();
    
    // Inicializa o BypassLoginSDK automaticamente na inicializacao
    initBypassSDK();
}

// Atualiza o estado das features a cada frame (apÃ³s UI input)
void Menu::UpdateExtras() {
    // ---- Combat ----
    if (uiSilentAim)  silentAimFeature.Start(); else silentAimFeature.Stop();
    if (uiAimLock)   aimLockFeature.Start();   else aimLockFeature.Stop();
    if (uiAimMagnet) aimMagnetFeature.Start(); else aimMagnetFeature.Stop();
    if (uiMagnet)    magnetFeature.Start();    else magnetFeature.Stop();
    if (uiGhostHack) ghostHackFeature.Start(); else ghostHackFeature.Stop();
    if (uiEnemyPull) enemyPullFeature.Start(); else enemyPullFeature.Stop();
    // No Recoil jÃ¡ controlado por flag noRecoilEnabled

    // ---- Movement (hooks) ----
    nikuHooksFeature.SetSpeedRun(uiSpeedRun);
    nikuHooksFeature.SetGoldBody(uiGoldBody);
    nikuHooksFeature.SetDoubleGun(uiDoubleGun);
    nikuHooksFeature.SetFreeMove(uiFreeMove);
    nikuHooksFeature.SetMediRun(uiMediRun);
    nikuHooksFeature.SetNoRecoil(uiNoRecoil);
    nikuHooksFeature.SetMovementShoot(uiMovementShoot);

    // Up/Down Player
    if (uiUpPlayer) {
        upDownFeature.EnableUp(0.5f);
    } else if (uiDownPlayer) {
        upDownFeature.EnableDown(0.5f);
    } else {
        upDownFeature.Disable();
    }

    // Teleport â€“ a aÃ§Ã£o de teleporte jÃ¡ ocorre no UI ao pressionar o botÃ£o
    // Magnet update
    if (uiMagnet) {
        uint64_t target = 0;
        bool firing = false;
        if (sharedState.closestEnemy) {
            target = (uint64_t)sharedState.closestEnemy;
            firing = *(bool*)((uint64_t)sharedState.localPlayerObj + 0x7E0);
        }
        magnetFeature.SetEnabled(uiMagnet, target, firing);
    }
}

// Called from OnMainThreadTick for features that need main thread
void Menu::UpdateMainThreadFeatures() {
    // NoRecoil update
    if (noRecoilEnabled) {
        noRecoilFeature.Update();
    }
}
