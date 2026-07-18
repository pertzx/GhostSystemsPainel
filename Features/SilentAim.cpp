#include "SilentAim.h"
#include <unistd.h>
#include <chrono>
#include <cmath>
#include <thread>
#include "SharedAccess.h"
#include "../OffsetResolver.h"
#include "../Il2CppHelper.h"

using namespace GhostSystems;

// Static members initialization
std::mutex SilentAimFeature::silentLock;
void* SilentAimFeature::g_HitObjInfo = nullptr;
GhostSystems::Vector3 SilentAimFeature::g_TargetDir = {0,0,0};
bool SilentAimFeature::g_HasData = false;

SilentAimFeature::SilentAimFeature() {}
SilentAimFeature::~SilentAimFeature() { Stop(); }

void SilentAimFeature::RenderUI() {}

void SilentAimFeature::Start() {
    if (running) return;
    running = true;
    workerThread = std::thread(&SilentAimFeature::Worker, this);
}

void SilentAimFeature::Stop() {
    if (!running) return;
    running = false;
    {
        std::lock_guard<std::mutex> lock(silentLock);
        g_HasData = false;
        g_HitObjInfo = nullptr;
    }
    if (workerThread.joinable()) workerThread.join();
}

// Helper para obter posição da cabeça do inimigo
static GhostSystems::Vector3 GetHeadPosition(GhostSystems::PlayerEntity& entity) {
    return {entity.position.x, entity.position.y + 1.6f, entity.position.z};
}

// Helper para obter HP
static int GetHp(GhostSystems::PlayerEntity& entity) {
    return (int)entity.health;
}

void SilentAimFeature::Worker() {
    // Aguarda Il2Cpp estar pronto antes de começar
    int wait_count = 0;
    while (!Il2Cpp::domain_get && wait_count < 200) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        wait_count++;
    }

    // Resolve offsets on first run
    static size_t firingOffset = 0;
    static size_t ammoOriginOffset = 0;
    static size_t directionOffset = 0;
    static bool offsetsResolved = false;
    
    if (!offsetsResolved) {
        firingOffset = g_OffsetResolver().GetPlayerFieldOffset("firing") ?: 
                       g_OffsetResolver().GetPlayerFieldOffset("isFiring") ?:
                       g_OffsetResolver().GetPlayerFieldOffset("shooting");
        ammoOriginOffset = g_OffsetResolver().GetPlayerFieldOffset("ammo") ?:
                           g_OffsetResolver().GetPlayerFieldOffset("origin") ?:
                           g_OffsetResolver().GetPlayerFieldOffset("firePos");
        directionOffset = g_OffsetResolver().GetPlayerFieldOffset("direction") ?:
                          g_OffsetResolver().GetPlayerFieldOffset("aimDir");
        offsetsResolved = true;
    }
    
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 1ms = 1000Hz
        if (!enabled) continue;
        
        if (!g_GameState) continue;
        void* localPlayerObj = g_GameState->localPlayerObj;
        if (!localPlayerObj) continue;
        
        uint64_t localPlayer = (uint64_t)localPlayerObj;
        
        // Verificar se está atirando - use resolved offset or fallback
        bool isFiring = false;
        if (firingOffset) {
            isFiring = *(bool*)((uintptr_t)localPlayer + firingOffset);
        } else {
            // Fallback: try common offsets
            static const size_t fallbackOffsets[] = {0x7E0, 0x7D8, 0x7F0, 0x800};
            for (size_t off : fallbackOffsets) {
                if (*(bool*)((uintptr_t)localPlayer + off)) {
                    isFiring = true;
                    firingOffset = off;
                    break;
                }
            }
        }
        
        if (!isFiring) {
            std::lock_guard<std::mutex> lock(silentLock);
            g_HasData = false;
            g_HitObjInfo = nullptr;
            continue;
        }
        
        // Obter o alvo mais próximo do centro da tela
        GhostSystems::PlayerEntity* closestEnemy = nullptr;
        float closestDist = 999999.0f;
        {
            std::lock_guard<std::mutex> lock(g_GameState->mtx);
            if (g_GameState->entities.empty()) continue;
            
            for (auto& entity : g_GameState->entities) {
                if (!entity.isAlive() || entity.alignment != GhostSystems::Alignment::ENEMY) continue;
                float dist = entity.position.distance(g_GameState->localPlayerPos);
                if (dist < closestDist) {
                    closestDist = dist;
                    closestEnemy = &entity;
                }
            }
        }
        
        if (!closestEnemy) {
            std::lock_guard<std::mutex> lock(silentLock);
            g_HasData = false;
            g_HitObjInfo = nullptr;
            continue;
        }
        
        // Calcular direção do tiro
        GhostSystems::Vector3 enemyPos = GetHeadPosition(*closestEnemy);
        
        // Ajuste para baixa mira em inimigos com pouco HP
        int hp = GetHp(*closestEnemy);
        if (hp < 50) {
            enemyPos.y -= 0.45f;
        }
        
        // Obter ponto de origem do tiro (ammo base) - use resolved offset
        void* hitObjInfo = nullptr;
        if (ammoOriginOffset) {
            hitObjInfo = *(void**)((uintptr_t)localPlayer + ammoOriginOffset);
        } else {
            // Fallback offsets
            static const size_t fallbackAmmoOffsets[] = {0x5C0, 0x5B8, 0x5D0, 0x5E0};
            for (size_t off : fallbackAmmoOffsets) {
                void* test = *(void**)((uintptr_t)localPlayer + off);
                if (test) {
                    hitObjInfo = test;
                    ammoOriginOffset = off;
                    break;
                }
            }
        }
        
        if (!hitObjInfo) {
            std::lock_guard<std::mutex> lock(silentLock);
            g_HasData = false;
            continue;
        }
        
        GhostSystems::Vector3 ammoBase = *(GhostSystems::Vector3*)((uintptr_t)hitObjInfo + 0x10);
        
        // Calcular vetor direção
        GhostSystems::Vector3 dir;
        dir.x = enemyPos.x - ammoBase.x;
        dir.y = enemyPos.y - ammoBase.y;
        dir.z = enemyPos.z - ammoBase.z;
        
        // Normalizar
        float len = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
        if (len > 0.001f) {
            dir.x /= len;
            dir.y /= len;
            dir.z /= len;
        }
        
        // Armazenar dados protegidos por mutex
        {
            std::lock_guard<std::mutex> lock(silentLock);
            g_HitObjInfo = hitObjInfo;
            g_TargetDir = dir;
            g_HasData = true;
        }
    }
}

// Static thread for applying aim direction
static void ApplySilentAimThreadFunc() {
    // Aguarda Il2Cpp estar pronto
    int wait_count = 0;
    while (!Il2Cpp::domain_get && wait_count < 200) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        wait_count++;
    }

    static size_t directionWriteOffset = 0;
    static bool writeOffsetResolved = false;
    
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 1ms = ~1000Hz, mais amigavel que 1us
        void* currentHitObj = nullptr;
        GhostSystems::Vector3 currentDir = {0,0,0};
        bool valid = false;
        
        {
            std::lock_guard<std::mutex> lock(SilentAimFeature::silentLock);
            valid = SilentAimFeature::g_HasData;
            if (valid) {
                currentHitObj = SilentAimFeature::g_HitObjInfo;
                currentDir = SilentAimFeature::g_TargetDir;
                SilentAimFeature::g_HasData = false;
            }
        }
        
        if (valid && currentHitObj) {
            if (!writeOffsetResolved) {
                directionWriteOffset = g_OffsetResolver().GetPlayerFieldOffset("direction") ?:
                                       g_OffsetResolver().GetPlayerFieldOffset("aimDir") ?:
                                       0x20; // fallback
                writeOffsetResolved = true;
            }
            *(GhostSystems::Vector3*)((uintptr_t)currentHitObj + directionWriteOffset) = currentDir;
        }
    }
}

void SilentAimFeature::StartApplyThread() {
    static bool started = false;
    if (!started) {
        started = true;
        std::thread t([]{
            // Aguarda Il2Cpp antes de iniciar
            int wait_count = 0;
            while (!Il2Cpp::domain_get && wait_count < 200) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                wait_count++;
            }
            ApplySilentAimThreadFunc();
        });
        t.detach();
    }
}