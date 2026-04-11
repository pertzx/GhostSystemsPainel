#include "Menu.h"
#include "Obfuscator.h"
#include "Il2CppHelper.h"
#include <imgui.h>
#include <string>
#include <mutex>
#include <vector>
#include <fstream>
#include <sstream>

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

    void Menu::loadUserInfo() {
        std::ifstream file("/data/local/tmp/.gs_license");
        if (!file.is_open()) {
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::string key;
            std::string value;
            std::istringstream iss(line);
            if (std::getline(iss, key, '=') && std::getline(iss, value)) {
                if (key == "USUARIO") userName = value;
                else if (key == "PLANO") userPlan = value;
                else if (key == "TEMPO_RESTANTE") userTimeLeft = std::stoll(value);
                else if (key == "EXPIRA_EM") userExpireAt = std::stoll(value);
            }
        }
        file.close();

        std::remove("/data/local/tmp/.gs_license");
    }

    void Menu::drawFilters() {
        ImGui::Text("%s", OBFUSCATE("Filtros de Depuração"));
        ImGui::Separator();
        
        ImGui::Checkbox(OBFUSCATE("Apenas Vivos"), &filterAliveOnly);
        ImGui::SameLine();
        ImGui::Checkbox(OBFUSCATE("Apenas Humanos"), &filterHumansOnly);
        
        ImGui::SliderFloat(OBFUSCATE("Distância Máxima"), &maxDistanceFilter, 0.0f, 100.0f, "%.0f m");
        
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

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);

        ImGui::SetNextWindowSize(ImVec2(600, 450), ImGuiCond_FirstUseEver);

        // Renderiza ESP sempre que o menu renderiza
        drawESP();

        if (ImGui::Begin(OBFUSCATE("GhostSystems V1.0"), &isVisible, windowFlags)) {
            
            // Logica customizada para permitir arrastar tocando no fundo da janela (Mobile amigavel)
            if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                ImVec2 delta = ImGui::GetIO().MouseDelta;
                ImVec2 pos = ImGui::GetWindowPos();
                ImGui::SetWindowPos(ImVec2(pos.x + delta.x, pos.y + delta.y));
            }
            
            ImGui::Checkbox(OBFUSCATE("Ativar Painel"), &masterSwitch);

            ImGui::SameLine(ImGui::GetWindowWidth() - 280);
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", userName.c_str());
            ImGui::SameLine(ImGui::GetWindowWidth() - 150);
            ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.2f, 1.0f), "%s", userPlan.c_str());
            ImGui::SameLine(ImGui::GetWindowWidth() - 50);
            if (userExpireAt == -1) {
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "LIFETIME");
            } else if (userTimeLeft <= 0) {
                ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "EXPIRED");
            } else {
                int dias = userTimeLeft / 86400000;
                int horas = (userTimeLeft % 86400000) / 3600000;
                int mins = (userTimeLeft % 3600000) / 60000;
                if (dias > 0) ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "%dd %dh", dias, horas);
                else if (horas > 0) ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "%dh %dm", horas, mins);
                else ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "%dm", mins);
            }

            ImGui::Separator();
            
            if (masterSwitch) {
                if (ImGui::BeginTabBar("MenuTabs")) {
                if (ImGui::BeginTabItem("Combate")) {
                    ImGui::Checkbox(OBFUSCATE("Aimbot Tradicional"), &aimbotEnabled);
                const char* aimbotModes[] = { "Tradicional (Ao Atirar)", "Aimlock (Sempre)" };
                ImGui::Combo("Modo", &aimbotMode, aimbotModes, IM_ARRAYSIZE(aimbotModes));
                ImGui::Checkbox(OBFUSCATE("Mostrar FOV"), &aimbotDrawFov);
                ImGui::Checkbox(OBFUSCATE("Mirar em Aliados"), &aimbotTargetAllies);
                ImGui::Checkbox(OBFUSCATE("Ignorar Invisiveis (Atras da Parede)"), &aimbotVisibilityCheck);
                ImGui::Checkbox(OBFUSCATE("Mira Magnetica (Puxa inimigo)"), &aimbotMagnetic);
                ImGui::SliderFloat(OBFUSCATE("Raio do FOV"), &aimbotFov, 10.0f, 500.0f, "%.0f px");
                
                ImGui::Separator();
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Força & Lógica Absoluta");
                
                const char* hitboxes[] = { "Cabeça", "Pescoço", "Peito" };
                ImGui::Combo(OBFUSCATE("Alvo (Hitbox)"), &aimbotHitbox, hitboxes, IM_ARRAYSIZE(hitboxes));
                
                ImGui::SliderFloat(OBFUSCATE("Força Magnética (Puxão)"), &aimbotPullStrength, 1.0f, 5.0f, "%.1f");
                
                ImGui::Separator();
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Aimbot Delay (Peito -> Alvo)");
                ImGui::SliderFloat("Smooth Time (ms)", &aimbotSmoothTimeMs, 0.0f, 200.0f, "%.0f ms");
                ImGui::SliderFloat("Smooth Curve", &aimbotSmoothCurve, 1.0f, 10.0f, "%.1f");
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Modo Logica Pura Ativo: Sem delays ou aceleracoes.");
                
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "A mira foca no inimigo mais proximo do centro (FOV).");

                ImGui::Separator();
                ImGui::TextColored(ImVec4(1.0f, 0.2f, 1.0f, 1.0f), "=== SILENT AIM ===");
                ImGui::Checkbox(OBFUSCATE("Ativar Silent Aim"), &silentAimEnabled);
                    const char* approaches[] = {
                        "0: Hook GetFireDirection (Recomendado)",
                        "1: Hook StartFiring + Modificar",
                        "2: Hook LookAtPosition (Camera)",
                        "3: Hook GetLookDirection",
                        "4: Raycast/Predicao Custom",
                        "5: Modificar Rotacao Player"
                    };
                    ImGui::Combo(OBFUSCATE("Abordagem"), &silentAimApproach, approaches, IM_ARRAYSIZE(approaches));
                    ImGui::Checkbox(OBFUSCATE("Mostrar Debug"), &silentAimDrawDebug);
                    ImGui::SliderFloat(OBFUSCATE("Suavizacao (Smooth)"), &silentAimSmooth, 0.0f, 1.0f, "%.2f");
                    ImGui::SliderFloat(OBFUSCATE("Distancia Maxima"), &silentAimMaxDistance, 50.0f, 500.0f, "%.0f m");
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Status: %s", silentAimStatus.c_str());
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "ATENCAO: Teste cada abordagem para ver qual funciona!");
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Funcoes ESP")) {
                    ImGui::Checkbox(OBFUSCATE("Ativar ESP"), &espEnabled);
                    if (espEnabled) {
                        ImGui::Checkbox(OBFUSCATE("ESP Box"), &espBox);
                        if (espBox) {
                            // Removido opcao de outline
                        }
                        ImGui::Checkbox(OBFUSCATE("ESP Vida (Health)"), &espHealth);
                        ImGui::Checkbox(OBFUSCATE("ESP Nome"), &espName);
                        ImGui::Checkbox(OBFUSCATE("ESP Distancia"), &espDistance);
                        ImGui::Checkbox(OBFUSCATE("ESP Linha"), &espLine);
                        // ImGui::Checkbox("Esqueleto (Bones)", &espSkeleton); // DESATIVADO - POSSIVEL CAUSA DE BAN

                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Filtro de Distancia ESP");
                        ImGui::SliderFloat(OBFUSCATE("Maxima"), &espMaxDistance, 10.0f, 100.0f, "%.0f m");
                    }
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Estilos")) {
                    ImGui::Text("Configuracoes de Estilo e Cores");
                    ImGui::Separator();
                    
                    const char* boxStyles[] = { "Box Completa", "Box Cantos (Corners)" };
                    ImGui::Combo("Estilo da Box", &espBoxStyle, boxStyles, IM_ARRAYSIZE(boxStyles));
                    
                    ImGui::ColorEdit4("Cor Inimigo", espColorEnemy, ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4("Cor Aliado", espColorAlly, ImGuiColorEditFlags_NoInputs);
                    ImGui::ColorEdit4("Cor FOV", fovColor, ImGuiColorEditFlags_NoInputs);
                    
                    ImGui::Separator();
                    ImGui::Text("Preview Visual:");
                    ImGui::BeginChild("PreviewArea", ImVec2(0, 200), true);
                    ImDrawList* drawList = ImGui::GetWindowDrawList();
                    ImVec2 p = ImGui::GetCursorScreenPos();
                    
                    float previewW = 80.0f;
                    float previewH = 160.0f;
                    ImVec2 center(p.x + ImGui::GetWindowWidth() / 2.0f, p.y + 100.0f);
                    ImVec2 pTopLeft(center.x - previewW / 2.0f, center.y - previewH / 2.0f);
                    ImVec2 pBottomRight(center.x + previewW / 2.0f, center.y + previewH / 2.0f);
                    
                    ImU32 prevColor = ImGui::ColorConvertFloat4ToU32(ImVec4(espColorEnemy[0], espColorEnemy[1], espColorEnemy[2], espColorEnemy[3]));
                    
                    if (espBox) {
                        if (espBoxStyle == 0) {
                            drawList->AddRect(pTopLeft, pBottomRight, prevColor, 0.0f, 0, 2.0f);
                        } else {
                            float lineW = previewW * 0.25f;
                            float lineH = previewH * 0.25f;
                            float thick = 2.0f;
                            
                            // Top-Left
                            drawList->AddLine(ImVec2(pTopLeft.x, pTopLeft.y), ImVec2(pTopLeft.x + lineW, pTopLeft.y), prevColor, thick);
                            drawList->AddLine(ImVec2(pTopLeft.x, pTopLeft.y), ImVec2(pTopLeft.x, pTopLeft.y + lineH), prevColor, thick);
                            // Top-Right
                            drawList->AddLine(ImVec2(pBottomRight.x, pTopLeft.y), ImVec2(pBottomRight.x - lineW, pTopLeft.y), prevColor, thick);
                            drawList->AddLine(ImVec2(pBottomRight.x, pTopLeft.y), ImVec2(pBottomRight.x, pTopLeft.y + lineH), prevColor, thick);
                            // Bottom-Left
                            drawList->AddLine(ImVec2(pTopLeft.x, pBottomRight.y), ImVec2(pTopLeft.x + lineW, pBottomRight.y), prevColor, thick);
                            drawList->AddLine(ImVec2(pTopLeft.x, pBottomRight.y), ImVec2(pTopLeft.x, pBottomRight.y - lineH), prevColor, thick);
                            // Bottom-Right
                            drawList->AddLine(ImVec2(pBottomRight.x, pBottomRight.y), ImVec2(pBottomRight.x - lineW, pBottomRight.y), prevColor, thick);
                            drawList->AddLine(ImVec2(pBottomRight.x, pBottomRight.y), ImVec2(pBottomRight.x, pBottomRight.y - lineH), prevColor, thick);
                        }
                    }
                    
                    if (espName) {
                        ImFont* font = ImGui::GetFont();
                        float fontSize = ImGui::GetFontSize() * 1.3f;
                        const char* text = "Inimigo [Preview]";
                        ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
                        float espacoExtra = previewH * 0.15f;
                        drawList->AddText(font, fontSize, ImVec2(center.x - textSize.x / 2.0f, pTopLeft.y - textSize.y - espacoExtra), IM_COL32(255, 255, 255, 255), text);
                    }
                    
                    if (espHealth) {
                        float hpWidth = 4.0f;
                        ImVec2 hpTopLeft(pTopLeft.x - hpWidth - 4.0f, pTopLeft.y);
                        ImVec2 hpBottomRight(hpTopLeft.x + hpWidth, pBottomRight.y);
                        drawList->AddRectFilled(hpTopLeft, hpBottomRight, IM_COL32(0, 0, 0, 150));
                        ImVec2 hpFillTopLeft(hpTopLeft.x, pBottomRight.y - (previewH * 0.8f)); // 80% HP
                        drawList->AddRectFilled(hpFillTopLeft, hpBottomRight, IM_COL32(0, 255, 0, 255));
                    }
                    
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Misc")) {
                    ImGui::Checkbox("InfJump (Pulo Infinito)", &infJumpEnabled);
                    if (infJumpEnabled) {
                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Um botao flutuante aparecera na tela.");
                        ImGui::SliderFloat("Unidades do Pulo (Altura)", &infJumpStep, 0.1f, 10.0f, "%.1f m");
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
                } // isDebugMode
                ImGui::EndTabBar();
            } // BeginTabBar
        } // masterSwitch
        } // Fim if (ImGui::Begin(...))
        ImGui::End();
        
        ImGui::PopStyleVar(); // Pop da estilizacao
    } // Fim Menu::render()

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
        if (!masterSwitch) return; // Se o painel esta desativado, nao fazemos nada

        std::vector<PlayerEntity> localEntities;
        {
            std::lock_guard<std::mutex> lock(sharedState.mtx);
            localEntities = sharedState.entities;
        }

        if (localEntities.empty()) {
            masterSwitch = false; // Desativa tudo caso nao haja entidades (Lobby)
            return;
        }

        static void* get_mainMethod = nullptr;
        static void* worldToScreenMethod = nullptr;
        
        static void* componentKlass = nullptr;
        static void* getTransformMethod = nullptr;
        static void* transformKlass = nullptr;
        static void* getPosMethod = nullptr;
        static void* setPosMethod = nullptr;
        static void* getRotMethod = nullptr;
        static void* setRotMethod = nullptr;
        static void* quatKlass = nullptr;
        static void* lookRotMethod = nullptr;
        static void* slerpMethod = nullptr;
        static void* setAimRotationMethod = nullptr;
        static void* isFiringMethod = nullptr;
        static void* getHeadTFMethod = nullptr;
        static void* getHipTFMethod = nullptr;
        static void* getLeftAnkleTFMethod = nullptr;
        static void* getRightAnkleTFMethod = nullptr;
        static void* getLeftToeTFMethod = nullptr;
        static void* getRightToeTFMethod = nullptr;
        static void* getComponentMethod = nullptr;
        static void* isVisibleMethod = nullptr;
        static void* animatorTypeObject = nullptr;
        static void* getBoneTransformMethod = nullptr;
        static void* getAvatarMethod = nullptr;
        static void* avatarField = nullptr;
        static void* getAnimatorMethod = nullptr;
        static void* getGoComponentMethod = nullptr;
        static void* getComponentNormalMethod = nullptr;
        static void* getNewAnimComponentMethod = nullptr;

        static bool methodsSearched = false;

        struct CachedAnimatorInfo {
            void* animatorObj = nullptr;
            bool hasAttempted = false;
            std::unordered_map<int, void*> boneTransforms;
        };
        static std::unordered_map<void*, CachedAnimatorInfo> cachedAnimators;

        // Limpa cache de entidades que nao existem mais para evitar memory leak
        for (auto it = cachedAnimators.begin(); it != cachedAnimators.end(); ) {
            bool found = false;
            for (const auto& e : localEntities) {
                if (e.obj == it->first) { found = true; break; }
            }
            if (!found) it = cachedAnimators.erase(it);
            else ++it;
        }

        struct Vector3Args { float x, y, z; };

        /* // DESATIVADO - POSSIVEL CAUSA DE BAN
        auto getBonePosCached = [](CachedAnimatorInfo* info, int boneId, bool& success) -> Vector3Args {
            success = false;
            Vector3Args pos = {0, 0, 0};
            if (!info || !info->animatorObj || !getBoneTransformMethod || !getPosMethod) return pos;

            void* boneTransform = nullptr;
            auto bit = info->boneTransforms.find(boneId);
            if (bit != info->boneTransforms.end()) {
                boneTransform = bit->second;
            } else {
                void* exc = nullptr;
                void* args[1] = { &boneId };
                boneTransform = Il2Cpp::runtime_invoke(getBoneTransformMethod, info->animatorObj, args, &exc);
                if (exc == nullptr && boneTransform != nullptr) {
                    info->boneTransforms[boneId] = boneTransform;
                }
            }
            
            if (boneTransform) {
                void* exc = nullptr;
                void* posObj = Il2Cpp::runtime_invoke(getPosMethod, boneTransform, nullptr, &exc);
                if (posObj && !exc) {
                    pos = *(Vector3Args*)((uintptr_t)posObj + 0x10);
                    success = true;
                }
            }
            return pos;
        };

        auto getBonePos = [](void* animatorObj, int boneId, bool& success) -> Vector3Args {
            success = false;
            Vector3Args pos = {0, 0, 0};
            if (!animatorObj || !getBoneTransformMethod || !getPosMethod) return pos;

            void* exc = nullptr;
            void* args[1] = { &boneId };
            void* boneTransform = Il2Cpp::runtime_invoke(getBoneTransformMethod, animatorObj, args, &exc);
            
            if (boneTransform && !exc) {
                void* posObj = Il2Cpp::runtime_invoke(getPosMethod, boneTransform, nullptr, &exc);
                if (posObj && !exc) {
                    pos = *(Vector3Args*)((uintptr_t)posObj + 0x10);
                    success = true;
                }
            }
            return pos;
        };

        auto getTFPos = [](void* entityObj, void* method, bool& success) -> Vector3Args {
            success = false;
            Vector3Args pos = {0, 0, 0};
            if (!entityObj || !method || !getPosMethod) return pos;
            
            void* exc = nullptr;
            void* tf = Il2Cpp::runtime_invoke(method, entityObj, nullptr, &exc);
            if (tf && !exc) {
                void* posObj = Il2Cpp::runtime_invoke(getPosMethod, tf, nullptr, &exc);
                if (posObj && !exc) {
                    pos = *(Vector3Args*)((uintptr_t)posObj + 0x10);
                    success = true;
                }
            }
            return pos;
        };
        */ // FIM DESATIVADO

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
                setPosMethod = Il2Cpp::GetMethodRecursively(transformKlass, "set_position", 1);
                getRotMethod = Il2Cpp::GetMethodRecursively(transformKlass, "get_rotation", 0);
                setRotMethod = Il2Cpp::GetMethodRecursively(transformKlass, "set_rotation", 1);
            }
            
            quatKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Quaternion");
            if (quatKlass) {
                lookRotMethod = Il2Cpp::GetMethodRecursively(quatKlass, "LookRotation", 1);
                slerpMethod = Il2Cpp::GetMethodRecursively(quatKlass, "Slerp", 3);
            }

            // Cache do metodo SetAimRotation no Player
            if ((!setAimRotationMethod) && sharedState.localPlayerObj) {
                void* playerKlass = Il2Cpp::object_get_class(sharedState.localPlayerObj);
                if (playerKlass) {
                    setAimRotationMethod = Il2Cpp::GetMethodRecursively(playerKlass, "SetAimRotation", 2);
                }
            }
            // Busca metodo GetHeadTF para posicao real da cabeca (diferente do skeleton/bone que causa ban)
            if ((!getHeadTFMethod) && sharedState.localPlayerObj) {
                void* playerKlass = Il2Cpp::object_get_class(sharedState.localPlayerObj);
                if (playerKlass) {
                    getHeadTFMethod = Il2Cpp::GetMethodRecursively(playerKlass, "GetHeadTF", 0);
                }
            }
            // DESATIVADO - Skeleton/Bone causa ban (nao reativar)
            getHipTFMethod = nullptr;
            getLeftAnkleTFMethod = nullptr;
            getRightAnkleTFMethod = nullptr;
            getLeftToeTFMethod = nullptr;
            getRightToeTFMethod = nullptr;
            getBoneTransformMethod = nullptr;
            getAnimatorMethod = nullptr;
            getAvatarMethod = nullptr;
            getComponentMethod = nullptr;
            getGoComponentMethod = nullptr;
            getComponentNormalMethod = nullptr;
            isVisibleMethod = nullptr;
            isFiringMethod = nullptr;
            getNewAnimComponentMethod = nullptr;

            methodsSearched = true;
        }

        if (!get_mainMethod || !worldToScreenMethod) return;

        void* exc = nullptr;
        void* mainCamera = Il2Cpp::runtime_invoke(get_mainMethod, nullptr, nullptr, &exc);
        if (!mainCamera || exc) return;

        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        ImVec2 screenCenter(screenSize.x / 2.0f, screenSize.y / 2.0f);

        // Desenha o circulo de FOV do Aimbot
        if (aimbotDrawFov || (silentAimEnabled && silentAimDrawDebug)) {
            drawList->AddCircle(screenCenter, aimbotFov, ImGui::ColorConvertFloat4ToU32(ImVec4(fovColor[0], fovColor[1], fovColor[2], fovColor[3])), 64, 1.0f);
        }

        if (!espEnabled && !aimbotEnabled && !silentAimEnabled) return;

        struct QuaternionArgs { float x, y, z, w; };

        float closestAimbotDist = FLT_MAX;
        ImVec2 bestTargetPos = screenCenter;
        Vector3Args bestTargetWorldPosHead = {0, 0, 0};
        Vector3Args bestTargetWorldPosChest = {0, 0, 0};
        bool foundAimbotTarget = false;
        void* bestTargetObj = nullptr;

        for (const auto& entity : localEntities) {
            // Ignora se estiver morto e o filtro de mortos estiver ativo (opcional, aqui pularemos mortos sempre pra nao poluir)
            if (!entity.isAlive()) continue;
            
            // Calcula a distância 3D entre o jogador local e a entidade
            float distance3D = 0.0f;
            if (sharedState.localPlayerObj) {
                void* localTransform = Il2Cpp::runtime_invoke(getTransformMethod, sharedState.localPlayerObj, nullptr, &exc);
                if (localTransform && !exc) {
                    void* localPosObj = Il2Cpp::runtime_invoke(getPosMethod, localTransform, nullptr, &exc);
                    if (localPosObj && !exc) {
                        Vector3Args localPos = *(Vector3Args*)((uintptr_t)localPosObj + 0x10);
                        distance3D = sqrt(pow(entity.position.x - localPos.x, 2) + 
                                          pow(entity.position.y - localPos.y, 2) + 
                                          pow(entity.position.z - localPos.z, 2));
                    }
                }
            }

            // Box positions (pés até a cabeça)
            Vector3Args posFeet = {entity.position.x, entity.position.y, entity.position.z};
            
            // Usa GetHeadTF para posicao real da cabeca do enemy
            Vector3Args posHead = {entity.position.x, entity.position.y + 1.41f, entity.position.z};
            if (getHeadTFMethod && entity.obj) {
                void* headTransform = Il2Cpp::runtime_invoke(getHeadTFMethod, entity.obj, nullptr, &exc);
                if (headTransform && !exc) {
                    void* headPosObj = Il2Cpp::runtime_invoke(getPosMethod, headTransform, nullptr, &exc);
                    if (headPosObj && !exc) {
                        posHead = *(Vector3Args*)((uintptr_t)headPosObj + 0x10);
                    }
                }
            }

            void* argsFeet[1] = { &posFeet };
            void* argsHead[1] = { &posHead };

            void* w2sFeetObj = Il2Cpp::runtime_invoke(worldToScreenMethod, mainCamera, argsFeet, &exc);
            if (!w2sFeetObj || exc) continue;
            Vector3Args w2sFeet = *(Vector3Args*)((uintptr_t)w2sFeetObj + 0x10);

            void* w2sHeadObj = Il2Cpp::runtime_invoke(worldToScreenMethod, mainCamera, argsHead, &exc);
            if (!w2sHeadObj || exc) continue;
            Vector3Args w2sHead = *(Vector3Args*)((uintptr_t)w2sHeadObj + 0x10);

            // Filtro de Distância (Impede ESP e Aimbot de bugar a tela em inimigos muito longe)
            if (distance3D > espMaxDistance) {
                continue;
            }

            // Verifica se esta na frente da camera (Z > 0.0f)
            if (w2sFeet.z > 0.0f) {
                // Converte as coordenadas de tela do Unity (origem bottom-left) para as do ImGui (origem top-left)
                float yFeet = screenSize.y - w2sFeet.y;

                float xHead = w2sHead.x;
                float yHead = screenSize.y - w2sHead.y;

                float boxHeight = yFeet - yHead;
                // Previne calculos absurdos caso as coordenadas estejam distorcidas
                if (boxHeight < 0) boxHeight = -boxHeight; 
                float boxWidth = boxHeight / 2.0f; // Proporcao basica de jogador (largura é metade da altura)

                // Adicionando espaçamento extra para não ficar muito colado no corpo
                float paddingX = boxWidth * 0.35f;
                float paddingY = boxHeight * 0.15f;

                ImVec2 topLeft(xHead - (boxWidth / 2.0f) - paddingX, yHead - paddingY);
                ImVec2 bottomRight(xHead + (boxWidth / 2.0f) + paddingX, yFeet + paddingY);

                ImU32 color = (entity.alignment == Alignment::ENEMY) ? ImGui::ColorConvertFloat4ToU32(ImVec4(espColorEnemy[0], espColorEnemy[1], espColorEnemy[2], espColorEnemy[3])) : ImGui::ColorConvertFloat4ToU32(ImVec4(espColorAlly[0], espColorAlly[1], espColorAlly[2], espColorAlly[3]));

                // Lógica de FOV - Detecta alvos para Aimbot ou Silent Aim
                bool needsTarget = aimbotEnabled || silentAimEnabled;
                if (needsTarget) {
                    // Se não for para mirar em aliados e for um aliado, pula
                    if (!aimbotTargetAllies && entity.alignment == Alignment::ALLY) {
                        // Ignora para o Aimbot, mas continua desenhando ESP se estiver ativado
                    } else {
                        bool visible = true;
                        /* // DESATIVADO - POSSIVEL CAUSA DE BAN (isVisibleMethod)
                        if (aimbotVisibilityCheck && isVisibleMethod && entity.obj) {
                            void* exc = nullptr;
                            void* isVisObj = Il2Cpp::runtime_invoke(isVisibleMethod, entity.obj, nullptr, &exc);
                            if (isVisObj && !exc) {
                                visible = *(bool*)((uintptr_t)isVisObj + 0x10);
                            }
                        }
                        */ // FIM DESATIVADO

                        if (visible) {
                            float distToCenter = sqrt(pow(xHead - screenCenter.x, 2) + pow(yHead - screenCenter.y, 2));
                            if (distToCenter <= aimbotFov && distToCenter < closestAimbotDist) {
                            closestAimbotDist = distToCenter;
                            bestTargetPos = ImVec2(xHead, yHead);
                            bestTargetWorldPosHead = posHead;
                            
                            // Estimar peito
                            float height = posHead.y - posFeet.y;
                            bestTargetWorldPosChest = { posHead.x, posHead.y - (height * 0.2f), posHead.z };

                            // Tentar obter do esqueleto desativado, usando apenas estimativa matemática rápida e precisa
                            // (O método Animator do Unity é lento e muitas vezes retorna null em versões recentes do jogo)
                            
                            foundAimbotTarget = true;
                            bestTargetObj = entity.obj;
                            
                            // Guarda informacoes de Debug
                            aimbotTargetName = entity.name;
                            aimbotTargetDistFOV = distToCenter;
                            aimbotTargetDist3D = entity.distanceToLocal;
                        }
                    }
                }
            }

                if (espEnabled) {
                    if (espBox) {
                        // ... code omitted ...
                        if (espBoxStyle == 0) {
                            // Desenha o retangulo (Box ESP) completa
                            drawList->AddRect(topLeft, bottomRight, color, 0.0f, 0, 2.0f);
                        } else {
                            // Desenha apenas os cantos (Corners)
                            float lineW = (bottomRight.x - topLeft.x) * 0.25f;
                            float lineH = (bottomRight.y - topLeft.y) * 0.25f;
                            float thick = 2.0f;
                            
                            // Top-Left
                            drawList->AddLine(ImVec2(topLeft.x, topLeft.y), ImVec2(topLeft.x + lineW, topLeft.y), color, thick);
                            drawList->AddLine(ImVec2(topLeft.x, topLeft.y), ImVec2(topLeft.x, topLeft.y + lineH), color, thick);
                            // Top-Right
                            drawList->AddLine(ImVec2(bottomRight.x, topLeft.y), ImVec2(bottomRight.x - lineW, topLeft.y), color, thick);
                            drawList->AddLine(ImVec2(bottomRight.x, topLeft.y), ImVec2(bottomRight.x, topLeft.y + lineH), color, thick);
                            // Bottom-Left
                            drawList->AddLine(ImVec2(topLeft.x, bottomRight.y), ImVec2(topLeft.x + lineW, bottomRight.y), color, thick);
                            drawList->AddLine(ImVec2(topLeft.x, bottomRight.y), ImVec2(topLeft.x, bottomRight.y - lineH), color, thick);
                            // Bottom-Right
                            drawList->AddLine(ImVec2(bottomRight.x, bottomRight.y), ImVec2(bottomRight.x - lineW, bottomRight.y), color, thick);
                            drawList->AddLine(ImVec2(bottomRight.x, bottomRight.y), ImVec2(bottomRight.x, bottomRight.y - lineH), color, thick);
                        }
                        
                        // DEBUG: Mostra o teamId acima da box
                        char debugText[64];
                        snprintf(debugText, sizeof(debugText), "Team: %d", entity.teamId);
                        drawList->AddText(ImVec2(topLeft.x, topLeft.y - 15.0f), IM_COL32(255, 255, 255, 255), debugText);
                    }
                    
                    if (espHealth) {
                        // Desenha a barra de vida ao lado
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

                // DESATIVADO TEMPORARIAMENTE - POSSIVEL CAUSA DE BAN
                // if (espSkeleton) {
                //     if (!entity.obj) {
                //         drawList->AddText(topLeft, IM_COL32(255, 0, 0, 255), "entity.obj == null");
                //     } else if (!getComponentMethod) {
                //         drawList->AddText(topLeft, IM_COL32(255, 0, 0, 255), "getComponentMethod == null");
                //     } else if (!animatorTypeObject) {
                //         drawList->AddText(topLeft, IM_COL32(255, 0, 0, 255), "animatorTypeObject == null");
                //     } else if (!getBoneTransformMethod) {
                //         drawList->AddText(topLeft, IM_COL32(255, 0, 0, 255), "getBoneTransformMethod == null");
                //     } else {
                //         CachedAnimatorInfo* info = &cachedAnimators[entity.obj];
                //         if (!info->animatorObj && (ImGui::GetFrameCount() % 60 == 0) && animatorTypeObject) {
                //             void* exc = nullptr;
                //             void* animObj = nullptr;
                //             
                //             if (getComponentNormalMethod) {
                //                 void* args[1] = { animatorTypeObject };
                //                 animObj = Il2Cpp::runtime_invoke(getComponentNormalMethod, entity.obj, args, &exc);
                //                 if (exc) animObj = nullptr;
                //             }
                //             
                //             if (!animObj) {
                //                 void* getCompsMethod = Il2Cpp::GetMethodRecursively(Il2Cpp::object_get_class(entity.obj), "GetComponentsInChildren", 2);
                //                 if (getCompsMethod) {
                //                     bool includeInactive = true;
                //                     void* args[2] = { animatorTypeObject, &includeInactive };
                //                     void* arrayObj = Il2Cpp::runtime_invoke(getCompsMethod, entity.obj, args, &exc);
                //                     if (arrayObj && !exc) {
                //                         uint32_t length = *(uint32_t*)((uintptr_t)arrayObj + 0x18);
                //                         if (length > 0) {
                //                             animObj = *(void**)((uintptr_t)arrayObj + 0x20);
                //                         }
                //                     }
                //                 }
                //             }
                //             
                //             if (!animObj && getAnimatorMethod) {
                //                 animObj = Il2Cpp::runtime_invoke(getAnimatorMethod, entity.obj, nullptr, &exc);
                //                 if (exc) animObj = nullptr;
                //             }
                //             
                //             if (!animObj && getComponentMethod) {
                //                 exc = nullptr;
                //                 bool includeInactive = true;
                //                 void* args[2] = { animatorTypeObject, &includeInactive };
                //                 animObj = Il2Cpp::runtime_invoke(getComponentMethod, entity.obj, args, &exc);
                //                 if (exc) {
                //                     animObj = nullptr;
                //                     info->hasAttempted = true;
                //                 }
                //             }
                //
                //             if (!animObj && getGoComponentMethod) {
                //                 exc = nullptr;
                //                 void* goMethod = Il2Cpp::GetMethodRecursively(Il2Cpp::object_get_class(entity.obj), "get_gameObject", 0);
                //                 if (goMethod) {
                //                     void* goObj = Il2Cpp::runtime_invoke(goMethod, entity.obj, nullptr, &exc);
                //                     if (goObj && !exc) {
                //                         bool includeInactive = true;
                //                         void* args[2] = { animatorTypeObject, &includeInactive };
                //                         animObj = Il2Cpp::runtime_invoke(getGoComponentMethod, goObj, args, &exc);
                //                         if (exc) {
                //                             animObj = nullptr;
                //                             info->hasAttempted = true;
                //                         }
                //                     }
                //                 }
                //             }
                //
                //             if (!animObj && getNewAnimComponentMethod) {
                //                 exc = nullptr;
                //                 void* newAnimCompObj = Il2Cpp::runtime_invoke(getNewAnimComponentMethod, entity.obj, nullptr, &exc);
                //                 if (newAnimCompObj && !exc) {
                //                     bool includeInactive = true;
                //                     void* args[2] = { animatorTypeObject, &includeInactive };
                //                     animObj = Il2Cpp::runtime_invoke(getComponentMethod, newAnimCompObj, args, &exc);
                //                     if (exc) animObj = nullptr;
                //                 }
                //             }
                //             
                //             if (!animObj && (getComponentMethod || getGoComponentMethod)) {
                //                 void* avatarObj = nullptr;
                //                 if (getAvatarMethod) {
                //                     avatarObj = Il2Cpp::runtime_invoke(getAvatarMethod, entity.obj, nullptr, &exc);
                //                 } else if (avatarField) {
                //                     Il2Cpp::field_get_value(entity.obj, avatarField, &avatarObj);
                //                 }
                //                 
                //                 if (avatarObj) {
                //                     exc = nullptr;
                //                     bool includeInactive = true;
                //                     void* args[2] = { animatorTypeObject, &includeInactive };
                //                     void* avatarKlass = Il2Cpp::object_get_class(avatarObj);
                //                     if (avatarKlass && Il2Cpp::IsSubclassOf(avatarKlass, "GameObject") && getGoComponentMethod) {
                //                         animObj = Il2Cpp::runtime_invoke(getGoComponentMethod, avatarObj, args, &exc);
                //                     } else if (getComponentMethod) {
                //                         animObj = Il2Cpp::runtime_invoke(getComponentMethod, avatarObj, args, &exc);
                //                     }
                //                     if (exc) animObj = nullptr;
                //                 }
                //             }
                //             
                //             info->animatorObj = animObj;
                //         }
                //         
                //         if (!info->animatorObj) {
                //         } else {
                //             bool hasBone[25] = {false};
                //             ImVec2 boneScreen[25];
                //             
                //             int requiredBones[] = { 0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
                //         
                //         for (int i = 0; i < sizeof(requiredBones) / sizeof(int); ++i) {
                //             int bId = requiredBones[i];
                //             bool success = false;
                //             Vector3Args wPos = getBonePosCached(info, bId, success);
                //             if (success) {
                //                 void* args[1] = { &wPos };
                //                 void* exc2 = nullptr;
                //                 void* w2sObj = Il2Cpp::runtime_invoke(worldToScreenMethod, mainCamera, args, &exc2);
                //                 if (w2sObj && !exc2) {
                //                     Vector3Args sPos = *(Vector3Args*)((uintptr_t)w2sObj + 0x10);
                //                     if (sPos.z > 0) {
                //                         boneScreen[bId] = ImVec2(sPos.x, screenSize.y - sPos.y);
                //                         hasBone[bId] = true;
                //                     }   
                //                 }
                //             }
                //         }
                //         
                //         auto drawBoneLine = [&](int b1, int b2) {
                //             if (hasBone[b1] && hasBone[b2]) {
                //                 drawList->AddLine(boneScreen[b1], boneScreen[b2], color, 1.5f);
                //             }
                //         };
                //         
                //         drawBoneLine(10, 9);  // Head -> Neck
                //         drawBoneLine(9, 7);   // Neck -> Spine
                //         drawBoneLine(7, 0);   // Spine -> Hips
                //         
                //         drawBoneLine(9, 11);  // Neck -> LeftShoulder
                //         drawBoneLine(11, 13); // LeftShoulder -> LeftUpperArm
                //         drawBoneLine(13, 15); // LeftUpperArm -> LeftLowerArm
                //         drawBoneLine(15, 17); // LeftLowerArm -> LeftHand
                //         
                //         drawBoneLine(9, 12);  // Neck -> RightShoulder
                //         drawBoneLine(12, 14); // RightShoulder -> RightUpperArm
                //         drawBoneLine(14, 16); // RightUpperArm -> RightLowerArm
                //         drawBoneLine(16, 18); // RightLowerArm -> RightHand
                //         
                //         drawBoneLine(0, 1);   // Hips -> LeftUpperLeg
                //         drawBoneLine(1, 3);   // LeftUpperLeg -> LeftLowerLeg
                //         drawBoneLine(3, 5);   // LeftLowerLeg -> LeftFoot
                //         
                //         drawBoneLine(0, 2);   // Hips -> RightUpperLeg
                //         drawBoneLine(2, 4);   // RightUpperLeg -> RightLowerLeg
                //         drawBoneLine(4, 6);   // RightLowerLeg -> RightFoot
                //         }
                //     }
                // }

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

                    ImFont* font = ImGui::GetFont();
                    float fontSize = ImGui::GetFontSize() * 1.3f; // Aumenta o tamanho do texto em 30%
                    
                    // Calcula o tamanho real do texto com o novo tamanho de fonte
                    ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, textBuffer);
                    
                    // Ajusta a posicao do nome para ficar mais acima do esp box
                    float espacoExtraAcima = boxHeight * 0.15f; 
                    ImVec2 textPos(xHead - textSize.x / 2.0f, topLeft.y - textSize.y - espacoExtraAcima);

                    drawList->AddText(font, fontSize, textPos, IM_COL32(255, 255, 255, 255), textBuffer);
                }
            }
        }
    }

        // DEBUG ESP/SKELETON DESATIVADO TEMPORARIAMENTE
        // if (espEnabled && isDebugMode) {
        //     ImGui::SetNextWindowPos(ImVec2(screenSize.x - 320, 10), ImGuiCond_Once);
        //     ImGui::SetNextWindowSize(ImVec2(300, 280), ImGuiCond_Once);
        //     ImGui::Begin("Debug ESP / Skeleton", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        //     ImGui::TextColored(ImVec4(1, 1, 0, 1), "=== DEBUG ESP / SKELETON ===");
        //     ImGui::Separator();
        //
        //     ImGui::TextColored(ImVec4(0, 1, 0, 1), "Metodos IL2CPP:");
        //     ImGui::Text("getBoneTransformMethod: %s", getBoneTransformMethod ? "OK" : "NULL");
        //     ImGui::Text("animatorTypeObject: %s", animatorTypeObject ? "OK" : "NULL");
        //     ImGui::Text("getPosMethod: %s", getPosMethod ? "OK" : "NULL");
        //     ImGui::Text("worldToScreenMethod: %s", worldToScreenMethod ? "OK" : "NULL");
        //     ImGui::Text("getComponentMethod: %s", getComponentMethod ? "OK" : "NULL");
        //     ImGui::Text("getAnimatorMethod: %s", getAnimatorMethod ? "OK" : "NULL");
        //
        //     ImGui::Separator();
        //     ImGui::TextColored(ImVec4(0, 1, 0, 1), "Cache de Animators:");
        //     int cachedCount = 0;
        //     for (auto& pair : cachedAnimators) {
        //         if (pair.second.animatorObj) cachedCount++;
        //     }
        //     ImGui::Text("Animator cacheado: %d / %d", cachedCount, (int)cachedAnimators.size());
        //
        //     ImGui::Separator();
        //     ImGui::TextColored(ImVec4(1, 1, 0, 1), "Bones IDs:");
        //     ImGui::Text("Head=0 Neck=9 Spine=7 Hips=0");
        //     ImGui::Text("L.Shoulder=11 L.UpperArm=13 L.LowerArm=15 L.Hand=17");
        //     ImGui::Text("R.Shoulder=12 R.UpperArm=14 R.LowerArm=16 R.Hand=18");
        //     ImGui::Text("L.UpperLeg=1 L.LowerLeg=3 L.Foot=5");
        //     ImGui::Text("R.UpperLeg=2 R.LowerLeg=4 R.Foot=6");
        //
        //     ImGui::Separator();
        //     ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Conexao:");
        //     ImGui::Text("Cabeca->Pescoco(10->9) Pescoco->Spine(9->7)");
        //     ImGui::Text("Spine->Hips(7->0) Pescoco->Ombros->Bracos");
        //     ImGui::Text("Hips->Pernas");
        //
        //     ImGui::Separator();
        //     ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Se skeleton nao desenha:");
        //     ImGui::Text("- GetBoneTransform nao encontrado");
        //     ImGui::Text("- Animator nao achado na entity");
        //     ImGui::Text("- Bones nao convertidos pra screen");
        //
        //     ImGui::End();
        // }

        // Desenhar linha do Aimbot para o alvo travado
        aimbotHasTarget = foundAimbotTarget;
        if (!foundAimbotTarget) {
            aimbotTargetName = "Nenhum";
            aimbotTargetDistFOV = 0.0f;
            aimbotTargetDist3D = 0.0f;
            aimbotErrorLog = "Buscando alvos no FOV...";
        }

        // Debug visualization para alvos (sempre mostra se encontrou)
        if (foundAimbotTarget) {
            drawList->AddLine(screenCenter, bestTargetPos, IM_COL32(255, 0, 0, 200), 1.0f);
            drawList->AddCircle(bestTargetPos, 5.0f, IM_COL32(255, 0, 0, 200), 12, 1.0f);
        }

        // ==================== AIMBOT TRADICIONAL (Ao Atirar) ====================
        if (aimbotEnabled && foundAimbotTarget) {
            bool shouldAim = true;
            if (aimbotMode == 0) { // Tradicional (Ao Atirar)
                shouldAim = false;
                if (sharedState.localPlayerObj) {
                    bool isFiring = *(bool*)((uintptr_t)sharedState.localPlayerObj + 0x2E8);
                    aimbotErrorLog = isFiring ? "Detectou tiro (isFiring=true)" : "Sem tiro (isFiring=false)";
                    if (isFiring) {
                        shouldAim = true;
                    }
                } else {
                    aimbotErrorLog = "localPlayerObj eh NULL";
                }
                if (!shouldAim) {
                    // Continua para verificar Silent Aim
                } else {
                    // Lógica da Mira Magnética (Independente de atirar ou delay)
                    if (aimbotMagnetic && getTransformMethod && getPosMethod && getRotMethod) {
                        void* cameraTransform = Il2Cpp::runtime_invoke(getTransformMethod, mainCamera, nullptr, &exc);
                        if (cameraTransform && !exc) {
                            void* targetTransform = Il2Cpp::runtime_invoke(getTransformMethod, bestTargetObj, nullptr, &exc);
                            if (targetTransform && !exc) {
                                void* getForwardMethod = Il2Cpp::GetMethodRecursively(transformKlass, "get_forward", 0);
                                if (getForwardMethod) {
                                    void* forwardObj = Il2Cpp::runtime_invoke(getForwardMethod, cameraTransform, nullptr, &exc);
                                    void* camPosObj = Il2Cpp::runtime_invoke(getPosMethod, cameraTransform, nullptr, &exc);
                                    if (forwardObj && camPosObj && !exc) {
                                        Vector3Args forward = *(Vector3Args*)((uintptr_t)forwardObj + 0x10);
                                        Vector3Args camPos = *(Vector3Args*)((uintptr_t)camPosObj + 0x10);
                                        
                                        float magDist = aimbotTargetDist3D;
                                        if (magDist < 1.0f) magDist = 1.0f;
                                        
                                        Vector3Args magneticPos = {
                                            camPos.x + forward.x * magDist,
                                            camPos.y + forward.y * magDist,
                                            camPos.z + forward.z * magDist
                                        };
                                        
                                        float heightOffset = bestTargetWorldPosHead.y - bestTargetWorldPosChest.y;
                                        if (aimbotHitbox == 0) {
                                            magneticPos = bestTargetWorldPosHead;
                                        } else if (aimbotHitbox == 1) {
                                            magneticPos = bestTargetWorldPosChest;
                                            magneticPos.y += (heightOffset * 0.5f);
                                        } else {
                                            magneticPos = bestTargetWorldPosChest;
                                        } 

                                        void* setPosMethod = Il2Cpp::GetMethodRecursively(transformKlass, "set_position", 1);
                                        if (setPosMethod) {
                                            void* argsSetPos[1] = { &magneticPos };
                                            Il2Cpp::runtime_invoke(setPosMethod, targetTransform, argsSetPos, &exc);
                                            if (!exc) {
                                                aimbotErrorLog = "Inimigo puxado pela Mira Magnetica (Sempre Ativo)!";
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    float deltaTime = ImGui::GetIO().DeltaTime;
                    
                    bool delayPassed = false;
                    if (shouldAim && foundAimbotTarget) {
                        wasAimingLastFrame = true;
                        
                        if (aimbotTargetTimeMap.find(bestTargetObj) == aimbotTargetTimeMap.end()) {
                            aimbotTargetTimeMap[bestTargetObj] = 0.0f;
                        }
                        
                        aimbotTargetTimeMap[bestTargetObj] += (deltaTime * 1000.0f);
                        
                        if (aimbotTargetTimeMap[bestTargetObj] >= aimbotSmoothTimeMs) {
                            delayPassed = true;
                        }
                    } else {
                        wasAimingLastFrame = false;
                    }

                    if (shouldAim && foundAimbotTarget && getTransformMethod && getPosMethod && getRotMethod && setRotMethod && lookRotMethod) {
                        
                        for (auto it = aimbotTargetTimeMap.begin(); it != aimbotTargetTimeMap.end(); ) {
                            if (it->first != bestTargetObj) it = aimbotTargetTimeMap.erase(it);
                            else ++it;
                        }

                        void* cameraTransform = Il2Cpp::runtime_invoke(getTransformMethod, mainCamera, nullptr, &exc);
                        if (cameraTransform && !exc) {
                            void* camPosObj = Il2Cpp::runtime_invoke(getPosMethod, cameraTransform, nullptr, &exc);
                            if (camPosObj && !exc) {
                                Vector3Args camPos = *(Vector3Args*)((uintptr_t)camPosObj + 0x10);
                                
                                aimbotCamPosX = camPos.x;
                                aimbotCamPosY = camPos.y;
                                aimbotCamPosZ = camPos.z;

                                void* camRotObj = Il2Cpp::runtime_invoke(getRotMethod, cameraTransform, nullptr, &exc);
                                if (camRotObj && !exc) {
                                    QuaternionArgs camRot = *(QuaternionArgs*)((uintptr_t)camRotObj + 0x10);
                                    aimbotCamRotX = camRot.x;
                                    aimbotCamRotY = camRot.y;
                                    aimbotCamRotZ = camRot.z;
                                    aimbotCamRotW = camRot.w;
                                }

                                Vector3Args finalHitboxPos;
                                if (aimbotHitbox == 0) {
                                    finalHitboxPos = bestTargetWorldPosHead;
                                } else if (aimbotHitbox == 1) {
                                    finalHitboxPos = bestTargetWorldPosChest;
                                } else {
                                    finalHitboxPos = bestTargetWorldPosChest;
                                }

                                Vector3Args dir = {
                                    finalHitboxPos.x - camPos.x,
                                    finalHitboxPos.y - camPos.y,
                                    finalHitboxPos.z - camPos.z
                                };

                                float mag = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
                                if (mag > 0.001f) {
                                    dir.x /= mag;
                                    dir.y /= mag;
                                    dir.z /= mag;
                                }

                                void* argsRot[1] = { &dir };
                                void* targetRotObj = Il2Cpp::runtime_invoke(lookRotMethod, nullptr, argsRot, &exc);
                                if (targetRotObj && !exc) {
                                    QuaternionArgs targetRot = *(QuaternionArgs*)((uintptr_t)targetRotObj + 0x10);
                                    
                                    QuaternionArgs currentRot = { aimbotCamRotX, aimbotCamRotY, aimbotCamRotZ, aimbotCamRotW };
                                    
                                    float currentMag = sqrt(currentRot.x * currentRot.x + currentRot.y * currentRot.y + currentRot.z * currentRot.z + currentRot.w * currentRot.w);
                                    if (currentMag > 0.001f) {
                                        currentRot.x /= currentMag;
                                        currentRot.y /= currentMag;
                                        currentRot.z /= currentMag;
                                        currentRot.w /= currentMag;
                                    }
                                    
                                    float aimbotStrength = aimbotPullStrength;
                                    if (!delayPassed) {
                                        aimbotStrength *= (aimbotTargetTimeMap[bestTargetObj] / aimbotSmoothTimeMs);
                                    }
                                    
                                    float t = aimbotStrength * 0.01f;
                                    if (t > 1.0f) t = 1.0f;
                                    
                                    QuaternionArgs newRot = {
                                        currentRot.x + (targetRot.x - currentRot.x) * t,
                                        currentRot.y + (targetRot.y - currentRot.y) * t,
                                        currentRot.z + (targetRot.z - currentRot.z) * t,
                                        currentRot.w + (targetRot.w - currentRot.w) * t
                                    };
                                    
                                    float newMag = sqrt(newRot.x * newRot.x + newRot.y * newRot.y + newRot.z * newRot.z + newRot.w * newRot.w);
                                    if (newMag > 0.001f) {
                                        newRot.x /= newMag;
                                        newRot.y /= newMag;
                                        newRot.z /= newMag;
                                        newRot.w /= newMag;
                                    }
                                    
                                    bool aimSetSuccess = false;
                                    if (setAimRotationMethod && sharedState.localPlayerObj) {
                                        bool isTrue = true;
                                        void* argsAim[2] = { &newRot, &isTrue };
                                        Il2Cpp::runtime_invoke(setAimRotationMethod, sharedState.localPlayerObj, argsAim, &exc);
                                        if (!exc) {
                                            aimSetSuccess = true;
                                        }
                                    }
                                    
                                    if (!aimSetSuccess && setRotMethod && cameraTransform) {
                                        void* argsSetRot[1] = { &newRot };
                                        Il2Cpp::runtime_invoke(setRotMethod, cameraTransform, argsSetRot, &exc);
                                        if (!exc) {
                                            aimbotErrorLog = "Aim FOV (set_rotation camera)";
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } else if (aimbotMode == 1) { // Aimlock (Sempre ativo quando aimbotEnabled)
                // Lógica da Mira Magnética
                if (aimbotMagnetic && getTransformMethod && getPosMethod && getRotMethod) {
                    void* cameraTransform = Il2Cpp::runtime_invoke(getTransformMethod, mainCamera, nullptr, &exc);
                    if (cameraTransform && !exc) {
                        void* targetTransform = Il2Cpp::runtime_invoke(getTransformMethod, bestTargetObj, nullptr, &exc);
                        if (targetTransform && !exc) {
                            void* getForwardMethod = Il2Cpp::GetMethodRecursively(transformKlass, "get_forward", 0);
                            if (getForwardMethod) {
                                void* forwardObj = Il2Cpp::runtime_invoke(getForwardMethod, cameraTransform, nullptr, &exc);
                                void* camPosObj = Il2Cpp::runtime_invoke(getPosMethod, cameraTransform, nullptr, &exc);
                                if (forwardObj && camPosObj && !exc) {
                                    Vector3Args forward = *(Vector3Args*)((uintptr_t)forwardObj + 0x10);
                                    Vector3Args camPos = *(Vector3Args*)((uintptr_t)camPosObj + 0x10);
                                    
                                    float magDist = aimbotTargetDist3D;
                                    if (magDist < 1.0f) magDist = 1.0f;
                                    
                                    Vector3Args magneticPos = {
                                        camPos.x + forward.x * magDist,
                                        camPos.y + forward.y * magDist,
                                        camPos.z + forward.z * magDist
                                    };
                                    
                                    float heightOffset = bestTargetWorldPosHead.y - bestTargetWorldPosChest.y;
                                    if (aimbotHitbox == 0) {
                                        magneticPos = bestTargetWorldPosHead;
                                    } else if (aimbotHitbox == 1) {
                                        magneticPos = bestTargetWorldPosChest;
                                        magneticPos.y += (heightOffset * 0.5f);
                                    } else {
                                        magneticPos = bestTargetWorldPosChest;
                                    } 

                                    void* setPosMethod = Il2Cpp::GetMethodRecursively(transformKlass, "set_position", 1);
                                    if (setPosMethod) {
                                        void* argsSetPos[1] = { &magneticPos };
                                        Il2Cpp::runtime_invoke(setPosMethod, targetTransform, argsSetPos, &exc);
                                        if (!exc) {
                                            aimbotErrorLog = "Inimigo puxado pela Mira Magnetica (Sempre Ativo)!";
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                float deltaTime = ImGui::GetIO().DeltaTime;
                
                bool delayPassed = true; // Aimlock não tem delay
                if (foundAimbotTarget) {
                    wasAimingLastFrame = true;
                    aimbotTargetTimeMap[bestTargetObj] = aimbotSmoothTimeMs;
                } else {
                    wasAimingLastFrame = false;
                }

                if (foundAimbotTarget && getTransformMethod && getPosMethod && getRotMethod && setRotMethod && lookRotMethod) {
                    
                    for (auto it = aimbotTargetTimeMap.begin(); it != aimbotTargetTimeMap.end(); ) {
                        if (it->first != bestTargetObj) it = aimbotTargetTimeMap.erase(it);
                        else ++it;
                    }

                    void* cameraTransform = Il2Cpp::runtime_invoke(getTransformMethod, mainCamera, nullptr, &exc);
                    if (cameraTransform && !exc) {
                        void* camPosObj = Il2Cpp::runtime_invoke(getPosMethod, cameraTransform, nullptr, &exc);
                        if (camPosObj && !exc) {
                            Vector3Args camPos = *(Vector3Args*)((uintptr_t)camPosObj + 0x10);
                            
                            aimbotCamPosX = camPos.x;
                            aimbotCamPosY = camPos.y;
                            aimbotCamPosZ = camPos.z;

                            void* camRotObj = Il2Cpp::runtime_invoke(getRotMethod, cameraTransform, nullptr, &exc);
                            if (camRotObj && !exc) {
                                QuaternionArgs camRot = *(QuaternionArgs*)((uintptr_t)camRotObj + 0x10);
                                aimbotCamRotX = camRot.x;
                                aimbotCamRotY = camRot.y;
                                aimbotCamRotZ = camRot.z;
                                aimbotCamRotW = camRot.w;
                            }

                            Vector3Args finalHitboxPos;
                            if (aimbotHitbox == 0) {
                                finalHitboxPos = bestTargetWorldPosHead;
                            } else if (aimbotHitbox == 1) {
                                finalHitboxPos = bestTargetWorldPosChest;
                            } else {
                                finalHitboxPos = bestTargetWorldPosChest;
                            }

                            Vector3Args dir = {
                                finalHitboxPos.x - camPos.x,
                                finalHitboxPos.y - camPos.y,
                                finalHitboxPos.z - camPos.z
                            };

                            float mag = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
                            if (mag > 0.001f) {
                                dir.x /= mag;
                                dir.y /= mag;
                                dir.z /= mag;
                            }

                            void* argsRot[1] = { &dir };
                            void* targetRotObj = Il2Cpp::runtime_invoke(lookRotMethod, nullptr, argsRot, &exc);
                            if (targetRotObj && !exc) {
                                QuaternionArgs targetRot = *(QuaternionArgs*)((uintptr_t)targetRotObj + 0x10);
                                
                                QuaternionArgs currentRot = { aimbotCamRotX, aimbotCamRotY, aimbotCamRotZ, aimbotCamRotW };
                                
                                float currentMag = sqrt(currentRot.x * currentRot.x + currentRot.y * currentRot.y + currentRot.z * currentRot.z + currentRot.w * currentRot.w);
                                if (currentMag > 0.001f) {
                                    currentRot.x /= currentMag;
                                    currentRot.y /= currentMag;
                                    currentRot.z /= currentMag;
                                    currentRot.w /= currentMag;
                                }
                                
                                float aimbotStrength = aimbotPullStrength;
                                if (!delayPassed) {
                                    aimbotStrength *= (aimbotTargetTimeMap[bestTargetObj] / aimbotSmoothTimeMs);
                                }
                                
                                float t = aimbotStrength * 0.01f;
                                if (t > 1.0f) t = 1.0f;
                                
                                QuaternionArgs newRot = {
                                    currentRot.x + (targetRot.x - currentRot.x) * t,
                                    currentRot.y + (targetRot.y - currentRot.y) * t,
                                    currentRot.z + (targetRot.z - currentRot.z) * t,
                                    currentRot.w + (targetRot.w - currentRot.w) * t
                                };
                                
                                float newMag = sqrt(newRot.x * newRot.x + newRot.y * newRot.y + newRot.z * newRot.z + newRot.w * newRot.w);
                                if (newMag > 0.001f) {
                                    newRot.x /= newMag;
                                    newRot.y /= newMag;
                                    newRot.z /= newMag;
                                    newRot.w /= newMag;
                                }
                                
                                bool aimSetSuccess = false;
                                if (setAimRotationMethod && sharedState.localPlayerObj) {
                                    bool isTrue = true;
                                    void* argsAim[2] = { &newRot, &isTrue };
                                    Il2Cpp::runtime_invoke(setAimRotationMethod, sharedState.localPlayerObj, argsAim, &exc);
                                    if (!exc) {
                                        aimSetSuccess = true;
                                    }
                                }
                                
                                if (!aimSetSuccess && setRotMethod && cameraTransform) {
                                    void* argsSetRot[1] = { &newRot };
                                    Il2Cpp::runtime_invoke(setRotMethod, cameraTransform, argsSetRot, &exc);
                                    if (!exc) {
                                        aimbotErrorLog = "Aim FOV (set_rotation camera)";
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // ==================== SILENT AIM LOGIC ====================
        if (silentAimEnabled && sharedState.localPlayerObj && foundAimbotTarget) {
            bool isFiring = *(bool*)((uintptr_t)sharedState.localPlayerObj + 0x2E8);
            if (!isFiring) {
                silentAimStatus = "Aguardando tiro...";
            } else {
                silentAimStatus = "Silent Aim ATIVO";

                switch (silentAimApproach) {
                    case 0: {
                        silentAimStatus = "Hook GetFireDirection (requer hook)";
                        break;
                    }
                    case 1: {
                        silentAimStatus = "Hook StartFiring (requer hook)";
                        break;
                    }
                    case 2: {
                        if (lookRotMethod && getTransformMethod && sharedState.localPlayerObj && getPosMethod && getRotMethod) {
                            void* localTransform = Il2Cpp::runtime_invoke(getTransformMethod, sharedState.localPlayerObj, nullptr, &exc);
                            if (localTransform && !exc) {
                                void* localPosObj = Il2Cpp::runtime_invoke(getPosMethod, localTransform, nullptr, &exc);
                                if (localPosObj && !exc) {
                                    Vector3Args localPos = *(Vector3Args*)((uintptr_t)localPosObj + 0x10);

                                    QuaternionArgs currentRot = { 0, 0, 0, 1 };
                                    if (getRotMethod) {
                                        void* localRotObj = Il2Cpp::runtime_invoke(getRotMethod, localTransform, nullptr, &exc);
                                        if (localRotObj && !exc) {
                                            currentRot = *(QuaternionArgs*)((uintptr_t)localRotObj + 0x10);
                                        }
                                    }

                                    float heightDiff = bestTargetWorldPosHead.y - bestTargetWorldPosChest.y;
                                    Vector3Args targetPos;
                                    if (aimbotHitbox == 0) {
                                        targetPos = bestTargetWorldPosHead;
                                    } else if (aimbotHitbox == 1) {
                                        targetPos = bestTargetWorldPosChest;
                                        targetPos.y += (heightDiff * 0.5f);
                                    } else {
                                        targetPos = bestTargetWorldPosChest;
                                    }

                                    Vector3Args dir = {
                                        targetPos.x - localPos.x,
                                        targetPos.y - localPos.y,
                                        targetPos.z - localPos.z
                                    };

                                    float mag = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
                                    if (mag > 0.001f) {
                                        dir.x /= mag;
                                        dir.y /= mag;
                                        dir.z /= mag;
                                    }

                                    void* argsRot[1] = { &dir };
                                    void* targetRotObj = Il2Cpp::runtime_invoke(lookRotMethod, nullptr, argsRot, &exc);
                                    if (targetRotObj && !exc) {
                                        QuaternionArgs targetRot = *(QuaternionArgs*)((uintptr_t)targetRotObj + 0x10);

                                        float smoothT = 1.0f - silentAimSmooth;
                                        if (smoothT < 0.01f) smoothT = 0.01f;

                                        QuaternionArgs newRot = {
                                            currentRot.x + (targetRot.x - currentRot.x) * smoothT,
                                            currentRot.y + (targetRot.y - currentRot.y) * smoothT,
                                            currentRot.z + (targetRot.z - currentRot.z) * smoothT,
                                            currentRot.w + (targetRot.w - currentRot.w) * smoothT
                                        };

                                        float newMag = sqrt(newRot.x * newRot.x + newRot.y * newRot.y + newRot.z * newRot.z + newRot.w * newRot.w);
                                        if (newMag > 0.001f) {
                                            newRot.x /= newMag;
                                            newRot.y /= newMag;
                                            newRot.z /= newMag;
                                            newRot.w /= newMag;
                                        }

                                        if (setAimRotationMethod) {
                                            bool isTrue = true;
                                            void* argsAim[2] = { &newRot, &isTrue };
                                            Il2Cpp::runtime_invoke(setAimRotationMethod, sharedState.localPlayerObj, argsAim, &exc);
                                            if (!exc) {
                                                silentAimStatus = "Silent Aim: LookAtPosition OK";
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }
                    case 3: {
                        silentAimStatus = "GetLookDirection (requer hook)";
                        break;
                    }
                    case 4: {
                        silentAimStatus = "Raycast Predicativo (beta)";

                        if (lookRotMethod && getTransformMethod && sharedState.localPlayerObj && getPosMethod) {
                            void* exc = nullptr;
                            void* localTransform = Il2Cpp::runtime_invoke(getTransformMethod, sharedState.localPlayerObj, nullptr, &exc);
                            if (localTransform && !exc) {
                                void* localPosObj = Il2Cpp::runtime_invoke(getPosMethod, localTransform, nullptr, &exc);
                                if (localPosObj && !exc) {
                                    Vector3Args localPos = *(Vector3Args*)((uintptr_t)localPosObj + 0x10);

                                    float heightDiff = bestTargetWorldPosHead.y - bestTargetWorldPosChest.y;
                                    Vector3Args targetPos;
                                    if (aimbotHitbox == 0) {
                                        targetPos = bestTargetWorldPosHead;
                                    } else if (aimbotHitbox == 1) {
                                        targetPos = bestTargetWorldPosChest;
                                        targetPos.y += (heightDiff * 0.5f);
                                    } else {
                                        targetPos = bestTargetWorldPosChest;
                                    }

                                    Vector3Args predictedPos = targetPos;

                                    Vector3Args dir = {
                                        predictedPos.x - localPos.x,
                                        predictedPos.y - localPos.y,
                                        predictedPos.z - localPos.z
                                    };

                                    float mag = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
                                    if (mag > 0.001f) {
                                        dir.x /= mag;
                                        dir.y /= mag;
                                        dir.z /= mag;
                                    }

                                    void* argsRot[1] = { &dir };
                                    void* targetRotObj = Il2Cpp::runtime_invoke(lookRotMethod, nullptr, argsRot, &exc);
                                    if (targetRotObj && !exc) {
                                        QuaternionArgs targetRot = *(QuaternionArgs*)((uintptr_t)targetRotObj + 0x10);

                                        if (setAimRotationMethod) {
                                            bool isTrue = true;
                                            void* argsAim[2] = { &targetRot, &isTrue };
                                            Il2Cpp::runtime_invoke(setAimRotationMethod, sharedState.localPlayerObj, argsAim, &exc);
                                            if (!exc) {
                                                silentAimStatus = "Silent Aim: Predicao OK";
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }
                    case 5: {
                        silentAimStatus = "Modificar Rotacao Player";

                        if (lookRotMethod && getTransformMethod && sharedState.localPlayerObj && getPosMethod) {
                            void* exc = nullptr;
                            void* localTransform = Il2Cpp::runtime_invoke(getTransformMethod, sharedState.localPlayerObj, nullptr, &exc);
                            if (localTransform && !exc) {
                                void* localPosObj = Il2Cpp::runtime_invoke(getPosMethod, localTransform, nullptr, &exc);
                                if (localPosObj && !exc) {
                                    Vector3Args localPos = *(Vector3Args*)((uintptr_t)localPosObj + 0x10);

                                    float heightDiff = bestTargetWorldPosHead.y - bestTargetWorldPosChest.y;
                                    Vector3Args targetPos;
                                    if (aimbotHitbox == 0) {
                                        targetPos = bestTargetWorldPosHead;
                                    } else if (aimbotHitbox == 1) {
                                        targetPos = bestTargetWorldPosChest;
                                        targetPos.y += (heightDiff * 0.5f);
                                    } else {
                                        targetPos = bestTargetWorldPosChest;
                                    }

                                    Vector3Args dir = {
                                        targetPos.x - localPos.x,
                                        targetPos.y - localPos.y,
                                        targetPos.z - localPos.z
                                    };

                                    float mag = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
                                    if (mag > 0.001f) {
                                        dir.x /= mag;
                                        dir.y /= mag;
                                        dir.z /= mag;
                                    }

                                    void* argsRot[1] = { &dir };
                                    void* targetRotObj = Il2Cpp::runtime_invoke(lookRotMethod, nullptr, argsRot, &exc);
                                    if (targetRotObj && !exc) {
                                        QuaternionArgs targetRot = *(QuaternionArgs*)((uintptr_t)targetRotObj + 0x10);

                                        if (setAimRotationMethod) {
                                            bool isTrue = true;
                                            void* argsAim[2] = { &targetRot, &isTrue };
                                            Il2Cpp::runtime_invoke(setAimRotationMethod, sharedState.localPlayerObj, argsAim, &exc);
                                            if (!exc) {
                                                silentAimStatus = "Silent Aim: Rotacao OK";
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }
                    default:
                        silentAimStatus = "Abordagem invalida";
                        break;
                }

                if (silentAimDrawDebug && foundAimbotTarget) {
                    ImVec2 targetScreen = bestTargetPos;
                    drawList->AddCircle(targetScreen, 8.0f, IM_COL32(255, 255, 0, 255), 16, 2.0f);
                    drawList->AddLine(screenCenter, targetScreen, IM_COL32(255, 255, 0, 100), 1.0f);
                }
            }
        } else {
            silentAimStatus = "Inativo";
        }

        // --- INFJUMP LOGIC ---
        if (infJumpEnabled && sharedState.localPlayerObj && getTransformMethod && getPosMethod && setPosMethod) {
            ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("InfJump Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "InfJump (Arraste aqui)");
                
                bool jumpUp = ImGui::Button("SUBIR (UP)", ImVec2(100, 60));
                ImGui::SameLine();
                bool jumpDown = ImGui::Button("DESCER (DN)", ImVec2(100, 60));
                
                if (jumpUp || jumpDown) {
                    void* exc = nullptr;
                    void* localTransform = Il2Cpp::runtime_invoke(getTransformMethod, sharedState.localPlayerObj, nullptr, &exc);
                    if (localTransform && !exc) {
                        void* posObj = Il2Cpp::runtime_invoke(getPosMethod, localTransform, nullptr, &exc);
                        if (posObj && !exc) {
                            Vector3Args pos = *(Vector3Args*)((uintptr_t)posObj + 0x10);
                            if (jumpUp) pos.y += infJumpStep;
                            if (jumpDown) pos.y -= infJumpStep;
                            
                            void* argsSetPos[1] = { &pos };
                            Il2Cpp::runtime_invoke(setPosMethod, localTransform, argsSetPos, &exc);
                        }
                    }
                }
            }
            ImGui::End();
        }
    }

} // namespace GhostSystems
