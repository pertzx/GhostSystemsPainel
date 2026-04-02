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
                    void* baseGameClass = Il2Cpp::GetClass("Assembly-CSharp.dll", "GCommon", "BaseGame");
                    if (baseGameClass) {
                        void* sAllEntitiesField = Il2Cpp::class_get_field_from_name(baseGameClass, "sAllEntities");
                        if (sAllEntitiesField) {
                            Il2Cpp::field_static_get_value(sAllEntitiesField, &dictionaryObj);
                        }
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

                                        // Extrair Nome - Ler string .NET / Il2Cpp corretamente
                                        void* getNickNameMethod = Il2Cpp::GetMethodRecursively(entityKlass, "GetNickName", 0);
                                        if (!getNickNameMethod) getNickNameMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_NickName", 0);
                                        if (!getNickNameMethod) getNickNameMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_Nickname", 0); // Outra variavel
                                        if (!getNickNameMethod) getNickNameMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_Name", 0);

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
                                        void* isAIMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_IsAI", 0); // Geralmente é um getter de propriedade
                                        if (!isAIMethod) isAIMethod = Il2Cpp::GetMethodRecursively(entityKlass, "IsAI", 0);
                                        if (!isAIMethod) isAIMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_isBot", 0);
                                        
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

                                        // Obter a posição (usando o getter ou offsets direto se possivel para nao lagar)
                                        void* getPosMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_Position", 0);
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

                                        void* getTeamMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_TeamID", 0);
                                        if (getTeamMethod) {
                                            void* teamObj = Il2Cpp::runtime_invoke(getTeamMethod, entityObj, nullptr, nullptr);
                                            if (teamObj) {
                                                // Pode ser Byte ou Int32 dependendo da versão, lendo Int32 por segurança e fazendo cast.
                                                // Se for byte o padding pode zerar os outros 3 bytes, mas é mais seguro ler 1 byte e dps ver.
                                                // No dump.cs tem `public Int32 get_TeamID()` e `public Byte get_TeamID()`.
                                                p.teamId = (int)(*(uint8_t*)((uintptr_t)teamObj + 0x10)); 
                                            }
                                        }

                                        void* getIsDeadMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_IsDead", 0);
                                        if (getIsDeadMethod) {
                                            void* deadObj = Il2Cpp::runtime_invoke(getIsDeadMethod, entityObj, nullptr, nullptr);
                                            if (deadObj) p.isKnocked = *(bool*)((uintptr_t)deadObj + 0x10);
                                        }

                                        // Verificar IsLocalPlayer para ignorar ou destacar o jogador local
                                        bool isLocal = false;
                                        void* isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "IsLocalPlayer", 0);
                                        if (!isLocalMethod) isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "IsLocalEntity", 0);
                                        if (!isLocalMethod) isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_IsLocal", 0);
                                        if (!isLocalMethod) isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "get_IsLocalPlayer", 0);
                                        if (!isLocalMethod) isLocalMethod = Il2Cpp::GetMethodRecursively(entityKlass, "IsLocalAvatar", 0);
                                        
                                        if (isLocalMethod) {
                                            void* isLocalObj = Il2Cpp::runtime_invoke(isLocalMethod, entityObj, nullptr, nullptr);
                                            if (isLocalObj) {
                                                isLocal = *(bool*)((uintptr_t)isLocalObj + 0x10);
                                            }
                                        }

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
                        if (p.teamId == currentLocalTeamId) {
                            p.alignment = Alignment::ALLY;
                        } else {
                            p.alignment = Alignment::ENEMY;
                        }
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
                std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Era um distrib(gen), fixado em 500ms para scan loop não pesar
            }
        }
    };

} // namespace GhostSystems
