#include "AimLock.h"
#include <unistd.h>
#include <chrono>
#include <cmath>
#include "SharedAccess.h"
#include "../OffsetResolver.h"

using namespace GhostSystems;

AimLockFeature::AimLockFeature() {}
AimLockFeature::~AimLockFeature() { Stop(); }

void AimLockFeature::RenderUI() {}

void AimLockFeature::Start() {
    if (running) return;
    running = true;
    thread = std::thread(&AimLockFeature::Worker, this);
}

void AimLockFeature::Stop() {
    if (!running) return;
    running = false;
    if (thread.joinable()) thread.join();
}

void AimLockFeature::Worker() {
    // Resolve offsets
    static size_t firingOffset = 0;
    static size_t rootNodeOffset = 0;
    static size_t positionOffset = 0;
    static bool offsetsResolved = false;
    
    if (!offsetsResolved) {
        firingOffset = g_OffsetResolver().GetPlayerFieldOffset("firing") ?:
                       g_OffsetResolver().GetPlayerFieldOffset("isFiring") ?:
                       g_OffsetResolver().GetPlayerFieldOffset("shooting");
        rootNodeOffset = g_OffsetResolver().GetPlayerFieldOffset("rootNode") ?:
                         g_OffsetResolver().GetPlayerFieldOffset("root") ?:
                         g_OffsetResolver().GetPlayerFieldOffset("transform");
        positionOffset = g_OffsetResolver().GetPlayerFieldOffset("position") ?:
                         g_OffsetResolver().GetPlayerFieldOffset("pos");
        offsetsResolved = true;
    }
    
    while (running) {
        if (!enabled) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        
        if (!g_GameState) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        
        void* localPlayerObj = g_GameState->localPlayerObj;
        if (!localPlayerObj) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        
        uint64_t localPlayer = (uint64_t)localPlayerObj;
        
        // Verificar se esta atirando
        bool isFiring = false;
        if (firingOffset) {
            isFiring = *(bool*)(localPlayer + firingOffset);
        } else {
            // Fallback offsets
            static const size_t fallbackOffsets[] = {0x7E0, 0x7D8, 0x7F0, 0x800, 0x7C0};
            for (size_t off : fallbackOffsets) {
                if (*(bool*)(localPlayer + off)) {
                    isFiring = true;
                    firingOffset = off;
                    break;
                }
            }
        }
        
        if (!isFiring) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }
        
        // Obter root node do player local
        uint64_t localRootNode = 0;
        if (rootNodeOffset) {
            localRootNode = *(uint64_t*)(localPlayer + rootNodeOffset);
        } else {
            static const size_t fallbackRootOffsets[] = {0x670, 0x668, 0x680, 0x650};
            for (size_t off : fallbackRootOffsets) {
                uint64_t test = *(uint64_t*)(localPlayer + off);
                if (test) {
                    localRootNode = test;
                    rootNodeOffset = off;
                    break;
                }
            }
        }
        
        if (!localRootNode) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        // Posicao do player local
        GhostSystems::Vector3 localWorldPos = {0,0,0};
        if (positionOffset) {
            localWorldPos = *(GhostSystems::Vector3*)(localRootNode + positionOffset);
        } else {
            static const size_t fallbackPosOffsets[] = {0x10, 0x20, 0x30, 0x40};
            for (size_t off : fallbackPosOffsets) {
                GhostSystems::Vector3 test = *(GhostSystems::Vector3*)(localRootNode + off);
                if (test.x != 0 || test.y != 0 || test.z != 0) {
                    localWorldPos = test;
                    positionOffset = off;
                    break;
                }
            }
        }
        
        if (localWorldPos.x == 0 && localWorldPos.y == 0 && localWorldPos.z == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        
        // Encontrar inimigo mais proximo
        {
            std::lock_guard<std::mutex> lock(g_GameState->mtx);
            if (g_GameState->entities.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
        }
        
        uint64_t bestEnemy = 0;
        float bestScreenDist = 1e9f;
        
        {
            std::lock_guard<std::mutex> lock(g_GameState->mtx);
            for (auto& entity : g_GameState->entities) {
                if (!entity.isAlive()) continue;
                if (entity.alignment == GhostSystems::Alignment::ALLY) continue;
                if (entity.isKnocked) continue;
                
                float dist = entity.position.distance(localWorldPos);
                if (dist < bestScreenDist) {
                    bestScreenDist = dist;
                    bestEnemy = (uint64_t)entity.obj;
                }
            }
        }

        if (!bestEnemy) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }
        
        // Obter rootNode do inimigo
        uint64_t enemyRootNode = 0;
        if (rootNodeOffset) {
            enemyRootNode = *(uint64_t*)(bestEnemy + rootNodeOffset);
        } else {
            static const size_t fallbackRootOffsets[] = {0x670, 0x668, 0x680, 0x650};
            for (size_t off : fallbackRootOffsets) {
                uint64_t test = *(uint64_t*)(bestEnemy + off);
                if (test) {
                    enemyRootNode = test;
                    break;
                }
            }
        }
        
        if (!enemyRootNode) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        GhostSystems::Vector3 enemyWorldPos = {0,0,0};
        if (positionOffset) {
            enemyWorldPos = *(GhostSystems::Vector3*)(enemyRootNode + positionOffset);
        } else {
            static const size_t fallbackPosOffsets[] = {0x10, 0x20, 0x30, 0x40};
            for (size_t off : fallbackPosOffsets) {
                GhostSystems::Vector3 test = *(GhostSystems::Vector3*)(enemyRootNode + off);
                if (test.x != 0 || test.y != 0 || test.z != 0) {
                    enemyWorldPos = test;
                    break;
                }
            }
        }
        
        if (enemyWorldPos.x == 0 && enemyWorldPos.y == 0 && enemyWorldPos.z == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        
        // Mudar posicao do inimigo (AimLock teleporta-o para a frente do player)
        *(GhostSystems::Vector3*)(enemyRootNode + (positionOffset ? positionOffset : 0x10)) = localWorldPos;

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}