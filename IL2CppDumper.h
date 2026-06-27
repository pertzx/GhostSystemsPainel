#pragma once

// ============================================
// GhostSystems - IL2CPP RUNTIME DUMPER + CACHED OFFSETS
// Gera offsets.h COMPLETO e faz cache em runtime
// ============================================

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <map>
#include "Il2CppHelper.h"
#include <thread>

namespace GhostSystems {

// Estrutura para armazenar offsets cachados
struct CachedOffset {
    const char* className;
    const char* fieldName;
    size_t offset;
    bool found;
};

// Classe principal do Dumper
class IL2CppDumper {
public:
    // ============================================
    // PRIMEIRO SCAN - Guarda valores em cache
    // ============================================
    static bool InitializeOffsets() {
        // Tenta carregar do cache primeiro
        if (LoadCachedOffsets()) {
            LOGI("[Offsets] Carregado do cache com sucesso!");
            return true;
        }
        
        // Se nao tem cache, faz scan completo
        LOGI("[Offsets] Realizando primeiro scan...");
        return ScanAndCacheAllOffsets();
    }
    
    // ============================================
    // DUMPER COMPLETO - Gera arquivo
    // ============================================
    static void DumpAll() {
        const char* outputPath = "/sdcard/Android/data/com.dts.freefireth/files/generated_offsets.h";
        const char* logPath = "/sdcard/Android/data/com.dts.freefireth/files/dump_complete.txt";
        
        FILE* logFp = fopen(logPath, "w");
        if (!logFp) logFp = stdout;
        
        fprintf(logFp, "=== GhostSystems IL2Cpp Dumper ===\n");
        fprintf(logFp, "Data: %s %s\n\n", __DATE__, __TIME__);
        
        std::vector<FieldInfo> allFields;
        
        // Scan de TODAS as classes relevantes
        ScanAllClasses(allFields, logFp);
        
        // Gera arquivo header
        GenerateHeaderFile(outputPath, allFields);
        
        fprintf(logFp, "\n=== DUMP CONCLUIDO ===\n");
        fprintf(logFp, "Arquivo: %s\n", outputPath);
        
        if (logFp != stdout) fclose(logFp);
        LOGI("[Dumper] Dump completo gerado: %s", outputPath);
    }
    
    // Dump de classe especifica
    static void DumpClass(const char* assembly, const char* namespaze, const char* className) {
        const char* logPath = "/sdcard/Android/data/com.dts.freefireth/files/dump_class.txt";
        FILE* fp = fopen(logPath, "a");
        if (!fp) fp = stdout;
        
        fprintf(fp, "\n=== %s::%s ===\n", namespaze ? namespaze : "", className);
        
        void* klass = Il2Cpp::GetClass(assembly, namespaze, className);
        if (!klass) {
            fprintf(fp, "Classe nao encontrada\n");
            if (fp != stdout) fclose(fp);
            return;
        }
        
        DumpClassFields(fp, klass, className);
        
        if (fp != stdout) fclose(fp);
    }
    
private:
    struct FieldInfo {
        std::string assembly;
        std::string namespaze;
        std::string className;
        std::string fieldName;
        size_t offset;
        std::string typeName;
    };
    
    // ============================================
    // SCAN E CACHE
    // ============================================
    static bool LoadCachedOffsets() {
        // Implementacao simplificada - retorna false para forcar scan
        // Em producao, leria de um arquivo binario ou memoria persistente
        return false;
    }
    
    static bool ScanAndCacheAllOffsets() {
        // Scan das classes principais
        ScanClass("Assembly-CSharp.dll", "COW", "GameFacade");
        ScanClass("Assembly-CSharp.dll", "COW", "GameData");
        ScanClass("Assembly-CSharp.dll", "COW", "Player");
        ScanClass("Assembly-CSharp.dll", "COW", "Soldier");
        ScanClass("Assembly-CSharp.dll", "COW", "Character");
        ScanClass("Assembly-CSharp.dll", "COW", "TeamService");
        
        // Unity
        ScanClass("UnityEngine.CoreModule.dll", "UnityEngine", "Camera");
        ScanClass("UnityEngine.CoreModule.dll", "UnityEngine", "Transform");
        ScanClass("UnityEngine.CoreModule.dll", "UnityEngine", "Component");
        
        LOGI("[Offsets] Scan inicial concluido");
        return true;
    }
    
    static void ScanClass(const char* assembly, const char* namespaze, const char* className) {
        void* klass = Il2Cpp::GetClass(assembly, namespaze, className);
        if (!klass) return;
        
        LOGI("[Scan] %s::%s encontrado", namespaze ? namespaze : "", className);
        
        void* iter = nullptr;
        void* field = nullptr;
        
        while ((field = Il2Cpp::class_get_fields(klass, &iter)) != nullptr) {
            const char* fieldName = Il2Cpp::field_get_name(field);
            size_t offset = Il2Cpp::field_get_offset(field);
            
            if (fieldName && offset > 0) {
                // Log apenas campos uteis
                if (IsFieldRelevant(fieldName)) {
                    LOGI("  [0x%04llX] %s", (unsigned long long)offset, fieldName);
                }
            }
        }
    }
    
    static bool IsFieldRelevant(const char* fieldName) {
        const char* relevant[] = {
            "position", "Position", "pos", "Pos",
            "health", "Health", "hp", "HP",
            "team", "Team", "TEAM",
            "alive", "Alive", "dead", "Dead",
            "name", "Name", "nick", "Nick",
            "matrix", "Matrix", "transform", "Transform",
            "camera", "Camera",
            "Current", "Match", "Game",
            nullptr
        };
        
        for (int i = 0; relevant[i]; i++) {
            if (strstr(fieldName, relevant[i])) return true;
        }
        return false;
    }
    
    // ============================================
    // SCAN COMPLETO DE TODAS CLASSES
    // ============================================
    static void ScanAllClasses(std::vector<FieldInfo>& allFields, FILE* logFp) {
        // Classes do Free Fire
        const char* assemblies[] = {
            "Assembly-CSharp.dll",
            "UnityEngine.CoreModule.dll",
            "UnityEngine.PhysicsModule.dll"
        };
        
        const char* cowClasses[] = {
            "GameFacade", "GameService", "GameData", "GamePlayData",
            "Player", "PlayerCharacter", "PlayerController",
            "Character", "Soldier", "ActorInfo", "CharacterMotor",
            "HumanoidCharacter", "DefaultPlayer", "PlayerEntity",
            "TeamService", "TeamData", "TeamMember",
            "Entity", "GameEntity", "LivingEntity",
            "DamageableEntity", "AttackableEntity",
            "Weapon", "GunController", "WeaponComponent",
            "PlayerHealthComponent", "HealthComponent",
            "PlayerMovement", "MovementComponent",
            "BattleRoyaleManager", "GameModeManager"
        };
        
        const char* unityClasses[] = {
            "Camera", "Transform", "Vector3", "Vector2", "Quaternion",
            "Physics", "Component", "GameObject", "Material",
            "Renderer", "MeshRenderer", "SkinnedMeshRenderer"
        };
        
        fprintf(logFp, "\n[1] COW Classes:\n");
        for (const char* cls : cowClasses) {
            ScanClassFull("Assembly-CSharp.dll", "COW", cls, allFields, logFp);
        }
        
        fprintf(logFp, "\n[2] Unity Classes:\n");
        for (const char* cls : unityClasses) {
            ScanClassFull("UnityEngine.CoreModule.dll", "UnityEngine", cls, allFields, logFp);
        }
    }
    
    static void ScanClassFull(const char* assembly, const char* namespaze,
                             const char* className, std::vector<FieldInfo>& allFields, FILE* logFp) {
        void* klass = Il2Cpp::GetClass(assembly, namespaze, className);
        if (!klass) {
            klass = Il2Cpp::GetClass(assembly, nullptr, className);
            if (!klass) return;
        }
        
        fprintf(logFp, "\nClass: %s.%s\n", namespaze ? namespaze : "", className);
        
        void* iter = nullptr;
        void* field = nullptr;
        int count = 0;
        
        while ((field = Il2Cpp::class_get_fields(klass, &iter)) != nullptr) {
            const char* fieldName = Il2Cpp::field_get_name(field);
            size_t offset = Il2Cpp::field_get_offset(field);
            
            void* fieldType = Il2Cpp::field_get_type(field);
            const char* typeName = fieldType ? Il2Cpp::type_get_name(fieldType) : "unknown";
            
            if (fieldName && fieldName[0] != 0) {
                fprintf(logFp, "  0x%04llX  %-35s  %s\n",
                       (unsigned long long)offset, fieldName, typeName ? typeName : "unknown");
                
                FieldInfo info;
                info.assembly = assembly;
                info.namespaze = namespaze ? namespaze : "";
                info.className = className;
                info.fieldName = fieldName;
                info.offset = offset;
                info.typeName = typeName ? typeName : "unknown";
                
                allFields.push_back(info);
                count++;
            }
        }
        
        fprintf(logFp, "  Total: %d campos\n", count);
    }
    
    static void DumpClassFields(FILE* fp, void* klass, const char* className) {
        if (!klass || !Il2Cpp::class_get_fields) return;
        
        void* iter = nullptr;
        int count = 0;
        
        while (void* field = Il2Cpp::class_get_fields(klass, &iter)) {
            const char* fieldName = Il2Cpp::field_get_name(field);
            size_t offset = Il2Cpp::field_get_offset(field);
            
            void* fieldType = Il2Cpp::field_get_type(field);
            const char* typeName = fieldType ? Il2Cpp::type_get_name(fieldType) : "unknown";
            
            if (fieldName) {
                fprintf(fp, "  0x%04llX  %-35s  %s\n",
                       (unsigned long long)offset, fieldName, typeName ? typeName : "unknown");
                count++;
            }
        }
        
        fprintf(fp, "  Total: %d campos\n", count);
    }
    
    // ============================================
    // GERA ARQUIVO HEADER
    // ============================================
    static void GenerateHeaderFile(const char* outputPath, const std::vector<FieldInfo>& allFields) {
        FILE* fp = fopen(outputPath, "w");
        if (!fp) return;
        
        fprintf(fp, "#pragma once\n");
        fprintf(fp, "// ============================================\n");
        fprintf(fp, "// GhostSystems - OFFSETS COMPLETOS\n");
        fprintf(fp, "// Gerado: %s %s\n", __DATE__, __TIME__);
        fprintf(fp, "// ============================================\n\n");
        
        fprintf(fp, "#include <cstdint>\n\n");
        fprintf(fp, "namespace GhostSystems {\n");
        fprintf(fp, "namespace Offsets {\n\n");
        
        // Player offsets
        fprintf(fp, "// === PLAYER OFFSETS ===\n");
        
        size_t playerPos = 0, playerHp = 0, playerMaxHp = 0;
        size_t playerTeam = 0, playerAlive = 0, playerBot = 0;
        
        for (const auto& f : allFields) {
            std::string fn = f.fieldName;
            
            if ((fn.find("position") != std::string::npos || fn.find("Position") != std::string::npos) && !playerPos) {
                playerPos = f.offset;
                fprintf(fp, "constexpr size_t PLAYER_POSITION = 0x%llX; // %s::%s\n",
                       (unsigned long long)f.offset, f.className.c_str(), fn.c_str());
            }
            if ((fn.find("health") != std::string::npos || fn.find("Health") != std::string::npos ||
                 fn.find("hp") != std::string::npos || fn.find("HP") != std::string::npos) &&
                fn.find("max") == std::string::npos && fn.find("Max") == std::string::npos && !playerHp) {
                playerHp = f.offset;
                fprintf(fp, "constexpr size_t PLAYER_HEALTH = 0x%llX; // %s::%s\n",
                       (unsigned long long)f.offset, f.className.c_str(), fn.c_str());
            }
            if ((fn.find("health") != std::string::npos || fn.find("Health") != std::string::npos ||
                 fn.find("hp") != std::string::npos || fn.find("HP") != std::string::npos) &&
                (fn.find("max") != std::string::npos || fn.find("Max") != std::string::npos) && !playerMaxHp) {
                playerMaxHp = f.offset;
                fprintf(fp, "constexpr size_t PLAYER_MAX_HEALTH = 0x%llX; // %s::%s\n",
                       (unsigned long long)f.offset, f.className.c_str(), fn.c_str());
            }
            if ((fn.find("team") != std::string::npos || fn.find("Team") != std::string::npos) && !playerTeam) {
                playerTeam = f.offset;
                fprintf(fp, "constexpr size_t PLAYER_TEAM = 0x%llX; // %s::%s\n",
                       (unsigned long long)f.offset, f.className.c_str(), fn.c_str());
            }
            if ((fn.find("alive") != std::string::npos || fn.find("Alive") != std::string::npos ||
                 fn.find("dead") != std::string::npos || fn.find("Dead") != std::string::npos) && !playerAlive) {
                playerAlive = f.offset;
                fprintf(fp, "constexpr size_t PLAYER_IS_ALIVE = 0x%llX; // %s::%s\n",
                       (unsigned long long)f.offset, f.className.c_str(), fn.c_str());
            }
            if ((fn.find("bot") != std::string::npos || fn.find("Bot") != std::string::npos ||
                 fn.find("ai") != std::string::npos || fn.find("AI") != std::string::npos) && !playerBot) {
                playerBot = f.offset;
                fprintf(fp, "constexpr size_t PLAYER_IS_BOT = 0x%llX; // %s::%s\n",
                       (unsigned long long)f.offset, f.className.c_str(), fn.c_str());
            }
        }
        
        if (!playerPos) fprintf(fp, "constexpr size_t PLAYER_POSITION = 0x0;\n");
        if (!playerHp) fprintf(fp, "constexpr size_t PLAYER_HEALTH = 0x0;\n");
        if (!playerMaxHp) fprintf(fp, "constexpr size_t PLAYER_MAX_HEALTH = 0x0;\n");
        if (!playerTeam) fprintf(fp, "constexpr size_t PLAYER_TEAM = 0x0;\n");
        if (!playerAlive) fprintf(fp, "constexpr size_t PLAYER_IS_ALIVE = 0x0;\n");
        if (!playerBot) fprintf(fp, "constexpr size_t PLAYER_IS_BOT = 0x0;\n");
        
        fprintf(fp, "\n");
        fprintf(fp, "} // namespace Offsets\n");
        fprintf(fp, "} // namespace GhostSystems\n");
        
        fclose(fp);
    }
};

} // namespace GhostSystems