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
        MemoryScanner(GameState& state) : sharedState(state), isRunning(false) {}

        ~MemoryScanner() {
            stop();
        }

        void start() {
            if (isRunning) return;
            isRunning = true;
            
            // Log offline para depuração
            logger.open("/sdcard/Download/GhostSystems_Scan.log", std::ios::app);
            if (logger.is_open()) {
                logger << "[INFO] Inicializando Scanner de Memoria na Thread..." << std::endl;
            }

            // Inicializar Il2Cpp
            if (!Il2Cpp::Initialize()) {
                if (logger.is_open()) logger << "[ERRO] Falha ao inicializar Il2Cpp." << std::endl;
                return;
            }

            scanThread = std::thread(&MemoryScanner::scanLoop, this);
            scanThread.detach();
        }

        void stop() {
            isRunning = false;
            if (logger.is_open()) {
                logger << "[INFO] Encerrando Scanner de Memoria." << std::endl;
                logger.close();
            }
        }

    private:
        GameState& sharedState;
        bool isRunning;
        std::thread scanThread;
        std::ofstream logger;

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
                    if (logger.is_open()) logger << "[SCAN] dictionaryObj is null." << std::endl;
                } else {
                        void* entriesArray = nullptr;
                        int count = 0;
                        if (Il2Cpp::GetDictionaryValues(dictionaryObj, &entriesArray, &count)) {
                            if (logger.is_open()) logger << "[SCAN] GetDictionaryValues success, count: " << count << std::endl;
                            if (entriesArray && count > 0) {
                            
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
                                    
                                    const char* className = Il2Cpp::class_get_name(entityKlass);
                                    
                                    // Log para debug
                                    if (logger.is_open()) {
                                        logger << "[DEBUG] Found Entity class: " << (className ? className : "null") << std::endl;
                                    }

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
                                        p.distanceToLocal = 100.0f;
                                        p.alignment = Alignment::ENEMY;

                                        static void* getNickNameMethod = nullptr;
                                        static void* isAIMethod = nullptr;
                                        static void* getPosMethod = nullptr;
                                        static void* getTeamMethod = nullptr;
                                        static void* teamField = nullptr;
                                        static void* getIsDeadMethod = nullptr;
                                        static void* isLocalMethod = nullptr;
                                        static void* isLocalField = nullptr;
                                        static void* getLocalPlayerMethod = nullptr;
                                        static bool methodsCached = false;

                                        static void* gameFacadeKlass = Il2Cpp::GetClass("Assembly-CSharp.dll", "COW", "GameFacade");
                                        static void* currentLocalPlayerMethod = nullptr;
                                        static void* isSameTeamMethod = nullptr;

                                        if (gameFacadeKlass && !currentLocalPlayerMethod) {
                                            currentLocalPlayerMethod = Il2Cpp::GetMethodRecursively(gameFacadeKlass, "CurrentLocalPlayer", 0);
                                            isSameTeamMethod = Il2Cpp::GetMethodRecursively(gameFacadeKlass, "IsSameTeam", 2);
                                        }

                                        uintptr_t localPlayerObj = 0;
                                        if (currentLocalPlayerMethod) {
                                            localPlayerObj = (uintptr_t)Il2Cpp::runtime_invoke(currentLocalPlayerMethod, nullptr, nullptr, nullptr);
                                        }

                                        if (!methodsCached) {
                                            getLocalPlayerMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_localPlayer", 0);
                                            if (!getLocalPlayerMethod) getLocalPlayerMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_LocalPlayer", 0);
                                            if (!getLocalPlayerMethod) getLocalPlayerMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_local", 0);

                                            getNickNameMethod = Il2Cpp::GetMethodRecursively(entityKlass, "GetNickName", 0);
                                            if (!getNickNameMethod) getNickNameMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_NickName", 0);
                                            if (!getNickNameMethod) getNickNameMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_Nickname", 0);
                                            if (!getNickNameMethod) getNickNameMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_Name", 0);

                                            isAIMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_IsAI", 0);
                                            if (!isAIMethod) isAIMethod = Il2Cpp::GetMethodRecursively(entityKlass, "IsAI", 0);
                                            if (!isAIMethod) isAIMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_isBot", 0);

                                            getPosMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_Position", 0);
                                            getTeamMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_Camp", 0);
                                            if (!getTeamMethod) getTeamMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_camp", 0);
                                            if (!getTeamMethod) getTeamMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_GroupId", 0);
                                            if (!getTeamMethod) getTeamMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_TeamID", 0);
                                            if (!getTeamMethod) getTeamMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_TeamId", 0);
                                            if (!getTeamMethod) getTeamMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_teamId", 0);
                                            if (!getTeamMethod) getTeamMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_Team", 0);

                                            if (!getTeamMethod) teamField = Il2Cpp::GetFieldRecursively(entityKlass, "camp");
                                            if (!getTeamMethod && !teamField) teamField = Il2Cpp::GetFieldRecursively(entityKlass, "Camp");
                                            if (!getTeamMethod && !teamField) teamField = Il2Cpp::GetFieldRecursively(entityKlass, "groupId");
                                            if (!getTeamMethod && !teamField) teamField = Il2Cpp::GetFieldRecursively(entityKlass, "GroupId");
                                            if (!getTeamMethod && !teamField) teamField = Il2Cpp::GetFieldRecursively(entityKlass, "teamId");
                                            if (!getTeamMethod && !teamField) teamField = Il2Cpp::GetFieldRecursively(entityKlass, "TeamId");
                                            if (!getTeamMethod && !teamField) teamField = Il2Cpp::GetFieldRecursively(entityKlass, "TeamID");
                                            if (!getTeamMethod && !teamField) teamField = Il2Cpp::GetFieldRecursively(entityKlass, "team");
                                            if (!getTeamMethod && !teamField) teamField = Il2Cpp::GetFieldRecursively(entityKlass, "Team");
                                            if (!getTeamMethod && !teamField) teamField = Il2Cpp::GetFieldRecursively(entityKlass, "m_TeamId");
                                            if (!getTeamMethod && !teamField) teamField = Il2Cpp::GetFieldRecursively(entityKlass, "m_GroupId");
                                            if (!getTeamMethod && !teamField) teamField = Il2Cpp::GetFieldRecursively(entityKlass, "m_Camp");
                                            if (!getTeamMethod && !teamField) teamField = Il2Cpp::GetFieldRecursively(entityKlass, "_teamId");
                                            if (!getTeamMethod && !teamField) teamField = Il2Cpp::GetFieldRecursively(entityKlass, "_camp");

                                            getIsDeadMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_IsDead", 0);

                                            isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_IsMainPlayer", 0);
                                            if (!isLocalMethod) isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "IsMainPlayer", 0);
                                            if (!isLocalMethod) isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_IsLocalPlayer", 0);
                                            if (!isLocalMethod) isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "IsLocalPlayer", 0);
                                            if (!isLocalMethod) isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_isLocalPlayer", 0);
                                            if (!isLocalMethod) isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "isLocalPlayer", 0);
                                            if (!isLocalMethod) isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_IsLocal", 0);
                                            if (!isLocalMethod) isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "IsLocal", 0);
                                            if (!isLocalMethod) isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_isLocal", 0);

                                            if (!isLocalMethod) isLocalField = Il2Cpp::GetFieldRecursively(entityKlass, "isMainPlayer");
                                            if (!isLocalMethod && !isLocalField) isLocalField = Il2Cpp::GetFieldRecursively(entityKlass, "IsMainPlayer");
                                            if (!isLocalMethod && !isLocalField) isLocalField = Il2Cpp::GetFieldRecursively(entityKlass, "isLocalPlayer");
                                            if (!isLocalMethod && !isLocalField) isLocalField = Il2Cpp::GetFieldRecursively(entityKlass, "IsLocalPlayer");
                                            if (!isLocalMethod && !isLocalField) isLocalField = Il2Cpp::GetFieldRecursively(entityKlass, "isLocal");
                                            if (!isLocalMethod && !isLocalField) isLocalField = Il2Cpp::GetFieldRecursively(entityKlass, "IsLocal");
                                            if (!isLocalMethod && !isLocalField) isLocalField = Il2Cpp::GetFieldRecursively(entityKlass, "m_IsLocalPlayer");
                                            if (!isLocalMethod && !isLocalField) isLocalField = Il2Cpp::GetFieldRecursively(entityKlass, "m_isLocalPlayer");
                                            if (!isLocalMethod && !isLocalField) isLocalField = Il2Cpp::GetFieldRecursively(entityKlass, "m_IsMainPlayer");
                                            if (!isLocalMethod && !isLocalField) isLocalField = Il2Cpp::GetFieldRecursively(entityKlass, "m_IsLocal");
                                            if (!isLocalMethod && !isLocalField) isLocalField = Il2Cpp::GetFieldRecursively(entityKlass, "_isLocal");

                                            if (!getLocalPlayerMethod) getLocalPlayerMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_localPlayer", 0);
                                            if (!getLocalPlayerMethod) getLocalPlayerMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_LocalPlayer", 0);
                                            if (!getLocalPlayerMethod) getLocalPlayerMethod = Il2Cpp::GetMethodRecursively(entityKlass, "GetLocalPlayer", 0);

                                            methodsCached = true;
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
                                             // Fallback offset - isAI na classe Player (IsClientBot 0x448 no novo dump)
                                             bool possibleBot = *(bool*)((uintptr_t)entityObj + 0x448); // Tenta IsClientBot; // 0x448
                                             p.isBot = possibleBot;
                                             // Outro offset comum: isBot; // 0x20
                                        }

                                        // Cache de classes e métodos para evitar lag massivo que trava o touch (runtime_invoke e GetMethodRecursively sao pesados num loop)
                                        // TODO: O ideal seria inicializar num setup inicial
                                        
                                        // HP e MaxHP (Tenta getter direto, se falhar, tenta via PlayerAttributes)
                                        bool hpFound = false;
                                        void* playerAttrObj = nullptr;
                                        static void* getCurHpMethod = nullptr;
                                        static void* getMaxHpMethod = nullptr;
                                        static bool hpMethodsSearched = false;

                                        if (!hpMethodsSearched) {
                                            getCurHpMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_CurHP", 0);
                                            if (!getCurHpMethod) getCurHpMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_HP", 0);
                                            getMaxHpMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_MaxHP", 0);
                                            hpMethodsSearched = true;
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
                                            // Em dump.cs, `protected PlayerAttributes JKPFFNEMJIF; // 0x708`
                                            // Como os offsets podem mudar, procuramos o campo pelo tipo ou assumimos 0x708 por enquanto.
                                            void* playerAttrField = Il2Cpp::class_get_field_from_name(entityKlass, "JKPFFNEMJIF");
                                            if (playerAttrField) {
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
                                                            // A classe tem campos como: private Int32 LGEAIKPHPED; // 0xa4 (provavel HP max/cur)
                                                            // private Int32 CHPNMIKDJNB; // 0xa8
                                                            int32_t possibleHp = *(int32_t*)((uintptr_t)playerAttrObj + 0xa4);
                                                            int32_t possibleHp2 = *(int32_t*)((uintptr_t)playerAttrObj + 0xa8); // Try the other one too

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
                                                // Fallback hardcoded para offset 0x708 caso o campo nao seja JKPFFNEMJIF
                                                playerAttrObj = *(void**)((uintptr_t)entityObj + 0x708);
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
                                                            int32_t possibleHp = *(int32_t*)((uintptr_t)playerAttrObj + 0xa4);
                                                            int32_t possibleHp2 = *(int32_t*)((uintptr_t)playerAttrObj + 0xa8);
                                                            
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

                                        // Salva o ponteiro do objeto
                                        p.obj = entityObj;

                                        if (getIsDeadMethod) {
                                            void* deadObj = Il2Cpp::runtime_invoke(getIsDeadMethod, entityObj, nullptr, nullptr);
                                            if (deadObj) p.isKnocked = *(bool*)((uintptr_t)deadObj + 0x10);
                                        }

                                        // Verificar IsLocalPlayer para ignorar ou destacar o jogador local
                                        bool isLocal = false;
                                        if (localPlayerObj != 0 && (uintptr_t)entityObj == localPlayerObj) {
                                            isLocal = true;
                                        } else {
                                            if (isLocalMethod) {
                                                void* isLocalObj = Il2Cpp::runtime_invoke(isLocalMethod, entityObj, nullptr, nullptr);
                                                if (isLocalObj) {
                                                    isLocal = *(bool*)((uintptr_t)isLocalObj + 0x10);
                                                }
                                            } else if (isLocalField) {
                                                Il2Cpp::field_get_value(entityObj, isLocalField, &isLocal);
                                            }
                                            
                                            if (!isLocal && getLocalPlayerMethod) {
                                                void* lpObj = Il2Cpp::runtime_invoke(getLocalPlayerMethod, nullptr, nullptr, nullptr);
                                                if (lpObj && lpObj == entityObj) {
                                                    isLocal = true;
                                                }
                                            }
                                        }

                                        // Set alignment via IsSameTeam se tivermos o localPlayerObj
                                        if (isLocal) {
                                            p.alignment = Alignment::ALLY;
                                        } else {
                                            if (localPlayerObj != 0 && isSameTeamMethod) {
                                                void* params[2] = { (void*)localPlayerObj, entityObj };
                                                void* sameTeamObj = Il2Cpp::runtime_invoke(isSameTeamMethod, nullptr, params, nullptr);
                                                if (sameTeamObj) {
                                                    bool sameTeam = *(bool*)((uintptr_t)sameTeamObj + 0x10);
                                                    if (sameTeam) {
                                                        p.alignment = Alignment::ALLY;
                                                    } else {
                                                        p.alignment = Alignment::ENEMY;
                                                    }
                                                } else {
                                                    p.alignment = Alignment::ENEMY;
                                                }
                                            } else {
                                                p.alignment = Alignment::ENEMY; // Fallback
                                            }
                                        }

                                        // Tenta ler teamId pra debug
                                        if (getTeamMethod) {
                                            void* teamObj = Il2Cpp::runtime_invoke(getTeamMethod, entityObj, nullptr, nullptr);
                                            if (teamObj) {
                                                void* teamKlass = Il2Cpp::object_get_class(teamObj);
                                                const char* teamKlassName = teamKlass ? Il2Cpp::class_get_name(teamKlass) : "";
                                                if (teamKlassName && strcmp(teamKlassName, "Byte") == 0) {
                                                    p.teamId = *(uint8_t*)((uintptr_t)teamObj + 0x10);
                                                } else if (teamKlassName && strcmp(teamKlassName, "SByte") == 0) {
                                                    p.teamId = *(int8_t*)((uintptr_t)teamObj + 0x10);
                                                } else if (teamKlassName && strcmp(teamKlassName, "Int16") == 0) {
                                                    p.teamId = *(int16_t*)((uintptr_t)teamObj + 0x10);
                                                } else {
                                                    p.teamId = *(int32_t*)((uintptr_t)teamObj + 0x10);
                                                }
                                            }
                                        } else if (teamField) {
                                            p.teamId = 0; // zera os 4 bytes para evitar lixo se o campo for byte
                                            Il2Cpp::field_get_value(entityObj, teamField, &p.teamId);
                                        } else if (playerAttrObj) {
                                            // Fallback: Check PlayerAttributes for teamId
                                            void* attrKlass = Il2Cpp::object_get_class(playerAttrObj);
                                            if (attrKlass) {
                                                void* attrTeamField = Il2Cpp::GetFieldRecursively(attrKlass, "camp");
                                                if (!attrTeamField) attrTeamField = Il2Cpp::GetFieldRecursively(attrKlass, "teamId");
                                                if (!attrTeamField) attrTeamField = Il2Cpp::GetFieldRecursively(attrKlass, "groupId");
                                                if (attrTeamField) {
                                                    Il2Cpp::field_get_value(playerAttrObj, attrTeamField, &p.teamId);
                                                }
                                            }
                                        }

                                        // Debug name append
                                        char debugName[256];
                                        snprintf(debugName, sizeof(debugName), "%s T:%d L:%d", p.name, p.teamId, isLocal ? 1 : 0);
                                        strncpy(p.name, debugName, sizeof(p.name) - 1);
                                        p.name[sizeof(p.name) - 1] = '\0';

                                        if (isLocal) {
                                            foundLocal = true;
                                            currentLocalPos = p.position;
                                            currentLocalTeamId = p.teamId;
                                            currentLocalObj = entityObj;
                                            continue; // Ignora o player local na lista de inimigos
                                        } else {
                                            updatedEntities.push_back(p);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                
                if (foundLocal) {
                    for (auto& p : updatedEntities) {
                        p.distanceToLocal = p.position.distance(currentLocalPos);
                        // Only fallback to teamId if alignment was not already set to ALLY by IsSameTeam
                        if (p.alignment != Alignment::ALLY) {
                            if (p.teamId == currentLocalTeamId && p.teamId != 0) {
                                p.alignment = Alignment::ALLY;
                            } else {
                                p.alignment = Alignment::ENEMY;
                            }
                        }
                    }
                } else {
                    // Fallback se não achar o jogador local: 
                    // Tenta adivinhar baseado na menor distância (se tivéssemos a câmera) ou apenas calcula distância mútua
                    for (auto& p : updatedEntities) {
                        p.alignment = Alignment::ENEMY;
                    }
                }

                {
                    std::lock_guard<std::mutex> lock(sharedState.mtx);
                    sharedState.entities = std::move(updatedEntities);
                    if (foundLocal) {
                        sharedState.localPlayerPos = currentLocalPos;
                        sharedState.localPlayerTeamId = currentLocalTeamId;
                        sharedState.localPlayerObj = currentLocalObj;
                    }
                }

                if (logger.is_open()) {
                    logger << "[SCAN] Atualizados " << sharedState.entities.size() << " jogadores." << std::endl;
                }

                // Aumentamos o tempo de sleep para desafogar a thread do Android (isso previne o travamento do ImGui / Touch)
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Fixado em 100ms para scan loop não pesar mas ser mais fluido
            }
        }
    };

} // namespace GhostSystems
