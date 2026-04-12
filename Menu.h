#pragma once

#include "Entity.h"
#include "Obfuscator.h"
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace GhostSystems {

    class Menu {
    public:
        Menu(GameState& state) : sharedState(state) {
            menuInstance = this;
            initStyle();
            loadUserInfo();
        }

        void render();
        void setVisible(bool visible) { isVisible = visible; }
        bool getVisible() const { return isVisible; }

        static void* hookedIsFiringMethod;
        static void* hookedGetFireDirectionMethod;
        static void* hookedStartFiringMethod;
        static void* hookedFireProjectileMethod;
        static void* hookedGetShotDirectionMethod;
        static void* hookedApplyBulletForceMethod;
        static bool isFiringHookActive;
        static bool pendingSilentAim;
        static Menu* menuInstance;

        bool silentAimEnabled = false;
        bool aimbotTargetAllies = true;
        float aimbotFov = 200.0f;
        GameState& sharedState;
        void* setAimRotationMethod = nullptr;
        void* lookRotMethod = nullptr;
        void* getTransformMethod = nullptr;
        void* getPosMethod = nullptr;
        void* getFireDirectionMethod = nullptr;
        void* startFiringMethod = nullptr;
        void* getHeadTFMethod = nullptr;
        int aimbotHitbox = 0;
        int silentAimApproach = 0;
        float silentAimMaxDistance = 300.0f;
        std::string silentAimStatus = OBFUSCATE("Inativo");



    private:
        void initStyle();
        void loadUserInfo();
        void drawEntityList();
        void drawFilters();
        void drawDebugPlayer();
        void drawESP();
        void drawIl2CppObject(void* obj, void* klass, const char* name, int depth, const std::string& path);

        bool isVisible = true;

        // Flag de produção vs desenvolvimento
        bool isDebugMode = true; // Mude para false para esconder as abas de debug

        // Master Switch
        bool masterSwitch = false;

        // Login System
        bool isLogged = true;

        // User Info
        std::string userName = OBFUSCATE("GhostSystems User");
        std::string userPlan = OBFUSCATE("Premium");
        long long userTimeLeft = 0;
        long long userExpireAt = -1;

        // Filtros da UI
        bool filterAliveOnly = true;
        bool filterHumansOnly = false;
        float maxDistanceFilter = 100.0f;
        int filterTeamId = -1; // -1 significa sem filtro

        // Configurações de ESP
        bool espEnabled = true;
        bool espBox = true;
        int espBoxStyle = 1; // 0 = Box Completa, 1 = Box Cantos (Corners)
        float espColorEnemy[4] = {1.0f, 1.0f, 1.0f, 1.0f}; // Branco
        float espColorAlly[4] = {1.0f, 1.0f, 1.0f, 1.0f}; // Branco
        float fovColor[4] = {1.0f, 1.0f, 1.0f, 1.0f}; // Branco
        bool espName = true;
        bool espDistance = true;
        bool espHealth = true;
        bool espLine = true;
        bool espSkeleton = false;
        float espMaxDistance = 100.0f;
        
        // Configurações de Aimbot
        bool aimbotEnabled = true;
        int aimbotMode = 0; // 0 = Tradicional (Ao Atirar), 1 = Aimlock (Sempre)
        bool aimbotDrawFov = true;
bool aimbotVisibilityCheck = true; // Só puxa se o player estiver visível
        bool aimbotMagnetic = false; // Mira Magnética (Puxa o inimigo pra frente da mira)

// Lógica de Smooth e Força (Aimbot Delay)
float aimbotPullStrength = 1.5f; // Força base do aimbot (aumentada pra ficar mais forte)
float aimbotSmoothTimeMs = 30.0f; // Tempo de transição peito -> alvo configurado (Valor inicial forte/baixo)
float aimbotSmoothCurve = 3.0f; // Curva de aceleração da puxada

std::unordered_map<void*, float> aimbotTargetTimeMap; // Guarda o tempo de foco por entidade

        // Variaveis de Debug Aimbot
        bool aimbotHasTarget = false;
        std::string aimbotTargetName = OBFUSCATE("Nenhum");
        float aimbotTargetDistFOV = 0.0f;
        float aimbotTargetDist3D = 0.0f;
        float aimbotCamPosX = 0.0f, aimbotCamPosY = 0.0f, aimbotCamPosZ = 0.0f;
        float aimbotCamRotX = 0.0f, aimbotCamRotY = 0.0f, aimbotCamRotZ = 0.0f, aimbotCamRotW = 0.0f;
        float aimbotTargetRotX = 0.0f, aimbotTargetRotY = 0.0f, aimbotTargetRotZ = 0.0f, aimbotTargetRotW = 0.0f;
        float aimbotNewRotX = 0.0f, aimbotNewRotY = 0.0f, aimbotNewRotZ = 0.0f, aimbotNewRotW = 0.0f;
        std::string aimbotErrorLog = OBFUSCATE("Nenhum erro");

        // Variaveis de controle de tempo (Delay) do Aimbot
        float aimbotDelayTimer = 0.0f;
        bool wasAimingLastFrame = false;

        // Filtros Debug Player
        struct PotentialValue {
            std::string path;
            void* obj;
            size_t offset;
            std::string type; // "float", "int", "bool"
        };
        std::vector<PotentialValue> debugPotentialValues;
        bool hasScannedValues = false;
        
        // Filtros de busca no scanner
        char searchFilter[64] = "";
        bool filterOnlyFloats = false;
        bool filterOnlyInts = false;

        // Configurações de Misc
        bool infJumpEnabled = false;
        float infJumpStep = 2.0f;

        // Configurações de Silent Aim
        bool silentAimDrawDebug = false;
        float silentAimSmooth = 0.1f; // Suavização do silent aim
        bool silentAimFovCheck = true; // Verificar FOV antes de aplicar

        void scanForPotentialValues(void* obj, void* klass, const std::string& path, int depth, std::unordered_set<void*>& visited);
    };

    // Define struct para Vector3 que será usado no hook
    struct HookVector3Args { float x, y, z; };

    namespace Hooks {
        bool hook_IsFiring(void* playerObj, void* exc);
        void* hook_GetFireDirection(void* playerObj, bool* isSkill);
        void hook_StartFiring(void* playerObj, void* weaponObj);
    }

} // namespace GhostSystems
