#pragma once

#include "Entity.h"
// Note: GameState needs void* localPlayerObj = nullptr;
#include "Il2CppHelper.h"
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include <random>

namespace GhostSystems {

    class MemoryScanner {
    public:
        MemoryScanner(GameState& state, FeatureConfig& config) : sharedState(state), featureConfig(config), isRunning(false) {}

        ~MemoryScanner() {
            stop();
        }

        void start() {
            if (isRunning) return;
            if (!Il2Cpp::domain_get) {
                if (!Il2Cpp::Initialize()) {
                    return;
                }
            }

            isRunning = true;

            scanThread = std::thread(&MemoryScanner::scanLoop, this);
            scanThread.detach();
        }

        void stop() {
            isRunning = false;
            if (logger.is_open()) logger.close();
        }

    private:
        GameState& sharedState;
        FeatureConfig& featureConfig;
        bool isRunning;
        std::thread scanThread;
        std::ofstream logger;

        Alignment calculateAlignment(PlayerEntity& entity, int localTeamId, void* localObj) {
            if (!featureConfig.teamCheckEnabled) {
                return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
            }

            switch (featureConfig.teamCheckMethod) {
                case 0: {
                    if (logger.is_open()) logger << "[TEAM] Method 0: GameData.GetMyTeamID" << std::endl;
                    void* teamClass = Il2Cpp::GetClass("Assembly-CSharp.dll", "COW", "GameData");
                    if (!teamClass) {
                        if (logger.is_open()) logger << "[TEAM] GameData class not found, fallback to teamId" << std::endl;
                        return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                    }
                    void* getMyTeamMethod = Il2Cpp::class_get_method_from_name(teamClass, "GetMyTeamID", 0);
                    if (!getMyTeamMethod) {
                        if (logger.is_open()) logger << "[TEAM] GetMyTeamID method not found, fallback to teamId" << std::endl;
                        return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                    }
                    void* result = Il2Cpp::runtime_invoke(getMyTeamMethod, nullptr, nullptr, nullptr);
                    if (result) {
                        int myTeamId = *(int32_t*)((uintptr_t)result + 0x10);
                        if (logger.is_open()) logger << "[TEAM] GetMyTeamID returned: " << myTeamId << std::endl;
                        return entity.teamId == myTeamId ? Alignment::ALLY : Alignment::ENEMY;
                    }
                    if (logger.is_open()) logger << "[TEAM] GetMyTeamID result null, fallback to teamId" << std::endl;
                    return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                }
                case 1: {
                    if (logger.is_open()) logger << "[TEAM] Method 1: PlayerCache.GetTeamByObject" << std::endl;
                    if (entity.teamId == localTeamId) return Alignment::ALLY;
                    void* playerCacheClass = Il2Cpp::GetClass("Assembly-CSharp.dll", "COW", "PlayerCache");
                    if (!playerCacheClass) {
                        if (logger.is_open()) logger << "[TEAM] PlayerCache class not found, fallback to teamId" << std::endl;
                        return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                    }
                    void* getTeamByObjMethod = Il2Cpp::class_get_method_from_name(playerCacheClass, "GetTeamByObject", 1);
                    if (!getTeamByObjMethod) {
                        if (logger.is_open()) logger << "[TEAM] GetTeamByObject method not found, fallback to teamId" << std::endl;
                        return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                    }
                    void* args[1] = { entity.obj };
                    void* result = Il2Cpp::runtime_invoke(getTeamByObjMethod, nullptr, args, nullptr);
                    if (result) {
                        int teamId = *(int32_t*)((uintptr_t)result + 0x10);
                        if (logger.is_open()) logger << "[TEAM] GetTeamByObject returned: " << teamId << std::endl;
                        return teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                    }
                    if (logger.is_open()) logger << "[TEAM] GetTeamByObject result null" << std::endl;
                    return Alignment::ENEMY;
                }
                case 2: {
                    if (logger.is_open()) logger << "[TEAM] Method 2: TeamService.IsAlly" << std::endl;
                    if (entity.teamId == localTeamId) return Alignment::ALLY;
                    void* teamServiceClass = Il2Cpp::GetClass("Assembly-CSharp.dll", "COW", "TeamService");
                    if (!teamServiceClass) {
                        if (logger.is_open()) logger << "[TEAM] TeamService class not found, fallback to teamId" << std::endl;
                        return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                    }
                    void* isAllyMethod = Il2Cpp::class_get_method_from_name(teamServiceClass, "IsAlly", 1);
                    if (!isAllyMethod) {
                        if (logger.is_open()) logger << "[TEAM] IsAlly method not found, fallback to teamId" << std::endl;
                        return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                    }
                    void* args[1] = { entity.obj };
                    void* result = Il2Cpp::runtime_invoke(isAllyMethod, nullptr, args, nullptr);
                    if (result) {
                        bool isAlly = *(bool*)((uintptr_t)result + 0x10);
                        if (logger.is_open()) logger << "[TEAM] IsAlly returned: " << (isAlly ? "true" : "false") << std::endl;
                        return isAlly ? Alignment::ALLY : Alignment::ENEMY;
                    }
                    if (logger.is_open()) logger << "[TEAM] IsAlly result null, fallback to teamId" << std::endl;
                    return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                }
                case 3: {
                    if (logger.is_open()) logger << "[TEAM] Method 3: Fallback teamId compare (local=" << localTeamId << ")" << std::endl;
                    return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                }
                case 4: {
                    if (logger.is_open()) logger << "[TEAM] Method 4: Always NEUTRAL" << std::endl;
                    return Alignment::NEUTRAL;
                }
                case 5: {
                    if (logger.is_open()) logger << "[TEAM] Method 5: Alignment.GetAlignment" << std::endl;
                    if (entity.teamId == localTeamId) return Alignment::ALLY;
                    void* alignmentClass = Il2Cpp::GetClass("Assembly-CSharp.dll", "COW", "Alignment");
                    if (!alignmentClass) {
                        if (logger.is_open()) logger << "[TEAM] Alignment class not found, fallback to teamId" << std::endl;
                        return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                    }
                    void* getAlignmentMethod = Il2Cpp::class_get_method_from_name(alignmentClass, "GetAlignment", 1);
                    if (!getAlignmentMethod) {
                        if (logger.is_open()) logger << "[TEAM] GetAlignment method not found, fallback to teamId" << std::endl;
                        return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                    }
                    void* args[1] = { entity.obj };
                    void* result = Il2Cpp::runtime_invoke(getAlignmentMethod, nullptr, args, nullptr);
                    if (result) {
                        int alignVal = *(int32_t*)((uintptr_t)result + 0x10);
                        if (logger.is_open()) logger << "[TEAM] GetAlignment returned: " << alignVal << std::endl;
                        return alignVal == 0 ? Alignment::ALLY : Alignment::ENEMY;
                    }
                    if (logger.is_open()) logger << "[TEAM] GetAlignment result null, fallback to teamId" << std::endl;
                    return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                }
                case 6: {
                    if (logger.is_open()) logger << "[TEAM] Method 6: IsSameTeamWithPlayerID (AttackableEntity)" << std::endl;
                    if (!localObj || !entity.obj) {
                        if (logger.is_open()) logger << "[TEAM] localObj or entity.obj null, fallback to teamId" << std::endl;
                        return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                    }
                    void* attackableClass = Il2Cpp::GetClass("Assembly-CSharp.dll", "COW.GamePlay", "AttackableEntity");
                    if (!attackableClass) {
                        if (logger.is_open()) logger << "[TEAM] AttackableEntity class not found" << std::endl;
                        return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                    }
                    void* isSameTeamMethod = Il2Cpp::class_get_method_from_name(attackableClass, "IsSameTeamWithPlayerID", 1);
                    if (!isSameTeamMethod) {
                        if (logger.is_open()) logger << "[TEAM] IsSameTeamWithPlayerID method not found, fallback to teamId" << std::endl;
                        return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                    }
                    uint64_t localPlayerId = *(uint64_t*)((uintptr_t)localObj + 0x3B0);
                    uint64_t entityPlayerId = *(uint64_t*)((uintptr_t)entity.obj + 0x3B0);
                    if (localPlayerId == 0 || entityPlayerId == 0) {
                        if (logger.is_open()) logger << "[TEAM] Player ID is null, fallback to teamId" << std::endl;
                        return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                    }
                    struct IHAAMHPPLMG {
                        uint32_t NBPDJAAAFBH;
                        uint32_t JEDDPHIHGKL;
                        uint8_t IOICFFEKAIL;
                    };
                    IHAAMHPPLMG playerIdStruct = {0};
                    uint32_t lowPart = (uint32_t)(entityPlayerId & 0xFFFFFFFF);
                    uint32_t highPart = (uint32_t)((entityPlayerId >> 32) & 0xFFFFFFFF);
                    playerIdStruct.NBPDJAAAFBH = lowPart;
                    playerIdStruct.JEDDPHIHGKL = highPart;
                    playerIdStruct.IOICFFEKAIL = 0;
                    void* args[1] = { &playerIdStruct };
                    void* result = Il2Cpp::runtime_invoke(isSameTeamMethod, localObj, args, nullptr);
                    if (result) {
                        bool isSameTeam = *(bool*)((uintptr_t)result + 0x10);
                        if (logger.is_open()) logger << "[TEAM] IsSameTeamWithPlayerID(playerId=" << entityPlayerId << ") returned: " << (isSameTeam ? "ally" : "enemy") << std::endl;
                        return isSameTeam ? Alignment::ALLY : Alignment::ENEMY;
                    }
                    if (logger.is_open()) logger << "[TEAM] IsSameTeamWithPlayerID result null, fallback to teamId" << std::endl;
                    return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                }
                case 7: {
                    if (logger.is_open()) logger << "[TEAM] Method 7: Fallback teamId compare (local=" << localTeamId << ")" << std::endl;
                    return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                }
                default: {
                    if (logger.is_open()) logger << "[TEAM] Method default: Fallback teamId compare (local=" << localTeamId << ")" << std::endl;
                    return entity.teamId == localTeamId ? Alignment::ALLY : Alignment::ENEMY;
                }
            }
        }

        bool checkWall(PlayerEntity& target, Vector3& localPos) {
            if (logger.is_open()) logger << "[WALL] === checkWall called ===" << std::endl;
            if (logger.is_open()) logger << "[WALL] localPos=(" << localPos.x << "," << localPos.y << "," << localPos.z << ")" << std::endl;
            if (logger.is_open()) logger << "[WALL] targetPos=(" << target.position.x << "," << target.position.y << "," << target.position.z << ")" << std::endl;

            if (!featureConfig.wallCheckEnabled) {
                if (logger.is_open()) logger << "[WALL] wallCheckEnabled=false, skipping" << std::endl;
                return false;
            }

            float eyeHeightOffset = 1.6f;
            Vector3 eyePos = localPos;
            eyePos.y += eyeHeightOffset;

            Vector3 targetEyePos = target.position;
            targetEyePos.y += eyeHeightOffset;

            Vector3 dir = targetEyePos;
            dir.x -= eyePos.x;
            dir.y -= eyePos.y;
            dir.z -= eyePos.z;
            float dist = sqrtf(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
            if (dist > 0.001f) {
                dir.x /= dist; dir.y /= dist; dir.z /= dist;
            }
            if (logger.is_open()) logger << "[WALL] distance=" << dist << " dir=(" << dir.x << "," << dir.y << "," << dir.z << ")" << std::endl;

            if (dist < 0.5f) {
                if (logger.is_open()) logger << "[WALL] Target very close, assuming clear" << std::endl;
                return false;
            }

            bool result = false;

            if (featureConfig.wallCheckMethod == 0 || featureConfig.wallCheckMethod == 1) {
                void* physicsClass = Il2Cpp::GetClass("UnityEngine.PhysicsModule.dll", "UnityEngine", "Physics");
                if (!physicsClass) {
                    if (logger.is_open()) logger << "[WALL] FAIL: Physics class not found!" << std::endl;
                    return false;
                }
                if (logger.is_open()) logger << "[WALL] Physics class found, trying Raycast(3)" << std::endl;

                void* raycastMethod = Il2Cpp::class_get_method_from_name(physicsClass, "Raycast", 5);
                if (!raycastMethod) {
                    raycastMethod = Il2Cpp::class_get_method_from_name(physicsClass, "Raycast", 4);
                }
                if (!raycastMethod) {
                    raycastMethod = Il2Cpp::class_get_method_from_name(physicsClass, "Raycast", 3);
                }

                if (raycastMethod) {
                    if (logger.is_open()) logger << "[WALL] Raycast found, invoking with eye positions..." << std::endl;

                    float maxDist = dist;
                    Vector3 origin = eyePos;
                    Vector3 direction = dir;
                    int layerMask = 0xFFFFFFFF;

                    void* args[6] = { nullptr, &origin, &direction, nullptr, &maxDist, &layerMask };

                    void* ret = Il2Cpp::runtime_invoke(raycastMethod, nullptr, args, nullptr);
                    if (ret) {
                        bool hit = *(bool*)ret;
                        if (logger.is_open()) logger << "[WALL] Raycast returned hit=" << (hit ? "true" : "false") << std::endl;
                        result = hit;
                    } else {
                        if (logger.is_open()) logger << "[WALL] Raycast result=null" << std::endl;
                    }
                } else {
                    if (logger.is_open()) logger << "[WALL] Raycast not found, trying Linecast" << std::endl;
                    void* linecastMethod = Il2Cpp::class_get_method_from_name(physicsClass, "Linecast", 3);
                    if (!linecastMethod) {
                        linecastMethod = Il2Cpp::class_get_method_from_name(physicsClass, "Linecast", 4);
                    }
                    if (!linecastMethod) {
                        linecastMethod = Il2Cpp::class_get_method_from_name(physicsClass, "Linecast", 5);
                    }

                    if (linecastMethod) {
                        if (logger.is_open()) logger << "[WALL] Linecast found, invoking..." << std::endl;
                        Vector3 start = eyePos;
                        Vector3 end = targetEyePos;
                        void* invokeArgs[4] = { nullptr, &start, &end, nullptr };
                        void* ret = Il2Cpp::runtime_invoke(linecastMethod, nullptr, invokeArgs, nullptr);
                        if (ret) {
                            bool hit = *(bool*)ret;
                            if (logger.is_open()) logger << "[WALL] Linecast hit=" << (hit ? "true" : "false") << std::endl;
                            result = hit;
                        }
                    } else {
                        if (logger.is_open()) logger << "[WALL] Linecast not found either" << std::endl;
                    }
                }
            } else if (featureConfig.wallCheckMethod == 2) {
                if (logger.is_open()) logger << "[WALL] Method 2: Distance-based check" << std::endl;
                if (dist < 3.0f) {
                    if (logger.is_open()) logger << "[WALL] Target very close (" << dist << "m), assuming no wall" << std::endl;
                    result = false;
                } else {
                    result = false;
                }
            } else {
                if (logger.is_open()) logger << "[WALL] Method 3: Disabled" << std::endl;
                result = false;
            }

            if (logger.is_open()) logger << "[WALL] Final result: " << (result ? "BLOCKED" : "CLEAR") << std::endl;
            return result;
        }

        void scanLoop() {
            if (Il2Cpp::thread_attach && Il2Cpp::domain_get) {
                Il2Cpp::thread_attach(Il2Cpp::domain_get());
            }

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distrib(30, 80);

            void* gameFacadeClass = Il2Cpp::GetClass("Assembly-CSharp.dll", "COW", "GameFacade");
            if (!gameFacadeClass) {
                if (logger.is_open()) logger << "[ERRO] GameFacade não encontrado." << std::endl;
                return;
            }
            void* currentMatchGameField = Il2Cpp::class_get_field_from_name(gameFacadeClass, "CurrentMatchGame");
            
            // Cache de BaseGame para fallback
            void* baseGameClass = Il2Cpp::GetClass("Assembly-CSharp.dll", "GCommon", "BaseGame");
            void* sAllEntitiesField = baseGameClass ? Il2Cpp::class_get_field_from_name(baseGameClass, "sAllEntities") : nullptr;

            while (isRunning) {
                std::vector<PlayerEntity> updatedEntities;
                Vector3 currentLocalPos = {0, 0, 0};
                int currentLocalTeamId = -1;
                bool foundLocal = false;
                void* currentLocalObj = nullptr;
                
                void* matchGame = nullptr;
                if (currentMatchGameField) {
                    Il2Cpp::field_static_get_value(currentMatchGameField, &matchGame);
                }

                void* dictionaryObj = nullptr;

                if (!matchGame) {
                    if (logger.is_open()) logger << "[SCAN] matchGame is null, trying BaseGame.sAllEntities..." << std::endl;
                    if (sAllEntitiesField) {
                        Il2Cpp::field_static_get_value(sAllEntitiesField, &dictionaryObj);
                    }
                } else {
                    if (logger.is_open()) logger << "[SCAN] matchGame found." << std::endl;
                    // Pegar dicionário de entidades (m_ReplicationEntitis)
                    void* matchGameKlass = Il2Cpp::object_get_class(matchGame);
                    void* repEntField = Il2Cpp::class_get_field_from_name(matchGameKlass, "m_ReplicationEntitis");
                    
                    if (!repEntField) {
                        if (logger.is_open()) logger << "[SCAN] m_ReplicationEntitis field not found." << std::endl;
                    } else {
                        Il2Cpp::field_get_value(matchGame, repEntField, &dictionaryObj);
                    }
                }

                if (!dictionaryObj) {
                } else {
                        void* entriesArray = nullptr;
                        int count = 0;
                        if (Il2Cpp::GetDictionaryValues(dictionaryObj, &entriesArray, &count)) {
                            if (entriesArray && count > 0) {
                            if (Il2Cpp::array_length) {
                                int arrayLen = (int)Il2Cpp::array_length(entriesArray);
                                if (arrayLen > 0 && count > arrayLen) count = arrayLen;
                            }
                            
                            // Em C#, array data comeca no offset 0x20.
                            // O Entry struct no Dictionary<uint, ReplicationEntity> tem:
                            // int hashCode (4), int next (4), uint key (4), padding (4), ReplicationEntity value (8)
                            // Total size: 24 bytes (0x18) na arquitetura 64-bits
                            
                            size_t elementSize = (sizeof(void*) == 8 ? 0x18 : 0x10); 
                            if (Il2Cpp::class_get_array_element_size) {
                                void* arrayKlass = Il2Cpp::object_get_class(entriesArray);
                                elementSize = Il2Cpp::class_get_array_element_size(arrayKlass);
                            }

                            uintptr_t dataStart = (uintptr_t)entriesArray + (sizeof(void*) == 8 ? 0x20 : 0x10);
                            
                            for (int i = 0; i < count; i++) {
                                uintptr_t entryAddr = dataStart + (i * elementSize);
                                
                                int hashCode = *(int*)entryAddr;
                                if (hashCode < 0) continue; // Entry empty or deleted

                                // Lê o valor (ponteiro para ReplicationEntity) do Entry struct.
                                size_t valueOffset = (sizeof(void*) == 8 ? 0x10 : 0xC);
                                void* entityObj = *(void**)(entryAddr + valueOffset);

                                if (entityObj) {
                                    // Obter a classe da entidade para checar se eh Player
                                    void* entityKlass = Il2Cpp::object_get_class(entityObj);
                                    if (!entityKlass) continue;
                                    
                                    // const char* className = Il2Cpp::class_get_name(entityKlass);

                                    if (Il2Cpp::IsSubclassOf(entityKlass, "Player")) {
                                        PlayerEntity p;
                                        p.baseAddress = (uintptr_t)entityObj;
                                        
                                        snprintf(p.name, sizeof(p.name), "Player_%d", i);
                                        p.health = 100.0f;
                                        p.maxHealth = 100.0f;
                                        p.position = { 0, 0, 0 };
                                        p.isBot = false;
                                        p.isKnocked = false;
                                        p.teamId = 1;
                                        p.weaponId = 0;
                                        memset(p.weaponName, 0, sizeof(p.weaponName));
                                        p.distanceToLocal = 100.0f;
                                        p.alignment = Alignment::ENEMY;

                                        static void* cachedMethodsKlass = nullptr;
                                        static void* getNickNameMethod = nullptr;
                                        static void* isAIMethod = nullptr;
                                        static void* getPosMethod = nullptr;
                                        static void* getTeamMethod = nullptr;
                                        static void* getIsDeadMethod = nullptr;
                                        static void* isLocalMethod = nullptr;

                                        if (cachedMethodsKlass != entityKlass) {
                                            getNickNameMethod = Il2Cpp::GetMethodRecursively(entityKlass, "GetNickName", 0);
                                            if (!getNickNameMethod) getNickNameMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_NickName", 0);
                                            if (!getNickNameMethod) getNickNameMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_Nickname", 0);
                                            if (!getNickNameMethod) getNickNameMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_Name", 0);

                                            isAIMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_IsAI", 0);
                                            if (!isAIMethod) isAIMethod = Il2Cpp::GetMethodRecursively(entityKlass, "IsAI", 0);
                                            if (!isAIMethod) isAIMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_isBot", 0);

                                            getPosMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_Position", 0);
                                            getTeamMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_TeamID", 0);
                                            if (!getTeamMethod) getTeamMethod = Il2Cpp::GetMethodRecursively(entityKlass, "GetTeamID", 0);
                                            if (!getTeamMethod) getTeamMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_Team", 0);
                                            if (!getTeamMethod) getTeamMethod = Il2Cpp::GetMethodRecursively(entityKlass, "TeamID", 0);
                                            getIsDeadMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_IsDead", 0);

                                            isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "IsLocalPlayer", 0);
                                            if (!isLocalMethod) isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "IsLocalEntity", 0);
                                            if (!isLocalMethod) isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_IsLocal", 0);
                                            if (!isLocalMethod) isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_IsLocalPlayer", 0);
                                            if (!isLocalMethod) isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "IsLocalAvatar", 0);
                                            cachedMethodsKlass = entityKlass;
                                        }

                                        if (getNickNameMethod) {
                                            void* nameObj = Il2Cpp::runtime_invoke(getNickNameMethod, entityObj, nullptr, nullptr);
                                            if (nameObj) {
                                                // Il2Cpp String: 0x10 = length (int32), 0x14 = chars (utf16)
                                                int32_t length = *(int32_t*)((uintptr_t)nameObj + 0x10);
                                                if (length > 0 && length < 100) {
                                                    uint16_t* chars = (uint16_t*)((uintptr_t)nameObj + 0x14);
                                                    char utf8Name[256] = {0};
                                                    int outIdx = 0;
                                                    for (int i = 0; i < length && outIdx < 254; ++i) {
                                                        uint16_t c = chars[i];
                                                        if (c < 0x80) {
                                                            utf8Name[outIdx++] = (char)c;
                                                        } else {
                                                            // Conversao simples (substitui chars especiais por '?')
                                                            utf8Name[outIdx++] = '?';
                                                        }
                                                    }
                                                    utf8Name[outIdx] = '\0';
                                                    strncpy(p.name, utf8Name, sizeof(p.name) - 1);
                                                } else {
                                                    strcpy(p.name, "Player");
                                                }
                                            } else {
                                                strcpy(p.name, "Player");
                                            }
                                        } else {
                                            strcpy(p.name, "Player");
                                        }

                                        // IsAI (Bot) ou fallbacks
                                        if (isAIMethod) {
                                            void* aiObj = Il2Cpp::runtime_invoke(isAIMethod, entityObj, nullptr, nullptr);
                                            if (aiObj) p.isBot = *(bool*)((uintptr_t)aiObj + 0x10);
                                        } else {
                                             // Fallback offset - isAI na classe Player ou similar (ex: 0x40 em EPPlayerInfo ou isBot 0x38/0x20)
                                             bool possibleBot = *(bool*)((uintptr_t)entityObj + 0x38); // Tenta IsBot; // 0x38
                                             p.isBot = possibleBot;
                                             // Outro offset comum: isBot; // 0x20 
                                        }

                                        // Cache de classes e métodos para evitar lag massivo que trava o touch (runtime_invoke e GetMethodRecursively sao pesados num loop)
                                        // TODO: O ideal seria inicializar num setup inicial
                                        
                                        // HP e MaxHP (Tenta getter direto, se falhar, tenta via PlayerAttributes)
                                        bool hpFound = false;
                                        static void* cachedHpKlass = nullptr;
                                        static void* getCurHpMethod = nullptr;
                                        static void* getMaxHpMethod = nullptr;

                                        if (cachedHpKlass != entityKlass) {
                                            getCurHpMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_CurHP", 0);
                                            if (!getCurHpMethod) getCurHpMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_HP", 0);
                                            getMaxHpMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_MaxHP", 0);
                                            cachedHpKlass = entityKlass;
                                        }

                                        if (getCurHpMethod) {
                                            void* hpObj = Il2Cpp::runtime_invoke(getCurHpMethod, entityObj, nullptr, nullptr);
                                            if (hpObj) {
                                                p.health = (float)(*(int32_t*)((uintptr_t)hpObj + 0x10));
                                                if (p.health > 1.0f) {
                                                    hpFound = true;
                                                }
                                            }
                                        }

                                        if (getMaxHpMethod) {
                                            void* maxHpObj = Il2Cpp::runtime_invoke(getMaxHpMethod, entityObj, nullptr, nullptr);
                                            if (maxHpObj) p.maxHealth = (float)(*(int32_t*)((uintptr_t)maxHpObj + 0x10));
                                        }

                                        if (!hpFound) {
                                            // Fallback para PlayerAttributes
                                            // Em dump.cs, `protected PlayerAttributes JKPFFNEMJIF; // 0x688`
                                            // Como os offsets podem mudar, procuramos o campo pelo tipo ou assumimos 0x688 por enquanto.
                                            void* playerAttrField = Il2Cpp::class_get_field_from_name(entityKlass, "JKPFFNEMJIF");
                                            if (playerAttrField) {
                                                void* playerAttrObj = nullptr;
                                                Il2Cpp::field_get_value(entityObj, playerAttrField, &playerAttrObj);
                                                if (playerAttrObj) {
                                                    void* attrKlass = Il2Cpp::object_get_class(playerAttrObj);
                                                    if (attrKlass) {
                                                        void* attrHpMethod = Il2Cpp::GetMethodRecursively(attrKlass, "get_HP", 0);
                                                        if (attrHpMethod) {
                                                            void* attrHpObj = Il2Cpp::runtime_invoke(attrHpMethod, playerAttrObj, nullptr, nullptr);
                                                            if (attrHpObj) {
                                                                p.health = (float)(*(int32_t*)((uintptr_t)attrHpObj + 0x10));
                                                                if (p.health > 1.0f) {
                                                                    hpFound = true;
                                                                }
                                                            }
                                                        } else {
                                                            // Se nao tem get_HP(), tentar pegar o campo HP direto do PlayerAttributes
                                                            // A classe tem campos como: private Int32 LGEAIKPHPED; // 0x9c (provavel HP max/cur)
                                                            // private Int32 CHPNMIKDJNB; // 0xa0
                                                            int32_t possibleHp = *(int32_t*)((uintptr_t)playerAttrObj + 0x9c);
                                                            int32_t possibleHp2 = *(int32_t*)((uintptr_t)playerAttrObj + 0xa0); // Try the other one too

                                                            if (possibleHp > 1 && possibleHp <= 300) {
                                                                p.health = (float)possibleHp;
                                                                hpFound = true;
                                                            } else if (possibleHp2 > 1 && possibleHp2 <= 300) {
                                                                p.health = (float)possibleHp2;
                                                                hpFound = true;
                                                            }
                                                        }
                                                        void* attrMaxHpMethod = Il2Cpp::GetMethodRecursively(attrKlass, "get_MaxHP", 0);
                                                        if (attrMaxHpMethod) {
                                                            void* attrMaxHpObj = Il2Cpp::runtime_invoke(attrMaxHpMethod, playerAttrObj, nullptr, nullptr);
                                                            if (attrMaxHpObj) p.maxHealth = (float)(*(int32_t*)((uintptr_t)attrMaxHpObj + 0x10));
                                                        }
                                                    }
                                                }
                                            } 
                                            
                                            if (!hpFound) {
                                                // Fallback hardcoded para offset 0x688 caso o campo nao seja JKPFFNEMJIF
                                                void* playerAttrObj = *(void**)((uintptr_t)entityObj + 0x688);
                                                if (playerAttrObj) {
                                                    void* attrKlass = Il2Cpp::object_get_class(playerAttrObj);
                                                    if (attrKlass) {
                                                        void* attrHpMethod = Il2Cpp::GetMethodRecursively(attrKlass, "get_HP", 0);
                                                        if (attrHpMethod) {
                                                            void* attrHpObj = Il2Cpp::runtime_invoke(attrHpMethod, playerAttrObj, nullptr, nullptr);
                                                            if (attrHpObj) {
                                                                p.health = (float)(*(int32_t*)((uintptr_t)attrHpObj + 0x10));
                                                                if (p.health > 1.0f) {
                                                                    hpFound = true;
                                                                }
                                                            }
                                                        } else {
                                                            int32_t possibleHp = *(int32_t*)((uintptr_t)playerAttrObj + 0x9c);
                                                            int32_t possibleHp2 = *(int32_t*)((uintptr_t)playerAttrObj + 0xa0);
                                                            
                                                            if (possibleHp > 1 && possibleHp <= 300) {
                                                                p.health = (float)possibleHp;
                                                                hpFound = true;
                                                            } else if (possibleHp2 > 1 && possibleHp2 <= 300) {
                                                                p.health = (float)possibleHp2;
                                                                hpFound = true;
                                                            }
                                                        }
                                                        void* attrMaxHpMethod = Il2Cpp::GetMethodRecursively(attrKlass, "get_MaxHP", 0);
                                                        if (attrMaxHpMethod) {
                                                            void* attrMaxHpObj = Il2Cpp::runtime_invoke(attrMaxHpMethod, playerAttrObj, nullptr, nullptr);
                                                            if (attrMaxHpObj) p.maxHealth = (float)(*(int32_t*)((uintptr_t)attrMaxHpObj + 0x10));
                                                        }
                                                    }
                                                }
                                            }
                                            
                                            // Tentar tambem ler HP de DDMILIACFIC m_PlayerHPChangeEvt ou HPDKELNKDGG (0x424) no proprio Player
                                            if (!hpFound) {
                                                uint32_t hpFallback = *(uint32_t*)((uintptr_t)entityObj + 0x424);
                                                if (hpFallback > 1 && hpFallback <= 300) {
                                                    p.health = (float)hpFallback;
                                                    hpFound = true;
                                                }
                                            }
                                        }
                                        
                                        // Fallback get_HP() 
                                        if (!hpFound) {
                                            void* getHpMethodFallback = Il2Cpp::GetMethodRecursively(entityKlass, "get_HP", 0);
                                            if (getHpMethodFallback) {
                                                void* hpObjFallback = Il2Cpp::runtime_invoke(getHpMethodFallback, entityObj, nullptr, nullptr);
                                                if (hpObjFallback) {
                                                    int32_t hpVal = *(int32_t*)((uintptr_t)hpObjFallback + 0x10);
                                                    if (hpVal > 1 && hpVal <= 300) {
                                                        p.health = (float)hpVal;
                                                        hpFound = true;
                                                    }
                                                }
                                            }
                                        }

                                        // Ler Arma Atual (GetWeaponOnHand / ActiveUISightingWeapon)
                                        static void* getWeaponOnHandMethod = nullptr;
                                        static void* (*getWeaponOnHand_fn)(void*, void*) = nullptr;
                                        static bool triedWeaponMethod = false;
                                        if (!triedWeaponMethod && !getWeaponOnHandMethod) {
                                            void* playerClass = Il2Cpp::GetClass("Assembly-CSharp.dll", "COW.GamePlay", "Player");
                                            if (!playerClass) playerClass = Il2Cpp::GetClass("Assembly-CSharp", "COW.GamePlay", "Player");
                                            if (playerClass) {
                                                getWeaponOnHandMethod = Il2Cpp::class_get_method_from_name(playerClass, "GetWeaponOnHand", 0);
                                                if (getWeaponOnHandMethod) {
                                                    // Em il2cpp, MethodInfo struct tem methodPointer no offset 0x0
                                                    getWeaponOnHand_fn = (void* (*)(void*, void*))(*(void**)getWeaponOnHandMethod);
                                                }
                                            }
                                            triedWeaponMethod = true;
                                        }

                                        void* weaponObj = nullptr;
                                        if (getWeaponOnHand_fn) {
                                            weaponObj = getWeaponOnHand_fn(entityObj, getWeaponOnHandMethod);
                                        } else if (getWeaponOnHandMethod) {
                                            void* excWep = nullptr;
                                            weaponObj = Il2Cpp::runtime_invoke(getWeaponOnHandMethod, entityObj, nullptr, &excWep);
                                        }
                                        
                                        if (!weaponObj) {
                                            // Fallback: Offset 0x5a0 para Player.ActiveUISightingWeapon (dump.cs)
                                            weaponObj = *(void**)((uintptr_t)entityObj + 0x5a0);
                                        }

                                        if (weaponObj) {
                                            // Em ARM64, os dois primeiros ponteiros de um objeto gerenciado sao Klass (0x0) e Monitor (0x8)
                                            // Em KOGBJLFDJHC (classe base da arma):
                                            // public UInt32 JDPDFBINIJE; // 0x10 (WeaponID)
                                            // protected String HCIHACHPOLO; // 0x18 (WeaponName)
                                            
                                            uint32_t weaponId = *(uint32_t*)((uintptr_t)weaponObj + 0x10);
                                            if (weaponId > 0 && weaponId < 1000000) {
                                                p.weaponId = weaponId;
                                            }

                                            void* weaponNameObj = *(void**)((uintptr_t)weaponObj + 0x18);
                                            if (weaponNameObj) {
                                                int32_t length = *(int32_t*)((uintptr_t)weaponNameObj + 0x10);
                                                if (length > 0 && length < 64) {
                                                    uint16_t* chars = (uint16_t*)((uintptr_t)weaponNameObj + 0x14);
                                                    char utf8Name[64] = {0};
                                                    int outIdx = 0;
                                                    for (int j = 0; j < length && outIdx < 62; ++j) {
                                                        uint16_t c = chars[j];
                                                        if (c < 0x80) utf8Name[outIdx++] = (char)c;
                                                        else utf8Name[outIdx++] = '?';
                                                    }
                                                    utf8Name[outIdx] = '\0';
                                                    strncpy(p.weaponName, utf8Name, sizeof(p.weaponName) - 1);
                                                }
                                            }
                                        }

                                        // Obter a posição (usando o getter ou offsets direto se possivel para nao lagar)
                                        if (getPosMethod) {
                                            void* posObj = Il2Cpp::runtime_invoke(getPosMethod, entityObj, nullptr, nullptr);
                                            if (posObj) {
                                                // Retorno de Vector3 (struct). Como runtime_invoke retorna object boxado para structs:
                                                float* posData = (float*)((uintptr_t)posObj + 0x10);
                                                p.position = { posData[0], posData[1], posData[2] };
                                            }
                                        } else {
                                            // Fallback offset posição, se existir. Vamos deixar vazio por enquanto.
                                        }

                                        if (getTeamMethod) {
                                            void* teamObj = Il2Cpp::runtime_invoke(getTeamMethod, entityObj, nullptr, nullptr);
                                            if (teamObj) {
                                                p.teamId = *(int32_t*)((uintptr_t)teamObj + 0x10);
                                            } else {
                                                p.teamId = *(int32_t*)((uintptr_t)entityObj + 0x3D8);
                                            }
                                        } else {
                                            p.teamId = *(int32_t*)((uintptr_t)entityObj + 0x3D8);
                                        }

                                        // Salva o ponteiro do objeto
                                        p.obj = entityObj;

                                        if (getIsDeadMethod) {
                                            void* deadObj = Il2Cpp::runtime_invoke(getIsDeadMethod, entityObj, nullptr, nullptr);
                                            if (deadObj) p.isKnocked = *(bool*)((uintptr_t)deadObj + 0x10);
                                        }

                                        // Verificar IsLocalPlayer para ignorar ou destacar o jogador local
                                        bool isLocal = false;
                                        if (isLocalMethod) {
                                            void* isLocalObj = Il2Cpp::runtime_invoke(isLocalMethod, entityObj, nullptr, nullptr);
                                            if (isLocalObj) {
                                                isLocal = *(bool*)((uintptr_t)isLocalObj + 0x10);
                                            }
                                        }

                                        if (isLocal && !foundLocal) {
                                            foundLocal = true;
                                            currentLocalPos = p.position;
                                            currentLocalTeamId = p.teamId;
                                            currentLocalObj = entityObj;
                                            continue; // Ignora apenas o primeiro local detectado
                                        }

                                        // Se IsLocal estiver bugado e retornar true para vários, não esvazia a lista:
                                        // mantém os demais jogadores na Entity List.
                                        updatedEntities.push_back(p);
                                    }
                                }
                            }
                        }
                    }
                }
                
                if (foundLocal) {
                    {
                        std::lock_guard<std::mutex> lockConfig(featureConfig.mtx);
                        for (auto& p : updatedEntities) {
                            p.distanceToLocal = p.position.distance(currentLocalPos);
                            p.alignment = calculateAlignment(p, currentLocalTeamId, currentLocalObj);
                            if (featureConfig.wallCheckEnabled && p.alignment == Alignment::ENEMY && p.distanceToLocal <= 350.0f) {
                                  // Removido: O WallCheck pesado e inseguro (Raycast na thread secundária) foi transferido
                                  // para o OnMainThreadTick no Menu.cpp, o que previne o travamento (crash/freeze) do jogo.
                                  // A propriedade p.isVisible agora é atualizada na thread principal.
                              }
                        }
                    }
                }

                {
                      std::lock_guard<std::mutex> lock(sharedState.mtx);
                      
                      // Preservar o estado de visibilidade (isVisible) gerado pelo WallCheck no Main Thread.
                      // Como o MemoryScanner recria as entidades, isso evita que o estado seja resetado
                      // para 'true' a cada 100ms, causando piscadas (flickering) na tela e mira acidental.
                      for (auto& newEnt : updatedEntities) {
                          for (const auto& oldEnt : sharedState.entities) {
                              if (newEnt.obj == oldEnt.obj && newEnt.baseAddress == oldEnt.baseAddress) {
                                  newEnt.isVisible = oldEnt.isVisible;
                                  newEnt.isInFov = oldEnt.isInFov;
                                  newEnt.lastWallCheckMs = oldEnt.lastWallCheckMs;
                                  break;
                              }
                          }
                      }

                      sharedState.entities = std::move(updatedEntities);
                      if (foundLocal) {
                          sharedState.localPlayerPos = currentLocalPos;
                          sharedState.localPlayerTeamId = currentLocalTeamId;
                          sharedState.localPlayerObj = currentLocalObj;
                      }
                  }

                // Aumentamos o tempo de sleep para desafogar a thread do Android (isso previne o travamento do ImGui / Touch)
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Fixado em 100ms para scan loop não pesar mas ser mais fluido
            }
        }
    };

} // namespace GhostSystems
