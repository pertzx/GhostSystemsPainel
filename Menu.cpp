#include "Menu.h"
#include "Obfuscator.h"
#include "Il2CppHelper.h"
#include <imgui.h>

namespace GhostSystems {

    void Menu::initStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 8.0f;
        style.FrameRounding = 6.0f;
        style.ScrollbarRounding = 6.0f;
        style.GrabRounding = 6.0f;
        
        // Aumenta o tamanho da title bar e botões para facilitar o toque no celular
        style.FramePadding = ImVec2(12.0f, 24.0f);
        style.ItemSpacing = ImVec2(12.0f, 12.0f);
        
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.12f, 0.95f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.3f, 1.0f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.35f, 1.0f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.35f, 0.4f, 1.0f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.25f, 0.3f, 1.0f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.3f, 0.3f, 0.35f, 1.0f);
    }

    void Menu::drawFilters() {
        ImGui::Text("%s", OBFUSCATE("Filtros de Depuração"));
        ImGui::Separator();
        
        ImGui::Checkbox(OBFUSCATE("Apenas Vivos"), &filterAliveOnly);
        ImGui::SameLine();
        ImGui::Checkbox(OBFUSCATE("Apenas Humanos"), &filterHumansOnly);
        
        ImGui::SliderFloat(OBFUSCATE("Distância Máxima"), &maxDistanceFilter, 0.0f, 2000.0f, "%.0f m");
        
        // Filtro de time (-1 == todos)
        ImGui::SliderInt(OBFUSCATE("Filtro de Team ID"), &filterTeamId, -1, 10, filterTeamId == -1 ? "Todos" : "%d");
        ImGui::Separator();
    }

    void Menu::drawEntityList() {
        // Garantindo acesso seguro aos dados lidos pelo MemoryScanner
        std::vector<PlayerEntity> localEntities;
        {
            std::lock_guard<std::mutex> lock(sharedState.mtx);
            localEntities = sharedState.entities; // Copia leve para minimizar lock na thread de renderização
        }

        if (ImGui::BeginTable("EntityTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 300))) {
            ImGui::TableSetupScrollFreeze(0, 1); // Fixa o header
            ImGui::TableSetupColumn("Nome", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Vida", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Coord (X,Y,Z)", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Tipo", ImGuiTableColumnFlags_WidthFixed, 50.0f);
            ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 40.0f);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableHeadersRow();

            for (const auto& entity : localEntities) {
                // Aplica filtros
                if (filterAliveOnly && !entity.isAlive()) continue;
                if (filterHumansOnly && entity.isBot) continue;
                if (entity.distanceToLocal > maxDistanceFilter) continue;
                if (filterTeamId != -1 && entity.teamId != filterTeamId) continue;

                ImGui::TableNextRow();
                
                // Nome
                ImGui::TableSetColumnIndex(0);
                ImVec4 nameColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default Branco
                if (entity.alignment == Alignment::ENEMY) nameColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
                else if (entity.alignment == Alignment::ALLY) nameColor = ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
                
                ImGui::TextColored(nameColor, "%s", entity.name);

                // Vida (Barra de progresso)
                ImGui::TableSetColumnIndex(1);
                float hpPct = entity.health / entity.maxHealth;
                ImVec4 hpColor = hpPct > 0.5f ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : (hpPct > 0.2f ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, hpColor);
                char hpText[32];
                snprintf(hpText, sizeof(hpText), "%.0f/%.0f", entity.health, entity.maxHealth);
                ImGui::ProgressBar(hpPct, ImVec2(-FLT_MIN, 0), hpText);
                ImGui::PopStyleColor();

                // Coords
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.2f, %.2f, %.2f", entity.position.x, entity.position.y, entity.position.z);

                // Tipo
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%s", entity.isBot ? "BOT" : "PLAYER");

                // Time
                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%d", entity.teamId);

                // Status (Knocked / Vivo / Morto)
                ImGui::TableSetColumnIndex(5);
                if (!entity.isAlive()) {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "MORTO");
                } else if (entity.isKnocked) {
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "DERRUBADO");
                } else {
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "VIVO");
                }
            }
            ImGui::EndTable();
        }
    }

    void Menu::render() {
        if (!isVisible) return;

        // O painel precisa ser arrastavel, entao removemos ImGuiWindowFlags_NoMove e ImGuiWindowFlags_NoTitleBar se houver.
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
        
        // Adicionamos estilizacao
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);

        ImGui::SetNextWindowSize(ImVec2(600, 450), ImGuiCond_FirstUseEver);

        // Renderiza ESP sempre que o menu renderiza
        drawESP();

        if (ImGui::Begin(OBFUSCATE("GhostSystems - Debugger de EntityList"), &isVisible, windowFlags)) {
            
            // Logica customizada para permitir arrastar tocando no fundo da janela (Mobile amigavel)
            if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                ImVec2 delta = ImGui::GetIO().MouseDelta;
                ImVec2 pos = ImGui::GetWindowPos();
                ImGui::SetWindowPos(ImVec2(pos.x + delta.x, pos.y + delta.y));
            }
            
            if (ImGui::BeginTabBar("MenuTabs")) {
                if (ImGui::BeginTabItem("Aimbot")) {
                    ImGui::Checkbox(OBFUSCATE("Ativar Aimbot"), &aimbotEnabled);
                    if (aimbotEnabled) {
                const char* aimbotModes[] = { "Tradicional (Ao Atirar)", "Aimlock (Sempre)" };
                ImGui::Combo("Modo", &aimbotMode, aimbotModes, IM_ARRAYSIZE(aimbotModes));
                ImGui::Checkbox(OBFUSCATE("Mostrar FOV"), &aimbotDrawFov);
                ImGui::Checkbox(OBFUSCATE("Mirar em Aliados"), &aimbotTargetAllies);
                ImGui::SliderFloat(OBFUSCATE("Raio do FOV"), &aimbotFov, 10.0f, 500.0f, "%.0f px");
                ImGui::SliderInt(OBFUSCATE("Atraso/Delay (ms)"), &aimbotTimeMs, 0, 300, "%d ms");
                
                if (aimbotTimeMs < 50) {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Rage Aim (Sem Atraso)");
                } else {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Safe Aim (Com Atraso)");
                }
                
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "A mira foca no inimigo mais proximo do centro (FOV).");
            }
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Funcoes ESP")) {
                    ImGui::Checkbox(OBFUSCATE("Ativar ESP"), &espEnabled);
                    if (espEnabled) {
                        ImGui::Checkbox(OBFUSCATE("ESP Box"), &espBox);
                        ImGui::Checkbox(OBFUSCATE("ESP Nome"), &espName);
                        ImGui::Checkbox(OBFUSCATE("ESP Distancia"), &espDistance);
                        ImGui::Checkbox(OBFUSCATE("ESP Linha"), &espLine);
                    }
                    ImGui::EndTabItem();
                }

                if (isDebugMode) {
                    if (ImGui::BeginTabItem("Entity List (Debug)")) {
                        drawFilters();
                        
                        ImGui::Text("%s", OBFUSCATE("Entidades Encontradas:"));
                        drawEntityList();
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Debug Player")) {
                        drawDebugPlayer();
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Debug Aimbot")) {
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "STATUS: %s", aimbotHasTarget ? "ALVO TRAVADO" : "BUSCANDO...");
                        ImGui::Separator();
                        ImGui::Text("Alvo Atual: %s", aimbotTargetName.c_str());
                        ImGui::Text("Distancia FOV (px): %.2f", aimbotTargetDistFOV);
                        ImGui::Text("Distancia 3D (m): %.2f", aimbotTargetDist3D);
                        
                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Posicao da Camera (World):");
                        ImGui::Text("X: %.2f | Y: %.2f | Z: %.2f", aimbotCamPosX, aimbotCamPosY, aimbotCamPosZ);
                        
                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Rotacao da Camera Atual (Quaternion):");
                        ImGui::Text("X: %.2f | Y: %.2f | Z: %.2f | W: %.2f", aimbotCamRotX, aimbotCamRotY, aimbotCamRotZ, aimbotCamRotW);

                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Rotacao do Alvo (LookRotation):");
                        ImGui::Text("X: %.2f | Y: %.2f | Z: %.2f | W: %.2f", aimbotTargetRotX, aimbotTargetRotY, aimbotTargetRotZ, aimbotTargetRotW);

                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(0, 1, 1, 1), "Nova Rotacao Calculada (Slerp):");
                        ImGui::Text("X: %.2f | Y: %.2f | Z: %.2f | W: %.2f", aimbotNewRotX, aimbotNewRotY, aimbotNewRotZ, aimbotNewRotW);
                        
                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Logs / Erros (Aimbot Engine):");
                        ImGui::TextWrapped("%s", aimbotErrorLog.c_str());

                        ImGui::EndTabItem();
                    }
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
        
        ImGui::PopStyleVar(); // Pop da estilizacao
    }

    void Menu::scanForPotentialValues(void* obj, void* klass, const std::string& path, int depth, std::unordered_set<void*>& visited) {
        if (!obj || !klass || depth > 1) return;
        if (visited.count(obj)) return;
        visited.insert(obj);

        void* current_klass = klass;
        while (current_klass) {
            const char* currentClassName = Il2Cpp::class_get_name(current_klass);
            if (!currentClassName) currentClassName = "UnknownClass";

            void* iter = nullptr;
            void* field = nullptr;
            while ((field = Il2Cpp::class_get_fields(current_klass, &iter)) != nullptr) {
                const char* fieldName = Il2Cpp::field_get_name(field);
                size_t offset = Il2Cpp::field_get_offset(field);
                void* type = Il2Cpp::field_get_type(field);
                const char* typeName = type ? Il2Cpp::type_get_name(type) : "UnknownType";

                if (!fieldName) fieldName = "UnknownField";
                std::string currentPath = path + "." + currentClassName + "." + fieldName;

                bool isArray = strstr(typeName, "[]") != nullptr || strstr(typeName, "Array") != nullptr;
                
                // Limite rigoroso de offset para evitar acesso fora da struct/classe real
                if (offset == 0 || offset > 0x800) continue; 

                void* valueAddr = (void*)((uintptr_t)obj + offset);
                
                if (!isArray && strstr(typeName, "Single")) {
                    debugPotentialValues.push_back({currentPath, obj, offset, "float"});
                } else if (!isArray && strstr(typeName, "Int32")) {
                    debugPotentialValues.push_back({currentPath, obj, offset, "int"});
                } else if (!isArray && strstr(typeName, "Boolean")) {
                    debugPotentialValues.push_back({currentPath, obj, offset, "bool"});
                } else if (!isArray && type) {
                    void* fieldClass = Il2Cpp::class_from_type(type);
                    if (fieldClass) {
                        bool isValueType = Il2Cpp::class_is_valuetype(fieldClass);
                        if (isValueType && depth < 2) {
                            // Se for struct (ValueType) como Vector3, ela está INLINE na memória, logo não precisa ler ponteiro
                            // apenas passamos o próprio valueAddr como se fosse o "obj" base.
                            scanForPotentialValues(valueAddr, fieldClass, currentPath, depth + 1, visited);
                        } else if (!isValueType && depth < 1) {
                            // ESPECÍFICO PARA ACHAR HP:
                            // Só vamos tentar ler classes filhas (ReferenceTypes) se o nome delas sugerir "Atributos", "Health", "State"
                            // No Free Fire o HP fica dentro de PlayerAttributes (que é ofuscado, então olhamos substrings genéricas)
                            std::string lowerName = fieldName;
                            for (char& c : lowerName) c = tolower(c);
                            std::string lowerClass = Il2Cpp::class_get_name(fieldClass) ? Il2Cpp::class_get_name(fieldClass) : "";
                            for (char& c : lowerClass) c = tolower(c);

                            bool mightBeHpContainer = (
                                lowerName.find("attr") != std::string::npos || lowerClass.find("attr") != std::string::npos ||
                                lowerName.find("health") != std::string::npos || lowerClass.find("health") != std::string::npos ||
                                lowerName.find("state") != std::string::npos || lowerClass.find("state") != std::string::npos
                            );

                            if (mightBeHpContainer) {
                                void* childObj = *(void**)valueAddr;
                                // Validação estrita: deve ser um ponteiro > 10MB
                                if ((uintptr_t)childObj > 0xA00000) {
                                    scanForPotentialValues(childObj, fieldClass, currentPath, depth + 1, visited);
                                }
                            }
                        }
                    }
                }
            }
            current_klass = Il2Cpp::class_get_parent ? Il2Cpp::class_get_parent(current_klass) : nullptr;
        }
    }

    void Menu::drawDebugPlayer() {
        void* localPlayerObj = nullptr;
        {
            std::lock_guard<std::mutex> lock(sharedState.mtx);
            localPlayerObj = sharedState.localPlayerObj;
        }

        if (!localPlayerObj) {
            ImGui::Text("Nenhum jogador local encontrado ou Il2Cpp offline.");
            return;
        }

        void* klass = Il2Cpp::object_get_class(localPlayerObj);
        if (!klass) {
            ImGui::Text("Falha ao obter a classe do jogador local.");
            return;
        }

        if (ImGui::Button("Escanear Valores Possiveis (HP/Yaw/Pitch)")) {
            debugPotentialValues.clear();
            std::unordered_set<void*> visited;
            scanForPotentialValues(localPlayerObj, klass, "LocalPlayer", 0, visited);
            hasScannedValues = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Limpar")) {
            debugPotentialValues.clear();
            hasScannedValues = false;
        }

        if (hasScannedValues) {
            ImGui::Separator();
            ImGui::Text("Valores: %zu", debugPotentialValues.size());
            ImGui::SameLine();
            ImGui::Checkbox("Floats", &filterOnlyFloats); ImGui::SameLine();
            ImGui::Checkbox("Ints", &filterOnlyInts);
            ImGui::PushItemWidth(150);
            ImGui::InputText("Buscar", searchFilter, sizeof(searchFilter));
            ImGui::PopItemWidth();
            ImGui::Separator();

            static float doScroll = 0.0f;
            
            if (ImGui::BeginChild("DebugPlayerValuesScroll", ImVec2(0, 0), true)) {
                if (doScroll != 0.0f) {
                    ImGui::SetScrollY(ImGui::GetScrollY() + doScroll);
                    doScroll = 0.0f;
                }

                auto toLower = [](std::string s) {
                    for (auto& c : s) c = tolower(c);
                    return s;
                };
                std::string filterStr = toLower(searchFilter);

                for (auto& val : debugPotentialValues) {
                    if (filterOnlyFloats && val.type != "float") continue;
                    if (filterOnlyInts && val.type != "int") continue;
                    
                    if (!filterStr.empty() && toLower(val.path).find(filterStr) == std::string::npos) continue;

                    // Ler valor atual
                    std::string valStr;
                    if (val.type == "float") {
                        valStr = std::to_string(*(float*)((uintptr_t)val.obj + val.offset));
                    } else if (val.type == "int") {
                        valStr = std::to_string(*(int32_t*)((uintptr_t)val.obj + val.offset));
                    } else if (val.type == "bool") {
                        valStr = *(bool*)((uintptr_t)val.obj + val.offset) ? "true" : "false";
                    }

                    // Destacar cores para facilitar
                    if (val.type == "float") ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
                    else if (val.type == "int") ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.4f, 1.0f));
                    else ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));

                    ImGui::Text("[%s] %s: %s", val.type.c_str(), val.path.c_str(), valStr.c_str());
                    
                    ImGui::PopStyleColor();
                }
            }
            ImGui::EndChild();

            ImVec2 childPos = ImGui::GetItemRectMin();
            ImVec2 childSize = ImGui::GetItemRectSize();

            // Botões flutuantes para scroll contínuo (usando IsItemActive)
            ImGui::SetCursorScreenPos(ImVec2(childPos.x + childSize.x - 70, childPos.y + childSize.y - 130));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.8f));
            ImGui::Button("Up", ImVec2(50, 50));
            if (ImGui::IsItemActive()) doScroll = -15.0f; // Scroll suave contínuo

            ImGui::SetCursorScreenPos(ImVec2(childPos.x + childSize.x - 70, childPos.y + childSize.y - 70));
            ImGui::Button("Dn", ImVec2(50, 50));
            if (ImGui::IsItemActive()) doScroll = 15.0f; // Scroll suave contínuo

            ImGui::PopStyleColor();
        } else {
            ImGui::Text("Clique em 'Escanear' para buscar floats e ints no Player.");
        }
    }

    void Menu::drawESP() {
        std::vector<PlayerEntity> localEntities;
        {
            std::lock_guard<std::mutex> lock(sharedState.mtx);
            localEntities = sharedState.entities;
        }

        if (localEntities.empty()) return;

        static void* get_mainMethod = nullptr;
        static void* worldToScreenMethod = nullptr;
        
        static void* componentKlass = nullptr;
        static void* getTransformMethod = nullptr;
        static void* transformKlass = nullptr;
        static void* getPosMethod = nullptr;
        static void* getRotMethod = nullptr;
        static void* setRotMethod = nullptr;
        static void* quatKlass = nullptr;
        static void* lookRotMethod = nullptr;
        static void* slerpMethod = nullptr;
        static void* setAimRotationMethod = nullptr;
        static void* isFiringMethod = nullptr;

        static bool methodsSearched = false;

        if (!methodsSearched) {
            void* cameraKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Camera");
            if (cameraKlass) {
                get_mainMethod = Il2Cpp::GetMethodRecursively(cameraKlass, "get_main", 0);
                worldToScreenMethod = Il2Cpp::GetMethodRecursively(cameraKlass, "WorldToScreenPoint", 1);
            }
            
            componentKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Component");
            if (componentKlass) {
                getTransformMethod = Il2Cpp::GetMethodRecursively(componentKlass, "get_transform", 0);
            }
            
            transformKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Transform");
            if (transformKlass) {
                getPosMethod = Il2Cpp::GetMethodRecursively(transformKlass, "get_position", 0);
                getRotMethod = Il2Cpp::GetMethodRecursively(transformKlass, "get_rotation", 0);
                setRotMethod = Il2Cpp::GetMethodRecursively(transformKlass, "set_rotation", 1);
            }
            
            quatKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Quaternion");
            if (quatKlass) {
                lookRotMethod = Il2Cpp::GetMethodRecursively(quatKlass, "LookRotation", 1);
                slerpMethod = Il2Cpp::GetMethodRecursively(quatKlass, "Slerp", 3);
            }

            methodsSearched = true;
        }

        // Cache do metodo SetAimRotation no Player
        if (!setAimRotationMethod && sharedState.localPlayerObj) {
            void* playerKlass = Il2Cpp::object_get_class(sharedState.localPlayerObj);
            if (playerKlass) {
                setAimRotationMethod = Il2Cpp::GetMethodRecursively(playerKlass, "SetAimRotation", 2);
                isFiringMethod = Il2Cpp::GetMethodRecursively(playerKlass, "IsFiring", 0);
            }
        }

        if (!get_mainMethod || !worldToScreenMethod) return;

        void* exc = nullptr;
        void* mainCamera = Il2Cpp::runtime_invoke(get_mainMethod, nullptr, nullptr, &exc);
        if (!mainCamera || exc) return;

        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        ImVec2 screenCenter(screenSize.x / 2.0f, screenSize.y / 2.0f);

        // Desenha o circulo de FOV do Aimbot
        if (aimbotEnabled && aimbotDrawFov) {
            drawList->AddCircle(screenCenter, aimbotFov, IM_COL32(255, 255, 255, 100), 64, 1.0f);
        }

        if (!espEnabled && !aimbotEnabled) return;

        struct Vector3Args { float x, y, z; };
        struct QuaternionArgs { float x, y, z, w; };

        float closestAimbotDist = 999999.0f;
        ImVec2 bestTargetPos(0, 0);
        Vector3Args bestTargetWorldPos = {0, 0, 0};
        bool foundAimbotTarget = false;

        for (const auto& entity : localEntities) {
            // Ignora se estiver morto e o filtro de mortos estiver ativo (opcional, aqui pularemos mortos sempre pra nao poluir)
            if (!entity.isAlive()) continue;

            // Box positions (pés até a cabeça)
            Vector3Args posFeet = {entity.position.x, entity.position.y, entity.position.z};
            // Adicionamos a altura do jogador (1.34) na coordenada Y para pegar a cabeca exata
            Vector3Args posHead = {entity.position.x, entity.position.y + 1.34f, entity.position.z};

            void* argsFeet[1] = { &posFeet };
            void* argsHead[1] = { &posHead };

            void* w2sFeetObj = Il2Cpp::runtime_invoke(worldToScreenMethod, mainCamera, argsFeet, &exc);
            if (!w2sFeetObj || exc) continue;
            Vector3Args w2sFeet = *(Vector3Args*)((uintptr_t)w2sFeetObj + 0x10);

            void* w2sHeadObj = Il2Cpp::runtime_invoke(worldToScreenMethod, mainCamera, argsHead, &exc);
            if (!w2sHeadObj || exc) continue;
            Vector3Args w2sHead = *(Vector3Args*)((uintptr_t)w2sHeadObj + 0x10);

            // Verifica se esta na frente da camera (Z > 0.0f)
            if (w2sFeet.z > 0.0f) {
                // Converte as coordenadas de tela do Unity (origem bottom-left) para as do ImGui (origem top-left)
                float xFeet = w2sFeet.x;
                float yFeet = screenSize.y - w2sFeet.y;

                float xHead = w2sHead.x;
                float yHead = screenSize.y - w2sHead.y;

                float boxHeight = yFeet - yHead;
                // Previne calculos absurdos caso as coordenadas estejam distorcidas
                if (boxHeight < 0) boxHeight = -boxHeight; 
                float boxWidth = boxHeight / 2.0f; // Proporcao basica de jogador (largura é metade da altura)

                ImVec2 topLeft(xHead - boxWidth / 2.0f, yHead);
                ImVec2 bottomRight(xFeet + boxWidth / 2.0f, yFeet);

                ImU32 color = (entity.alignment == Alignment::ENEMY) ? IM_COL32(255, 0, 0, 255) : IM_COL32(0, 255, 0, 255);

                // Lógica de FOV (Aimbot) - Checa a distancia para o centro da tela
                if (aimbotEnabled) {
                    float distToCenter = sqrt(pow(xHead - screenCenter.x, 2) + pow(yHead - screenCenter.y, 2));
                    if (distToCenter <= aimbotFov && distToCenter < closestAimbotDist) {
                        closestAimbotDist = distToCenter;
                        bestTargetPos = ImVec2(xHead, yHead);
                        bestTargetWorldPos = posHead; // Marca a cabeca como alvo em 3D
                        foundAimbotTarget = true;
                        
                        // Guarda informacoes de Debug
                        aimbotTargetName = entity.name;
                        aimbotTargetDistFOV = distToCenter;
                        aimbotTargetDist3D = entity.distanceToLocal;
                    }
                }
                
                if (espEnabled) {
                    if (espBox) {
                    // Desenha o retangulo (Box ESP)
                    drawList->AddRect(topLeft, bottomRight, color, 0.0f, 0, 1.5f);
                    
                    // Desenha a barra de vida ao lado (opcional, bom pra debug visual)
                    if (entity.maxHealth > 0) {
                        float hpPct = entity.health / entity.maxHealth;
                        if (hpPct > 1.0f) hpPct = 1.0f;
                        if (hpPct < 0.0f) hpPct = 0.0f;

                        ImVec2 hpTopLeft(topLeft.x - 6.0f, yHead + (boxHeight * (1.0f - hpPct)));
                        ImVec2 hpBottomRight(topLeft.x - 2.0f, yFeet);
                        
                        ImU32 hpColor = IM_COL32(0, 255, 0, 255);
                        if (hpPct < 0.5f) hpColor = IM_COL32(255, 255, 0, 255);
                        if (hpPct < 0.25f) hpColor = IM_COL32(255, 0, 0, 255);

                        // Fundo da barra (preto)
                        drawList->AddRectFilled(ImVec2(topLeft.x - 6.0f, yHead), ImVec2(topLeft.x - 2.0f, yFeet), IM_COL32(0, 0, 0, 150));
                        // Barra de HP
                        drawList->AddRectFilled(hpTopLeft, hpBottomRight, hpColor);
                    }
                }

                if (espLine) {
                    // Linha do topo da tela ate o jogador (DrawLine)
                    drawList->AddLine(ImVec2(screenSize.x / 2.0f, 0.0f), ImVec2(xHead, yHead), color, 1.0f);
                }

                if (espName || espDistance) {
                    char textBuffer[128];
                    if (espName && espDistance) {
                        snprintf(textBuffer, sizeof(textBuffer), "%s [%.0fm]", entity.name, entity.distanceToLocal);
                    } else if (espName) {
                        snprintf(textBuffer, sizeof(textBuffer), "%s", entity.name);
                    } else {
                        snprintf(textBuffer, sizeof(textBuffer), "[%.0fm]", entity.distanceToLocal);
                    }

                    ImVec2 textSize = ImGui::CalcTextSize(textBuffer);
                    ImVec2 textPos(xHead - textSize.x / 2.0f, yHead - textSize.y - 2.0f);
                    
                    // Contorno do texto
                    drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 255), textBuffer);
                    drawList->AddText(ImVec2(textPos.x - 1, textPos.y - 1), IM_COL32(0, 0, 0, 255), textBuffer);
                    drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), textBuffer);
                }
                } // Fim do espEnabled
            }
        }

        // Desenhar linha do Aimbot para o alvo travado
        aimbotHasTarget = foundAimbotTarget;
        if (!foundAimbotTarget) {
            aimbotTargetName = "Nenhum";
            aimbotTargetDistFOV = 0.0f;
            aimbotTargetDist3D = 0.0f;
            aimbotErrorLog = "Buscando alvos no FOV...";
        }

        if (aimbotEnabled && foundAimbotTarget) {
            drawList->AddLine(screenCenter, bestTargetPos, IM_COL32(255, 0, 0, 255), 2.0f);
            drawList->AddCircle(bestTargetPos, 5.0f, IM_COL32(255, 0, 0, 255), 12, 2.0f);

            bool shouldAim = true;
            if (aimbotMode == 0) { // Tradicional (Ao Atirar)
                shouldAim = false;
                if (isFiringMethod && sharedState.localPlayerObj) {
                    void* isFiringObj = Il2Cpp::runtime_invoke(isFiringMethod, sharedState.localPlayerObj, nullptr, &exc);
                    if (isFiringObj && !exc) {
                        shouldAim = *(bool*)((uintptr_t)isFiringObj + 0x10);
                    }
                }
                if (!shouldAim) {
                    aimbotErrorLog = "Aguardando disparo (Modo Tradicional)...";
                }
            }

            float deltaTime = ImGui::GetIO().DeltaTime;
            
            // Controle do Delay (Time ms) antes de cravar a mira
            bool delayPassed = true;
            if (shouldAim) {
                if (!wasAimingLastFrame) {
                    aimbotDelayTimer = 0.0f;
                }
                aimbotDelayTimer += deltaTime * 1000.0f; // Converte para ms
                
                if (aimbotDelayTimer < (float)aimbotTimeMs) {
                    delayPassed = false;
                    aimbotErrorLog = "Aguardando Delay (Time ms)...";
                }
                wasAimingLastFrame = true;
            } else {
                wasAimingLastFrame = false;
                aimbotDelayTimer = 0.0f;
            }

            // APLICAR ROTAÇÃO NA CÂMERA (Aim Lock 3D)
            if (shouldAim && delayPassed && getTransformMethod && getPosMethod && getRotMethod && setRotMethod && lookRotMethod) {
                void* cameraTransform = Il2Cpp::runtime_invoke(getTransformMethod, mainCamera, nullptr, &exc);
                if (cameraTransform && !exc) {
                    void* camPosObj = Il2Cpp::runtime_invoke(getPosMethod, cameraTransform, nullptr, &exc);
                    if (camPosObj && !exc) {
                        Vector3Args camPos = *(Vector3Args*)((uintptr_t)camPosObj + 0x10);
                        
                        aimbotCamPosX = camPos.x;
                        aimbotCamPosY = camPos.y;
                        aimbotCamPosZ = camPos.z;

                        // Vetor direcional para o alvo (Cabeca)
                        Vector3Args dir = { bestTargetWorldPos.x - camPos.x, bestTargetWorldPos.y - camPos.y, bestTargetWorldPos.z - camPos.z };
                        
                        // LookRotation
                        void* argsRot[1] = { &dir };
                        void* targetRotObj = Il2Cpp::runtime_invoke(lookRotMethod, nullptr, argsRot, &exc);
                        if (targetRotObj && !exc) {
                            QuaternionArgs targetRot = *(QuaternionArgs*)((uintptr_t)targetRotObj + 0x10);
                            
                            aimbotTargetRotX = targetRot.x;
                            aimbotTargetRotY = targetRot.y;
                            aimbotTargetRotZ = targetRot.z;
                            aimbotTargetRotW = targetRot.w;

                            // Como o Delay (Time ms) ja passou, cravamos instantaneamente no alvo (Rage)
                            // Nao usamos mais Slerp, apenas definimos a rotacao alvo diretamente.
                            aimbotNewRotX = targetRot.x;
                            aimbotNewRotY = targetRot.y;
                            aimbotNewRotZ = targetRot.z;
                            aimbotNewRotW = targetRot.w;
                            
                            // set_rotation (escreve a rotacao na camera principal)
                            void* argsSetRot[1] = { &targetRot };
                            Il2Cpp::runtime_invoke(setRotMethod, cameraTransform, argsSetRot, &exc);
                            
                            bool aimSetSuccess = false;
                            if (setAimRotationMethod && sharedState.localPlayerObj) {
                                bool isTrue = true;
                                void* argsAim[2] = { &targetRot, &isTrue };
                                Il2Cpp::runtime_invoke(setAimRotationMethod, sharedState.localPlayerObj, argsAim, &exc);
                                if (!exc) {
                                    aimSetSuccess = true;
                                }
                            }
                            
                            if (exc) {
                                aimbotErrorLog = "Erro ao invocar set_rotation / SetAimRotation.";
                            } else {
                                if (aimSetSuccess) {
                                    aimbotErrorLog = "Mira cravada (Delay concluido)!";
                                } else {
                                    aimbotErrorLog = "Rotacao da camera aplicada, mas falhou no Player Aim.";
                                }
                            }
                        } else {
                            aimbotErrorLog = "Erro ao invocar LookRotation.";
                        }
                    } else {
                        aimbotErrorLog = "Erro ao obter get_position da Camera.";
                    }
                } else {
                    aimbotErrorLog = "Erro ao obter get_transform da Camera.";
                }
            } else {
                aimbotErrorLog = "Metodos do Unity (Transform/Quaternion) nao encontrados no cache.";
            }
        }
    }

} // namespace GhostSystems
