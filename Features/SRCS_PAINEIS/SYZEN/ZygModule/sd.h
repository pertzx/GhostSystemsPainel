#include <imports.h>
#include <imguiconfig.h>
#include "ESP.h"
#include "particles.h"
#include <string>
#include <chrono>
#include <map>

#ifndef OBFUSCATE
#define OBFUSCATE(str) str
#endif


bool init = true;

// Variáveis de controle para os botões Ativar/Desativar Funções
extern bool AimbotRage;
extern bool EspCircle;
extern bool Active;

// Static variable to store the user-selected color (Minimalist Accent Color: Dark Navy Blue)
// Cor de destaque alterada para Azul Marinho Escuro (0.1, 0.1, 0.3, 1.0)
static ImVec4 selected_color = ImVec4(0.1f, 0.1f, 0.3f, 1.0f); 

// Static variable for Esp Line color
static ImVec4 esp_line_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default to white

// Static variable for Esp Box color
static ImVec4 esp_box_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default to white

// Static variable for Esp Name color
static ImVec4 esp_name_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default to white

// Static variable for Esp Life color
static ImVec4 esp_life_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default to white

static ParticleSystem particleSystem(150, 0.05f);
static auto lastTime = std::chrono::high_resolution_clock::now();

static int espBoxStyle  = 0; // 0=Quadrado, 1=Arredondado, etc

void ApplyMinimalistTheme() {
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- Cores Minimalistas ---
    const ImVec4 dark_bg(0.08f, 0.08f, 0.08f, 1.00f);      // Fundo
    const ImVec4 medium_bg(0.12f, 0.12f, 0.12f, 1.00f);    // Fundo 2
    const ImVec4 accent_color = selected_color;            // azul
    const ImVec4 accent_hover(accent_color.x * 1.2f, accent_color.y * 1.2f, accent_color.z * 1.2f, accent_color.w);
    const ImVec4 accent_active(accent_color.x * 0.8f, accent_color.y * 0.8f, accent_color.z * 0.8f, accent_color.w);
    const ImVec4 text_color(0.90f, 0.90f, 0.90f, 1.00f);   // Cor do Texto

    // Fundo
    colors[ImGuiCol_WindowBg]        = dark_bg;
    colors[ImGuiCol_ChildBg]         = dark_bg;
    colors[ImGuiCol_PopupBg]         = medium_bg;

    // Título (Flat e com a cor de destaque)
    colors[ImGuiCol_TitleBg]         = accent_color;
    colors[ImGuiCol_TitleBgActive]   = accent_color;
    colors[ImGuiCol_TitleBgCollapsed]= accent_color;
    colors[ImGuiCol_Text]            = text_color;

    // Elementos Interativos (Botões, Headers)
    colors[ImGuiCol_Button]          = medium_bg;
    colors[ImGuiCol_ButtonHovered]   = accent_hover;
    colors[ImGuiCol_ButtonActive]    = accent_active;

    colors[ImGuiCol_Header]          = medium_bg;
    colors[ImGuiCol_HeaderHovered]   = accent_hover;
    colors[ImGuiCol_HeaderActive]    = accent_active;

    // Frames (Input, Slider, Checkbox Background)
    colors[ImGuiCol_FrameBg]         = medium_bg;
    colors[ImGuiCol_FrameBgHovered]  = accent_hover;
    colors[ImGuiCol_FrameBgActive]   = accent_active;

    // Slider e Checkmark
    colors[ImGuiCol_SliderGrab]      = accent_color;
    colors[ImGuiCol_SliderGrabActive]= accent_active;
    colors[ImGuiCol_CheckMark]       = accent_color;

    // Separadores
    colors[ImGuiCol_Separator]       = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]= accent_color;
    colors[ImGuiCol_SeparatorActive] = accent_active;

    // Scrollbar (Minimalista)
    colors[ImGuiCol_ScrollbarBg]     = ImVec4(0.10f, 0.10f, 0.10f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]   = ImVec4(0.30f, 0.30f, 0.30f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    // --- Estilo Minimalista (Flat) ---
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize  = 0.0f;
    style.TabBorderSize    = 0.0f;
    colors[ImGuiCol_Border]       = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Arredondamento sutil
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.WindowRounding = 4.0f;
    style.PopupRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.TabRounding = 3.0f;

    // Espaçamento
    style.FramePadding = ImVec2(8, 6);
    style.ItemSpacing = ImVec2(8, 4);
    style.ItemInnerSpacing = ImVec2(6, 4);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 10.0f;

    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.DisplaySafeAreaPadding = ImVec2(4, 4);
}

// Refatoração do CustomCheckbox12 para um visual futurista (clean, retangular, solid-fill)
bool CustomCheckbox12(const char* label, bool* v) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    // Aumenta o tamanho do quadrado para um visual mais futurista/destacado
    const float square_sz = ImGui::GetFrameHeight() * 1.2f; 
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos, ImVec2(pos.x + square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), pos.y + label_size.y + style.FramePadding.y * 2.0f));
    
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    
    if (pressed) {
        *v = !(*v);
        ImGui::MarkItemEdited(id);
    }

    const ImRect check_bb(pos, ImVec2(pos.x + square_sz, pos.y + square_sz));
    
    // --- Lógica de Animação (Preservada) ---
    static std::map<ImGuiID, float> animTimes;
    float& animTime = animTimes[id];
    
    if (pressed) {
        animTime = 1.0f;
    } else if (animTime > 0.0f) {
        animTime -= g.IO.DeltaTime * 8.0f;
        if (animTime < 0.0f) animTime = 0.0f;
    }
    
    ImRect anim_bb = check_bb;
    
    // Cor de fundo do checkbox
    ImU32 bg_color;
    ImU32 border_color;
    
    if (*v) {
        // Cor de destaque para ativo (Azul Marinho Escuro)
        bg_color = ImGui::GetColorU32(selected_color);
        border_color = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.5f)); // Borda sutil branca
    } else {
        // Fundo sutil para inativo
        bg_color = ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
        border_color = ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, 0.5f)); // Borda cinza sutil
    }
    
    // Desenha o fundo do checkbox (retangular, sem arredondamento)
    window->DrawList->AddRectFilled(anim_bb.Min, anim_bb.Max, bg_color, 0.0f);
    
    // Desenha a borda (futurista: linha fina)
    window->DrawList->AddRect(anim_bb.Min, anim_bb.Max, border_color, 0.0f, 0, 0.5f); // Borda mais fina (0.5f)
    
    // Desenha o checkmark se estiver ativo (X simples ou um traço)
    if (*v) {
        // Cor do checkmark (branco)
        ImU32 check_color = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        
        // Desenha um check mark (✓)
        float pad = square_sz * 0.2f;
        ImVec2 p1 = ImVec2(anim_bb.Min.x + square_sz * 0.2f, anim_bb.Min.y + square_sz * 0.5f); // Ponto inicial da perna curta
        ImVec2 p2 = ImVec2(anim_bb.Min.x + square_sz * 0.4f, anim_bb.Max.y - square_sz * 0.2f); // Ponto de dobra
        ImVec2 p3 = ImVec2(anim_bb.Max.x - square_sz * 0.2f, anim_bb.Min.y + square_sz * 0.2f); // Ponto final da perna longa
        // ImVec2 p4 = ImVec2(anim_bb.Max.x - pad, anim_bb.Min.y + pad); // Removido para o check mark
        
        window->DrawList->AddLine(p1, p2, check_color, 1.5f); // Perna curta do check mark (espessura 1.5f)
        window->DrawList->AddLine(p2, p3, check_color, 1.5f); // Perna longa do check mark (espessura 1.5f)
    }
    
    // Renderiza o texto do label
    if (label_size.x > 0.0f) {
        ImGui::RenderText(ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, pos.y + style.FramePadding.y), label);
    }

    return pressed;
}

static bool g_isMenuVisible = true; // Renomeado para maior clareza

struct ThreeFingerState {
    int lastCount = 0;
    double lastFlipTime = 0.0;
};
static ThreeFingerState g_threeFingerState;

static inline int getTouchCount() {
    using Fn = int (*)();
    static Fn fn = nullptr;
    if (!fn) {
        fn = (Fn)Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"),
                                            OBFUSCATE("UnityEngine"),
                                            OBFUSCATE("Input"),
                                            OBFUSCATE("get_touchCount"));
    }
    return fn ? fn() : 0;
}

static inline void UpdateThreeFingerGesture() {
    const int touchCount = getTouchCount();
    if (touchCount >= 5 && g_threeFingerState.lastCount < 5) {
        double currentTime = ImGui::GetTime();
        if (currentTime - g_threeFingerState.lastFlipTime > 0.3) {
            g_isMenuVisible = !g_isMenuVisible; // Inverte o estado
            g_threeFingerState.lastFlipTime = currentTime;
        }
    }
    g_threeFingerState.lastCount = touchCount;
}

// Função auxiliar para criar o botão de aba horizontal
bool TabButtonHorizontal(const char* label, int tabIndex, int& activeTab) {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Salva o estilo atual
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Fundo transparente por padrão
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.15f, 0.15f, 1.0f)); // Hover sutil
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.20f, 0.20f, 1.0f)); // Active sutil
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f); // Sem arredondamento
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(15, 8)); // Padding para altura
    
    // Se a aba estiver ativa, aplica o estilo de destaque
    if (activeTab == tabIndex) {
        ImGui::PushStyleColor(ImGuiCol_Button, selected_color); // Fundo com cor de destaque (Azul Marinho Escuro)
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, selected_color);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, selected_color);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // Texto branco no fundo escuro
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_Text]); // Texto padrão
    }
    
    bool clicked = ImGui::Button(label);
    
    // Restaura o estilo
    if (activeTab == tabIndex) {
        ImGui::PopStyleColor(4);
    } else {
        ImGui::PopStyleColor(1);
    }
    
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
    
    if (clicked) {
        activeTab = tabIndex;
    }
    
    return clicked;
}


void static menu() {
	if (!g_isMenuVisible) return;
    static bool IsBall = true;
    static float ANIM_SPEED = 0.25f;
    static float Velua = IsBall ? 0.0f : 1.0f;
    Velua = ImClamp(Velua + (ImGui::GetIO().DeltaTime / ANIM_SPEED) * (IsBall ? 1.0f : -1.0f), 0.0f, 1.0f);

    // Cores de animação de abertura (mantidas, mas com o novo accent)
    ImVec4 startColor = ImColor(0.08f, 0.08f, 0.08f, 1.0f); // Novo dark_bg
    ImVec4 endColor   = ImColor(0.12f, 0.12f, 0.12f, 1.0f); // Novo medium_bg
    ImVec4 currentColor = ImLerp(startColor, endColor, Velua);

    ImVec2 windowSize = ImVec2(670 * Velua, 450 * Velua);

    if (Velua > 0.0f) {
        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, currentColor);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, currentColor);
    }

    ApplyMinimalistTheme(); // Aplica o novo tema

    static int activeTab = 1;
    static bool KeyBindTest = false;

    if (init) {
    // Remove ImGuiWindowFlags_NoTitleBar para manter o título e o botão de fechar
    if (ImGui::Begin("SH4DE CLIENT ", &g_isMenuVisible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) {

        // --- NAVEGAÇÃO POR ABAS HORIZONTAIS (TOPO) ---
        // Usando ChildWindow para conter as abas e garantir que fiquem no topo
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); // Remove padding para as tabs ficarem coladas
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.12f, 1.0f)); // Fundo das tabs
        ImGui::BeginChild("TabsContainer", ImVec2(-1, 35), false, ImGuiWindowFlags_NoScrollbar);
        ImGui::PopStyleVar(); // Pop WindowPadding

        // Botões de Aba
        TabButtonHorizontal("AIMBOT", 1, activeTab); ImGui::SameLine();
        TabButtonHorizontal("VISUAL", 2, activeTab); ImGui::SameLine();
        TabButtonHorizontal("CONFIGS", 3, activeTab); ImGui::SameLine();
        TabButtonHorizontal("INFO", 4, activeTab);

        ImGui::EndChild();
        ImGui::PopStyleColor(); // Pop ChildBg

        // --- CONTEÚDO (ABAIXO DAS ABAS) ---
        ImGui::Spacing(); // Espaçamento entre as tabs e o conteúdo
        
        // O conteúdo agora ocupa o restante da janela
        ImGui::BeginChild("Content", ImVec2(-1, -1), true);

            if (activeTab == 1) {
ImGui::TextColored(selected_color, "AIMBOT Settings"); // Título da Aba
	            ImGui::Separator();
	            
	            // Botões de Controle Rápido
	            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x / 2, ImGui::GetStyle().ItemSpacing.y));
	            
	            if (ImGui::Button(OBFUSCATE("Ativar Funções"), ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0))) {
	                AimbotRage = true;
	                EspCircle = true;
	                Active = true;
	            }
	            ImGui::SameLine();
	            if (ImGui::Button(OBFUSCATE("Desativar Funções"), ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
	                AimbotRage = false;
	                EspCircle = false;
	                Active = false;
	            }
	            
	            ImGui::PopStyleVar();
	            ImGui::Separator(); // Separador para separar os botões das checkboxes
            
            // Layout em uma coluna (checkboxes um abaixo do outro)
            
            CustomCheckbox12("Aimbot Rage", &AimbotRage);
            CustomCheckbox12("Aimbot Legit", &AimbotLegit);
            	ImGui::Checkbox("Aim Silent", &AimSilent);
            CustomCheckbox12("Exibir FOV", &EspCircle);

            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Sliders e Combos
            ImGui::SliderFloat("Aim Fov", &Fov, 0, 1000, "%.0f");  

            const char* items[] = { "Cabeça", OBFUSCATE("Pescoço"), OBFUSCATE("Peito"), OBFUSCATE("Quadril") };
            const char* current_item = items[aimPosition];
            ImGui::PushItemWidth(-1); // Combo box ocupa a largura total
            if (ImGui::BeginCombo("Puxada", current_item)) {
                for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
                    bool is_selected = (aimPosition == n);
                    if (ImGui::Selectable(items[n], is_selected)) {
                        aimPosition = n;
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            
            
            }
            
            }

            if (activeTab == 2) {
                ImGui::TextColored(selected_color, "VISUAL Settings (ESP)"); // Título da Aba
                ImGui::Separator();
                
                // Layout em uma coluna (checkboxes um abaixo do outro)
                
                CustomCheckbox12(" Ativar Esp", &Active);
                CustomCheckbox12(OBFUSCATE("ESP Linha"), &EspLine);
                CustomCheckbox12(OBFUSCATE("ESP Caixa"), &EspBox);
                CustomCheckbox12(" Esp Skeleton", &EspSkeleton);
                
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                
                // Combos de Estilo (mantidos, mas com layout mais limpo)
                ImGui::Text("Estilos:");
                ImGui::Separator();
                
                float comboWidth = -1; // Ocupa a largura restante da coluna
                
                ImGui::TextUnformatted("Posição da Esp");
            static const char* lineStyles[] = { OBFUSCATE("Up"), OBFUSCATE("Down") };
            ImGui::SetNextItemWidth(-1);
            ImGui::Combo("##LineStyles", &tempLineStyle, lineStyles, IM_ARRAYSIZE(lineStyles));

                
                
            }

            if (activeTab == 3) {
                ImGui::TextColored(selected_color, "CONFIGS (Miscellaneous & Colors)"); // Título da Aba
                ImGui::Separator();
                
                // Layout em uma coluna (checkboxes um abaixo do outro)
                
                ImGui::Text("Funções Extras:");
                //CustomCheckbox12("Tp Player(RISK DE BUG", &TPPlayer);
                CustomCheckbox12("Voar Player", &UpPlayer);
                CustomCheckbox12("Speed Timer", &SpeedHack);
                
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::Text("Cores:");
                // ColorEdit4 com estilo minimalista (já aplicado pelo tema)
                if (ImGui::ColorEdit4("Cor do Painel", (float*)&selected_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar)) {
                    ApplyMinimalistTheme(); // Aplica o novo tema ao mudar a cor
                }
                             
            ImGui::ColorEdit4("Cor ESP", (float*)&colorHexagono, ImGuiColorEditFlags_NoInputs);
            ImGui::SameLine();
            ImGui::TextUnformatted("Mudar Cor Esp");
            }

            if (activeTab == 4) {
                ImGui::TextColored(selected_color, "INFO"); // Título da Aba
                ImGui::Separator();
                
                ImGui::Text("Status:");
                ImGui::SameLine(0, 10);
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), " Online");
                
                ImGui::Spacing();
                ImGui::TextWrapped("MOD FEITO POR @breonnmodz tg");
                ImGui::Spacing();
                ImGui::Separator();
               
            }

            ImGui::EndChild(); // End Content
        }
        ImGui::End(); // End SH4DE CLIENT
    }

