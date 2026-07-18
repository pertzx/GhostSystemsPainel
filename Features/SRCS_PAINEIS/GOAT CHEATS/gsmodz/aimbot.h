#pragma once
#include "ESP.h"
void UpdateSpeedhack() {
    void* COW_GameFacade_TypeInfo = *(void**)getAddressIL2CPP(0xB023498);
    if (COW_GameFacade_TypeInfo)
    {
        void* COW_GameFacade_TypeInfo_Fields = *(void**)((uintptr_t)COW_GameFacade_TypeInfo + 0xB8);
        if (COW_GameFacade_TypeInfo_Fields)
        {
            void* CurrentMatchGame = *(void**)((uintptr_t)COW_GameFacade_TypeInfo_Fields + 0x8);
            if (CurrentMatchGame)
            {
                auto TimeService = *(void**)((uintptr_t)CurrentMatchGame + 0x20);
                if (TimeService != nullptr)
                {
                    auto Time = *(float*)((uintptr_t)TimeService + 0x2c);
                    if (SpeedHack)
                    {
                        if (Time != 0.065f)
                        {
                            *(float*)((uintptr_t)TimeService + 0x2c) = 0.065f;
                        }

                    }
                    else {
                        if (Time != 0.033f)
                        {
                            *(float*)((uintptr_t)TimeService + 0x2c) = 0.033f;
                        }
                    }
                }
            }
        }
    }
}



// usa a mesma variável global do menu
extern int aimPosition; // 0=CABEÇA, 1=PESCOÇO, 2=PEITO, 3=QUADRIL

bool IsVisible(void * player){
    if (player != NULL) {
        void *hitObj = NULL;
        Vector3 cameraLocation = Transform_GetPosition(Component_GetTransform(Camera_main()));
        Vector3 headLocation   = Transform_GetPosition(Component_GetTransform(Player_GetHeadCollider(player)));
        return !Physics_Raycast(cameraLocation, headLocation, 12, &hitObj);
    }
    return false;
}

void* GetClosestEnemy() {
    float shortestDistance = MAX_DISTANCE;
    void* closestEnemy = NULL; 
    void* get_MatchGame = Curent_Match();
    void* LocalPlayer   = GetLocalPlayer(get_MatchGame);

    if (LocalPlayer != NULL && get_MatchGame != NULL && Startgs) {
        monoDictionary<uint8_t*, void**>* players =
            *(monoDictionary<uint8_t*, void**>**)((long)get_MatchGame + ListPlayer);

        for (int u = 0; u < players->getNumValues(); u++) {
            void* Player = players->getValues()[u]; 
            if (Player != NULL && !get_isLocalTeam(Player) 
                && (ignoreKnockedEnemies || !get_IsDieing(Player))
                && get_isVisible(Player) && get_MaxHP(Player)) {

                Vector3 PlayerPos      = getPosition(Player);
                Vector3 LocalPlayerPos = getPosition(LocalPlayer);
                float distance         = Vector3::Distance(LocalPlayerPos, PlayerPos);

                Vector3 targetDir = Vector3::Normalized(PlayerPos - LocalPlayerPos);
                float angle = Vector3::Angle(
                                  targetDir,
                                  GetForward(Component_GetTransform(Camera_main()))
                              ) * AIM_ANGLE_MULTIPLIER;

                if (distance < Aimdis && angle <= Fov_Aim) {
                    if (angle < shortestDistance) {
                        shortestDistance = angle;
                        closestEnemy     = Player;
                    }
                }
            }
        }
    }
    return closestEnemy;
}

// ================== PONTO DE MIRA AJUSTADO ==================
// 0=CABEÇA, 1=PESCOÇO, 2=PEITO, 3=QUADRIL
static inline Vector3 GetAdjustedPosition(void* enemy) {
    Vector3 head = GetHeadPosition(enemy);
    Vector3 hip  = GetHipPosition(enemy);

    switch (aimPosition) {
        case 1: // PESCOÇO: ~25% do caminho cabeça→quadril
            return Vector3::Lerp(head, hip, 0.25f);

        case 2: // PEITO: ~55% do caminho quadril→cabeça
            return Vector3::Lerp(hip, head, 0.55f);

        case 3: // QUADRIL
            return hip;

        case 0:
        default: // CABEÇA
            return head;
    }
}
// ==============================================================================

Quaternion GetCameraRotation(void* localPlayer) {
    return GetRotation(Component_GetTransform(Camera_main()));
}

void Aimbott() {
    if (!AimbotEsp) return;
			void* current_Match = Curent_Match();
		if (current_Match == NULL) return; // Adicionado: Verifica se a partida é válida

		void* local_player = GetLocalPlayer(current_Match);
		if (local_player == NULL) return; // Adicionado: Verifica se o jogador local é válido

		// Adicionado: Verifica se o jogador local está vivo antes de tentar acessar seus atributos
		if (get_IsDieing(local_player)) return; 

		void* closestEnemy = GetClosestEnemy();
		
		// Move esta linha para dentro da verificação de local_player válido
		*(bool*)((uintptr_t)local_player + 0x688) = GhostHack;

		if (closestEnemy != NULL) { // current_Match e local_player já foram verificados

			Vector3 enemyLocation = GetAdjustedPosition(closestEnemy);
			Vector3 playerLocation = CameraMain(local_player);}
	


    if (closestEnemy != NULL && local_player != NULL && current_Match != NULL) {
        Vector3 enemyLocation   = GetAdjustedPosition(closestEnemy);
        Vector3 playerLocation  = CameraMain(local_player);

		if (AimVisible && !IsVisible(closestEnemy)) return;
			
        Quaternion currentRotation = GetCameraRotation(local_player);
        Quaternion targetRotation  = GetRotationToLocation(enemyLocation, 0.1f, playerLocation);

        Quaternion smoothRotation;
        if (aimSmoothing == 0) {
            smoothRotation = targetRotation;
        } else {
            float smoothFactor = aimSmoothing / 10.0f;
            if (smoothFactor > 1.0f) smoothFactor = 1.0f;
            smoothRotation = Quaternion::Slerp(currentRotation, targetRotation, smoothFactor);
        }

        bool isScopeOn = get_IsSighting(local_player);
        bool isFiring  = get_IsFiring(local_player);

        if (aimbotAuto) {
            set_aim(local_player, smoothRotation);
        } else if (aimbotTrigger && isFiring) {
            set_aim(local_player, smoothRotation);
        } else if (aimbotAim && isScopeOn) {
            set_aim(local_player, smoothRotation);
        } else if (aimbotTrigger && aimbotAim && (isFiring || isScopeOn)) {
            set_aim(local_player, smoothRotation);
        }
    }
}
bool(*_ResetGuest)(void* _this);
bool ResetGuest(void* _this){
    if (_this != NULL) {
        if (ResetGuest1) {
            return true; 
            remove(OBFUSCATE("/storage/emulated/0/com.garena.msdk/guest100067.dat"));
        }
    }
    return false;
}


