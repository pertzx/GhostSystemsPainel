#pragma once

#include "Entity.h"
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace GhostSystems {

    class Menu {
    public:
        Menu(GameState& state) : sharedState(state) {
            initStyle();
            loadUserInfo();
        }

        void render();
        void setVisible(bool visible) { isVisible = visible; }
        bool getVisible() const { return isVisible; }

    private:
        void initStyle();
        void loadUserInfo();
        void drawEntityList();
        void drawFilters();
        void drawDebugPlayer();
        void drawESP();
        void drawIl2CppObject(void* obj, void* klass, const char* name, int depth, const std::string& path);

        GameState& sharedState;
        bool isVisible = true;
        
        // Flag de produção vs desenvolvimento
        bool isDebugMode = true; // Mude para false para esconder as abas de debug

        // Master Switch
        bool masterSwitch = false;

        // Login System
        bool isLogged = true;

        // User Info
        std::string userName = "GhostSystems User";
        std::string userPlan = "Premium";
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
        bool aimbotTargetAllies = true;
float aimbotFov = 200.0f;
bool aimbotVisibilityCheck = true; // Só puxa se o player estiver visível
bool aimbotMagnetic = false; // Mira Magnética (Puxa o inimigo pra frente da mira)
int aimbotHitbox = 0; // 0 = Cabeça, 1 = Pescoço, 2 = Peito

// Lógica de Smooth e Força (Aimbot Delay)
float aimbotPullStrength = 1.5f; // Força base do aimbot (aumentada pra ficar mais forte)
float aimbotSmoothTimeMs = 30.0f; // Tempo de transição peito -> alvo configurado (Valor inicial forte/baixo)
float aimbotSmoothCurve = 3.0f; // Curva de aceleração da puxada

std::unordered_map<void*, float> aimbotTargetTimeMap; // Guarda o tempo de foco por entidade

        // Variaveis de Debug Aimbot
        bool aimbotHasTarget = false;
        std::string aimbotTargetName = "Nenhum";
        float aimbotTargetDistFOV = 0.0f;
        float aimbotTargetDist3D = 0.0f;
        float aimbotCamPosX = 0.0f, aimbotCamPosY = 0.0f, aimbotCamPosZ = 0.0f;
        float aimbotCamRotX = 0.0f, aimbotCamRotY = 0.0f, aimbotCamRotZ = 0.0f, aimbotCamRotW = 0.0f;
        float aimbotTargetRotX = 0.0f, aimbotTargetRotY = 0.0f, aimbotTargetRotZ = 0.0f, aimbotTargetRotW = 0.0f;
        float aimbotNewRotX = 0.0f, aimbotNewRotY = 0.0f, aimbotNewRotZ = 0.0f, aimbotNewRotW = 0.0f;
        std::string aimbotErrorLog = "Nenhum erro";

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
        bool silentAimEnabled = false;
        int silentAimApproach = 0; // 0=Hook GetFireDirection, 1=Hook StartFiring, 2=Hook LookAtPosition, 3=Hook GetLookDirection, 4=Raycast Predicition, 5=Modificar Rotacao
        bool silentAimDrawDebug = false;
        float silentAimSmooth = 0.1f; // Suavização do silent aim
        bool silentAimFovCheck = true; // Verificar FOV antes de aplicar
        float silentAimMaxDistance = 300.0f; // Distância máxima do silent aim
        std::string silentAimStatus = "Inativo";

        void scanForPotentialValues(void* obj, void* klass, const std::string& path, int depth, std::unordered_set<void*>& visited);
    };

} // namespace GhostSystems
