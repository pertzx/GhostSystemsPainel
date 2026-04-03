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
        }

        void render();
        void setVisible(bool visible) { isVisible = visible; }
        bool getVisible() const { return isVisible; }

    private:
        void initStyle();
        void drawEntityList();
        void drawFilters();
        void drawDebugPlayer();
        void drawESP();
        void drawIl2CppObject(void* obj, void* klass, const char* name, int depth, const std::string& path);

        GameState& sharedState;
        bool isVisible = true;
        
        // Flag de produção vs desenvolvimento
        bool isDebugMode = true; // Mude para false para esconder as abas de debug

        // Filtros da UI
        bool filterAliveOnly = true;
        bool filterHumansOnly = false;
        float maxDistanceFilter = 1000.0f;
        int filterTeamId = -1; // -1 significa sem filtro

        // Configurações de ESP
        bool espEnabled = true;
        bool espBox = true;
        bool espName = true;
        bool espDistance = true;
        bool espLine = true;

        // Configurações de Aimbot
        bool aimbotEnabled = true;
        int aimbotMode = 1; // 0 = Tradicional (Ao Atirar), 1 = Aimlock (Sempre)
        bool aimbotDrawFov = true;
        bool aimbotTargetAllies = true;
        float aimbotFov = 200.0f;
        int aimbotTimeMs = 100; // Tempo em milissegundos para puxar a mira

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

        void scanForPotentialValues(void* obj, void* klass, const std::string& path, int depth, std::unordered_set<void*>& visited);
    };

} // namespace GhostSystems
