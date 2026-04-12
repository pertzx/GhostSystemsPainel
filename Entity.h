#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <mutex>

namespace GhostSystems {

    struct Vector3 {
        float x, y, z;
        
        float distance(const Vector3& other) const {
            float dx = x - other.x;
            float dy = y - other.y;
            float dz = z - other.z;
            return __builtin_sqrtf(dx*dx + dy*dy + dz*dz);
        }
    };

    enum class Alignment {
        ALLY,
        ENEMY,
        NEUTRAL
    };

    struct PlayerEntity {
        uintptr_t baseAddress = 0;
        char name[64] = {0};
        float health = 0.0f;
        float maxHealth = 100.0f;
        Vector3 position = {0, 0, 0};
        bool isBot = false;
        bool isKnocked = false;
        int teamId = 0;
        float distanceToLocal = 0.0f;
        Alignment alignment = Alignment::ENEMY;
        void* obj = nullptr;
        
        // Extended info for new features
        int kills = 0;
        int deaths = 0;
        bool isVisible = true;
        bool isInVehicle = false;
        int weaponType = 0;
        int weaponSubType = 0;
        std::string weaponName;
        float currentDashSpeed = 0.0f;
        bool hasDashActive = false;
        
        bool isAlive() const { return health > 0.0f; }
        float getHealthPercentage() const { return (health / maxHealth) * 100.0f; }
    };

    // Shared State for Thread Safety
    struct GameState {
        std::mutex mtx;
        std::vector<PlayerEntity> entities;
        int maxEntities = 100;
        Vector3 localPlayerPos;
        int localPlayerTeamId;
        void* localPlayerObj = nullptr;

        void updateEntityVisibility(void* obj, bool isVisible) {
            std::lock_guard<std::mutex> lock(mtx);
            for (auto& entity : entities) {
                if (entity.obj == obj) {
                    entity.isVisible = isVisible;
                    break;
                }
            }
        }

        void updateEntityKills(void* obj, int kills) {
            std::lock_guard<std::mutex> lock(mtx);
            for (auto& entity : entities) {
                if (entity.obj == obj) {
                    entity.kills = kills;
                    break;
                }
            }
        }

        void updateEntityDeaths(void* obj, int deaths) {
            std::lock_guard<std::mutex> lock(mtx);
            for (auto& entity : entities) {
                if (entity.obj == obj) {
                    entity.deaths = deaths;
                    break;
                }
            }
        }

        void updateEntityTeamId(void* obj, int teamId) {
            std::lock_guard<std::mutex> lock(mtx);
            for (auto& entity : entities) {
                if (entity.obj == obj) {
                    entity.teamId = teamId;
                    break;
                }
            }
        }

        void updateEntityWeapon(void* obj, int weaponType, int weaponSubType, const std::string& weaponName) {
            std::lock_guard<std::mutex> lock(mtx);
            for (auto& entity : entities) {
                if (entity.obj == obj) {
                    entity.weaponType = weaponType;
                    entity.weaponSubType = weaponSubType;
                    entity.weaponName = weaponName;
                    break;
                }
            }
        }
    };

}
