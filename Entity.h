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
        void* obj = nullptr; // Ponteiro Il2Cpp do jogador
        
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

}
