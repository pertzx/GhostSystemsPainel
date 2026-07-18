#pragma once
#include "ESP.h"
#include "structs.h"
#include "pointers.h"
#include "minimemory.h"
#include <thread>
#include <unordered_map>


void UpdateSpeedhack() {
    if (Active) {
        void* Simulation = GetSimulationTimer();
	    if (Simulation != nullptr) {
			float FixedDeltaTime = GetTimer(Simulation);
            if(!saved) {
                active = FixedDeltaTime * 1.82f;
                desactive = FixedDeltaTime;
                saved = true;
            }
			if (SpeedHack) {
			    if (FixedDeltaTime != active) {
                    SetTimer(Simulation, active);
				}
			}
			else {
			    if (FixedDeltaTime != desactive) {
                    SetTimer(Simulation, desactive);
				}
			}
		}
    }
}

void* GetClosestEnemy(void* current_Match, void* local_Player) {
	float shortestDistance = MAX_DISTANCE;
	bestEnemy = nullptr;
	if (local_Player != nullptr && current_Match != nullptr && Startgs) {
		auto players = *(monoDictionary<uint8_t*, void**>**)((long)current_Match + ListPlayer);
		for (int u = 0; u < players->getNumValues(); u++) {
			void* Player = players->getValues()[u];
			if (Player != nullptr
				&& !get_isLocalTeam(Player)
				&& (ignoreKnockedEnemies || !get_IsDieing(Player))
				&& get_isVisible(Player)
				&& get_MaxHP(Player))
			{
				Vector3 PlayerPos = getPosition(Player);
				Vector3 LocalPlayerPos = getPosition(local_Player);

				float distance = Vector3::Distance(LocalPlayerPos, PlayerPos);
				Vector3 targetDir = Vector3::Normalized(PlayerPos - LocalPlayerPos);
				float angle = Vector3::Angle(targetDir, GetForward(Component_GetTransform(Camera_main()))) * AIM_ANGLE_MULTIPLIER;

				if (distance < Fov && angle < shortestDistance) {
					shortestDistance = angle;
					bestEnemy = Player;
				}
			}
		}
	}
	return bestEnemy;
}



void GetPointers() {
SetHighFPS(3);
	if (Active) {
		current_Match = Curent_Match();
		matchState = *(int*)((uintptr_t)current_Match + m_State);
		if (matchState == 1) {
			inMatch = true;
			local_player = GetLocalPlayer(current_Match);
            local_playerInMatch = *(void**)((uintptr_t)current_Match + m_LocalPlayer);
			mainCamera = Camera_main();
			if (local_player && local_playerInMatch) {
				//spofNick(local_playerInMatch);
				localPlayerAttributes = *(void**)((uintptr_t)local_player + m_PlayerAttributes);
				localTransform = Component_GetTransform(local_player);
				playerLocation = CameraMain(local_player);
				playerPosition = getPosition(local_player);
				playerForward = GetForwardAdjusted(Component_GetTransform(mainCamera), 342.5f);
				hitObjInfo = *(void**)((uintptr_t)local_playerInMatch + m_HawkerHitObject);
                ammoBase = *(Vector3*)((uintptr_t)hitObjInfo + m_HawkerStartPos);
				isFiring = get_IsFiring(local_player);
				isScoped = get_IsSighting(local_player);
			}
			else {
				localTransform = nullptr;
				playerLocation = Vector3::zero();
				isFiring = isScoped = isAlive = false;
			}
			closestEnemy = GetClosestEnemy(current_Match, local_player);
			
			
			if (closestEnemy) {
				enemyTransform = Component_GetTransform(closestEnemy);
				if (enemyTransform) {
					enemyPosition = Transform_GetPosition(enemyTransform);
					enemyPos = GetHeadPosition(closestEnemy);
                    dir = enemyPos - ammoBase;
					distanceToLocalPlayer = Vector3::Distance(getPosition(local_player), enemyPosition);
				}
				else {
					enemyPosition = Vector3::zero();
				}
			}
			else {
				enemyTransform = nullptr;
				enemyPosition = Vector3::zero();
			}
		}
		else {
			Active = false;
			inMatch = false;
		}
	}
}

Vector3 GetAdjustedPosition(void* closestEnemy) {
	Vector3 headPos = GetHeadPosition(closestEnemy);
	if (aimPosition == 1) {
		headPos.y += NECK_OFFSET;
	}
	else if (aimPosition == 2) {
		Vector3 hipPos = GetHipPosition(closestEnemy);
		headPos = Vector3::Lerp(headPos, hipPos, 0.5f);
	}
	else if (aimPosition == 3) {
		headPos = GetHipPosition(closestEnemy);
	}
	return headPos;
}

Quaternion GetCameraRotation(void* localPlayer) {
	return GetRotation(Component_GetTransform(Camera_main()));
}

bool IsInFov(Vector3 enemyPos, Vector3 playerPos, float fovLimit) {
	Vector3 m_HeadScreen = WorldToScreenPoint(mainCamera, GetHeadPosition(closestEnemy));
	float screenCenterX = savedScreenWidth / 2;
	float screenCenterY = savedScreenHeight / 2;
	float distanceToCenterX = m_HeadScreen.x - screenCenterX;
	float distanceToCenterY = m_HeadScreen.y - screenCenterY;
	float distanceToCenter = abs(distanceToCenterX) + abs(distanceToCenterY);
	if (distanceToCenter <= fovLimit) {
		return true;
	}
	else {
		return false;
	}
}

void AimbotLegitVoid() {
	if (Active && AimbotLegit) {
		if (closestEnemy != NULL && local_player != NULL && current_Match != NULL) {
			void* Current_Collider = *(void**)((uintptr_t)closestEnemy + 0x78);
			void* Head_Collider = Player_GetHeadCollider(closestEnemy);
			if (Current_Collider != Head_Collider) {
				SetAimCollider(closestEnemy, Head_Collider);
			}
		}
	}
}


void AimbotRageVoid() {
	    if ((Active && AimbotRage) || (Active && AimbotAuto)) {
		    if (closestEnemy != NULL && local_player != NULL && current_Match != NULL) {
			    Vector3 enemyLocation = GetAdjustedPosition(closestEnemy);
			    if (!IsInFov(enemyLocation, playerLocation, Fov)) return;
			    Quaternion targetRotation = GetRotationToLocation(enemyLocation, 0.1f, playerLocation);

			    if (AimbotAuto) {
				    set_aim2(local_player, targetRotation);
		    	}
			    else if (AimbotRage && isFiring) {
			    	set_aim2(local_player, targetRotation);
			    }
			    else if (AimbotAim && isScoped) {
				    set_aim2(local_player, targetRotation);
			    }
			    else if (AimbotRage && AimbotAim && (isFiring || isScoped)) {
				    set_aim2(local_player, targetRotation);
			    }
		    }
	    }
}


void AimSilentVoid() {
    while (true) {
		if (Active && AimSilent) {
            if (local_playerInMatch != nullptr && current_Match != nullptr && hitObjInfo != nullptr) {
                if (isFiring) {
                    *(Vector3*)((uintptr_t)hitObjInfo + m_HawkrtEndPos) = dir;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}
