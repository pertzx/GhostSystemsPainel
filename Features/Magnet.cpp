#include "Magnet.h"
#include <unistd.h>
#include <chrono>
#include <cmath>
#include <mutex>
#include "SharedAccess.h"
#include "../Entity.h"

using namespace GhostSystems;

MagnetFeature::MagnetFeature() {}
MagnetFeature::~MagnetFeature() { Stop(); }

void MagnetFeature::RenderUI() {}

void MagnetFeature::Start() {
    if (running) return;
    running = true;
    thread = std::thread(&MagnetFeature::Worker, this);
}

void MagnetFeature::Stop() {
    if (!running) return;
    running = false;
    if (thread.joinable()) thread.join();
}

// Estrutura de lock do magnet
struct MagnetLockInfo {
    uint64_t targetAddr;
    GhostSystems::Vector3 originalPos;
    bool isLocked;
    float distance;
    GhostSystems::Vector3 headOffset;
    GhostSystems::Vector3 lastPos;
};

static std::mutex g_magnetMutex;
static bool g_magnetEnabled = false;
static uint64_t g_magnetTarget = 0;
static bool g_magnetIsFiring = false;
static MagnetLockInfo g_magnetLockInfo = {0, {0,0,0}, false, 0.0f, {0,0,0}, {0,0,0}};

// Helper para obter posição de um bone
static GhostSystems::Vector3 GetNodePosition(uint64_t boneNode) {
    uint64_t transform = *(uint64_t*)(boneNode + 0x10);
    if (!transform) return {0,0,0};
    
    uint64_t matrixAddr = *(uint64_t*)(transform + 0x10);
    if (!matrixAddr) return {0,0,0};
    
    uint64_t transformData = *(uint64_t*)(matrixAddr + 0x38);
    if (!transformData) return {0,0,0};
    
    uint64_t matrixList = *(uint64_t*)(transformData + 0x18);
    int index = *(int*)(matrixAddr + 0x40);
    if (index < 0 || !matrixList) return {0,0,0};
    
    return *(GhostSystems::Vector3*)(matrixList + (index * 0x30));
}

void MagnetFeature::Worker() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        bool enabled, isFiring;
        uint64_t target;
        
        {
            std::lock_guard<std::mutex> lock(g_magnetMutex);
            enabled = g_magnetEnabled;
            target = g_magnetTarget;
            isFiring = g_magnetIsFiring;
        }
        
        if (!enabled || !target || !isFiring) {
            // Restaurar posição se desativado ou não atirando
            if (g_magnetLockInfo.isLocked && g_magnetLockInfo.targetAddr != 0) {
                uint64_t rootBone = *(uint64_t*)(g_magnetLockInfo.targetAddr + 0x670);
                if (rootBone) {
                    uint64_t transform = *(uint64_t*)(rootBone + 0x10);
                    if (transform) {
                        uint64_t matrix = *(uint64_t*)(transform + 0x10);
                        if (matrix) {
                            uint64_t transformData = *(uint64_t*)(matrix + 0x38);
                            int index = *(int*)(matrix + 0x40);
                            if (transformData && index >= 0) {
                                uint64_t matrixList = *(uint64_t*)(transformData + 0x18);
                                if (matrixList) {
                                    *(GhostSystems::Vector3*)(matrixList + (index * 0x30)) = g_magnetLockInfo.originalPos;
                                }
                            }
                        }
                    }
                }
            }
            g_magnetLockInfo.isLocked = false;
            g_magnetLockInfo.targetAddr = 0;
            continue;
        }
        
        // Obter bones do inimigo
        uint64_t rootBone = *(uint64_t*)(target + 0x670);
        uint64_t headBone = *(uint64_t*)(target + 0x648);
        
        if (!rootBone || !headBone) {
            g_magnetLockInfo.isLocked = false;
            continue;
        }
        
        // Obter posição atual
        GhostSystems::Vector3 currentRootPos = GetNodePosition(rootBone);
        GhostSystems::Vector3 currentHeadPos = GetNodePosition(headBone);
        
        if (currentRootPos.x == 0 && currentRootPos.y == 0 && currentRootPos.z == 0) {
            g_magnetLockInfo.isLocked = false;
            continue;
        }
        
        // Verificar se precisa criar novo lock
        if (!g_magnetLockInfo.isLocked || g_magnetLockInfo.targetAddr != target) {
            g_magnetLockInfo.targetAddr = target;
            g_magnetLockInfo.originalPos = currentRootPos;
            g_magnetLockInfo.isLocked = true;
            
            // Calcular distância e offset da cabeça
            float dx = currentHeadPos.x - currentRootPos.x;
            float dy = currentHeadPos.y - currentRootPos.y;
            float dz = currentHeadPos.z - currentRootPos.z;
            float dist = sqrtf(dx*dx + dy*dy + dz*dz);
            
            if (dist < 1.5f) dist = 1.5f;
            
            g_magnetLockInfo.distance = dist;
            g_magnetLockInfo.headOffset = {
                currentHeadPos.x - currentRootPos.x,
                currentHeadPos.y - currentRootPos.y,
                currentHeadPos.z - currentRootPos.z
            };
            g_magnetLockInfo.lastPos = currentRootPos;
        }
        
        // Obter posição da câmera (simplificado)
        GhostSystems::Vector3 camPos = {0,0,0}; // Needs actual camera position
        
        // Direção forward da câmera
        GhostSystems::Vector3 camFwd = {0, 1, 0}; // Needs view matrix
        
        // Normalizar
        float fwdMag = sqrtf(camFwd.x*camFwd.x + camFwd.y*camFwd.y + camFwd.z*camFwd.z);
        if (fwdMag > 0.001f) {
            camFwd.x /= fwdMag;
            camFwd.y /= fwdMag;
            camFwd.z /= fwdMag;
        }
        
        // Calcular posição desejada da cabeça
        GhostSystems::Vector3 desiredHead;
        desiredHead.x = camPos.x + camFwd.x * g_magnetLockInfo.distance;
        desiredHead.y = camPos.y + camFwd.y * g_magnetLockInfo.distance;
        desiredHead.z = camPos.z + camFwd.z * g_magnetLockInfo.distance;
        
        // Calcular posição desejada da raiz
        GhostSystems::Vector3 desiredRoot;
        desiredRoot.x = desiredHead.x - g_magnetLockInfo.headOffset.x;
        desiredRoot.y = desiredHead.y - g_magnetLockInfo.headOffset.y;
        desiredRoot.z = desiredHead.z - g_magnetLockInfo.headOffset.z;
        
        // Suavização (factor 0.4)
        float s = 0.4f;
        GhostSystems::Vector3 from = g_magnetLockInfo.lastPos;
        
        GhostSystems::Vector3 newPos;
        newPos.x = from.x + (desiredRoot.x - from.x) * s;
        newPos.y = from.y + (desiredRoot.y - from.y) * s;
        newPos.z = from.z + (desiredRoot.z - from.z) * s;
        
        g_magnetLockInfo.lastPos = newPos;
        
        // Escrever nova posição
        uint64_t transform = *(uint64_t*)(rootBone + 0x10);
        if (transform) {
            uint64_t matrix = *(uint64_t*)(transform + 0x10);
            if (matrix) {
                uint64_t transformData = *(uint64_t*)(matrix + 0x38);
                int index = *(int*)(matrix + 0x40);
                if (transformData && index >= 0) {
                    uint64_t matrixList = *(uint64_t*)(transformData + 0x18);
                    if (matrixList) {
                        *(GhostSystems::Vector3*)(matrixList + (index * 0x30)) = newPos;
                    }
                }
            }
        }
    }
}

// Funções de controle chamadas pelo Menu
void MagnetFeature::SetEnabled(bool enabled, uint64_t target, bool isFiring) {
    std::lock_guard<std::mutex> lock(g_magnetMutex);
    g_magnetEnabled = enabled;
    g_magnetTarget = target;
    g_magnetIsFiring = isFiring;
}