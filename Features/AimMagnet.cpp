#include "AimMagnet.h"
#include <unistd.h>
#include <chrono>
#include <cmath>
#include <map>
#include <mutex>
#include "SharedAccess.h"
#include "../Entity.h"

using namespace GhostSystems;

AimMagnetFeature::AimMagnetFeature() {}
AimMagnetFeature::~AimMagnetFeature() { Stop(); }

void AimMagnetFeature::RenderUI() {}

void AimMagnetFeature::Start() {
    if (running) return;
    running = true;
    thread = std::thread(&AimMagnetFeature::Worker, this);
}

void AimMagnetFeature::Stop() {
    if (!running) return;
    running = false;
    if (thread.joinable()) thread.join();
}

// Mapa de posicoes base dos inimigos
static std::map<uint64_t, GhostSystems::Vector3> BasePosMap;
static std::mutex mapMutex;

// Helper para obter posicao do inimigo
static GhostSystems::Vector3 GetEnemyHeadPosition(void* entity) {
    if (!entity) return {0,0,0};
    GhostSystems::PlayerEntity* ent = (GhostSystems::PlayerEntity*)entity;
    return {ent->position.x, ent->position.y + 1.6f, ent->position.z};
}

void AimMagnetFeature::Worker() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::microseconds(50));
        
        if (!enabled) {
            BasePosMap.clear();
            continue;
        }
        
        if (!g_GameState) continue;
        void* localPlayerObj = g_GameState->localPlayerObj;
        if (!localPlayerObj) continue;
        
        uint64_t localPlayer = (uint64_t)localPlayerObj;
        
        // Verificar se esta atirando
        bool isFiring = *(bool*)(localPlayer + 0x7E0);
        
        // Obter inimigo mais proximo
        void* closestEnemy = nullptr;
        {
            std::lock_guard<std::mutex> lock(g_GameState->mtx);
            if (!g_GameState->entities.empty()) {
                float closestDist = 999999.0f;
                for (auto& entity : g_GameState->entities) {
                    if (!entity.isAlive() || entity.alignment == GhostSystems::Alignment::ALLY) continue;
                    float dist = entity.position.distance(g_GameState->localPlayerPos);
                    if (dist < closestDist) {
                        closestDist = dist;
                        closestEnemy = entity.obj;
                    }
                }
            }
        }
        
        if (!closestEnemy) {
            BasePosMap.clear();
            continue;
        }
        
        uint64_t enemyKey = (uint64_t)closestEnemy;
        
        if (!isFiring) {
            // Nao esta atirando - limpar mapa do inimigo atual
            std::lock_guard<std::mutex> lock(mapMutex);
            BasePosMap.erase(enemyKey);
            continue;
        }
        
        // Obter raiz do inimigo
        uint64_t root = *(uint64_t*)((uint64_t)closestEnemy + 0x668);
        if (!root) {
            std::lock_guard<std::mutex> lock(mapMutex);
            BasePosMap.erase(enemyKey);
            continue;
        }
        
        // Obter posicao do inimigo
        GhostSystems::Vector3 enemyPos = GetEnemyHeadPosition(closestEnemy);
        
        // Armazenar ou recuperar posicao base
        {
            std::lock_guard<std::mutex> lock(mapMutex);
            if (BasePosMap.find(enemyKey) == BasePosMap.end()) {
                BasePosMap[enemyKey] = enemyPos;
            }
            enemyPos = BasePosMap[enemyKey];
        }
        
        // Calcular nova posicao (teleportar inimigo para frente do player)
        GhostSystems::Vector3 newPos;
        newPos.x = g_GameState->localPlayerPos.x + (enemyPos.x - g_GameState->localPlayerPos.x) * 0.5f;
        newPos.y = g_GameState->localPlayerPos.y + (enemyPos.y - g_GameState->localPlayerPos.y) * 0.5f;
        newPos.z = g_GameState->localPlayerPos.z + (enemyPos.z - g_GameState->localPlayerPos.z) * 0.5f;
        
        // Escrever posicao na transform do inimigo
        uint64_t p3 = *(uint64_t*)(root + 0x10);
        if (p3) {
            uint64_t transformPtr = *(uint64_t*)(p3 + 0x38);
            if (transformPtr) {
                *(GhostSystems::Vector3*)(transformPtr + 0x90) = newPos;
            }
        }
    }
}
