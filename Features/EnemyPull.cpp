#include "EnemyPull.h"
#include <unistd.h>
#include <chrono>
#include <cmath>
#include "SharedAccess.h"
#include "../Entity.h"

using namespace GhostSystems;

EnemyPullFeature::EnemyPullFeature() {}
EnemyPullFeature::~EnemyPullFeature() { Stop(); }

void EnemyPullFeature::RenderUI() {}

void EnemyPullFeature::Start() {
    if (running) return;
    running = true;
    thread = std::thread(&EnemyPullFeature::Worker, this);
}

void EnemyPullFeature::Stop() {
    if (!running) return;
    running = false;
    RestoreEnemyPosition();
    if (thread.joinable()) thread.join();
}

static uint64_t lockedEnemy = 0;
static GhostSystems::Vector3 lockedPos = {0,0,0};
static bool hasLock = false;
static uint64_t lockedEnemyMatrix = 0;

// Helper para obter raiz da matriz
static bool GetTransformRootAddr(uint64_t node, uint64_t& outAddr) {
    uint64_t transform = *(uint64_t*)(node + 0x10);
    if (!transform) return false;

    uint64_t native = *(uint64_t*)(transform + 0x10);
    if (!native) return false;

    uint64_t transformData = *(uint64_t*)(native + 0x38);
    int index = *(int*)(native + 0x40);
    if (!transformData || index < 0) return false;

    uint64_t matrixList = *(uint64_t*)(transformData + 0x18);
    uint64_t parentIndices = *(uint64_t*)(transformData + 0x20);
    if (!matrixList || !parentIndices) return false;

    int rootIdx = index;
    int tries = 0;
    while (tries < 50) {
        int parent = *(int*)(parentIndices + (rootIdx * 4));
        if (parent < 0) break;
        rootIdx = parent;
        tries++;
    }

    outAddr = matrixList + (rootIdx * 0x30);
    return true;
}

// Helper para obter posicao
static GhostSystems::Vector3 GetNodePosition(uint64_t node) {
    uint64_t matrixAddr = 0;
    if (!GetTransformRootAddr(node, matrixAddr)) return {0,0,0};
    return *(GhostSystems::Vector3*)(matrixAddr);
}

void EnemyPullFeature::RestoreEnemyPosition() {
    if (hasLock && lockedEnemy != 0 && lockedEnemyMatrix != 0) {
        *(GhostSystems::Vector3*)(lockedEnemyMatrix) = lockedPos;
    }
    lockedEnemy = 0;
    hasLock = false;
    lockedEnemyMatrix = 0;
}

void EnemyPullFeature::Worker() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        
        if (!enabled) {
            RestoreEnemyPosition();
            continue;
        }
        
        if (!g_GameState) {
            RestoreEnemyPosition();
            continue;
        }
        
        void* localPlayerObj = g_GameState->localPlayerObj;
        if (!localPlayerObj) {
            RestoreEnemyPosition();
            continue;
        }
        
        uint64_t localPlayer = (uint64_t)localPlayerObj;
        
        // Verificar se esta atirando
        bool firing = *(bool*)(localPlayer + 0x7E0);
        
        if (!firing) {
            RestoreEnemyPosition();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        
        // Se nao tem lock, adquirir novo inimigo
        if (!hasLock) {
            void* nearestEnemy = nullptr;
            {
                std::lock_guard<std::mutex> lock(g_GameState->mtx);
                if (!g_GameState->entities.empty()) {
                    float closestDist = 999999.0f;
                    for (auto& entity : g_GameState->entities) {
                        if (!entity.isAlive() || entity.alignment == GhostSystems::Alignment::ALLY) continue;
                        float dist = entity.position.distance(g_GameState->localPlayerPos);
                        if (dist < closestDist) {
                            closestDist = dist;
                            nearestEnemy = entity.obj;
                        }
                    }
                }
            }
            
            if (!nearestEnemy) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            
            uint64_t enemy = (uint64_t)nearestEnemy;
            uint64_t rn = *(uint64_t*)(enemy + 0x670);
            if (!rn) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            
            uint64_t ma = 0;
            if (!GetTransformRootAddr(rn, ma)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            
            lockedPos = *(GhostSystems::Vector3*)(ma);
            if (lockedPos.x == 0 && lockedPos.y == 0 && lockedPos.z == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            
            lockedEnemy = enemy;
            lockedEnemyMatrix = ma;
            hasLock = true;
            
            // Teleportar inimigo para frente do player (simplificado)
            GhostSystems::Vector3 tpPos = g_GameState->localPlayerPos;
            tpPos.x += 2.0f; // Offset de 2 unidades na frente
            
            *(GhostSystems::Vector3*)(ma) = tpPos;
        }
        
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }
}
