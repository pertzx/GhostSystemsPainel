#include "UpDown.h"
#include <unistd.h>
#include <chrono>
#include "SharedAccess.h"
#include "../Entity.h"

using namespace GhostSystems;

UpDownFeature::UpDownFeature() {}
UpDownFeature::~UpDownFeature() {}

void UpDownFeature::RenderUI() {}

void UpDownFeature::Start() {
    if (running) return;
    running = true;
    thread = std::thread(&UpDownFeature::Worker, this);
}

void UpDownFeature::Stop() {
    if (!running) return;
    running = false;
    if (thread.joinable()) thread.join();
}

void UpDownFeature::Worker() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        if (!g_GameState) continue;
        void* localPlayerObj = g_GameState->localPlayerObj;
        if (!localPlayerObj) continue;
        
        uint64_t localPlayer = (uint64_t)localPlayerObj;
        
        // Obter root node do player
        uint64_t rootNode = *(uint64_t*)(localPlayer + 0x670);
        if (!rootNode) continue;
        
        // Obter matriz da raiz
        uint64_t matrixAddr = 0;
        if (!GetRootMatrixAddr(rootNode, matrixAddr)) continue;
        
        // Ler posicao atual
        GhostSystems::Vector3 currentPos = *(GhostSystems::Vector3*)matrixAddr;
        
        if (enableUp) {
            currentPos.y += upSpeed;
            *(GhostSystems::Vector3*)matrixAddr = currentPos;
        }
        
        if (enableDown) {
            currentPos.y -= downSpeed;
            *(GhostSystems::Vector3*)matrixAddr = currentPos;
        }
    }
}

// Helper para obter endereco da matriz raiz (mesmo do AimLock)
bool UpDownFeature::GetRootMatrixAddr(uint64_t node, uint64_t& outAddr) {
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

void UpDownFeature::EnableUp(float speed) {
    upSpeed = speed > 0 ? speed : 0.5f;
    enableUp = true;
    enableDown = false;
    if (!running) Start();
}

void UpDownFeature::EnableDown(float speed) {
    downSpeed = speed > 0 ? speed : 0.5f;
    enableDown = true;
    enableUp = false;
    if (!running) Start();
}

void UpDownFeature::Disable() {
    enableUp = false;
    enableDown = false;
}
