#include "Menu.h"
#include "Obfuscator.h"
#include "Il2CppHelper.h"
#include "MemoryScanner.h"
#include "And64InlineHook.hpp"
#include <imgui.h>
#include <ctime>
#include <algorithm>
#include <android/log.h>

#define LOG_TAG "GhostSystems"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace GhostSystems {

extern Menu* g_Menu;

    void GhostSystems::Menu::initStyle() {
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

    void GhostSystems::Menu::drawFilters() {
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

    void GhostSystems::Menu::drawEntityList() {
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
                
                ImGui::PushStyleColor(ImGuiCol_Text, nameColor);
                if (ImGui::Selectable(entity.name, selectedEntityObj == entity.obj, ImGuiSelectableFlags_SpanAllColumns)) {
                    selectedEntityObj = entity.obj;
                }
                ImGui::PopStyleColor();

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
                ImVec4 teamColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                if (entity.alignment == Alignment::ENEMY) teamColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
                else if (entity.alignment == Alignment::ALLY) teamColor = ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
                ImGui::PushStyleColor(ImGuiCol_Text, teamColor);
                ImGui::Text("%d", entity.teamId);
                ImGui::PopStyleColor();

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

    // ===== NOVO: DEBUG COMPLETO DA ENTITYLIST =====
    void GhostSystems::Menu::drawEntityDebug() {
        std::vector<PlayerEntity> localEntities;
        void* localPlayer = nullptr;
        int localTeam = -1;
        {
            std::lock_guard<std::mutex> lock(sharedState.mtx);
            localEntities = sharedState.entities;
            localPlayer = sharedState.localPlayerObj;
            localTeam = sharedState.localPlayerTeamId;
        }
        
        // Info geral
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Debug de Entidades - Total: %zu", localEntities.size());
        ImGui::Text("Jogador Local: %p | TeamID: %d", localPlayer, localTeam);
        ImGui::Separator();
        
        // Filtros
        ImGui::InputText("Filtrar por nome", entityFilter, sizeof(entityFilter));
        ImGui::SameLine();
        ImGui::Checkbox("Só Inimigos", &showOnlyEnemies);
        ImGui::SameLine();
        ImGui::Checkbox("Só Aliados", &showOnlyAllies);
        ImGui::SameLine();
        ImGui::Checkbox("Ordenar por distancia", &sortByDistance);
        
        // Ordenar se necessario
        if (sortByDistance) {
            std::sort(localEntities.begin(), localEntities.end(), [](const PlayerEntity& a, const PlayerEntity& b) {
                return a.distanceToLocal < b.distanceToLocal;
            });
        }
        
        // Contadores
        int allyCount = 0, enemyCount = 0, neutralCount = 0;
        for (const auto& e : localEntities) {
            if (e.alignment == Alignment::ALLY) allyCount++;
            else if (e.alignment == Alignment::ENEMY) enemyCount++;
            else neutralCount++;
        }
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Aliados: %d", allyCount);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Inimigos: %d", enemyCount);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Neutros: %d", neutralCount);
        
        ImGui::Separator();
        
        // Tabela de debug detalhado
        if (ImGui::BeginTable("EntityDebugTable", 9, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable, ImVec2(0, 400))) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 30.0f);
            ImGui::TableSetupColumn("Obj Pointer", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Nome", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Dist", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("TeamID", ImGuiTableColumnFlags_WidthFixed, 50.0f);
            ImGui::TableSetupColumn("Alignment", ImGuiTableColumnFlags_WidthFixed, 70.0f);
            ImGui::TableSetupColumn("PlayerID", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Classe", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableHeadersRow();
            
            int idx = 0;
            for (const auto& entity : localEntities) {
                // Filtros
                if (showOnlyEnemies && entity.alignment != Alignment::ENEMY) continue;
                if (showOnlyAllies && entity.alignment != Alignment::ALLY) continue;
                if (strlen(entityFilter) > 0 && strstr(entity.name, entityFilter) == nullptr) continue;
                
                ImGui::TableNextRow();
                
                // Index
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", idx++);
                
                // Obj Pointer
                ImGui::TableSetColumnIndex(1);
                char ptrStr[32];
                snprintf(ptrStr, sizeof(ptrStr), "%p", entity.obj);
                if (ImGui::Selectable(ptrStr, selectedDebugEntity == idx, ImGuiSelectableFlags_SpanAllColumns)) {
                    selectedDebugEntity = idx;
                    selectedEntityObj = entity.obj;
                }
                
                // Nome
                ImGui::TableSetColumnIndex(2);
                ImVec4 nameColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                if (entity.alignment == Alignment::ENEMY) nameColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
                else if (entity.alignment == Alignment::ALLY) nameColor = ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
                ImGui::PushStyleColor(ImGuiCol_Text, nameColor);
                ImGui::Text("%s", entity.name);
                ImGui::PopStyleColor();
                
                // Distancia
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.1f", entity.distanceToLocal);
                
                // TeamID
                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%d", entity.teamId);
                
                // Alignment
                ImGui::TableSetColumnIndex(5);
                if (entity.alignment == Alignment::ALLY) ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "ALLY");
                else if (entity.alignment == Alignment::ENEMY) ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "ENEMY");
                else ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "NEUTRAL");
                
                // PlayerID (BHGGAEEHJCO) - tentar ler do offset
                ImGui::TableSetColumnIndex(6);
                if (entity.obj) {
                    uint64_t playerId = 0;
                    int offsetsToTry[] = {0x3B0, 0x3A0, 0x3C0, 0x3D0, 0x400, 0x420};
                    for (int off : offsetsToTry) {
                        uint64_t possibleId = *(uint64_t*)((uintptr_t)entity.obj + off);
                        if (possibleId != 0 && possibleId != 0xFFFFFFFFFFFFFFFF) {
                            playerId = possibleId;
                            break;
                        }
                    }
                    if (playerId != 0) {
                        ImGui::Text("%lX", (unsigned long)playerId);
                    } else {
                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "N/A");
                    }
                } else {
                    ImGui::Text("-");
                }
                
                // Classe
                ImGui::TableSetColumnIndex(7);
                if (entity.obj && Il2Cpp::object_get_class) {
                    void* klass = Il2Cpp::object_get_class(entity.obj);
                    if (klass && Il2Cpp::class_get_name) {
                        const char* className = Il2Cpp::class_get_name(klass);
                        ImGui::Text("%s", className ? className : "Unknown");
                    } else {
                        ImGui::Text("Invalid");
                    }
                } else {
                    ImGui::Text("-");
                }
                
                // Status
                ImGui::TableSetColumnIndex(8);
                if (!entity.isAlive()) ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "DEAD");
                else if (entity.isKnocked) ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "DOWN");
                else ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "ALIVE");
            }
            ImGui::EndTable();
        }
        
        // Detalhes da entidade selecionada
        if (selectedEntityObj && selectedDebugEntity >= 0 && selectedDebugEntity < (int)localEntities.size()) {
            const auto& selected = localEntities[selectedDebugEntity];
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Detalhes da Entidade Selecionada:");
            
            if (ImGui::BeginChild("EntityDetails", ImVec2(0, 200), true)) {
                ImGui::Text("Obj Pointer: %p", selected.obj);
                ImGui::Text("Base Address: %lX", (unsigned long)selected.baseAddress);
                ImGui::Text("Nome: %s", selected.name);
                ImGui::Text("Posicao: %.2f, %.2f, %.2f", selected.position.x, selected.position.y, selected.position.z);
                ImGui::Text("Vida: %.0f / %.0f", selected.health, selected.maxHealth);
                ImGui::Text("Distancia: %.2f", selected.distanceToLocal);
                ImGui::Text("TeamID: %d", selected.teamId);
                ImGui::Text("Alignment: %s", selected.alignment == Alignment::ALLY ? "ALLY" : (selected.alignment == Alignment::ENEMY ? "ENEMY" : "NEUTRAL"));
                ImGui::Text("IsBot: %s", selected.isBot ? "Sim" : "Nao");
                ImGui::Text("IsVisible: %s", selected.isVisible ? "Sim" : "Nao");
                
                // Tentar obter mais info
                if (selected.obj) {
                    void* klass = Il2Cpp::object_get_class(selected.obj);
                    if (klass) {
                        const char* className = Il2Cpp::class_get_name(klass);
                        ImGui::Text("Classe: %s", className ? className : "Unknown");
                        
                        // Tentar ler PlayerID de varios offsets
                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "PlayerID Offsets:");
                        int offsets[] = {0x3B0, 0x3A0, 0x3C0, 0x3D0, 0x3E0, 0x400, 0x420, 0x3B8, 0x3C8};
                        for (int off : offsets) {
                            uint64_t val = *(uint64_t*)((uintptr_t)selected.obj + off);
                                if (val != 0 && val != 0xFFFFFFFFFFFFFFFF) {
                                ImGui::Text("  [0x%X]: %lX (DEC: %lu)", off, (unsigned long)val, (unsigned long)val);
                            } else {
                                ImGui::TextDisabled("  [0x%X]: %lX", off, (unsigned long)val);
                            }
                        }
                        
                        // Tentar ler TeamID de varios offsets
                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "TeamID Offsets:");
                        int teamOffsets[] = {0x3D8, 0x3E0, 0x3B8, 0x3C0, 0x2F0, 0x2F8, 0x4A0, 0x4B0, 0x4C0};
                        for (int off : teamOffsets) {
                            int val = *(int*)((uintptr_t)selected.obj + off);
                            ImGui::Text("  [0x%X]: %d", off, val);
                        }
                        
                        // NOVO: TeamModeID (offset 0x3cc)
                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "NOVOS Offsets (dump.cs):");
                        uint32_t teamModeId = *(uint32_t*)((uintptr_t)selected.obj + 0x3cc);
                        ImGui::Text("  TeamModeID [0x3cc]: %u", teamModeId);
                        
                        // TeamColorStr (offset 0x7a0) - String pointer
                        void** teamColorStrPtr = (void**)((uintptr_t)selected.obj + 0x7a0);
                        if (teamColorStrPtr && *teamColorStrPtr) {
                            void* strObj = *teamColorStrPtr;
                            // Il2CppString layout: header(0x10) + length(4) + chars(2*length)
                            int32_t strLen = *(int32_t*)((uintptr_t)strObj + 0x10);
                            if (strLen > 0 && strLen < 100) {
                                // Tentar ler a string
                                char strBuffer[64] = {0};
                                void* chars = (void*)((uintptr_t)strObj + 0x14);
                                if (chars) {
                                    // Converter de UTF-16 para ASCII
                                    for (int i = 0; i < strLen && i < 63; i++) {
                                        strBuffer[i] = ((char*)chars)[i * 2];
                                    }
                                    ImGui::Text("  TeamColorStr [0x7a0]: %s", strBuffer);
                                } else {
                                    ImGui::Text("  TeamColorStr [0x7a0]: <ptr:%p>", strObj);
                                }
                            } else {
                                ImGui::TextDisabled("  TeamColorStr [0x7a0]: <empty or invalid len:%d>", strLen);
                            }
                        } else {
                            ImGui::TextDisabled("  TeamColorStr [0x7a0]: null");
                        }
                        
                        // PlayerID correto (BMIGBNMBAJH) offset 0x3a0
                        void** playerIdPtr = (void**)((uintptr_t)selected.obj + 0x3a0);
                        if (playerIdPtr && *playerIdPtr) {
                            void* idObj = *playerIdPtr;
                            uint64_t idVal = *(uint64_t*)((uintptr_t)idObj + 0x20);
                            ImGui::Text("  PlayerID [0x3a0]: 0x%lX", (unsigned long)idVal);
                        } else {
                            ImGui::TextDisabled("  PlayerID [0x3a0]: null");
                        }
                    }
                }
            }
            ImGui::EndChild();
        }
    }

    void GhostSystems::Menu::render() {
        if (!isVisible) return;

        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* drawList = ImGui::GetForegroundDrawList();

        // ---- ICONE FLUTUANTE ----
        const float iconRadius = 28.0f;
        const float iconMargin = 16.0f;

        ImVec2 iconCenter(iconPosition.x + iconRadius, iconPosition.y + iconRadius);

        // Desenha o circulo de fundo (logo do cheat)
        drawList->AddCircleFilled(iconCenter, iconRadius, IM_COL32(20, 20, 30, 220));
        drawList->AddCircle(iconCenter, iconRadius, IM_COL32(60, 180, 255, 255), 64, 2.5f);
        drawList->AddCircle(iconCenter, iconRadius - 2.0f, IM_COL32(30, 120, 255, 150), 64, 1.5f);

        // Texto "GS" no centro do icone
        const char* logoText = "GS";
        ImVec2 textSize = ImGui::CalcTextSize(logoText);
        drawList->AddText(ImVec2(iconCenter.x - textSize.x / 2.0f, iconCenter.y - textSize.y / 2.0f),
                          IM_COL32(255, 255, 255, 255), logoText);

        // Clique no icone toggle show/hide do painel
        ImVec2 iconMin(iconPosition.x, iconPosition.y);
        ImVec2 iconMax(iconPosition.x + iconRadius * 2, iconPosition.y + iconRadius * 2);
        bool iconHovered = io.MousePos.x >= iconMin.x && io.MousePos.x <= iconMax.x && 
                           io.MousePos.y >= iconMin.y && io.MousePos.y <= iconMax.y;
        if (iconHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            panelMinimized = !panelMinimized;
        }

        // Arrastar o icone (so quando o painel esta minimizado)
        if (panelMinimized) {
            if (iconHovered && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                ImVec2 delta = io.MouseDelta;
                iconPosition.x += delta.x;
                iconPosition.y += delta.y;

                // Clamp para nao sair da tela
                if (iconPosition.x < 0) iconPosition.x = 0;
                if (iconPosition.y < 0) iconPosition.y = 0;
                if (iconPosition.x > io.DisplaySize.x - iconRadius * 2) iconPosition.x = io.DisplaySize.x - iconRadius * 2;
                if (iconPosition.y > io.DisplaySize.y - iconRadius * 2) iconPosition.y = io.DisplaySize.y - iconRadius * 2;
            }

            // Tooltip no icone
            if (iconHovered) {
                ImVec2 tooltipPos(iconCenter.x, iconPosition.y + iconRadius * 2 + 6);
                const char* tip = panelMinimized ? "Clique para abrir" : "Clique para minimizar";
                ImVec2 tipSize = ImGui::CalcTextSize(tip);
                drawList->AddRectFilled(ImVec2(tooltipPos.x - tipSize.x / 2.0f - 6, tooltipPos.y),
                                        ImVec2(tooltipPos.x + tipSize.x / 2.0f + 6, tooltipPos.y + tipSize.y + 4),
                                        IM_COL32(0, 0, 0, 200));
                drawList->AddText(ImVec2(tooltipPos.x - tipSize.x / 2.0f, tooltipPos.y + 2),
                                  IM_COL32(255, 255, 255, 220), tip);
            }
        }

        // Renderiza ESP sempre
        drawESP();

        // Se o painel estiver minimizado, para aqui
        if (panelMinimized) return;

        // ---- PAINEL PRINCIPAL ----
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);

        ImVec2 displaySize = io.DisplaySize;

        // Centraliza sempre
        panelSize = ImVec2(600, 450);
        panelPosition = ImVec2(displaySize.x * 0.5f - panelSize.x * 0.5f,
                               displaySize.y * 0.5f - panelSize.y * 0.5f);

        ImGui::SetNextWindowSize(panelSize, ImGuiCond_Always);
        ImGui::SetNextWindowPos(panelPosition, ImGuiCond_Always);

        if (ImGui::Begin(OBFUSCATE("##GhostPanel"), nullptr, windowFlags)) {
            panelPosition = ImGui::GetWindowPos();
            panelSize = ImGui::GetWindowSize();

            // Atualiza posicao do icone (top/left do painel)
            iconPosition = ImVec2(panelPosition.x + iconMargin, panelPosition.y + iconMargin);

            // Arrastar tocando no fundo da janela (Mobile amigavel)
            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                ImVec2 delta = io.MouseDelta;
                ImGui::SetWindowPos(ImVec2(panelPosition.x + delta.x, panelPosition.y + delta.y));
            }

            // Barra de titulo customizada
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.22f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.30f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.30f, 0.30f, 0.35f, 1.0f));

            ImGui::TextColored(ImVec4(0.0f, 0.7f, 1.0f, 1.0f), "%s", OBFUSCATE("GHOSTSYSTEMS V1.0"));
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 28);
            if (ImGui::Button(OBFUSCATE("_"), ImVec2(24, 24))) {
                panelMinimized = true;
            }

            ImGui::PopStyleColor(3);
            ImGui::Separator();

            bool prevMasterSwitch = masterSwitch;
            ImGui::Checkbox(OBFUSCATE("Ativar Painel"), &masterSwitch);
            if (masterSwitch && !prevMasterSwitch) {
                if (!scannerStarted && ::g_Scanner) {
                    ::g_Scanner->start();
                    scannerStarted = true;
                }
            } else if (!masterSwitch && prevMasterSwitch) {
                if (scannerStarted && ::g_Scanner) {
                    ::g_Scanner->stop();
                    scannerStarted = false;
                }
            }
            ImGui::Separator();

            if (masterSwitch) {
                if (ImGui::BeginTabBar("MenuTabs")) {
                    if (ImGui::BeginTabItem(OBFUSCATE("Aimbot"))) {
                        ImGui::Checkbox(OBFUSCATE("Ativar Aimbot"), &aimbotEnabled);
                        if (aimbotEnabled) {
                            const char* aimbotModes[] = { "Tradicional (Ao Atirar)", "Aimlock (Sempre)" };
                            ImGui::Combo("Modo", &aimbotMode, aimbotModes, IM_ARRAYSIZE(aimbotModes));
                            ImGui::Checkbox(OBFUSCATE("Mostrar FOV"), &aimbotDrawFov);
                            ImGui::Checkbox(OBFUSCATE("Mirar em Aliados"), &aimbotTargetAllies);
                            ImGui::Checkbox(OBFUSCATE("Mira Magnetica (Puxa inimigo)"), &aimbotMagnetic);
                            ImGui::SliderFloat(OBFUSCATE("Raio do FOV"), &aimbotFov, 10.0f, 500.0f, "%.0f px");
                            ImGui::SliderInt(OBFUSCATE("Atraso/Delay (ms)"), &aimbotTimeMs, 0, 300, "%d ms");

                            if (aimbotTimeMs < 50) {
                                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Rage Aim (Força Máxima)");
                            } else {
                                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Safe Aim (Suave)");
                            }

                            ImGui::Separator();
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Configurações de Transição (Peito -> Cabeça)");
                            ImGui::SliderFloat(OBFUSCATE("Tempo p/ Cabeça (ms)"), &aimbotTransitionTimeMs, 0.0f, 2000.0f, "%.0f ms");
                            if (aimbotTransitionTimeMs < 50.0f) {
                                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Transição: RAGE (Instantâneo)");
                            } else {
                                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Transição: SAFE (Suave)");
                            }
                            ImGui::SliderFloat(OBFUSCATE("Força/Curva"), &aimbotTransitionCurve, 1.0f, 10.0f, "%.1f");

                            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", OBFUSCATE("A mira foca no inimigo mais proximo do centro (FOV)."));
                        }
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem(OBFUSCATE("ESP"))) {
                        ImGui::Checkbox(OBFUSCATE("Ativar ESP"), &espEnabled);
                        if (espEnabled) {
                            ImGui::Checkbox(OBFUSCATE("ESP Box"), &espBox);
                            if (espBox) {
                                const char* boxModes[] = { OBFUSCATE("Box Padrao"), OBFUSCATE("Outline (Contorno)") };
                                ImGui::Combo(OBFUSCATE("Modo Box"), &espBoxMode, boxModes, IM_ARRAYSIZE(boxModes));
                            }
                            ImGui::Checkbox(OBFUSCATE("ESP Vida (Health)"), &espHealth);
                            ImGui::Checkbox(OBFUSCATE("ESP Nome"), &espName);
                            ImGui::Checkbox(OBFUSCATE("ESP Distancia"), &espDistance);
                            ImGui::Checkbox(OBFUSCATE("ESP Linha"), &espLine);
                            ImGui::Checkbox(OBFUSCATE("Esqueleto (Bones)"), &espSkeleton);

                            ImGui::Separator();
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", OBFUSCATE("Filtro de Distancia ESP"));
                            ImGui::SliderFloat(OBFUSCATE("Maxima"), &espMaxDistance, 10.0f, 300.0f, "%.0f m");

                            ImGui::Separator();
                            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "%s", OBFUSCATE("Wall-Check (Raycast)"));
                            ImGui::Checkbox(OBFUSCATE("Ativar Wall-Check"), &featureConfig.wallCheckEnabled);
                            if (featureConfig.wallCheckEnabled) {
                                const char* wallCheckMethods[] = {
                                    OBFUSCATE("Physics.Raycast"),
                                    OBFUSCATE("Physics.Linecast"),
                                    OBFUSCATE("Physics.CheckSphere"),
                                    OBFUSCATE("Custom Raycast")
                                };
                                ImGui::Combo(OBFUSCATE("Metodo"), &featureConfig.wallCheckMethod, wallCheckMethods, IM_ARRAYSIZE(wallCheckMethods));
                                
                                ImGui::Separator();
                                ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%s", OBFUSCATE("Raycast Preciso (Bones)"));
                                ImGui::Checkbox(OBFUSCATE("Testar multiplos pontos (cabeca/pescoco/peito)"), &wallCheckMultiplePoints);
                                
                                const char* originOptions[] = {
                                    OBFUSCATE("Posicao do pe"),
                                    OBFUSCATE("Posicao do peito"),
                                    OBFUSCATE("Posicao da cabeca (Recomendado)")
                                };
                                ImGui::Combo(OBFUSCATE("Origem do jogador local"), &wallCheckLocalOrigin, originOptions, IM_ARRAYSIZE(originOptions));
                                
                                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", OBFUSCATE("Origem na cabeca + teste multiplo = mais preciso"));
                            }
                        }
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem(OBFUSCATE("Configuracoes"))) {
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", OBFUSCATE("Modo Stream / Ocultacao"));
                        ImGui::Separator();
                        ImGui::Checkbox(OBFUSCATE("MODO STREAM (Invisivel p/ captura)"), &featureConfig.streamModeEnabled);
                        ImGui::Checkbox(OBFUSCATE("Toggle 3 Dedos (Mostra/Oculta Painel)"), &featureConfig.threeFingerToggleEnabled);

                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%s", OBFUSCATE("Network / Fake Lag"));
                        ImGui::Checkbox(OBFUSCATE("Ativar Fake Lag"), &fakeLagEnabled);
                        if (fakeLagEnabled) {
                            ImGui::SliderInt(OBFUSCATE("Quantidade (ms)"), &fakeLagAmount, 10, 500, "%d ms");
                            ImGui::Checkbox(OBFUSCATE("Lag Adaptativo"), &fakeLagAdaptive);
                            if (fakeLagAdaptive) {
                                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", OBFUSCATE("Ajusta automaticamente baseado no ping"));
                            }
                        }

                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "%s", OBFUSCATE("Team-Check (Nao mirrar aliados)"));
                        ImGui::Checkbox(OBFUSCATE("Ativar Team-Check"), &featureConfig.teamCheckEnabled);
                        if (featureConfig.teamCheckEnabled) {
                            const char* teamCheckMethods[] = {
                                OBFUSCATE("0: GetMyTeamID"),
                                OBFUSCATE("1: PlayerCache.GetTeamByObject"),
                                OBFUSCATE("2: TeamService.IsAlly"),
                                OBFUSCATE("3: TeamID Compare"),
                                OBFUSCATE("4: Always Neutral"),
                                OBFUSCATE("5: Alignment.GetAlignment"),
                                OBFUSCATE("6: IsSameTeamWithPlayerID (BHGGAEEHJCO)"),
                                OBFUSCATE("7: GameFacade.IsSameTeam(Player)"),
                                OBFUSCATE("8: GameFacade.IsSameTeam(PlayerID)"),
                                OBFUSCATE("9: PlayerID Direct Compare"),
                                OBFUSCATE("10: GameFacade.CheckSameTeam"),
                                OBFUSCATE("11: GetRelationWithTrackedPlayer"),
                                OBFUSCATE("12: Field Scan 'm_Team'"),
                                OBFUSCATE("13: TeamColorStr Compare (0x7a0) RECOMENDADO"),
                                OBFUSCATE("14: Player::IsSameTeam(BHGGAEEHJCO) RECOMENDADO"),
                                OBFUSCATE("15: Raw Memory Compare (0x3cc, 0x3a0)")
                            };
                            ImGui::Combo(OBFUSCATE("Metodo Team-Check"), &featureConfig.teamCheckMethod, teamCheckMethods, IM_ARRAYSIZE(teamCheckMethods));
                            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", OBFUSCATE("NOVOS RECOMENDADOS: 13, 14 - Baseados na estrutura real do dump.cs"));
                            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", OBFUSCATE("Use a aba 'Entity Debug' para ver os valores reais dos offsets"));
                        }
                        ImGui::EndTabItem();
                    }

                    if (isDebugMode) {
                        if (ImGui::BeginTabItem(OBFUSCATE("Entity List (Debug)"))) {
                            drawFilters();

                            ImGui::Text("%s", OBFUSCATE("Entidades Encontradas:"));
                            drawEntityList();
                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem(OBFUSCATE("Entity Debug"))) {
                            drawEntityDebug();
                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem(OBFUSCATE("Entity Props"))) {
                            if (selectedEntityObj) {
                                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Inspecionando Objeto: %p", selectedEntityObj);
                                ImGui::Separator();
                                ImGui::BeginChild("PropsPanel", ImVec2(0, 0), true);
                                drawEntityProperties(selectedEntityObj);
                                ImGui::EndChild();
                            } else {
                                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Nenhuma entidade selecionada!");
                                ImGui::Text("Vá para a aba 'Entity List (Debug)' e clique no nome de um jogador.");
                            }
                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem(OBFUSCATE("Debug Player"))) {
                            drawDebugPlayer();
                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem(OBFUSCATE("Debug Aimbot"))) {
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

                    if (ImGui::BeginTabItem("Bypass")) {
                        drawBypassManager();
                        ImGui::EndTabItem();
                    }
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();

        ImGui::PopStyleVar();
    }

    // ============ NOVAS FUNCOES PREMIUM ============
    
    void GhostSystems::Menu::initBypassManager() {
        if (!bypassManagerInitialized) {
            bypassManager.Initialize();
            bypassManagerInitialized = true;
            LOGI("[Menu] BypassManager inicializado");
            activateAllBypass();
        }
    }

    void GhostSystems::Menu::activateAllBypass() {
        if (bypassManagerInitialized) {
            bypassManager.ApplyAll();
            LOGI("[Menu] Todas as tecnicas de bypass ativadas automaticamente");
        }
    }

    void GhostSystems::Menu::drawBypassManager() {
        if (!bypassManagerInitialized) {
            initBypassManager();
        }

        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Sistema de Protecao Anti-Ban");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "40 tecnicas de bypass implementadas");
        ImGui::Separator();

        bool allProtected = bypassManager.IsFullyProtected();
        size_t applied = bypassManager.GetAppliedCount();
        size_t total = bypassManager.GetTotalCount();

        if (allProtected) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✓ PROTECAO TOTAL ATIVA");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "⚠ PROTECAO PARCIAL (%zu/%zu)", applied, total);
        }

        ImGui::Separator();

        if (ImGui::Button("Aplicar Todas as Tecnicas", ImVec2(200, 35))) {
            bypassManager.ApplyAll();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reverter Todas", ImVec2(150, 35))) {
            bypassManager.RevertAll();
        }

        ImGui::Separator();
        ImGui::Text("Tecnicas de Protecao:");
        
        auto& entries = bypassManager.GetEntries();
        
        if (ImGui::BeginTable("BypassTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Tecnica", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 100);
            ImGui::TableSetupColumn("Acao", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableHeadersRow();

            for (auto& entry : entries) {
                ImGui::TableNextRow();
                
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("[%s] %s", entry.id.c_str(), entry.name.c_str());
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("%s", entry.description.c_str());
                }

                ImGui::TableSetColumnIndex(1);
                switch (entry.status) {
                    case BypassStatus::APPLIED:
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✓ Ativo");
                        break;
                    case BypassStatus::APPLYING:
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "⏳ Aplicando...");
                        break;
                    case BypassStatus::FAILED:
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "✗ Falhou");
                        break;
                    default:
                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "○ Inativo");
                }

                ImGui::TableSetColumnIndex(2);
                if (!entry.isApplied) {
                    if (ImGui::Button((std::string("Aplicar##") + entry.id).c_str())) {
                        entry.applyFunc();
                    }
                } else {
                    if (ImGui::Button((std::string("Reverter##") + entry.id).c_str())) {
                        entry.revertFunc();
                    }
                }
            }
            ImGui::EndTable();
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
            "Nota: Estas tecnicas sao PASSIVAS e nao injetam codigo no anticheat.");
    }

    void GhostSystems::Menu::scanForPotentialValues(void* obj, void* klass, const std::string& path, int depth, std::unordered_set<void*>& visited) {
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

    void GhostSystems::Menu::drawDebugPlayer() {
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

    void Menu::OnMainThreadTick() {
        if (!sharedState.localPlayerObj || !featureConfig.wallCheckEnabled) return;

        static void* physicsKlass = nullptr;
        static void* cachedRaycastMethod = nullptr;
        static void* cachedLinecastMethod = nullptr;
        static uint32_t raycastParamCount = 0;
        static uint32_t linecastParamCount = 0;
        static bool methodsCached = false;

        // Otimização EXTREMA: Cache dos métodos da Unity (Isso remove 99% do lag)
        if (!methodsCached) {
            physicsKlass = Il2Cpp::GetClass("UnityEngine.PhysicsModule.dll", "UnityEngine", "Physics");
            if (physicsKlass) {
                cachedRaycastMethod = Il2Cpp::FindRaycastMethodSafe(physicsKlass, &raycastParamCount);
                cachedLinecastMethod = Il2Cpp::FindLinecastMethodSafe(physicsKlass, &linecastParamCount);
            }
            methodsCached = true;
        }

        if (!physicsKlass) return;

        void* currentMethod = nullptr;
        uint32_t currentParamCount = 0;
        bool isLinecast = false;

        // Respeita a escolha do usuário no painel (Combo Box: 0 = Raycast, 1 = Linecast, etc)
        if (featureConfig.wallCheckMethod == 1) {
            // Força Linecast
            currentMethod = cachedLinecastMethod;
            currentParamCount = linecastParamCount;
            if (currentMethod) isLinecast = true;
        } else {
            // Tenta Raycast por padrão
            currentMethod = cachedRaycastMethod;
            currentParamCount = raycastParamCount;
            if (!currentMethod) {
                // Fallback de segurança se o Raycast falhar
                currentMethod = cachedLinecastMethod;
                currentParamCount = linecastParamCount;
                if (currentMethod) isLinecast = true;
            }
        }

        if (!currentMethod) return;

        // Removed global 30ms throttle. We use per-entity throttling based on FOV priority
          auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

          std::lock_guard<std::mutex> lock(sharedState.mtx);

          struct Vector3Args { float x, y, z; };
            Vector3Args localPos = { sharedState.localPlayerPos.x, sharedState.localPlayerPos.y, sharedState.localPlayerPos.z };

            static void* getTransformMethod = nullptr;
            static void* getPosMethod = nullptr;
            static bool transformMethodsCached = false;

            if (!transformMethodsCached) {
                void* componentKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Component");
                  if (!componentKlass) {
                      componentKlass = Il2Cpp::GetClass("UnityEngine.dll", "UnityEngine", "Component");
                  }
                  if (componentKlass) {
                    getTransformMethod = Il2Cpp::GetMethodRecursively(componentKlass, "get_transform", 0);
                }
                void* transformKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Transform");
                if (transformKlass) {
                    getPosMethod = Il2Cpp::GetMethodRecursively(transformKlass, "get_position", 0);
                }
                transformMethodsCached = true;
            }

            // Atualiza posicao do jogador local em tempo real
            if (getTransformMethod && getPosMethod && sharedState.localPlayerObj) {
                void* excPos = nullptr;
                void* localTransform = Il2Cpp::runtime_invoke(getTransformMethod, sharedState.localPlayerObj, nullptr, &excPos);
                if (localTransform && !excPos) {
                    void* localPosObj = Il2Cpp::runtime_invoke(getPosMethod, localTransform, nullptr, &excPos);
                    if (localPosObj && !excPos) {
                        localPos = *(Vector3Args*)((uintptr_t)localPosObj + 0x10);
                        sharedState.localPlayerPos.x = localPos.x;
                        sharedState.localPlayerPos.y = localPos.y;
                        sharedState.localPlayerPos.z = localPos.z;
                    }
                }
            }

            for (auto& entity : sharedState.entities) {
                if (!entity.isAlive() || entity.baseAddress == 0 || entity.obj == nullptr) continue;

                // Atualiza posicao real do pe do alvo em tempo real
                if (getTransformMethod && getPosMethod && entity.obj) {
                    void* excPos = nullptr;
                    void* entityTransform = Il2Cpp::runtime_invoke(getTransformMethod, entity.obj, nullptr, &excPos);
                    if (entityTransform && !excPos) {
                        void* entityPosObj = Il2Cpp::runtime_invoke(getPosMethod, entityTransform, nullptr, &excPos);
                        if (entityPosObj && !excPos) {
                            Vector3Args newPos = *(Vector3Args*)((uintptr_t)entityPosObj + 0x10);
                            entity.position.x = newPos.x;
                            entity.position.y = newPos.y;
                            entity.position.z = newPos.z;
                        }
                    }
                }

                if (entity.alignment != Alignment::ENEMY) {
                  entity.isVisible = true; // allies are visible
                  continue;
              }

              // Throttle individual: Alta prioridade (16ms = ~60FPS) para inimigos no FOV. Baixa prioridade (150ms) para os outros.
              uint32_t throttleTime = entity.isInFov ? 16 : 150;
              if (nowMs - entity.lastWallCheckMs < throttleTime) continue; // Pula se ainda nao deu o tempo
              entity.lastWallCheckMs = nowMs;

              // ===== NOVO: Raycast preciso com bones =====
              // Obter posicao de origem do jogador local baseado na configuracao
              float localOriginX, localOriginY, localOriginZ;
              bool hasLocalOrigin = false;
              
              if (wallCheckLocalOrigin == 2) {
                  // Cabeca
                  hasLocalOrigin = getLocalPlayerBonePosition(2, &localOriginX, &localOriginY, &localOriginZ);
              } else if (wallCheckLocalOrigin == 1) {
                  // Peito
                  hasLocalOrigin = getLocalPlayerBonePosition(1, &localOriginX, &localOriginY, &localOriginZ);
              }
              
              if (!hasLocalOrigin) {
                  // Fallback: usar posicao do transform + offset estimado
                  localOriginX = localPos.x;
                  localOriginY = localPos.y + (wallCheckLocalOrigin == 2 ? 1.6f : (wallCheckLocalOrigin == 1 ? 1.4f : 0.0f));
                  localOriginZ = localPos.z;
              }
              
              // Pontos a testar no alvo (prioridade: Cabeca > Pescoco > Peito)
              struct TargetPoint {
                  float x, y, z;
                  int boneType; // 2=cabeca, 3=pescoco, 1=peito
              };
              std::vector<TargetPoint> targetPoints;
              
              // Tentar obter posicoes reais dos bones do alvo
              float headX, headY, headZ;
              float neckX, neckY, neckZ;
              float chestX, chestY, chestZ;
              
              if (getEntityBonePosition(entity.obj, 2, &headX, &headY, &headZ)) {
                  targetPoints.push_back({headX, headY, headZ, 2});
              }
              if (wallCheckMultiplePoints && getEntityBonePosition(entity.obj, 3, &neckX, &neckY, &neckZ)) {
                  targetPoints.push_back({neckX, neckY, neckZ, 3});
              }
              if (wallCheckMultiplePoints && getEntityBonePosition(entity.obj, 1, &chestX, &chestY, &chestZ)) {
                  targetPoints.push_back({chestX, chestY, chestZ, 1});
              }
              
              // Se nao conseguiu nenhum bone real, usar posicao estimada
              if (targetPoints.empty()) {
                  targetPoints.push_back({
                      entity.position.x, 
                      entity.position.y + 1.6f, // Estimativa da cabeca
                      entity.position.z, 
                      2
                  });
              }
              
              // Testar visibilidade para cada ponto (se qualquer um estiver visivel, o alvo eh visivel)
              bool anyVisible = false;
              int layerMask = ~((1 << 2) | (1 << 4) | (1 << 5)); // Tudo exceto IgnoreRaycast, Water, UI
              
              for (const auto& target : targetPoints) {
                  float dx = target.x - localOriginX;
                  float dy = target.y - localOriginY;
                  float dz = target.z - localOriginZ;
                  float dist = sqrt(dx*dx + dy*dy + dz*dz);
                  
                  if (dist <= 1.0f) {
                      anyVisible = true; // Muito proximo, considerar visivel
                      break;
                  }
                  
                  Vector3Args dir = { dx/dist, dy/dist, dz/dist };
                  Vector3Args origin = {
                      localOriginX + dir.x * 0.2f, // Comeca um pouco a frente da origem
                      localOriginY + dir.y * 0.2f,
                      localOriginZ + dir.z * 0.2f
                  };
                  float rayDist = dist - 0.5f; // Reduz para nao bater no proprio alvo
                  if (rayDist < 0.1f) rayDist = 0.1f;
                  
                  bool hitObstacle = false;
                  
                  if (isLinecast) {
                      Vector3Args end = { target.x, target.y, target.z };
                      if (currentParamCount == 3) {
                          typedef bool (*Linecast_3_t)(Vector3Args, Vector3Args, int, void*);
                          Linecast_3_t pLinecast = (Linecast_3_t)*(void**)currentMethod;
                          if (pLinecast) hitObstacle = pLinecast(origin, end, layerMask, currentMethod);
                      } else if (currentParamCount == 4) {
                          typedef bool (*Linecast_4_t)(Vector3Args, Vector3Args, int, int, void*);
                          Linecast_4_t pLinecast = (Linecast_4_t)*(void**)currentMethod;
                          if (pLinecast) hitObstacle = pLinecast(origin, end, layerMask, 0, currentMethod);
                      }
                  } else {
                      if (currentParamCount == 5) {
                          typedef bool (*Raycast_5_t)(Vector3Args, Vector3Args, float, int, int, void*);
                          Raycast_5_t pRaycast = (Raycast_5_t)*(void**)currentMethod;
                          if (pRaycast) hitObstacle = pRaycast(origin, dir, rayDist, layerMask, 0, currentMethod);
                      } else if (currentParamCount == 4) {
                          typedef bool (*Raycast_4_t)(Vector3Args, Vector3Args, float, int, void*);
                          Raycast_4_t pRaycast = (Raycast_4_t)*(void**)currentMethod;
                          if (pRaycast) hitObstacle = pRaycast(origin, dir, rayDist, layerMask, currentMethod);
                      } else if (currentParamCount > 5) {
                          typedef bool (*Raycast_6_t)(Vector3Args, Vector3Args, float, int, int, int, void*);
                          Raycast_6_t pRaycast = (Raycast_6_t)*(void**)currentMethod;
                          if (pRaycast) hitObstacle = pRaycast(origin, dir, rayDist, layerMask, 0, 0, currentMethod);
                      }
                  }
                  
                  if (!hitObstacle) {
                      anyVisible = true; // Pelo menos um ponto esta visivel
                      break;
                  }
              }
              
              entity.isVisible = anyVisible;
        }
    }

    void GhostSystems::Menu::drawESP() {
        if (!masterSwitch) return; // Se o painel esta desativado, nao fazemos nada

        std::vector<PlayerEntity> localEntities;
        void* localPlayerObj = nullptr;
        {
            std::lock_guard<std::mutex> lock(sharedState.mtx);
            localEntities = sharedState.entities;
            localPlayerObj = sharedState.localPlayerObj;
        }

        static int noEntityFrames = 0;
        if (!localPlayerObj || localEntities.empty()) {
            noEntityFrames++;
            if (noEntityFrames >= 30) {
                masterSwitch = false;
                if (scannerStarted && ::g_Scanner) {
                    ::g_Scanner->stop();
                    scannerStarted = false;
                }
            }
            return;
        }
        noEntityFrames = 0;

        static void* get_mainMethod = nullptr;
        static void* worldToScreenMethod = nullptr;
        
        static void* componentKlass = nullptr;
        static void* getTransformMethod = nullptr;
        static void* transformKlass = nullptr;
        static void* getPosMethod = nullptr;
        static void* getChildCountMethod = nullptr;
        static void* getChildMethod = nullptr;
        static void* getNameMethod = nullptr;
        static void* getRotMethod = nullptr;
        static void* setRotMethod = nullptr;
        static void* quatKlass = nullptr;
        static void* lookRotMethod = nullptr;
        static void* setAimRotationMethod = nullptr;
        static void* isFiringMethod = nullptr;
        static void* getHeadTFMethod = nullptr;
        static void* getComponentMethod = nullptr;
        static void* animatorTypeObject = nullptr;
        static void* getBoneTransformMethod = nullptr;
          static void* raycastMethod = nullptr;
          static uint32_t raycastParamCount = 0;

          static bool methodsSearched = false;

          struct CachedAnimatorInfo {
            void* animatorObj = nullptr;
            bool hasAttempted = false;
            uint32_t lastAttemptMs = 0;
            std::unordered_map<int, void*> boneTransforms;
            bool genericRigCached = false;
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

        auto mapNameToBoneId = [](const char* originalName) -> int {
            if (!originalName) return -1;
            
            char name[128];
            strncpy(name, originalName, 127);
            name[127] = '\0';
            for(int i = 0; name[i]; i++){
                name[i] = tolower(name[i]);
            }
            
            if (strstr(name, "head")) return 10;
            if (strstr(name, "neck")) return 9;
            
            if (strstr(name, "spine2") || strstr(name, "spine 2") || strstr(name, "chest")) return 8;
            if (strstr(name, "spine1") || strstr(name, "spine 1")) return 7;
            
            // A ordem importa: primeiro checa pelvis, se nao, checa spine genérico
            if (strstr(name, "pelvis") || strstr(name, "hips") || strstr(name, "hip")) return 0;
            if (strstr(name, "spine")) return 7;
            
            // Braços Esquerdos
            if (strstr(name, "l clavicle") || strstr(name, "l_clavicle") || strstr(name, "leftshoulder") || strstr(name, "left shoulder")) return 11;
            if (strstr(name, "l upperarm") || strstr(name, "l_upperarm") || strstr(name, "leftarm") || strstr(name, "left arm")) return 13;
            if (strstr(name, "l forearm") || strstr(name, "l_forearm") || strstr(name, "leftforearm") || strstr(name, "left forearm")) return 15;
            if (strstr(name, "l hand") || strstr(name, "l_hand") || strstr(name, "lefthand") || strstr(name, "left hand")) return 17;
            
            // Braços Direitos
            if (strstr(name, "r clavicle") || strstr(name, "r_clavicle") || strstr(name, "rightshoulder") || strstr(name, "right shoulder")) return 12;
            if (strstr(name, "r upperarm") || strstr(name, "r_upperarm") || strstr(name, "rightarm") || strstr(name, "right arm")) return 14;
            if (strstr(name, "r forearm") || strstr(name, "r_forearm") || strstr(name, "rightforearm") || strstr(name, "right forearm")) return 16;
            if (strstr(name, "r hand") || strstr(name, "r_hand") || strstr(name, "righthand") || strstr(name, "right hand")) return 18;
            
            // Pernas Esquerdas
            if (strstr(name, "l thigh") || strstr(name, "l_thigh") || strstr(name, "leftupleg") || strstr(name, "left upleg") || strstr(name, "left thigh")) return 1;
            if (strstr(name, "l calf") || strstr(name, "l_calf") || strstr(name, "leftleg") || strstr(name, "left leg") || strstr(name, "left calf")) return 3;
            if (strstr(name, "l foot") || strstr(name, "l_foot") || strstr(name, "leftfoot") || strstr(name, "left foot")) return 5;
            if (strstr(name, "l toe") || strstr(name, "l_toe") || strstr(name, "lefttoe") || strstr(name, "left toe")) return 5;
            
            // Pernas Direitas
            if (strstr(name, "r thigh") || strstr(name, "r_thigh") || strstr(name, "rightupleg") || strstr(name, "right upleg") || strstr(name, "right thigh")) return 2;
            if (strstr(name, "r calf") || strstr(name, "r_calf") || strstr(name, "rightleg") || strstr(name, "right leg") || strstr(name, "right calf")) return 4;
            if (strstr(name, "r foot") || strstr(name, "r_foot") || strstr(name, "rightfoot") || strstr(name, "right foot")) return 6;
            if (strstr(name, "r toe") || strstr(name, "r_toe") || strstr(name, "righttoe") || strstr(name, "right toe")) return 6;
            
            // Se for exatamente "bip01" e não "bip01 xxx", assumimos como pelvis/root
            if (strcmp(name, "bip01") == 0) return 0;
            
            return -1;
        };

        std::function<void(void*, CachedAnimatorInfo*)> cacheGenericBones = [&](void* transform, CachedAnimatorInfo* info) {
            if (!transform || !getNameMethod || !getChildCountMethod || !getChildMethod) return;
            
            void* nameObj = Il2Cpp::runtime_invoke(getNameMethod, transform, nullptr, nullptr);
            if (nameObj) {
                int len = *(int*)((uintptr_t)nameObj + 0x10);
                const char* chars = (const char*)((uintptr_t)nameObj + 0x14);
                char cStr[128] = {0};
                for (int i = 0; i < len && i < 127; ++i) cStr[i] = chars[i*2];
                
                int bId = mapNameToBoneId(cStr);
                if (bId != -1 && info->boneTransforms.find(bId) == info->boneTransforms.end()) {
                    info->boneTransforms[bId] = transform;
                }
            }
            
            void* countExc = nullptr;
            void* countObj = Il2Cpp::runtime_invoke(getChildCountMethod, transform, nullptr, &countExc);
            if (countObj && !countExc) {
                int childCount = *(int*)((uintptr_t)countObj + 0x10);
                for (int i = 0; i < childCount; ++i) {
                    void* args[1] = { &i };
                    void* childExc = nullptr;
                    void* childTransform = Il2Cpp::runtime_invoke(getChildMethod, transform, args, &childExc);
                    if (childTransform && !childExc) {
                        cacheGenericBones(childTransform, info);
                    }
                }
            }
        };

        auto getBonePosCached = [&](CachedAnimatorInfo* info, int boneId, bool& success) -> Vector3Args {
            success = false;
            Vector3Args pos = {0, 0, 0};
            if (!info || !info->animatorObj || !getPosMethod) return pos;

            void* boneTransform = nullptr;
            auto bit = info->boneTransforms.find(boneId);
            if (bit != info->boneTransforms.end()) {
                boneTransform = bit->second;
            } else {
                if (!info->genericRigCached && getBoneTransformMethod) {
                    void* exc = nullptr;
                    void* args[1] = { &boneId };
                    boneTransform = Il2Cpp::runtime_invoke(getBoneTransformMethod, info->animatorObj, args, &exc);
                    
                    if (exc || !boneTransform) {
                        info->genericRigCached = true;
                        if (getTransformMethod) {
                            void* rootTransform = Il2Cpp::runtime_invoke(getTransformMethod, info->animatorObj, nullptr, nullptr);
                            if (rootTransform) {
                                cacheGenericBones(rootTransform, info);
                            }
                        }
                        bit = info->boneTransforms.find(boneId);
                        if (bit != info->boneTransforms.end()) boneTransform = bit->second;
                    } else {
                        info->boneTransforms[boneId] = boneTransform;
                    }
                } else if (info->genericRigCached) {
                    // Já cacheou a árvore toda, não tenta procurar de novo
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

        if (!methodsSearched) {
            void* cameraKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Camera");
            if (cameraKlass) {
                get_mainMethod = Il2Cpp::GetMethodRecursively(cameraKlass, "get_main", 0);
                worldToScreenMethod = Il2Cpp::GetMethodRecursively(cameraKlass, "WorldToScreenPoint", 1);
            }
            
            componentKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Component");
              if (!componentKlass) {
                  componentKlass = Il2Cpp::GetClass("UnityEngine.dll", "UnityEngine", "Component");
              }
              if (componentKlass) {
                getTransformMethod = Il2Cpp::GetMethodRecursively(componentKlass, "get_transform", 0);
            }
            
            transformKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Transform");
            if (transformKlass) {
                getPosMethod = Il2Cpp::GetMethodRecursively(transformKlass, "get_position", 0);
                getRotMethod = Il2Cpp::GetMethodRecursively(transformKlass, "get_rotation", 0);
                setRotMethod = Il2Cpp::GetMethodRecursively(transformKlass, "set_rotation", 1);
                getChildCountMethod = Il2Cpp::GetMethodRecursively(transformKlass, "get_childCount", 0);
                getChildMethod = Il2Cpp::GetMethodRecursively(transformKlass, "GetChild", 1);
            }
            
            void* objectKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Object");
            if (objectKlass) {
                getNameMethod = Il2Cpp::GetMethodRecursively(objectKlass, "get_name", 0);
            }
            
            quatKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Quaternion");
            if (quatKlass) {
                lookRotMethod = Il2Cpp::GetMethodRecursively(quatKlass, "LookRotation", 1);
            }

            void* physicsKlass = Il2Cpp::GetClass("UnityEngine.PhysicsModule.dll", "UnityEngine", "Physics");
            if (physicsKlass) {
                raycastMethod = Il2Cpp::FindRaycastMethodSafe(physicsKlass, &raycastParamCount);
                if (!raycastMethod) {
                    raycastMethod = Il2Cpp::FindLinecastMethodSafe(physicsKlass, &raycastParamCount);
                }
            }

            methodsSearched = true;
        }

        // Cache do metodo SetAimRotation no Player e outros metodos dependentes do jogador
        if ((!setAimRotationMethod || !getBoneTransformMethod) && sharedState.localPlayerObj) {
            void* playerKlass = Il2Cpp::object_get_class(sharedState.localPlayerObj);
              if (playerKlass) {
                  setAimRotationMethod = Il2Cpp::GetMethodRecursively(playerKlass, "SetAimRotation", 2);
                  isFiringMethod = Il2Cpp::GetMethodRecursively(playerKlass, "IsFiring", 0);
                  getHeadTFMethod = Il2Cpp::GetMethodRecursively(playerKlass, "GetHeadTF", 0);
                    // Busca manual robusta para evitar overloads genéricos (que causam null/crash)
                      getComponentMethod = nullptr;
                      void* iter = nullptr;
                      while (void* method = Il2Cpp::class_get_methods(componentKlass, &iter)) {
                          const char* methodName = Il2Cpp::method_get_name(method);
                          if (strcmp(methodName, "GetComponentInChildren") == 0) {
                              uint32_t pCount = Il2Cpp::method_get_param_count(method);
                              if (pCount > 0) {
                                  void* paramType = Il2Cpp::method_get_param(method, 0);
                                  const char* paramTypeName = Il2Cpp::type_get_name(paramType);
                                  if (paramTypeName && strstr(paramTypeName, "Type")) {
                                      getComponentMethod = method;
                                      if (pCount == 2) break; // Preferência pela sobrecarga de 2 args (Type, bool)
                                  }
                              }
                          }
                      }
                }

              void* animatorKlass = Il2Cpp::GetClass("UnityEngine.AnimationModule.dll", "UnityEngine", "Animator");
              if (!animatorKlass) {
                  animatorKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Animator");
              }
              if (!animatorKlass) {
                  animatorKlass = Il2Cpp::GetClass("UnityEngine.dll", "UnityEngine", "Animator");
              }
              if (animatorKlass) {
                  getBoneTransformMethod = nullptr;
                      void* iter = nullptr;
                      while (void* method = Il2Cpp::class_get_methods(animatorKlass, &iter)) {
                          const char* methodName = Il2Cpp::method_get_name(method);
                          if (strcmp(methodName, "GetBoneTransform") == 0 && Il2Cpp::method_get_param_count(method) == 1) {
                              void* paramType = Il2Cpp::method_get_param(method, 0);
                              const char* paramTypeName = Il2Cpp::type_get_name(paramType);
                              if (paramTypeName && (strstr(paramTypeName, "HumanBodyBones") || strstr(paramTypeName, "Int32"))) {
                                  getBoneTransformMethod = method;
                                  break;
                              }
                          }
                      }
                      if (!getBoneTransformMethod) {
                          // Fallback se a reflexão bruta falhar
                          getBoneTransformMethod = Il2Cpp::GetMethodRecursively(animatorKlass, "GetBoneTransform", 1);
                      }
                  animatorTypeObject = Il2Cpp::type_get_object(Il2Cpp::class_get_type(animatorKlass));
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

        struct QuaternionArgs { float x, y, z, w; };

        float closestAimbotDist = FLT_MAX;
          ImVec2 bestTargetPos = screenCenter;
          Vector3Args bestTargetWorldPosHead = {0, 0, 0};
          Vector3Args bestTargetWorldPosChest = {0, 0, 0};
          bool foundAimbotTarget = false;
            void* bestTargetObj = nullptr;
            std::vector<std::pair<void*, bool>> fovStates;

            for (auto& entity : localEntities) {
                // Atualiza posicao real do pe em tempo real no MainThread para evitar desync
                if (getTransformMethod && entity.obj) {
                    void* excPos = nullptr;
                    void* entityTransform = Il2Cpp::runtime_invoke(getTransformMethod, entity.obj, nullptr, &excPos);
                    if (entityTransform && !excPos) {
                        void* entityPosObj = Il2Cpp::runtime_invoke(getPosMethod, entityTransform, nullptr, &excPos);
                        if (entityPosObj && !excPos) {
                            Vector3Args newPos = *(Vector3Args*)((uintptr_t)entityPosObj + 0x10);
                            entity.position.x = newPos.x;
                            entity.position.y = newPos.y;
                            entity.position.z = newPos.z;
                        }
                    }
                }

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
            
            // Tenta pegar a posicao real da cabeca pelo Transform GetHeadTF()
            Vector3Args posHead = {entity.position.x, entity.position.y + 1.41f, entity.position.z}; // fallback caso falhe
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

            // Declara variáveis de coordenadas de tela
            float xFeet = 0.0f, yFeet = 0.0f;
            float xHead = 0.0f, yHead = 0.0f;
            float boxHeight = 0.0f;
            ImVec2 topLeft, bottomRight;
            ImU32 color = IM_COL32(0, 255, 0, 255);

            // Verifica se esta na frente da camera (Z > 0.0f)
            if (w2sFeet.z > 0.0f) {
                // Converte as coordenadas de tela do Unity (origem bottom-left) para as do ImGui (origem top-left)
                xFeet = w2sFeet.x;
                yFeet = screenSize.y - w2sFeet.y;

                xHead = w2sHead.x;
                yHead = screenSize.y - w2sHead.y;

                boxHeight = yFeet - yHead;
                // Previne calculos absurdos caso as coordenadas estejam distorcidas
                if (boxHeight < 0) boxHeight = -boxHeight; 
                float boxWidth = boxHeight / 2.0f; // Proporcao basica de jogador (largura é metade da altura)

                float centerX = (xHead + xFeet) / 2.0f;
                topLeft = ImVec2(centerX - boxWidth / 2.0f, yHead);
                bottomRight = ImVec2(centerX + boxWidth / 2.0f, yFeet);

                // Usa a flag isVisible atualizada pelo Main Thread Raycast (OnMainThreadTick)
                bool entityVisibleForESP = entity.isVisible;

                if (entity.alignment == Alignment::ENEMY) {
                    if (featureConfig.wallCheckEnabled) {
                        color = entityVisibleForESP ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255);
                    } else {
                        color = IM_COL32(0, 255, 0, 255);
                    }
                }

                // Lógica de FOV (Aimbot) - Checa a distancia para o centro da tela
                  float distToCenter = sqrt(pow(xHead - screenCenter.x, 2) + pow(yHead - screenCenter.y, 2));
                  bool inFov = (distToCenter <= aimbotFov * 1.5f); // 1.5x margin for priority scanning
                  fovStates.push_back({entity.obj, inFov});

                  if (aimbotEnabled) {
                      bool visible = true;
                      // Se o WallCheck global estiver ativo, o Aimbot so mira se estiver visivel
                      if (featureConfig.wallCheckEnabled) {
                          visible = entity.isVisible;
                      }

                      if (visible) {
                          if (distToCenter <= aimbotFov && distToCenter < closestAimbotDist) {
                            closestAimbotDist = distToCenter;
                            bestTargetPos = ImVec2(xHead, yHead);
                            bestTargetWorldPosHead = posHead;
                            
                            // Estimar peito
                            float height = posHead.y - posFeet.y;
                            bestTargetWorldPosChest = { posHead.x, posHead.y - (height * 0.2f), posHead.z };

                            // Tentar obter do esqueleto (Spine = 7, Neck = 9)
                                CachedAnimatorInfo* info = &cachedAnimators[entity.obj];
                                auto nowMsLocal = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                                if ((!info->hasAttempted || (nowMsLocal - info->lastAttemptMs > 2000 && !info->animatorObj)) && animatorTypeObject) {
                                      void* exc = nullptr;
                                      bool includeInactive = true;
                                      void* args[2] = { animatorTypeObject, &includeInactive };
                                      uint32_t pCount = Il2Cpp::method_get_param_count(getComponentMethod);
                                      void* animObj = Il2Cpp::runtime_invoke(getComponentMethod, entity.obj, pCount == 2 ? args : &args[0], &exc);
                                      info->animatorObj = (!exc) ? animObj : nullptr;
                                      info->hasAttempted = true;
                                    info->lastAttemptMs = nowMsLocal;
                                }
                            if (info->animatorObj) {
                                bool successSpine = false;
                                Vector3Args spinePos = getBonePosCached(info, 7, successSpine);
                                if (successSpine) bestTargetWorldPosChest = spinePos;
                            }
                            
                            foundAimbotTarget = true;
                            bestTargetObj = entity.obj;
                            
                            // Guarda informacoes de Debug
                            aimbotTargetName = entity.name;
                            aimbotTargetDistFOV = distToCenter;
                            aimbotTargetDist3D = entity.distanceToLocal;
                        }
                    }
                }
                
                if (espEnabled) {
                    if (espBox) {
                        // Desenha o retangulo (Box ESP)
                        if (espBoxMode == 0) {
                            drawList->AddRect(topLeft, bottomRight, color, 0.0f, 0, 1.5f);
                        } else {
                            // Outline
                            drawList->AddRect(ImVec2(topLeft.x - 1, topLeft.y - 1), ImVec2(bottomRight.x + 1, bottomRight.y + 1), IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.5f);
                            drawList->AddRect(ImVec2(topLeft.x + 1, topLeft.y + 1), ImVec2(bottomRight.x - 1, bottomRight.y - 1), IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.5f);
                            drawList->AddRect(topLeft, bottomRight, color, 0.0f, 0, 1.5f);
                        }
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

                if (espSkeleton) {
                    if (!entity.obj) {
                          drawList->AddText(topLeft, IM_COL32(255, 0, 0, 255), "entity.obj == null");
                      } else if (!getComponentMethod) {
                          drawList->AddText(topLeft, IM_COL32(255, 0, 0, 255), "getComponentMethod == null");
                      } else if (!animatorTypeObject) {
                          drawList->AddText(topLeft, IM_COL32(255, 0, 0, 255), "animatorTypeObject == null");
                      } else if (!getBoneTransformMethod) {
                          drawList->AddText(topLeft, IM_COL32(255, 0, 0, 255), "getBoneTransformMethod == null");
                      } else if (!getPosMethod) {
                          drawList->AddText(topLeft, IM_COL32(255, 0, 0, 255), "getPosMethod == null");
                      } else {
                          CachedAnimatorInfo* info = &cachedAnimators[entity.obj];
                        auto nowMsLocal = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
                        if (!info->hasAttempted || (nowMsLocal - info->lastAttemptMs > 2000 && !info->animatorObj)) {
                              void* exc = nullptr;
                              bool includeInactive = true;
                              void* args[2] = { animatorTypeObject, &includeInactive };
                              uint32_t pCount = Il2Cpp::method_get_param_count(getComponentMethod);
                              void* animObj = Il2Cpp::runtime_invoke(getComponentMethod, entity.obj, pCount == 2 ? args : &args[0], &exc);
                              info->animatorObj = (!exc) ? animObj : nullptr;
                              info->hasAttempted = true;
                              info->lastAttemptMs = nowMsLocal;
                          }
                        
                        if (!info->animatorObj) {
                            drawList->AddText(topLeft, IM_COL32(255, 0, 0, 255), "animatorObj == null");
                        } else {
                            bool hasBone[30] = {false};
                            ImVec2 boneScreen[30];
                            
                            // Lista de bones que precisamos para o esqueleto basico (HumanBodyBones do Unity)
                            int requiredBones[] = { 0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
                        
                            for (int i = 0; i < sizeof(requiredBones) / sizeof(int); ++i) {
                                int bId = requiredBones[i];
                                bool success = false;
                                Vector3Args wPos = getBonePosCached(info, bId, success);
                                if (success) {
                                    void* args[1] = { &wPos };
                                    void* exc2 = nullptr;
                                    void* w2sObj = Il2Cpp::runtime_invoke(worldToScreenMethod, mainCamera, args, &exc2);
                                    if (w2sObj && !exc2) {
                                        Vector3Args sPos = *(Vector3Args*)((uintptr_t)w2sObj + 0x10);
                                        if (sPos.z > 0) {
                                            boneScreen[bId] = ImVec2(sPos.x, screenSize.y - sPos.y);
                                            hasBone[bId] = true;
                                        }   
                                    }
                                }
                            }
                            
                            auto drawBoneLine = [&](int b1, int b2) {
                                  if (hasBone[b1] && hasBone[b2]) {
                                      drawList->AddLine(boneScreen[b1], boneScreen[b2], color, 1.5f);
                                  }
                              };
                            
                            // Desenha Esqueleto
                            drawBoneLine(10, 9);  // Head -> Neck
                            drawBoneLine(9, 7);   // Neck -> Spine
                            drawBoneLine(7, 0);   // Spine -> Hips
                            
                            // Bracos (Esquerdo)
                            drawBoneLine(9, 11);  // Neck -> LeftShoulder
                            drawBoneLine(11, 13); // LeftShoulder -> LeftUpperArm
                            drawBoneLine(13, 15); // LeftUpperArm -> LeftLowerArm
                            drawBoneLine(15, 17); // LeftLowerArm -> LeftHand
                            
                            // Bracos (Direito)
                            drawBoneLine(9, 12);  // Neck -> RightShoulder
                            drawBoneLine(12, 14); // RightShoulder -> RightUpperArm
                            drawBoneLine(14, 16); // RightUpperArm -> RightLowerArm
                            drawBoneLine(16, 18); // RightLowerArm -> RightHand
                            
                            // Pernas (Esquerda)
                            drawBoneLine(0, 1);   // Hips -> LeftUpperLeg
                            drawBoneLine(1, 3);   // LeftUpperLeg -> LeftLowerLeg
                            drawBoneLine(3, 5);   // LeftLowerLeg -> LeftFoot
                            
                            // Pernas (Direita)
                              drawBoneLine(0, 2);   // Hips -> RightUpperLeg
                              drawBoneLine(2, 4);   // RightUpperLeg -> RightLowerLeg
                              drawBoneLine(4, 6);   // RightLowerLeg -> RightFoot

                              bool anySuccess = false;
                              for (int i = 0; i < 30; ++i) if (hasBone[i]) anySuccess = true;
                              if (!anySuccess) {
                                  // Debug text to understand why no bones are drawing
                                  void* excDebug = nullptr;
                                    int bIdDebug = 10; // Head
                                    void* argsDebug[1] = { &bIdDebug };
                                    void* boneTransformDebug = Il2Cpp::runtime_invoke(getBoneTransformMethod, info->animatorObj, argsDebug, &excDebug);
                                    if (excDebug) {
                                        std::string excMsg = "Exception";
                                        void* exceptionKlass = Il2Cpp::object_get_class(excDebug);
                                        if (exceptionKlass) {
                                            void* getMessageMethod = Il2Cpp::GetMethodRecursively(exceptionKlass, "get_Message", 0);
                                            if (getMessageMethod) {
                                                void* strObj = Il2Cpp::runtime_invoke(getMessageMethod, excDebug, nullptr, nullptr);
                                                if (strObj) {
                                                    const char* strChars = (const char*)((uintptr_t)strObj + 0x14);
                                                    int strLen = *(int*)((uintptr_t)strObj + 0x10);
                                                    char cStr[128] = {0};
                                                    for (int c = 0; c < strLen && c < 127; ++c) {
                                                        cStr[c] = strChars[c * 2];
                                                    }
                                                    excMsg = std::string(cStr);
                                                }
                                            }
                                        }
                                        std::string drawStr = "GetBoneTransform throw: " + excMsg;
                                        drawList->AddText(topLeft, IM_COL32(255, 165, 0, 255), drawStr.c_str());
                                    } else if (!boneTransformDebug) {
                                      drawList->AddText(topLeft, IM_COL32(255, 165, 0, 255), "boneTransform is null");
                                  } else {
                                      void* posObjDebug = Il2Cpp::runtime_invoke(getPosMethod, boneTransformDebug, nullptr, &excDebug);
                                      if (excDebug) {
                                          drawList->AddText(topLeft, IM_COL32(255, 165, 0, 255), "getPos threw exception");
                                      } else if (!posObjDebug) {
                                          drawList->AddText(topLeft, IM_COL32(255, 165, 0, 255), "posObj is null");
                                      } else {
                                          drawList->AddText(topLeft, IM_COL32(255, 165, 0, 255), "W2S or Z<0 failed");
                                      }
                                  }
                              }
                          }
                      }
                  } // Fim do espSkeleton

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
            drawList->AddLine(screenCenter, bestTargetPos, IM_COL32(255, 0, 0, 200), 1.0f);
            drawList->AddCircle(bestTargetPos, 5.0f, IM_COL32(255, 0, 0, 200), 12, 1.0f);

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
                                
                                // Usa estritamente a distância 3D original que o alvo estava do jogador
                                float magDist = aimbotTargetDist3D;
                                if (magDist < 1.0f) magDist = 1.0f; // Evita puxar para dentro da câmera
                                
                                // A posição magnética será: Câmera + (Forward * Distância Original)
                                Vector3Args magneticPos = {
                                    camPos.x + forward.x * magDist,
                                    camPos.y + forward.y * magDist,
                                    camPos.z + forward.z * magDist
                                };
                                
                                // Descida suave para a cabeça colar na mira
                                float heightDiff = bestTargetWorldPosHead.y - bestTargetWorldPosChest.y;
                                magneticPos.y -= (heightDiff * 4.5f); 

                                void* setPosMethod = Il2Cpp::GetMethodRecursively(transformKlass, "set_position", 1);
                                if (setPosMethod) {
                                    void* argsSetPos[1] = { &magneticPos };
                                    Il2Cpp::runtime_invoke(setPosMethod, targetTransform, argsSetPos, &exc);
                                    if (!exc) {
                                        aimbotErrorLog = "Inimigo puxado pela Mira Magnetica (Sempre Ativo)!";
                                        return; // Sai da função de render para pular a lógica normal de girar a câmera
                                    }
                                }
                            }
                        }
                    }
                }
            }

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
                // Atualizar timer do alvo atual
                aimbotTargetTimeMap[bestTargetObj] += deltaTime * 1000.0f; // ms
                
                // Limpar timers antigos
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

                        // Calcular interpolacao Peito -> Cabeca
                        float timeOnTarget = aimbotTargetTimeMap[bestTargetObj];
                        float t = 0.0f;
                        if (aimbotTransitionTimeMs > 0.0f) {
                            t = timeOnTarget / aimbotTransitionTimeMs;
                            if (t > 1.0f) t = 1.0f;
                            // Aplicar curva de aceleracao agressiva
                            t = pow(t, aimbotTransitionCurve);
                        } else {
                            t = 1.0f; // Direto na cabeca se o tempo for 0
                        }

                        Vector3Args currentTargetPos = {
                            bestTargetWorldPosChest.x + (bestTargetWorldPosHead.x - bestTargetWorldPosChest.x) * t,
                            bestTargetWorldPosChest.y + (bestTargetWorldPosHead.y - bestTargetWorldPosChest.y) * t,
                            bestTargetWorldPosChest.z + (bestTargetWorldPosHead.z - bestTargetWorldPosChest.z) * t
                        };

                        // Vetor direcional para o alvo (Cabeca ou Peito dependendo do t)
                        Vector3Args dir = { currentTargetPos.x - camPos.x, currentTargetPos.y - camPos.y, currentTargetPos.z - camPos.z };

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

          // Sync fov states back to shared state so MainThread can prioritize raycasts
          if (!fovStates.empty()) {
              std::lock_guard<std::mutex> lock(sharedState.mtx);
              for (auto& sharedEnt : sharedState.entities) {
                  for (const auto& state : fovStates) {
                      if (sharedEnt.obj == state.first) {
                          sharedEnt.isInFov = state.second;
                          break;
                      }
                  }
              }
          }
      }

    void GhostSystems::Menu::drawEntityProperties(void* obj, int depth) {
        if (!obj) {
            ImGui::Text("Objeto nulo");
            return;
        }

        if (depth > 5) { // Limite de recursão
            ImGui::Text("Limite de profundidade atingido");
            return;
        }

        void* klass = Il2Cpp::object_get_class(obj);
        if (!klass) {
            ImGui::Text("Classe nao encontrada");
            return;
        }

        const char* className = Il2Cpp::class_get_name(klass);
        if (depth == 0) {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Classe: %s", className ? className : "Unknown");
        }

        void* iter = nullptr;
        void* field;

        std::string tableId = "PropsTable_" + std::to_string((uintptr_t)obj) + "_" + std::to_string(depth);

        if (ImGui::BeginTable(tableId.c_str(), 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
            ImGui::TableSetupColumn("Campo", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Tipo", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Valor", ImGuiTableColumnFlags_WidthStretch);
            if (depth == 0) ImGui::TableHeadersRow();

            while ((field = Il2Cpp::class_get_fields(klass, &iter)) != nullptr) {
                const char* fieldName = Il2Cpp::field_get_name(field);
                void* fieldType = Il2Cpp::field_get_type(field);
                char* typeName = fieldType ? Il2Cpp::type_get_name(fieldType) : nullptr;
                size_t offset = Il2Cpp::field_get_offset(field);
                std::string tName = typeName ? typeName : "";

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                
                bool isExpanded = false;
                void* childObj = nullptr;

                if (offset > 0 && offset < 0x2000) {
                    void* fieldAddr = (void*)((uintptr_t)obj + offset);
                    if (tName.find("System.") == std::string::npos && tName.find("int") == std::string::npos && 
                        tName.find("float") == std::string::npos && tName.find("bool") == std::string::npos &&
                        tName.find("string") == std::string::npos) {
                        childObj = *(void**)fieldAddr;
                        if (childObj && (uintptr_t)childObj > 0x10000) {
                            isExpanded = ImGui::TreeNodeEx((void*)(uintptr_t)field, ImGuiTreeNodeFlags_SpanFullWidth, "%s", fieldName ? fieldName : "Unknown");
                        } else {
                            ImGui::Text("%s", fieldName ? fieldName : "Unknown");
                        }
                    } else {
                        ImGui::Text("%s", fieldName ? fieldName : "Unknown");
                    }
                } else {
                    ImGui::Text("%s", fieldName ? fieldName : "Unknown");
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", typeName ? typeName : "Unknown");

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("0x%X", (unsigned int)offset);

                ImGui::TableSetColumnIndex(3);
                if (offset > 0 && offset < 0x2000) {
                    void* fieldAddr = (void*)((uintptr_t)obj + offset);

                    if (tName == "System.Int32" || tName == "int") {
                        ImGui::Text("%d", *(int*)fieldAddr);
                    } else if (tName == "System.Single" || tName == "float") {
                        ImGui::Text("%f", *(float*)fieldAddr);
                    } else if (tName == "System.Boolean" || tName == "bool") {
                        ImGui::Text("%s", *(bool*)fieldAddr ? "True" : "False");
                    } else if (tName == "System.String" || tName == "string") {
                        void* strObj = *(void**)fieldAddr;
                        if (strObj && (uintptr_t)strObj > 0x10000) {
                            int len = *(int*)((uintptr_t)strObj + 0x10);
                            if (len > 0 && len < 100) {
                                // Tentar ler os chars (UTF-16)
                                char16_t* chars = (char16_t*)((uintptr_t)strObj + 0x14);
                                char buffer[256] = {0};
                                for (int i = 0; i < len && i < 255; i++) {
                                    buffer[i] = (char)chars[i];
                                }
                                ImGui::Text("\"%s\"", buffer);
                            } else {
                                ImGui::Text("String[len=%d]", len);
                            }
                        } else {
                            ImGui::Text("null");
                        }
                    } else {
                        ImGui::Text("%p", *(void**)fieldAddr);
                    }
                } else {
                    ImGui::Text("N/A");
                }

                if (isExpanded) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Indent();
                    drawEntityProperties(childObj, depth + 1);
                    ImGui::Unindent();
                    ImGui::TreePop();
                }

                if (typeName && Il2Cpp::free_func) {
                    Il2Cpp::free_func(typeName);
                }
            }
            ImGui::EndTable();
        }
    }

    void GhostSystems::Menu::drawBypass() {
        if (ImGui::CollapsingHeader(OBFUSCATE("## Bypass Network Monitor"), ImGuiTreeNodeFlags_DefaultOpen)) {
            bool wasEnabled = bypassEnabled;
            ImGui::Checkbox(OBFUSCATE("Ativar Bypass"), &bypassEnabled);

            if (bypassEnabled && !wasEnabled) {
                hookUnityWebRequest();
            } else if (!bypassEnabled && wasEnabled) {
                unhookUnityWebRequest();
            }

            if (!bypassEnabled) {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Bypass desativado - Ative para capturar requisicoes");
                return;
            }

            ImGui::Separator();
            drawBypassStats();
            ImGui::Separator();

            if (ImGui::BeginTabBar("BypassTabs")) {
                if (ImGui::BeginTabItem("Dashboard")) {
                    drawBypassDashboard();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Config")) {
                    drawBypassConfig();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Requests")) {
                    drawBypassList();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Details")) {
                    drawBypassDetails();
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
    }

    void GhostSystems::Menu::drawBypassConfig() {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "=== Meus Dados ===");
        ImGui::InputText("Player ID", myPlayerId, IM_ARRAYSIZE(myPlayerId));
        ImGui::InputText("Player Name", myPlayerName, IM_ARRAYSIZE(myPlayerName));
        ImGui::Checkbox(OBFUSCATE("Auto-Detectar Meus Dados"), &bypassAutoDetectMyData);

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "=== Captura ===");
        ImGui::Checkbox(OBFUSCATE("Capturar Todas as Requisicoes"), &bypassCaptureAll);
        ImGui::SliderInt(OBFUSCATE("Max Requisicoes"), &bypassMaxRequests, 100, 5000, "%d");

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "=== Visualizacao ===");
        ImGui::Checkbox(OBFUSCATE("Auto-Scroll"), &bypassAutoScroll);
        ImGui::Checkbox(OBFUSCATE("Hex View (Body)"), &bypassHexView);
        ImGui::Checkbox(OBFUSCATE("Beautify JSON"), &bypassJsonBeautify);
        ImGui::Checkbox(OBFUSCATE("Highlight Matches"), &bypassHighlightMatches);
        ImGui::Checkbox(OBFUSCATE("Case Sensitive"), &bypassCaseSensitive);

        const char* viewModes[] = { "List", "List+Details", "Compact" };
        ImGui::Combo(OBFUSCATE("View Mode"), &bypassViewMode, viewModes, IM_ARRAYSIZE(viewModes));

        const char* sortOptions[] = { "Mais Recentes", "ID", "Metodo", "Suspeitas Primeiro" };
        ImGui::Combo(OBFUSCATE("Ordenar Por"), &bypassSortBy, sortOptions, IM_ARRAYSIZE(sortOptions));

        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "=== Keywords Suspeitas ===");

        ImGui::TextWrapped("Padrao: antic heat, report, cheat, hack, ghost, violacao, ban, telemetry, vguard, xigncode, battleye, vac, hacktool, exploit, injection, packet, speedhack, wallhack, aimbot, esp, glow, chams, triggerbot");

        ImGui::Text("\nCustom Keywords:");
        ImGui::InputText("Add Keyword", bypassSearchFilter, 64);
        ImGui::SameLine();
        if (ImGui::Button("+")) {
            if (strlen(bypassSearchFilter) > 0) {
                bypassCustomKeywords.push_back(std::string(bypassSearchFilter));
                bypassSearchFilter[0] = '\0';
            }
        }

        for (int i = 0; i < (int)bypassCustomKeywords.size(); i++) {
            ImGui::SameLine();
            ImGui::PushID(i);
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "[x]");
            if (ImGui::IsItemClicked()) {
                bypassCustomKeywords.erase(bypassCustomKeywords.begin() + i);
            }
            ImGui::PopID();
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "=== Acoes ===");
        if (ImGui::Button(OBFUSCATE("Limpar Tudo"))) {
            bypassAllRequests.clear();
            bypassSuspectRequests.clear();
            bypassSelectedRequestIdx = -1;
            bypassRequestDetails.clear();
            bypassStatsDirty = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(OBFUSCATE("Limpar Suspeitas"))) {
            bypassSuspectRequests.clear();
            bypassStatsDirty = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(OBFUSCATE("Export All"))) {
            exportAllToFile("/sdcard/Download/GhostSystems_Requests.log");
        }
    }

    void GhostSystems::Menu::drawBypassStats() {
        if (bypassStatsDirty) {
            calculateBypassStats();
            bypassStatsDirty = false;
        }

        if (bypassCriticalAlert && bypassCriticalCount > 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));

            char alertText[128];
            snprintf(alertText, sizeof(alertText), "ALERTA! %d request(s) CRITICO(s) detectado(s)!", bypassCriticalCount);
            if (ImGui::Button(alertText, ImVec2(0, 35))) {
                bypassCriticalAlert = false;
                bypassShowSuspectsOnly = true;
            }

            ImGui::PopStyleColor(4);
        }

        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "=== ESTATISTICAS ===");

        ImGui::BeginGroup();
        ImGui::Text("GET: %d", bypassStatsGet);
        ImGui::SameLine();
        ImGui::Text(" | POST: %d", bypassStatsPost);
        ImGui::SameLine();
        ImGui::Text(" | PUT: %d", bypassStatsPut);
        ImGui::SameLine();
        ImGui::Text(" | OUTROS: %d", bypassStatsOther);
        ImGui::EndGroup();

        ImGui::SameLine();

        if (bypassStatsSuspect > 0) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), " | SUSPEITAS: %d", bypassStatsSuspect);
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), " | SUSPEITAS: 0");
        }

        ImGui::Text("Total em Memoria: %d / %d", (int)bypassAllRequests.size(), bypassMaxRequests);
    }

    void GhostSystems::Menu::calculateBypassStats() {
        bypassStatsGet = 0;
        bypassStatsPost = 0;
        bypassStatsPut = 0;
        bypassStatsOther = 0;
        bypassStatsSuspect = 0;

        for (auto& req : bypassAllRequests) {
            switch (req.method) {
                case METHOD_GET: bypassStatsGet++; break;
                case METHOD_POST: bypassStatsPost++; break;
                case METHOD_PUT: bypassStatsPut++; break;
                default: bypassStatsOther++; break;
            }
            if (req.isSuspect) bypassStatsSuspect++;
        }
    }

    void GhostSystems::Menu::drawBypassList() {
        ImGui::Checkbox(OBFUSCATE("Mostrar Apenas Suspeitas"), &bypassShowSuspectsOnly);

        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        ImGui::InputText("##methodFilter", bypassMethodFilter, 16);
        ImGui::SameLine();
        ImGui::Text("Metodo");

        ImGui::SameLine();
        ImGui::SetNextItemWidth(200);
        ImGui::InputText("##urlFilter", bypassUrlFilter, 64);
        ImGui::SameLine();
        ImGui::Text("URL");

        ImGui::SameLine();
        ImGui::Checkbox(OBFUSCATE("Buscar Body"), &bypassSearchBody);
        if (bypassSearchBody) {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(200);
            ImGui::InputText("##bodySearch", bypassBodySearch, 64);
            ImGui::SameLine();
            ImGui::Text("Body");
        }

        std::vector<Menu::HttpRequest> displayList;
        {
            std::lock_guard<std::mutex> lock(bypassMutex);
            displayList = bypassShowSuspectsOnly ? bypassSuspectRequests : bypassAllRequests;
        }

        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Mostrando %d de %d requisicoes",
            (int)displayList.size(),
            bypassShowSuspectsOnly ? (int)bypassSuspectRequests.size() : (int)bypassAllRequests.size());

        if (ImGui::BeginTable("RequestsTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 320))) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40.0f);
            ImGui::TableSetupColumn("Hora", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Metodo", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Endpoint", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 70.0f);
            ImGui::TableHeadersRow();

            for (int i = 0; i < (int)displayList.size(); i++) {
                Menu::HttpRequest& req = displayList[i];

                if (strlen(bypassUrlFilter) > 0) {
                    if (req.url.find(bypassUrlFilter) == std::string::npos &&
                        req.endpoint.find(bypassUrlFilter) == std::string::npos) continue;
                }

                if (strlen(bypassMethodFilter) > 0) {
                    std::string methodUpper = req.methodStr;
                    std::string filterUpper = bypassMethodFilter;
                    for (char& c : methodUpper) c = toupper(c);
                    for (char& c : filterUpper) c = toupper(c);
                    if (methodUpper.find(filterUpper) == std::string::npos) continue;
                }

                if (bypassSearchBody && strlen(bypassBodySearch) > 0) {
                    if (req.body.find(bypassBodySearch) == std::string::npos &&
                        req.raw.find(bypassBodySearch) == std::string::npos) continue;
                }

                ImGui::TableNextRow();

                ImVec4 methodColor = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);
                if (req.method == METHOD_POST) methodColor = ImVec4(0.0f, 0.6f, 1.0f, 1.0f);
                else if (req.method == METHOD_PUT) methodColor = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
                else if (req.method == METHOD_DELETE) methodColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%lu", req.id);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", req.timestamp.c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::PushStyleColor(ImGuiCol_Text, methodColor);
                ImGui::Text("%s", req.methodStr.c_str());
                ImGui::PopStyleColor();

                ImGui::TableSetColumnIndex(3);
                bool hasPlayerData = containsPlayerData(req.url + req.body);
                ImVec4 endpointColor = req.isSuspect ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : (hasPlayerData ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, endpointColor);

                std::string endpointShort = req.endpoint.length() > 50 ? req.endpoint.substr(0, 50) + "..." : req.endpoint;
                if (bypassHighlightPlayerData && hasPlayerData) {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[P]");
                }
                ImGui::SameLine();
                ImGui::Text("%s", endpointShort.c_str());
                ImGui::PopStyleColor();

                ImGui::TableSetColumnIndex(4);
                if (req.isSuspect) {
                    ImVec4 levelColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                    if (req.suspectLevel == SUSPECT_CRITICAL) levelColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                    else if (req.suspectLevel == SUSPECT_HIGH) levelColor = ImVec4(1.0f, 0.3f, 0.0f, 1.0f);
                    else if (req.suspectLevel == SUSPECT_MEDIUM) levelColor = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
                    else if (req.suspectLevel == SUSPECT_LOW) levelColor = ImVec4(0.6f, 0.6f, 0.0f, 1.0f);

                    ImGui::PushStyleColor(ImGuiCol_Text, levelColor);
                    const char* levelStr = "LOW";
                    if (req.suspectLevel == SUSPECT_MEDIUM) levelStr = "MED";
                    else if (req.suspectLevel == SUSPECT_HIGH) levelStr = "HIGH";
                    else if (req.suspectLevel == SUSPECT_CRITICAL) levelStr = "CRIT";
                    ImGui::Text("%s", levelStr);
                    ImGui::PopStyleColor();
                } else {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "OK");
                }

                if (ImGui::Selectable("##row", false, ImGuiSelectableFlags_SpanAllColumns)) {
                    std::lock_guard<std::mutex> lock(bypassMutex);
                    bypassSelectedRequestIdx = i;
                    bypassSelectedRequestList = bypassShowSuspectsOnly ? 1 : 0;
                }
            }
            ImGui::EndTable();
        }

        if (bypassAutoScroll && (int)displayList.size() > 0) {
            ImGui::SetScrollHereY(1.0f);
        }

        if (ImGui::Button(OBFUSCATE("Ver Detalhes")) && bypassSelectedRequestIdx >= 0) {
            ImGui::SetWindowFocus("DetailsTab");
        }
        ImGui::SameLine();
        if (ImGui::Button(OBFUSCATE("Exportar Filtradas"))) {
            char path[256];
            snprintf(path, sizeof(path), "/sdcard/Download/GhostSystems_Filtered_%ld.log", time(nullptr));
            for (auto& req : displayList) {
                exportRequestToFile(req, path);
            }
        }
    }

    void GhostSystems::Menu::drawBypassDetails() {
        if (bypassSelectedRequestIdx < 0) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Selecione uma requisicao na aba Requests");
            return;
        }

        std::vector<Menu::HttpRequest>& displayList = bypassSelectedRequestList == 1 ? bypassSuspectRequests : bypassAllRequests;
        if (bypassSelectedRequestIdx >= (int)displayList.size()) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Indice invalido!");
            return;
        }

        Menu::HttpRequest& req = displayList[bypassSelectedRequestIdx];

        if (ImGui::BeginTabBar("DetailsTabs")) {
            if (ImGui::BeginTabItem("Overview")) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "=== REQUISICAO #%lu ===", req.id);

                ImGui::Columns(2, "OverviewCols", true);
                ImGui::SetColumnWidth(0, 120);
                ImGui::Text("Timestamp:"); ImGui::NextColumn();
                ImGui::Text("%s", req.timestamp.c_str()); ImGui::NextColumn();

                ImGui::Text("Method:"); ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "%s", req.methodStr.c_str()); ImGui::NextColumn();

                ImGui::Text("URL:"); ImGui::NextColumn();
                ImGui::TextWrapped("%s", req.url.c_str()); ImGui::NextColumn();

                ImGui::Text("Endpoint:"); ImGui::NextColumn();
                ImGui::Text("%s", req.endpoint.c_str()); ImGui::NextColumn();

                ImGui::Text("Domain:"); ImGui::NextColumn();
                ImGui::Text("%s", req.parsedUrl.domain.c_str()); ImGui::NextColumn();

                ImGui::Text("Body Size:"); ImGui::NextColumn();
                ImGui::Text("%d bytes", req.bodySize); ImGui::NextColumn();

                ImGui::Text("Total Size:"); ImGui::NextColumn();
                ImGui::Text("%d bytes", req.totalSize); ImGui::NextColumn();

                ImGui::Text("Response Time:"); ImGui::NextColumn();
                ImGui::Text("%.2f ms", req.responseTimeMs); ImGui::NextColumn();

                ImGui::Text("Status:"); ImGui::NextColumn();
                if (req.isSuspect) {
                    ImVec4 levelColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                    if (req.suspectLevel == SUSPECT_CRITICAL) levelColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                    else if (req.suspectLevel == SUSPECT_HIGH) levelColor = ImVec4(1.0f, 0.3f, 0.0f, 1.0f);
                    else if (req.suspectLevel == SUSPECT_MEDIUM) levelColor = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
                    else levelColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                    ImGui::TextColored(levelColor, "SUSPECT - %s", req.suspectReason.c_str());
                } else {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "CLEAN");
                }
                ImGui::NextColumn();

                if (!req.matchedPatterns.empty()) {
                    ImGui::Text("Patterns:"); ImGui::NextColumn();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
                    std::string patterns;
                    for (auto& p : req.matchedPatterns) patterns += p + ", ";
                    ImGui::TextWrapped("%s", patterns.c_str());
                    ImGui::PopStyleColor();
                }
                ImGui::NextColumn();

                ImGui::Columns(1);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Headers")) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "=== HEADERS (%d) ===", (int)req.headers.size());

                if (ImGui::BeginTable("HeadersTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 200.0f);
                    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    for (auto& h : req.headers) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "%s", h.name.c_str());
                        ImGui::TableSetColumnIndex(1);

                        bool isPlayerData = containsPlayerData(h.value);
                        bool isSuspicious = false;
                        for (auto& kw : bypassSuspectKeywords) {
                            if (h.value.find(kw) != std::string::npos) {
                                isSuspicious = true;
                                break;
                            }
                        }

                        if (isPlayerData) {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                            ImGui::TextWrapped("%s", h.value.c_str());
                            ImGui::PopStyleColor();
                        } else if (isSuspicious) {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
                            ImGui::TextWrapped("%s", h.value.c_str());
                            ImGui::PopStyleColor();
                        } else {
                            ImGui::TextWrapped("%s", h.value.c_str());
                        }
                    }
                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Body")) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "=== BODY (%d bytes) ===", req.bodySize);

                ImGui::Checkbox(OBFUSCATE("Hex View"), &bypassHexView);
                ImGui::SameLine();
                ImGui::Checkbox(OBFUSCATE("Beautify JSON"), &bypassJsonBeautify);
                ImGui::SameLine();
                ImGui::Checkbox(OBFUSCATE("Highlight Player Data"), &bypassHighlightPlayerData);

                ImGui::BeginChild("BodyContent", ImVec2(0, 300), true);

                if (req.body.empty()) {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(empty body)");
                } else {
                    std::string displayBody = req.body;
                    if (bypassJsonBeautify && displayBody.find('{') != std::string::npos) {
                        formatJson(displayBody);
                    }

                    if (bypassHexView) {
                        displayBody = bytesToHex(displayBody);
                    }

                    if (bypassHighlightPlayerData && bypassSelectedRequestIdx >= 0) {
                        std::vector<Menu::HttpRequest>& listSrc = bypassSelectedRequestList == 1 ? bypassSuspectRequests : bypassAllRequests;
                        if (bypassSelectedRequestIdx < (int)listSrc.size()) {
            if (strlen(myPlayerId) > 0) {
                                size_t pos = displayBody.find(myPlayerId);
                                if (pos != std::string::npos) {
                                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                                    ImGui::Text("(Player ID)");
                                    ImGui::PopStyleColor();
                                    ImGui::SameLine();
                                }
                            }
                        }
                    }

                    ImGui::TextWrapped("%s", displayBody.c_str());
                }

                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Raw")) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "=== RAW COMPLETO ===");
                ImGui::BeginChild("RawContent", ImVec2(0, 400), true);
                ImGui::TextWrapped("%s", req.raw.c_str());
                ImGui::EndChild();

                if (ImGui::Button(OBFUSCATE("Copy Raw"))) {
                    ImGui::SetClipboardText(req.raw.c_str());
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Response")) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "=== RESPONSE (%d bytes) ===", (int)req.response.size());
                ImGui::BeginChild("ResponseContent", ImVec2(0, 400), true);

                if (req.response.empty()) {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(no response captured)");
                } else {
                    std::string displayResp = req.response;
                    if (bypassJsonBeautify && displayResp.find('{') != std::string::npos) {
                        formatJson(displayResp);
                    }
                    ImGui::TextWrapped("%s", displayResp.c_str());
                }

                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::Separator();
        if (ImGui::Button(OBFUSCATE("Exportar Esta Requisicao"))) {
            exportRequestToFile(req, "/sdcard/Download/GhostSystems_Request.log");
        }
    }

    void GhostSystems::Menu::checkRequestForSuspicious(GhostSystems::Menu::HttpRequest& req) {
        req.isSuspect = false;
        req.suspectLevel = SUSPECT_NONE;
        req.suspectReason = "";
        req.matchedPatterns.clear();

        std::string combined = req.url + req.endpoint + req.body + req.raw + req.response;

        std::vector<std::pair<std::string, SuspiciousLevel>> allKeywords = {
            {"anticheat", SUSPECT_CRITICAL}, {"report", SUSPECT_HIGH}, {"cheat", SUSPECT_CRITICAL},
            {"hack", SUSPECT_CRITICAL}, {"cheatfreefire", SUSPECT_CRITICAL}, {"ghost", SUSPECT_HIGH},
            {"violacao", SUSPECT_CRITICAL}, {"ban", SUSPECT_HIGH}, {"telemetry", SUSPECT_MEDIUM},
            {"analytics", SUSPECT_MEDIUM}, {"vguard", SUSPECT_CRITICAL}, {"xigncode", SUSPECT_CRITICAL},
            {"easyanticheat", SUSPECT_CRITICAL}, {"battleye", SUSPECT_CRITICAL}, {"vac", SUSPECT_CRITICAL},
            {"nprotect", SUSPECT_HIGH}, {"hacktool", SUSPECT_CRITICAL}, {"exploit", SUSPECT_CRITICAL},
            {"injection", SUSPECT_HIGH}, {"packet", SUSPECT_MEDIUM}, {"intercept", SUSPECT_HIGH},
            {"modify", SUSPECT_MEDIUM}, {"speedhack", SUSPECT_CRITICAL}, {"wallhack", SUSPECT_CRITICAL},
            {"aimbot", SUSPECT_CRITICAL}, {"esp", SUSPECT_HIGH}, {"glow", SUSPECT_MEDIUM},
            {"chams", SUSPECT_MEDIUM}, {"triggerbot", SUSPECT_CRITICAL}, {"freefire", SUSPECT_LOW},
            {"garena", SUSPECT_LOW}, {"security", SUSPECT_MEDIUM}, {"suspicious", SUSPECT_MEDIUM},
            {"detection", SUSPECT_HIGH}, {"banimento", SUSPECT_CRITICAL}, {"violation", SUSPECT_HIGH},
            {"punir", SUSPECT_HIGH}, {"punishment", SUSPECT_HIGH}, {"violate", SUSPECT_HIGH},
            {"malicious", SUSPECT_MEDIUM}, {"threat", SUSPECT_MEDIUM}, {"kernel", SUSPECT_HIGH}
        };

        for (auto& kw : bypassCustomKeywords) {
            allKeywords.push_back({kw, SUSPECT_HIGH});
        }

        for (auto& kp : allKeywords) {
            std::string kw = kp.first;
            std::string searchIn = combined;

            if (!bypassCaseSensitive) {
                searchIn = "";
                for (char c : combined) searchIn += tolower(c);
                std::string lowerKw = "";
                for (char c : kw) lowerKw += tolower(c);
                kw = lowerKw;
            }

            if (searchIn.find(kw) != std::string::npos) {
                req.isSuspect = true;
                if (std::find(req.matchedPatterns.begin(), req.matchedPatterns.end(), kp.first) == req.matchedPatterns.end()) {
                    req.matchedPatterns.push_back(kp.first);
                }
                if (kp.second > req.suspectLevel) {
                    req.suspectLevel = kp.second;
                    req.suspectReason = "Keyword: " + kp.first;
                }
            }
        }

        if (containsPlayerData(req.url) || containsPlayerData(req.body) || containsPlayerData(req.raw)) {
            req.isSuspect = true;
            if (std::find(req.matchedPatterns.begin(), req.matchedPatterns.end(), "PLAYER_DATA") == req.matchedPatterns.end()) {
                req.matchedPatterns.push_back("PLAYER_DATA");
            }
            if (req.suspectLevel < SUSPECT_CRITICAL) {
                req.suspectLevel = SUSPECT_CRITICAL;
                req.suspectReason = "Contem dados do player (ID/Nome)!";
            }
        }

        std::string domain = req.parsedUrl.domain;
        if (!domain.empty()) {
            std::string suspiciousDomains[] = {
                "anticheat", "report", "garena", "freefire", "tencent",
                "battleye", "xigncode", "vac", "security", "telemetry"
            };
            for (auto& sd : suspiciousDomains) {
                if (domain.find(sd) != std::string::npos) {
                    req.isSuspect = true;
                    if (std::find(req.matchedPatterns.begin(), req.matchedPatterns.end(), "SUSPICIOUS_DOMAIN") == req.matchedPatterns.end()) {
                        req.matchedPatterns.push_back("SUSPICIOUS_DOMAIN: " + domain);
                    }
                    if (req.suspectLevel < SUSPECT_HIGH) {
                        req.suspectLevel = SUSPECT_HIGH;
                        req.suspectReason = "Dominio suspeito: " + domain;
                    }
                }
            }
        }

        std::string sensitivePatterns[] = {
            "token", "auth", "bearer", "password", "passwd", "secret",
            "session", "cookie", "credential", "privatekey", "apikey", "api_key",
            "deviceid", "device_id", "androidid", "imei", "macaddress", "mac_address",
            "android_id", "googlead", "advertisingid", "advertising_id", "gaid"
        };
        for (auto& sp : sensitivePatterns) {
            std::string searchIn = combined;
            if (!bypassCaseSensitive) {
                searchIn = "";
                for (char c : combined) searchIn += tolower(c);
            }
            if (searchIn.find(sp) != std::string::npos) {
                if (std::find(req.matchedPatterns.begin(), req.matchedPatterns.end(), "SENSITIVE: " + sp) == req.matchedPatterns.end()) {
                    req.matchedPatterns.push_back("SENSITIVE: " + sp);
                }
            }
        }

        bool hasGzip = false, hasDeflate = false;
        for (auto& h : req.headers) {
            std::string hNameLower = h.name;
            std::string hValueLower = h.value;
            for (char& c : hNameLower) c = tolower(c);
            for (char& c : hValueLower) c = tolower(c);
            if (hNameLower.find("content-encoding") != std::string::npos) {
                if (hValueLower.find("gzip") != std::string::npos) hasGzip = true;
                if (hValueLower.find("deflate") != std::string::npos) hasDeflate = true;
            }
        }
        if (hasGzip || hasDeflate) {
            if (std::find(req.matchedPatterns.begin(), req.matchedPatterns.end(), "ENCODED_GZIP") == req.matchedPatterns.end()) {
                req.matchedPatterns.push_back(std::string("ENCODED: ") + (hasGzip ? "gzip" : "deflate"));
            }
        }
    }

    bool GhostSystems::Menu::containsPlayerData(const std::string& text) {
        if (text.empty()) return false;

        if (strlen(myPlayerId) > 0) {
            if (text.find(myPlayerId) != std::string::npos) return true;
            std::string idNoDash = myPlayerId;
            while (idNoDash.find('-') != std::string::npos) idNoDash.erase(idNoDash.find('-'), 1);
            if (!idNoDash.empty() && text.find(idNoDash) != std::string::npos) return true;
            if (strlen(myPlayerId) >= 6) {
                std::string partialId = std::string(myPlayerId).substr(0, strlen(myPlayerId) - 2);
                if (text.find(partialId) != std::string::npos) return true;
            }
        }
        if (strlen(myPlayerName) > 0) {
            if (text.find(myPlayerName) != std::string::npos) return true;
            std::string lowerName = "";
            for (char c : std::string(myPlayerName)) lowerName += tolower(c);
            std::string lowerText = "";
            for (char c : text) lowerText += tolower(c);
            if (lowerText.find(lowerName) != std::string::npos) return true;
        }

        const char* ffIdPatterns[] = {
            "8[0-9]{10}", "[0-9]{9,11}", "FF_[A-Z0-9]{10,}",
            "[A-Z0-9]{16,20}", "player[_]?[0-9]{8,12}"
        };
        for (auto& pattern : ffIdPatterns) {
            size_t pos = text.find(pattern);
            if (pos != std::string::npos) return true;
        }

        return false;
    }

    void GhostSystems::Menu::addBypassRequest(const GhostSystems::Menu::HttpRequest& req) {
        if (!bypassEnabled || !bypassCaptureAll) return;

        std::lock_guard<std::mutex> lock(bypassMutex);

        Menu::HttpRequest newReq = req;
        newReq.id = ++bypassRequestIdCounter;

        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%H:%M:%S", timeinfo);
        newReq.timestamp = std::string(timestamp);

        newReq.bodySize = newReq.body.size();
        newReq.totalSize = newReq.raw.size();
        newReq.responseTimeMs = 0.0f;

        parseUrl(newReq);
        checkRequestForSuspicious(newReq);

        bypassAllRequests.push_back(newReq);
        if (newReq.isSuspect) {
            if (newReq.suspectLevel == SUSPECT_CRITICAL) {
                bypassCriticalCount++;
                bypassCriticalAlert = true;
            }
            bypassSuspectRequests.push_back(newReq);
        }

        if ((int)bypassAllRequests.size() > bypassMaxRequests) {
            bypassAllRequests.erase(bypassAllRequests.begin());
        }
        if ((int)bypassSuspectRequests.size() > bypassMaxRequests / 2) {
            bypassSuspectRequests.erase(bypassSuspectRequests.begin());
        }

        bypassStatsDirty = true;

        if (bypassLogToFile && bypassAutoExport) {
            static char lastExportPath[256] = "";
            if (strlen(bypassAutoExportPath) > 0 && strcmp(lastExportPath, bypassAutoExportPath) != 0) {
                strcpy(lastExportPath, bypassAutoExportPath);
            }
            if (strlen(lastExportPath) > 0) {
                exportRequestToFile(newReq, lastExportPath);
            }
        }
    }

    void GhostSystems::Menu::parseUrl(GhostSystems::Menu::HttpRequest& req) {
        req.parsedUrl = ParsedUrl();

        std::string url = req.url;
        size_t schemeEnd = url.find("://");
        if (schemeEnd != std::string::npos) {
            req.parsedUrl.scheme = url.substr(0, schemeEnd);
            url = url.substr(schemeEnd + 3);
        }

        size_t pathStart = url.find('/');
        if (pathStart != std::string::npos) {
            req.parsedUrl.domain = url.substr(0, pathStart);
            req.parsedUrl.path = url.substr(pathStart);

            size_t queryStart = req.parsedUrl.path.find('?');
            if (queryStart != std::string::npos) {
                req.parsedUrl.query = req.parsedUrl.path.substr(queryStart + 1);
                req.parsedUrl.path = req.parsedUrl.path.substr(0, queryStart);
            }
        } else {
            req.parsedUrl.domain = url;
            req.parsedUrl.path = "/";
        }

        size_t colonPos = req.parsedUrl.domain.find(':');
        if (colonPos != std::string::npos) {
            std::string portStr = req.parsedUrl.domain.substr(colonPos + 1);
            req.parsedUrl.port = atoi(portStr.c_str());
            req.parsedUrl.domain = req.parsedUrl.domain.substr(0, colonPos);
        } else {
            req.parsedUrl.port = (req.parsedUrl.scheme == "https") ? 443 : 80;
        }

        req.endpoint = req.parsedUrl.path;
        if (!req.parsedUrl.query.empty()) {
            req.endpoint += "?" + req.parsedUrl.query;
        }
    }

    void GhostSystems::Menu::formatJson(std::string& json) {
        std::string result;
        int indent = 0;
        bool inString = false;

        for (size_t i = 0; i < json.size(); i++) {
            char c = json[i];
            char prev = (i > 0) ? json[i-1] : 0;

            if (c == '"' && prev != '\\') {
                inString = !inString;
                result += c;
            } else if (!inString) {
                if (c == '{' || c == '[') {
                    result += c;
                    result += '\n';
                    indent++;
                    for (int j = 0; j < indent; j++) result += "  ";
                } else if (c == '}' || c == ']') {
                    result += '\n';
                    indent--;
                    for (int j = 0; j < indent; j++) result += "  ";
                    result += c;
                } else if (c == ',') {
                    result += c;
                    result += '\n';
                    for (int j = 0; j < indent; j++) result += "  ";
                } else if (c == ':') {
                    result += ": ";
                } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                    continue;
                } else {
                    result += c;
                }
            } else {
                result += c;
            }
        }

        json = result;
    }

    std::string GhostSystems::Menu::bytesToHex(const std::string& data) {
        std::string result;
        const char hexChars[] = "0123456789ABCDEF";

        for (size_t i = 0; i < data.size(); i++) {
            unsigned char c = data[i];
            result += hexChars[(c >> 4) & 0x0F];
            result += hexChars[c & 0x0F];
            result += " ";

            if ((i + 1) % 16 == 0) {
                result += " ";
                for (size_t j = i - 15; j <= i; j++) {
                    char p = data[j];
                    result += (p >= 32 && p < 127) ? p : '.';
                }
                result += '\n';
            } else if ((i + 1) % 8 == 0) {
                result += " ";
            }
        }

        size_t remainder = data.size() % 16;
        if (remainder != 0) {
            for (size_t j = 0; j < 16 - remainder; j++) {
                result += "   ";
                if ((remainder + j) % 8 == 0) result += " ";
            }
            result += " ";
            for (size_t j = data.size() - remainder; j < data.size(); j++) {
                char p = data[j];
                result += (p >= 32 && p < 127) ? p : '.';
            }
        }

        return result;
    }

    void GhostSystems::Menu::exportRequestToFile(const GhostSystems::Menu::HttpRequest& req, const char* filepath) {
        FILE* f = fopen(filepath, "a");
        if (!f) return;

        fprintf(f, "=== GhostSystems Request Export ===\n");
        fprintf(f, "ID: %lu | Timestamp: %s | Method: %s\n", req.id, req.timestamp.c_str(), req.methodStr.c_str());
        fprintf(f, "URL: %s\n", req.url.c_str());
        fprintf(f, "Endpoint: %s\n", req.endpoint.c_str());
        fprintf(f, "Suspect: %s | Level: %d | Reason: %s\n", req.isSuspect ? "YES" : "NO", req.suspectLevel, req.suspectReason.c_str());
        fprintf(f, "Patterns: ");
        for (auto& p : req.matchedPatterns) fprintf(f, "%s, ", p.c_str());
        fprintf(f, "\n");
        fprintf(f, "\n--- HEADERS ---\n");
        for (auto& h : req.headers) fprintf(f, "%s: %s\n", h.name.c_str(), h.value.c_str());
        fprintf(f, "\n--- BODY ---\n%s\n", req.body.c_str());
        fprintf(f, "\n--- RAW ---\n%s\n", req.raw.c_str());
        fprintf(f, "\n--- RESPONSE ---\n%s\n", req.response.c_str());
        fprintf(f, "\n\n");

        fclose(f);
    }

    void GhostSystems::Menu::exportAllToFile(const char* filepath) {
        FILE* f = fopen(filepath, "w");
        if (!f) return;

        fprintf(f, "=== GhostSystems FULL Export ===\n");
        fprintf(f, "Total Requests: %d | Suspects: %d\n\n", (int)bypassAllRequests.size(), (int)bypassSuspectRequests.size());

        for (auto& req : bypassAllRequests) {
            fprintf(f, "[#%lu] %s %s | Suspect: %s\n", req.id, req.methodStr.c_str(), req.endpoint.c_str(), req.isSuspect ? "YES" : "NO");
            if (req.isSuspect) {
                fprintf(f, "  Reason: %s\n", req.suspectReason.c_str());
                fprintf(f, "  Patterns: ");
                for (auto& p : req.matchedPatterns) fprintf(f, "%s, ", p.c_str());
                fprintf(f, "\n");
            }
            fprintf(f, "  Headers: %d | Body: %d bytes\n", (int)req.headers.size(), req.bodySize);
            fprintf(f, "  URL: %s\n", req.url.c_str());
            fprintf(f, "\n");
        }

        fclose(f);
    }

    bool GhostSystems::Menu::matchRegex(const std::string& pattern, const std::string& text) {
        if (!bypassUseRegex) {
            std::string lowerPattern = pattern;
            std::string lowerText = text;
            for (char& c : lowerPattern) c = tolower(c);
            for (char& c : lowerText) c = tolower(c);
            return lowerText.find(lowerPattern) != std::string::npos;
        }
        try {
            std::regex re;
            if (bypassCaseSensitive) {
                re = std::regex(pattern);
            } else {
                re = std::regex(pattern, std::regex_constants::icase);
            }
            return std::regex_search(text, re);
        } catch (...) {
            return false;
        }
    }

    std::string GhostSystems::Menu::decompressGzip(const std::string& data) {
        LOGI("Gzip decompression not available - returning raw data");
        return data;
    }

    std::string GhostSystems::Menu::decompressDeflate(const std::string& data) {
        LOGI("Deflate decompression not available - returning raw data");
        return data;
    }

    bool GhostSystems::Menu::initSqlite() {
        LOGI("SQLite not available in this build");
        return false;
    }

    void GhostSystems::Menu::closeSqlite() {
        if (bypassSqliteDb) {
            bypassSqliteDb = nullptr;
            bypassSqliteInitialized = false;
        }
    }

    bool GhostSystems::Menu::exportToSqlite(const GhostSystems::Menu::HttpRequest& req) {
        LOGI("SQLite not available in this build");
        return false;
    }

    bool GhostSystems::Menu::exportAllToSqlite() {
        if (!bypassSqliteInitialized) return false;
        for (auto& req : bypassAllRequests) {
            if (!exportToSqlite(req)) return false;
        }
        return true;
    }

    bool GhostSystems::Menu::scanForPlayerId(const std::string& regionName) {
        const char* idPatterns[] = {
            "8[0-9]{10}", "[0-9]{9,11}", "FF_[A-Z0-9]{10,}",
            "[A-Z0-9]{16,20}", "player[_]?[0-9]{8,12}",
            "[0-9]{4}[-][0-9]{4}[-][0-9]{4}", "[0-9]{12}"
        };
        char buffer[4096];
        for (int i = 0; i < 100; i++) {
            snprintf(buffer, sizeof(buffer), "/sdcard/Android/data/com.dts.freefireth/files/%d", i);
            FILE* f = fopen(buffer, "rb");
            if (!f) continue;
            size_t bytesRead;
            while ((bytesRead = fread(buffer, 1, sizeof(buffer), f)) > 0) {
                for (auto& pat : idPatterns) {
                    if (matchRegex(pat, std::string(buffer, bytesRead))) {
                        fclose(f);
                        return true;
                    }
                }
            }
            fclose(f);
        }
        return false;
    }

    bool GhostSystems::Menu::scanForPlayerName(const std::string& regionName) {
        char buffer[4096];
        snprintf(buffer, sizeof(buffer), "/sdcard/Android/data/com.dts.freefireth/shared_prefs/%s.xml", regionName.c_str());
        FILE* f = fopen(buffer, "rb");
        if (!f) return false;
        size_t len = fread(buffer, 1, sizeof(buffer) - 1, f);
        buffer[len] = '\0';
        fclose(f);
        const char* namePatterns[] = {"player_name", "nickname", "user_name", "display_name"};
        for (auto& pat : namePatterns) {
            char* found = strstr(buffer, pat);
            if (found) {
                char* start = strchr(found, '>');
                if (!start) continue;
                start++;
                char* end = strchr(start, '<');
                if (!end) continue;
                size_t nameLen = end - start;
                if (nameLen > 0 && nameLen < 64) {
                    strncpy(bypassDetectedPlayerName, start, nameLen);
                    bypassDetectedPlayerName[nameLen] = '\0';
                    return true;
                }
            }
        }
        return false;
    }

    bool GhostSystems::Menu::scanMemoryForPlayerData() {
        if (bypassScanInProgress) return false;
        bypassScanInProgress = true;
        bypassScanComplete = false;
        bypassPlayerDataFound = false;
        memset(bypassDetectedPlayerId, 0, sizeof(bypassDetectedPlayerId));
        memset(bypassDetectedPlayerName, 0, sizeof(bypassDetectedPlayerName));
        const char* regions[] = {"GarenaFreeFire", "com.dts.freefireth", "player_data", "user_info"};
        for (auto& region : regions) {
            if (scanForPlayerId(region)) {
                bypassPlayerDataFound = true;
                break;
            }
            if (scanForPlayerName(region)) {
                bypassPlayerDataFound = true;
                break;
            }
        }
        bypassScanComplete = true;
        bypassScanInProgress = false;
        return bypassPlayerDataFound;
    }

    void* GhostSystems::Menu::getUnityWebRequestSendInternal() {
        void* unityWebRequestClass = Il2Cpp::GetClass("UnityEngine.UnityWebRequestModule.dll", "UnityEngine", "UnityWebRequest");
        if (!unityWebRequestClass) {
            unityWebRequestClass = Il2Cpp::GetClass("UnityEngine.UnityWebRequestModule", "UnityEngine", "UnityWebRequest");
        }
        if (!unityWebRequestClass) return nullptr;

        void* method = Il2Cpp::GetMethodRecursively(unityWebRequestClass, "SendWebRequest", 0);
        if (!method) {
            method = Il2Cpp::class_get_method_from_name(unityWebRequestClass, "SendWebRequest", 0);
        }
        return method;
    }

    void* Menu::getUnityWebRequestReceiveCallback() {
        void* unityWebRequestClass = Il2Cpp::GetClass("UnityEngine.UnityWebRequestModule.dll", "UnityEngine", "UnityWebRequest");
        if (!unityWebRequestClass) {
            unityWebRequestClass = Il2Cpp::GetClass("UnityEngine.UnityWebRequestModule", "UnityEngine", "UnityWebRequest");
        }
        if (!unityWebRequestClass) return nullptr;

        void* method = Il2Cpp::GetMethodRecursively(unityWebRequestClass, "ReceiveCallback", -1);
        if (!method) {
            method = Il2Cpp::GetMethodRecursively(unityWebRequestClass, "ProcessResponse", -1);
        }
        return method;
    }

    bool GhostSystems::Menu::hookUnityWebRequest() {
        if (bypassUnityWebRequestHooked) return true;

        void* targetMethod = getUnityWebRequestSendInternal();
        if (!targetMethod) {
            LOGI("[Bypass] Failed to find UnityWebRequest.SendWebRequest method");
            return false;
        }

        orig_SendWebRequest = nullptr;
        A64HookFunction(targetMethod, (void*)hook_UnityWebRequest_SendWebRequest, &orig_SendWebRequest);

        void* receiveCallback = getUnityWebRequestReceiveCallback();
        if (receiveCallback) {
            orig_ReceiveCallback = nullptr;
            A64HookFunction(receiveCallback, (void*)hook_UnityWebRequest_ReceiveCallback, &orig_ReceiveCallback);
        }

        bypassUnityWebRequestHooked = true;
        LOGI("[Bypass] UnityWebRequest hooks applied successfully");
        return true;
    }

    void GhostSystems::Menu::unhookUnityWebRequest() {
        if (!bypassUnityWebRequestHooked) return;
        bypassUnityWebRequestHooked = false;
    }

    bool GhostSystems::Menu::isUnityWebRequestHooked() {
        return bypassUnityWebRequestHooked;
    }

    thread_local Menu::HttpRequest Menu::t_CurrentCapturedRequest;

    GhostSystems::Menu::HttpRequest& GhostSystems::Menu::getCurrentCapturedRequest() {
        return t_CurrentCapturedRequest;
    }

    void GhostSystems::Menu::setCurrentCapturedRequest(const GhostSystems::Menu::HttpRequest& req) {
        t_CurrentCapturedRequest = req;
    }

    void GhostSystems::Menu::hook_UnityWebRequest_SendWebRequest(void* unityWebRequest) {
        if (!unityWebRequest || !g_Menu) return;

        if (!g_Menu->bypassEnabled) {
            if (g_Menu->orig_SendWebRequest != nullptr) {
                ((void(*)(void*))g_Menu->orig_SendWebRequest)(unityWebRequest);
            }
            return;
        }

        Menu::HttpRequest req;
        req.id = g_Menu->bypassRequestIdCounter++;
        req.method = METHOD_OTHER;
        req.isSuspect = false;
        req.suspectLevel = SUSPECT_NONE;
        req.bodySize = 0;
        req.totalSize = 0;
        req.responseTimeMs = 0.0f;

        time_t now = time(nullptr);
        char timeStr[64];
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", localtime(&now));
        req.timestamp = timeStr;

        void* klass = Il2Cpp::object_get_class(unityWebRequest);
        if (klass) {
            void* urlMethod = Il2Cpp::class_get_method_from_name(klass, "get_url", 0);
            if (!urlMethod) urlMethod = Il2Cpp::class_get_method_from_name(klass, "GetURL", 0);
            if (urlMethod) {
                void* urlObj = Il2Cpp::runtime_invoke(urlMethod, unityWebRequest, nullptr, nullptr);
                if (urlObj) {
                    void* uriKlass = Il2Cpp::object_get_class(urlObj);
                    if (uriKlass) {
                        void* toStringMethod = Il2Cpp::class_get_method_from_name(uriKlass, "ToString", 0);
                        if (!toStringMethod) toStringMethod = Il2Cpp::class_get_method_from_name(uriKlass, "toString", 0);
                        if (toStringMethod) {
                            void* result = Il2Cpp::runtime_invoke(toStringMethod, urlObj, nullptr, nullptr);
                            if (result) {
                                const char* str = (const char*)Il2Cpp::Unsafe_GetPointer(result);
                                if (str) {
                                    req.url = str;
                                    req.raw = str;
                                }
                            }
                        }
                    }
                }
            }

            void* methodGetter = Il2Cpp::class_get_method_from_name(klass, "get_method", 0);
            if (!methodGetter) methodGetter = Il2Cpp::class_get_method_from_name(klass, "GetMethod", 0);
            if (methodGetter) {
                void* methodObj = Il2Cpp::runtime_invoke(methodGetter, unityWebRequest, nullptr, nullptr);
                if (methodObj) {
                    const char* methodStr = (const char*)Il2Cpp::Unsafe_GetPointer(methodObj);
                    if (methodStr) {
                        req.methodStr = methodStr;
                        if (req.methodStr == "GET") req.method = METHOD_GET;
                        else if (req.methodStr == "POST") req.method = METHOD_POST;
                        else if (req.methodStr == "PUT") req.method = METHOD_PUT;
                        else if (req.methodStr == "DELETE") req.method = METHOD_DELETE;
                        else if (req.methodStr == "PATCH") req.method = METHOD_PATCH;
                        else req.method = METHOD_OTHER;
                    }
                }
            }

            void* headersGetter = Il2Cpp::class_get_method_from_name(klass, "get_headers", 0);
            if (!headersGetter) headersGetter = Il2Cpp::class_get_method_from_name(klass, "GetHeaders", 0);
            if (headersGetter) {
                void* headersObj = Il2Cpp::runtime_invoke(headersGetter, unityWebRequest, nullptr, nullptr);
                if (headersObj) {
                    void* headersKlass = Il2Cpp::object_get_class(headersObj);
                    if (headersKlass) {
                        void* getEnumeratorMethod = Il2Cpp::class_get_method_from_name(headersKlass, "GetEnumerator", 0);
                        if (getEnumeratorMethod) {
                            void* enumerator = Il2Cpp::runtime_invoke(getEnumeratorMethod, headersObj, nullptr, nullptr);
                            if (enumerator) {
                                void* enumKlass = Il2Cpp::object_get_class(enumerator);
                                if (enumKlass) {
                                    void* moveNextMethod = Il2Cpp::class_get_method_from_name(enumKlass, "MoveNext", 0);
                                    void* getCurrentMethod = Il2Cpp::class_get_method_from_name(enumKlass, "get_Current", 0);
                                    if (moveNextMethod && getCurrentMethod) {
                                        while (true) {
                                            void* moveResult = Il2Cpp::runtime_invoke(moveNextMethod, enumerator, nullptr, nullptr);
                                            if (!moveResult || !*(bool*)((uintptr_t)moveResult + 0x10)) break;

                                            void* current = Il2Cpp::runtime_invoke(getCurrentMethod, enumerator, nullptr, nullptr);
                                            if (current) {
                                                void* currentKlass = Il2Cpp::object_get_class(current);
                                                if (currentKlass) {
                                                    void* keyMethod = Il2Cpp::class_get_method_from_name(currentKlass, "get_Key", 0);
                                                    void* valueMethod = Il2Cpp::class_get_method_from_name(currentKlass, "get_Value", 0);
                                                    if (keyMethod && valueMethod) {
                                                        HttpHeader header;
                                                        void* keyObj = Il2Cpp::runtime_invoke(keyMethod, current, nullptr, nullptr);
                                                        void* valueObj = Il2Cpp::runtime_invoke(valueMethod, current, nullptr, nullptr);
                                                        if (keyObj) {
                                                            const char* keyStr = (const char*)Il2Cpp::Unsafe_GetPointer(keyObj);
                                                            if (keyStr) header.name = keyStr;
                                                        }
                                                        if (valueObj) {
                                                            const char* valStr = (const char*)Il2Cpp::Unsafe_GetPointer(valueObj);
                                                            if (valStr) header.value = valStr;
                                                        }
                                                        if (!header.name.empty()) {
                                                            req.headers.push_back(header);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            void* bodyGetter = Il2Cpp::class_get_method_from_name(klass, "get_uploadHandler", 0);
            if (!bodyGetter) bodyGetter = Il2Cpp::class_get_method_from_name(klass, "GetUploadHandler", 0);
            if (bodyGetter) {
                void* bodyObj = Il2Cpp::runtime_invoke(bodyGetter, unityWebRequest, nullptr, nullptr);
                if (bodyObj) {
                    void* bodyKlass = Il2Cpp::object_get_class(bodyObj);
                    if (bodyKlass) {
                        void* dataGetter = Il2Cpp::class_get_method_from_name(bodyKlass, "get_data", 0);
                        if (!dataGetter) dataGetter = Il2Cpp::class_get_method_from_name(bodyKlass, "GetData", 0);
                        if (dataGetter) {
                            void* dataObj = Il2Cpp::runtime_invoke(dataGetter, bodyObj, nullptr, nullptr);
                            if (dataObj) {
                                const char* dataStr = (const char*)Il2Cpp::Unsafe_GetPointer(dataObj);
                                if (dataStr) {
                                    req.body = dataStr;
                                    req.bodySize = strlen(dataStr);
                                }
                            }
                        }
                    }
                }
            }
        }

        g_Menu->parseUrl(req);
        g_Menu->checkRequestForSuspicious(req);
        g_Menu->addBypassRequest(req);
        t_CurrentCapturedRequest = req;
    }

    void GhostSystems::Menu::hook_UnityWebRequest_ReceiveCallback(void* unityWebRequest, void* operationHandle) {
        if (!unityWebRequest || !g_Menu) return;

        if (!g_Menu->bypassEnabled) {
            if (g_Menu->orig_ReceiveCallback) {
                ((void(*)(void*, void*))g_Menu->orig_ReceiveCallback)(unityWebRequest, operationHandle);
            }
            return;
        }

        if (!t_CurrentCapturedRequest.url.empty()) {
            Menu::HttpRequest& req = t_CurrentCapturedRequest;

            void* klass = Il2Cpp::object_get_class(unityWebRequest);
            if (klass) {
                void* responseGetter = Il2Cpp::class_get_method_from_name(klass, "get_response", 0);
                if (!responseGetter) responseGetter = Il2Cpp::class_get_method_from_name(klass, "GetResponse", 0);
                if (responseGetter) {
                    void* responseObj = Il2Cpp::runtime_invoke(responseGetter, unityWebRequest, nullptr, nullptr);
                    if (responseObj) {
                        void* respKlass = Il2Cpp::object_get_class(responseObj);
                        if (respKlass) {
                            void* downloadHandlerGetter = Il2Cpp::class_get_method_from_name(klass, "get_downloadHandler", 0);
                            if (!downloadHandlerGetter) downloadHandlerGetter = Il2Cpp::class_get_method_from_name(klass, "GetDownloadHandler", 0);
                            if (downloadHandlerGetter) {
                                void* handlerObj = Il2Cpp::runtime_invoke(downloadHandlerGetter, unityWebRequest, nullptr, nullptr);
                                if (handlerObj) {
                                    void* handlerKlass = Il2Cpp::object_get_class(handlerObj);
                                    if (handlerKlass) {
                                        void* textGetter = Il2Cpp::class_get_method_from_name(handlerKlass, "get_text", 0);
                                        if (!textGetter) textGetter = Il2Cpp::class_get_method_from_name(handlerKlass, "GetText", 0);
                                        if (textGetter) {
                                            void* textObj = Il2Cpp::runtime_invoke(textGetter, handlerObj, nullptr, nullptr);
                                            if (textObj) {
                                                const char* textStr = (const char*)Il2Cpp::Unsafe_GetPointer(textObj);
                                                if (textStr) {
                                                    req.response = textStr;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            for (auto& capturedReq : g_Menu->bypassAllRequests) {
                if (capturedReq.id == req.id) {
                    capturedReq.response = req.response;
                    break;
                }
            }
        }
    }

    void GhostSystems::Menu::drawBypassDashboard() {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "=== REAL-TIME DASHBOARD ===");

        ImGui::Text("Requests/Segundo: %d", bypassRequestsPerSecond);
        ImGui::Text("Requests (Ultimo Min): %d", bypassRequestsLastMinute);

        ImGui::PlotLines("Requests", bypassDashboardData, 60, bypassDashboardIndex, nullptr, 0.0f, 50.0f, ImVec2(0, 100));

        if (bypassUnityWebRequestHooked) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Network Hook: ATIVO");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Network Hook: INATIVO");
            if (ImGui::Button("Ativar Hook")) {
                hookUnityWebRequest();
            }
        }

        ImGui::Separator();
        ImGui::Text("Player ID Detectado: %s", strlen(bypassDetectedPlayerId) > 0 ? bypassDetectedPlayerId : "N/A");
        ImGui::Text("Player Name Detectado: %s", strlen(bypassDetectedPlayerName) > 0 ? bypassDetectedPlayerName : "N/A");

        if (bypassScanInProgress) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Escaneando memoria...");
        } else if (!bypassScanComplete) {
            if (ImGui::Button("Auto-Detectar Player Data")) {
                scanMemoryForPlayerData();
            }
        } else if (bypassPlayerDataFound) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Player Data encontrado!");
            if (strlen(bypassDetectedPlayerId) > 0) strncpy(myPlayerId, bypassDetectedPlayerId, 32);
            if (strlen(bypassDetectedPlayerName) > 0) strncpy(myPlayerName, bypassDetectedPlayerName, 64);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Player Data NAO encontrado");
        }
    }

    // ===== IMPLEMENTACAO CACHE DE OFFSETS =====
    size_t GhostSystems::Menu::getCachedFieldOffset(void* obj, const char* fieldName) {
        std::string key = std::string("field_") + fieldName;
        auto it = cachedFieldOffsets.find(key);
        if (it != cachedFieldOffsets.end()) {
            return it->second;
        }
        
        void* klass = Il2Cpp::object_get_class(obj);
        if (klass && Il2Cpp::class_get_field_from_name) {
            void* field = Il2Cpp::class_get_field_from_name(klass, fieldName);
            if (field && Il2Cpp::field_get_offset) {
                size_t offset = Il2Cpp::field_get_offset(field);
                cachedFieldOffsets[key] = offset;
                return offset;
            }
        }
        return 0;
    }
    
    void* GhostSystems::Menu::getCachedMethod(const char* className, const char* methodName, int paramCount) {
        std::string key = std::string("method_") + className + "_" + methodName + "_" + std::to_string(paramCount);
        auto it = cachedMethods.find(key);
        if (it != cachedMethods.end()) {
            return it->second;
        }
        
        void* klass = Il2Cpp::GetClass("Assembly-CSharp.dll", "", className);
        if (klass) {
            void* method = Il2Cpp::GetMethodRecursively(klass, methodName, paramCount);
            if (method) {
                cachedMethods[key] = method;
                return method;
            }
        }
        return nullptr;
    }
    
    void* GhostSystems::Menu::getCachedClass(const char* assemblyName, const char* namespaceName, const char* className) {
        std::string key = std::string("class_") + assemblyName + "_" + namespaceName + "_" + className;
        auto it = cachedClasses.find(key);
        if (it != cachedClasses.end()) {
            return it->second;
        }
        
        void* klass = Il2Cpp::GetClass(assemblyName, namespaceName, className);
        if (klass) {
            cachedClasses[key] = klass;
            return klass;
        }
        return nullptr;
    }
    
    // ===== IMPLEMENTACAO: OBTER POSICAO DE BONES =====
    // boneType: 0 = Pe, 1 = Peito, 2 = Cabeca, 3 = Pescoco
    bool GhostSystems::Menu::getLocalPlayerBonePosition(int boneType, float* outX, float* outY, float* outZ) {
        if (!sharedState.localPlayerObj || !outX || !outY || !outZ) return false;
        
        // Cache dos metodos de bones (primeira vez)
        if (!boneMethodsCached) {
            void* playerKlass = Il2Cpp::object_get_class(sharedState.localPlayerObj);
            if (playerKlass) {
                getHeadTFMethod = Il2Cpp::GetMethodRecursively(playerKlass, "GetHeadTF", 0);
                getNeckTFMethod = Il2Cpp::GetMethodRecursively(playerKlass, "GetNeckTF", 0);
                
                // Metodo para peito pode ter nome diferente
                void* getChestMethod = Il2Cpp::GetMethodRecursively(playerKlass, "GetChestTF", 0);
                if (!getChestMethod) {
                    getChestMethod = Il2Cpp::GetMethodRecursively(playerKlass, "GetSpineTF", 0);
                }
                getChestTFMethod = getChestMethod;
            }
            boneMethodsCached = true;
        }
        
        void* targetMethod = nullptr;
        switch (boneType) {
            case 2: targetMethod = getHeadTFMethod; break;  // Cabeca
            case 3: targetMethod = getNeckTFMethod; break;  // Pescoco
            case 1: targetMethod = getChestTFMethod; break; // Peito
            default: return false; // Pe nao tem metodo especifico, usa transform
        }
        
        if (!targetMethod) return false;
        
        void* exc = nullptr;
        void* boneTransform = Il2Cpp::runtime_invoke(targetMethod, sharedState.localPlayerObj, nullptr, &exc);
        if (!boneTransform || exc) return false;
        
        // Cache do get_position
        static void* getPosMethod = nullptr;
        if (!getPosMethod) {
            void* transformKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Transform");
            if (transformKlass) {
                getPosMethod = Il2Cpp::GetMethodRecursively(transformKlass, "get_position", 0);
            }
        }
        
        if (!getPosMethod) return false;
        
        void* posObj = Il2Cpp::runtime_invoke(getPosMethod, boneTransform, nullptr, &exc);
        if (!posObj || exc) return false;
        
        struct Vector3Args { float x, y, z; };
        Vector3Args pos = *(Vector3Args*)((uintptr_t)posObj + 0x10);
        
        *outX = pos.x;
        *outY = pos.y;
        *outZ = pos.z;
        
        return true;
    }
    
    bool GhostSystems::Menu::getEntityBonePosition(void* entityObj, int boneType, float* outX, float* outY, float* outZ) {
        if (!entityObj || !outX || !outY || !outZ) return false;
        
        void* playerKlass = Il2Cpp::object_get_class(entityObj);
        if (!playerKlass) return false;
        
        void* targetMethod = nullptr;
        switch (boneType) {
            case 2: // Cabeca
                targetMethod = Il2Cpp::GetMethodRecursively(playerKlass, "GetHeadTF", 0);
                break;
            case 3: // Pescoco
                targetMethod = Il2Cpp::GetMethodRecursively(playerKlass, "GetNeckTF", 0);
                break;
            case 1: { // Peito
                targetMethod = Il2Cpp::GetMethodRecursively(playerKlass, "GetChestTF", 0);
                if (!targetMethod) {
                    targetMethod = Il2Cpp::GetMethodRecursively(playerKlass, "GetSpineTF", 0);
                }
                break;
            }
            default: return false;
        }
        
        if (!targetMethod) return false;
        
        void* exc = nullptr;
        void* boneTransform = Il2Cpp::runtime_invoke(targetMethod, entityObj, nullptr, &exc);
        if (!boneTransform || exc) return false;
        
        static void* getPosMethod = nullptr;
        if (!getPosMethod) {
            void* transformKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Transform");
            if (transformKlass) {
                getPosMethod = Il2Cpp::GetMethodRecursively(transformKlass, "get_position", 0);
            }
        }
        
        if (!getPosMethod) return false;
        
        void* posObj = Il2Cpp::runtime_invoke(getPosMethod, boneTransform, nullptr, &exc);
        if (!posObj || exc) return false;
        
        struct Vector3Args { float x, y, z; };
        Vector3Args pos = *(Vector3Args*)((uintptr_t)posObj + 0x10);
        
        *outX = pos.x;
        *outY = pos.y;
        *outZ = pos.z;
        
        return true;
    }

} // namespace GhostSystems
