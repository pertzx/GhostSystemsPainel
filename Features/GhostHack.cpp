#include "GhostHack.h"
#include <unistd.h>
#include <chrono>
#include <cmath>
#include "SharedAccess.h"
#include "../Entity.h"

using namespace GhostSystems;
GhostHackFeature::GhostHackFeature() {}
GhostHackFeature::~GhostHackFeature() { Stop(); }

void GhostHackFeature::RenderUI() {}

void GhostHackFeature::Start() {
    if (running) return;
    running = true;
    thread = std::thread(&GhostHackFeature::Worker, this);
}

void GhostHackFeature::Stop() {
    if (!running) return;
    running = false;
    RestorePosition();
    if (thread.joinable()) thread.join();
}

static GhostSystems::Vector3 savedPos = {0,0,0};
static bool hasSavedPos = false;
static uint64_t savedPlayerRoot = 0;

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

void GhostHackFeature::RestorePosition() {
    if (hasSavedPos && savedPlayerRoot != 0) {
        uint64_t matrixAddr = 0;
        if (GetTransformRootAddr(savedPlayerRoot, matrixAddr)) {
            *(GhostSystems::Vector3*)(matrixAddr) = savedPos;
        }
    }
    hasSavedPos = false;
    savedPlayerRoot = 0;
}

void GhostHackFeature::Worker() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        if (!enabled) {
            RestorePosition();
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
        
        uint64_t localRoot = *(uint64_t*)(localPlayer + 0x670);
        if (!localRoot) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        
        uint64_t localMatAddr = 0;
        if (!GetTransformRootAddr(localRoot, localMatAddr)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        
        // Salvar posicao original na primeira ativacao
        if (!hasSavedPos) {
            savedPos = *(GhostSystems::Vector3*)(localMatAddr);
            hasSavedPos = true;
            savedPlayerRoot = localRoot;
        }
        
        // Obter inimigo mais proximo
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
        
        if (nearestEnemy) {
            uint64_t enemyRoot = *(uint64_t*)((uint64_t)nearestEnemy + 0x670);
            if (enemyRoot) {
                GhostSystems::Vector3 enemyPos = GetNodePosition(enemyRoot);
                if (!(enemyPos.x == 0 && enemyPos.y == 0 && enemyPos.z == 0)) {
                    // Teleportar player para posicao do inimigo
                    *(GhostSystems::Vector3*)(localMatAddr) = enemyPos;
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
