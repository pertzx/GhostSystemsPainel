#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <mutex>
#include <cmath>

namespace GhostSystems {
    struct QuaternionArgs {
        float x, y, z, w;
    };

    struct Vector3 {
        float x, y, z;

        float distance(const Vector3& other) const {
            float dx = x - other.x;
            float dy = y - other.y;
            float dz = z - other.z;
            return std::sqrt(dx*dx + dy*dy + dz*dz);
        }
    };

    enum class Alignment {
        ALLY,
        ENEMY,
        NEUTRAL
    };

    struct PlayerEntity {
        uintptr_t baseAddress;
        char name[64];
        float health;
        float maxHealth;
        Vector3 position;
        bool isBot;
        bool isKnocked;
    int teamId;
    char weaponName[64];
    int weaponId;
    float distanceToLocal;
        Alignment alignment;
        void* obj; // Ponteiro Il2Cpp do jogador
        bool isVisible = true; // Added visibility flag
        bool isInFov = false; // Flag para priorizar scanner no Main Thread
        uint32_t lastWallCheckMs = 0; // Timestamp do ultimo raycast para throttle individual
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
    };

    struct FeatureConfig {
        std::mutex mtx;
        bool wallCheckEnabled = false;
        int wallCheckMethod = 0;
        bool teamCheckEnabled = true;
        int teamCheckMethod = 6;
        bool streamModeEnabled = false;
        bool threeFingerToggleEnabled = false;
    };

}
