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
        static bool methodsSearched = false;

        if (!methodsSearched) {
            void* cameraKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Camera");
            if (cameraKlass) {
                get_mainMethod = Il2Cpp::GetMethodRecursively(cameraKlass, "get_main", 0);
                worldToScreenMethod = Il2Cpp::GetMethodRecursively(cameraKlass, "WorldToScreenPoint", 1);
            }
            methodsSearched = true;
        }

        if (!get_mainMethod || !worldToScreenMethod) return;

        void* exc = nullptr;
        void* mainCamera = Il2Cpp::runtime_invoke(get_mainMethod, nullptr, nullptr, &exc);
        if (!mainCamera || exc) return;

        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        ImVec2 screenSize = ImGui::GetIO().DisplaySize;

        if (!espEnabled) return;

        for (const auto& entity : localEntities) {
            // Ignora se estiver morto e o filtro de mortos estiver ativo (opcional, aqui pularemos mortos sempre pra nao poluir)
            if (!entity.isAlive()) continue;

            // Box positions (pés até a cabeça)
            struct Vector3Args {
                float x, y, z;
            };

            Vector3Args posFeet = {entity.position.x, entity.position.y, entity.position.z};
            // Adicionamos a altura do jogador (1.41) na coordenada Y
            Vector3Args posHead = {entity.position.x, entity.position.y + 1.41f, entity.position.z};

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
            }
        }
    }

} // namespace GhostSystems
