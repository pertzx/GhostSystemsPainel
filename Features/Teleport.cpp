#include "Teleport.h"
#include <fstream>
#include <regex>
#include <android/log.h>
#include "SharedAccess.h"
#include "../Entity.h"

using namespace GhostSystems;

#define LOG_TAG "Teleport"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

std::vector<NamedLocation> TeleportFeature::LoadLocations() {
    std::vector<NamedLocation> result;
    
    const std::vector<std::string> files = {
        "docs/telegram_infos/KALAHARI.txt",
        "docs/telegram_infos/BERMUDA.txt"
    };
    
    // Regex para extrair Vector3(x, y, z)
    std::regex rgx(R"(Vector3\s*\(\s*([-0-9\.]+)f?\s*,\s*([-0-9\.]+)f?\s*,\s*([-0-9\.]+)f?\s*\))");
    
    for (const auto& path : files) {
        std::ifstream in(path);
        if (!in.is_open()) {
            LOGI("Failed to open %s", path.c_str());
            continue;
        }
        
        std::string line;
        std::string currentName;
        
        while (std::getline(in, line)) {
            // Ignorar linhas vazias
            if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) {
                continue;
            }
            
            // Se a linha nao contem "Vector3", e um nome de localizacao
            if (line.find("Vector3") == std::string::npos) {
                // Trim whitespace
                size_t start = line.find_first_not_of(" \t\r\n");
                size_t end = line.find_last_not_of(" \t\r\n");
                if (start != std::string::npos && end != std::string::npos) {
                    currentName = line.substr(start, end - start + 1);
                }
                continue;
            }
            
            // Tentar extrair coordenadas
            std::smatch m;
            if (std::regex_search(line, m, rgx) && m.size() == 4) {
                try {
                    float x = std::stof(m[1].str());
                    float y = std::stof(m[2].str());
                    float z = std::stof(m[3].str());
                    
                    GhostSystems::Vector3 vec = {x, y, z};
                    result.push_back({currentName, vec});
                    
                    LOGI("Loaded location: %s -> (%.2f, %.2f, %.2f)", 
                         currentName.c_str(), x, y, z);
                } catch (...) {
                    LOGI("Failed to parse vector: %s", line.c_str());
                }
                currentName.clear();
            }
        }
        
        in.close();
    }
    
    LOGI("Total locations loaded: %zu", result.size());
    return result;
}

void TeleportFeature::TeleportTo(const GhostSystems::Vector3& target) {
    if (!g_GameState) {
        LOGI("Teleport failed: g_GameState is null");
        return;
    }
    
    void* localPlayerObj = g_GameState->localPlayerObj;
    if (!localPlayerObj) {
        LOGI("Teleport failed: no local player");
        return;
    }
    
    uint64_t localPlayer = (uint64_t)localPlayerObj;
    uint64_t root = *(uint64_t*)(localPlayer + 0x670);
    
    if (!root) {
        LOGI("Teleport failed: no root node");
        return;
    }
    
    // Obter matriz de transform
    uint64_t transform = *(uint64_t*)(root + 0x10);
    if (!transform) {
        LOGI("Teleport failed: no transform");
        return;
    }
    
    uint64_t native = *(uint64_t*)(transform + 0x10);
    if (!native) {
        LOGI("Teleport failed: no native transform");
        return;
    }
    
    uint64_t transformData = *(uint64_t*)(native + 0x38);
    int index = *(int*)(native + 0x40);
    
    if (!transformData || index < 0) {
        LOGI("Teleport failed: invalid transform data");
        return;
    }
    
    uint64_t matrixList = *(uint64_t*)(transformData + 0x18);
    if (!matrixList) {
        LOGI("Teleport failed: no matrix list");
        return;
    }
    
    // Calcular endereco da matriz raiz
    uint64_t matrixAddr = matrixList + (index * 0x30);
    
    // Salvar posicao original (opcional)
    GhostSystems::Vector3 originalPos = *(GhostSystems::Vector3*)(matrixAddr);
    
    // Escrever nova posicao
    *(GhostSystems::Vector3*)(matrixAddr) = target;
    
    LOGI("Teleported player from (%.2f, %.2f, %.2f) to (%.2f, %.2f, %.2f)",
         originalPos.x, originalPos.y, originalPos.z,
         target.x, target.y, target.z);
}
