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

extern ImVec4 ThemeFov;
extern ImVec4 ThemeColer;
bool init = true;

static bool botao_selint = false;
static bool botao_ligado = false;
static bool botaospeedhack = false;
static bool botaoselint = false;

namespace Theme {
    const ImVec4 Border = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    const ImVec4 Background = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    const ImVec4 Text = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
}


	
// Static variable to store the user-selected color
static ImVec4 selected_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // VERMELHO PURÃO E FORTE (#FF0000)

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

void ApplyBlueTheme() {
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    const ImVec4 black_solid(0.00f, 0.00f, 0.0f, 1.00f);

    // Use the user-selected color
    colors[ImGuiCol_WindowBg]        = black_solid;
    colors[ImGuiCol_ChildBg]         = black_solid;
    colors[ImGuiCol_TitleBg]         = selected_color;
    colors[ImGuiCol_TitleBgActive]   = selected_color;
    colors[ImGuiCol_TitleBgCollapsed]= selected_color;

    colors[ImGuiCol_Button]          = selected_color;
    colors[ImGuiCol_ButtonHovered]   = ImVec4(selected_color.x * 1.1f, selected_color.y * 1.1f, selected_color.z * 1.1f, selected_color.w);
    colors[ImGuiCol_ButtonActive]    = ImVec4(selected_color.x * 0.9f, selected_color.y * 0.9f, selected_color.z * 0.9f, selected_color.w);

    colors[ImGuiCol_Header]          = selected_color;
    colors[ImGuiCol_HeaderHovered]   = ImVec4(selected_color.x * 1.1f, selected_color.y * 1.1f, selected_color.z * 1.1f, selected_color.w);
    colors[ImGuiCol_HeaderActive]    = ImVec4(selected_color.x * 0.9f, selected_color.y * 0.9f, selected_color.z * 0.9f, selected_color.w);

    colors[ImGuiCol_FrameBg]         = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]  = selected_color;
    colors[ImGuiCol_FrameBgActive]   = selected_color;

    colors[ImGuiCol_SliderGrab]      = selected_color;
    colors[ImGuiCol_SliderGrabActive]= selected_color;

    colors[ImGuiCol_CheckMark]       = selected_color;

    // Apply selected_color to RadioButton
    colors[ImGuiCol_CheckMark]       = selected_color; // Used for the active radio button dot

    // Definir fundo do combo box como preto
    colors[ImGuiCol_PopupBg]         = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

    // Garantir que não haja bordas ou sombras
    colors[ImGuiCol_Border]       = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize  = 0.0f; // Garantir que os botões não tenham bordas
    style.TabBorderSize    = 0.0f;

    // Manter bordas redondas e ajustar para evitar cantos pretos
    style.FrameRounding = 8.0f; // Mantém bordas arredondadas
    style.GrabRounding = 12.0f;
    style.WindowRounding = 10.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.TabRounding = 6.0f;
    style.GrabMinSize = 15.0f;

    // Ajustar padding para cobrir completamente os cantos arredondados
    style.FramePadding = ImVec2(12, 8); // Aumentado para evitar exposição do fundo preto
    style.ItemSpacing = ImVec2(8, 4);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 15.0f;

    // Tornar o scrollbar transparente
    colors[ImGuiCol_ScrollbarBg]     = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); // Fundo do scrollbar transparente
    colors[ImGuiCol_ScrollbarGrab]  = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); // Alça do scrollbar transparente

    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.DisplaySafeAreaPadding = ImVec2(4, 4);
}

bool CustomCheckbox12(const char* label, bool* v) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    const float square_sz = ImGui::GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos, ImVec2(pos.x + square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), pos.y + label_size.y + style.FramePadding.y * 2.0f));
    
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    
    if (hovered && g.IO.MouseClicked[0]) {
        *v = !(*v);
        ImGui::MarkItemEdited(id);
        pressed = true;
    }

    const ImRect check_bb(pos, ImVec2(pos.x + square_sz, pos.y + square_sz));

    static std::map<ImGuiID, float> animTimes;
    float& animTime = animTimes[id];

    if (pressed) {
        animTime = 0.0f;
    } else if (animTime > 0.0f) {
        animTime -= g.IO.DeltaTime * 8.0f;
        if (animTime < 0.0f) animTime = 0.0f;
    }

    float scale = 1.0f - (animTime * 0.2f);

    ImVec2 center = ImVec2((check_bb.Min.x + check_bb.Max.x) * 0.5f, (check_bb.Min.y + check_bb.Max.y) * 0.5f);
    ImVec2 size = ImVec2((check_bb.Max.x - check_bb.Min.x) * scale, (check_bb.Max.y - check_bb.Min.y) * scale);
    ImRect anim_bb = ImRect(center.x - size.x * 0.5f, center.y - size.y * 0.5f, center.x + size.x * 0.5f, center.y + size.y * 0.5f);

    // 🎨 COR DE FUNDO — sempre preenchido, mesmo desmarcado
    ImU32 bg_color = *v
        ? ImGui::GetColorU32(selected_color) // roxo/ativado
        : ImGui::GetColorU32(ImVec4(0.08f, 0.08f, 0.08f, 1.0f)); // cinza escuro (igual sua imagem)

    // 🟦 Fundo preenchido
    window->DrawList->AddRectFilled(anim_bb.Min, anim_bb.Max, bg_color, style.FrameRounding);

    // 🔲 Borda sutil
    ImU32 border_color = ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    window->DrawList->AddRect(anim_bb.Min, anim_bb.Max, border_color, style.FrameRounding, 0, 1.5f);

    if (label_size.x > 0.0f)
        ImGui::RenderText(ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, pos.y + style.FramePadding.y), label);

    return pressed;
}

static bool g_isMenuVisible = true;
static bool g_isModActive = true; // Controla se o mod está ativo

struct TouchState {
int lastCount = 0;
double lastFlipTime = 0.0;
double lastCloseTime = 0.0; // Para o gesto de fechar
};
static TouchState g_touchState;

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

static inline void UpdateTouchGestures() {
const int touchCount = getTouchCount();
double currentTime = ImGui::GetTime();

// Gesto de 4 dedos para minimizar/desminimizar o menu completamente  
if (touchCount == 3 && g_touchState.lastCount < 3) {  
    if (currentTime - g_touchState.lastFlipTime > 0.3) {  
        g_isMenuVisible = !g_isMenuVisible;  
        g_touchState.lastFlipTime = currentTime;  
    }  
}  

// Gesto de 5 dedos para fechar o mod totalmente  
if (touchCount == 10 && g_touchState.lastCount < 10) {  
    if (currentTime - g_touchState.lastCloseTime > 0.3) {  
        g_isModActive = false; // Desativa o mod  
        g_touchState.lastCloseTime = currentTime;  
    }  
}  

g_touchState.lastCount = touchCount;

}



void static menu() {
        if (!g_isModActive)
        return;

    UpdateTouchGestures();  
    ApplyBlueTheme();  

    ImGuiIO& io = ImGui::GetIO();  
    if (!g_isMenuVisible)  
        return;  
    static bool IsBall = true;
    static float ANIM_SPEED = 0.25f;
    static float Velua = IsBall ? 0.0f : 1.0f;
    Velua = ImClamp(Velua + (ImGui::GetIO().DeltaTime / ANIM_SPEED) * (IsBall ? 1.0f : -1.0f), 0.0f, 1.0f);

    ImVec4 startColor = ImColor(0.0f, 0.094f, 0.282f, 1.0f);
    ImVec4 endColor   = ImColor(0.0f, 0.2f, 0.5f, 1.0f);
    ImVec4 currentColor = ImLerp(startColor, endColor, Velua);

    ImVec2 windowSize = ImVec2(620 * Velua, 420 * Velua);

    if (Velua > 0.0f) {
        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, currentColor);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, currentColor);
    }

    ApplyBlueTheme();

    static int activeTab = 1;
    static bool KeyBindTest = false;

    if (init) {
	if (ImGui::Begin(OBFUSCATE("FFH4X NT XITERS ANDROID APK MOD"), &g_isMenuVisible,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) {
     // 🔧 Remove o espaço preto (sem título)
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, currentColor);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));
	
// --- MENU LATERAL (TABS) ---
ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

// Deixa a sidebar SEM fundo preto (transparente)
ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));

// Início do painel esquerdo
ImGui::BeginChild("LeftMenu", ImVec2(83, -0), false);

ImVec2 imageSize(28, 28);

if (ImGui::Button("##Aim", ImVec2(75, 75))) activeTab = 1;
{
    ImVec2 p = ImGui::GetItemRectMin();
    ImVec2 s = ImGui::GetItemRectSize();
    ImVec2 imgPos(
        p.x + (s.x - imageSize.x) * 0.5f,
        p.y + (s.y - imageSize.y) * 0.5f
    );
    ImGui::GetWindowDrawList()->AddImage(
        iconaim.textureId,
        imgPos,
        ImVec2(imgPos.x + imageSize.x, imgPos.y + imageSize.y)
    );
}

ImGui::Spacing();

if (ImGui::Button("##Esp", ImVec2(75, 75))) activeTab = 2;
{
    ImVec2 p = ImGui::GetItemRectMin();
    ImVec2 s = ImGui::GetItemRectSize();
    ImVec2 imgPos(
        p.x + (s.x - imageSize.x) * 0.5f,
        p.y + (s.y - imageSize.y) * 0.5f
    );
    ImGui::GetWindowDrawList()->AddImage(
        iconesp.textureId,
        imgPos,
        ImVec2(imgPos.x + imageSize.x, imgPos.y + imageSize.y)
    );
}

ImGui::Spacing();

if (ImGui::Button("##Misc", ImVec2(75, 75))) activeTab = 3;
{
    ImVec2 p = ImGui::GetItemRectMin();
    ImVec2 s = ImGui::GetItemRectSize();
    ImVec2 imgPos(
        p.x + (s.x - imageSize.x) * 0.5f,
        p.y + (s.y - imageSize.y) * 0.5f
    );
    ImGui::GetWindowDrawList()->AddImage(
        iconmisc.textureId,
        imgPos,
        ImVec2(imgPos.x + imageSize.x, imgPos.y + imageSize.y)
    );
}

ImGui::Spacing();

if (ImGui::Button("##Info", ImVec2(75, 75))) activeTab = 4;
{
    ImVec2 p = ImGui::GetItemRectMin();
    ImVec2 s = ImGui::GetItemRectSize();
    ImVec2 imgPos(
        p.x + (s.x - imageSize.x) * 0.5f,
        p.y + (s.y - imageSize.y) * 0.5f
    );
    ImGui::GetWindowDrawList()->AddImage(
        iconinfo.textureId,
        imgPos,
        ImVec2(imgPos.x + imageSize.x, imgPos.y + imageSize.y)
    );
}

ImGui::EndChild(); // <-- agora faz sentido
ImGui::PopStyleVar();
ImGui::PopStyleColor();

            // --- CONTEÚDO (DIREITA) ---
            ImGui::SameLine();
            ImGui::BeginChild("ChildContent", ImVec2(-1, -1), true);

            if (activeTab == 1) {
            ImGui::Separator();
ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
ImGui::Separator();
ImGui::PopStyleColor();
ImGui::Separator();
            CustomCheckbox12("Aimbot Rage", &AimbotRage);
			ImGui::SameLine(0, 22);
            CustomCheckbox12("Aimbot Legit", &AimbotLegit);
            CustomCheckbox12("Exibir FOV", &EspCircle);
			ImGui::SameLine(0, 50);
			CustomCheckbox12("Exibir Linha", &EsnapLine);
          ImGui::Separator();
          ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
          ImGui::Separator();
          ImGui::PopStyleColor();
          ImGui::Separator();
            ImGui::SliderInt(OBFUSCATE("Regular FOV"), &Fov, 0, 960, "%d.0");

           
            ImGui::Spacing();

            const char* items[] = { "Cabeça", OBFUSCATE("Pescoço"), OBFUSCATE("Peito"), OBFUSCATE("Quadril") };
            const char* current_item = items[aimPosition];
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

            ImGui::Separator();
ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
ImGui::Separator();
ImGui::PopStyleColor();
ImGui::Separator();
            
            ImGui::Text("Tipo de Aimbot:");
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.12f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, selected_color);
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, selected_color);
            ImGui::PushStyleColor(ImGuiCol_CheckMark, selected_color);
                if (ImGui::RadioButton(OBFUSCATE("Ao Olhar"), AimbotAuto)) {
                    AimbotAuto = true;
                    HidePanel = false;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton(OBFUSCATE("Ao Atirar"), !AimbotAuto)) {
                    AimbotAuto = false;
                    HidePanel = true;
                }
            }

            if (activeTab == 2) {
				ImGui::Separator();
            ImGui::PushStyleColor(ImGuiCol_Separator, selected_color);
            ImGui::Separator();
            ImGui::PopStyleColor();
            ImGui::Separator();
	    	CustomCheckbox12(" Ativar Esp", &Active);
	        ImGui::SameLine();
	    	CustomCheckbox12(" Esconder painel", &g_isMenuVisible);	    
            
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.12f, 0.54f));
                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.20f, 0.20f, 0.20f, 0.60f));
                ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.25f, 0.25f, 0.25f, 0.67f));
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(selected_color)); // #9B07EBFF
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

                float comboWidth = 180.0f;
				
                CustomCheckbox12(OBFUSCATE("ESP Linha"), &EspLine);
                CustomCheckbox12(OBFUSCATE("ESP Caixa"), &EspBox);
                CustomCheckbox12(OBFUSCATE("ESP Vida"), &EspVida);
                CustomCheckbox12(OBFUSCATE("Esp Nome"), &Nick);
                CustomCheckbox12(" Esp Inimigos", &Inimigos);
				CustomCheckbox12(" Esp Skeleton", &EspSkeleton);
            }

            if (activeTab == 3) {
ImGui::Separator();
ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
ImGui::Separator();
ImGui::PopStyleColor();
ImGui::Separator();

                ImGui::Text("Cores");
                ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                if (ImGui::ColorEdit4("Cor do Painel", (float*)&selected_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar)) {
                    ApplyBlueTheme();
                }
				ImGui::SameLine();
				ImGui::ColorEdit4(OBFUSCATE("Cor do Fov"), (float*)&ThemeFov, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_PickerHueBar);
            ImGui::PopStyleColor();
                ImGui::ColorEdit4(OBFUSCATE("Cor da Esp"), (float*)&ThemeColer, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_PickerHueBar);
                ImGui::PopStyleColor();
                ImGui::Text("Função Extra");
                CustomCheckbox12("Ativar Selint", &botaoselint);
				CustomCheckbox12("Ativar Speed", &botaospeedhack);
            }

            if (activeTab == 4) {
                ImGui::Text("Status:");
                ImGui::SameLine(0, 10);
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), " Online");
            }

            ImGui::EndChild();
        }
        ImGui::End();
    }

    if (Velua > 0.0f) {
        ImGui::PopStyleColor(2);
    }
}

void RenderBotaoFlutuante()
{
    if (botaospeedhack) {
        // Cores para o botão com tema elétrico
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.2f, 0.9f));          // Fundo azul escuro
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.4f, 0.9f));   // Hover azul médio
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.6f, 0.9f));    // Active azul mais claro
        
        // Posição flutuante (canto superior esquerdo)
        ImGui::SetNextWindowPos(ImVec2(30, 30), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(160, 100)); // Um pouco maior
        
        // Janela com borda azul neon
        ImGui::PushStyleColor(ImGuiCol_Border, Theme::Border);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
        
        ImGui::Begin("BotaoFlutuante", nullptr, 
                     ImGuiWindowFlags_NoTitleBar | 
                     ImGuiWindowFlags_NoResize | 
                     ImGuiWindowFlags_NoScrollbar | 
                     ImGuiWindowFlags_NoBackground);
        
        // Texto do botão baseado no estado
        const char* texto_botao = botao_ligado ? "SPEED" : "SPEED";
        
        // Cor do texto baseado no estado - Azul neon quando ligado
        ImVec4 botao_ligado, cor_texto;
cor_texto = botao_ligado = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

        
        ImGui::PushStyleColor(ImGuiCol_Text, cor_texto);
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Fonte padrão
        
        // Botão com tamanho ajustado
        if (ImGui::Button(texto_botao, ImVec2(140, 35)))
        {
            bool botao_ligado = false;
            botao_ligado = !botao_ligado;

            SpeedHack = !SpeedHack;
        }
        
        ImGui::PopFont();
        ImGui::PopStyleColor(); // Remove a cor do texto
        ImGui::End();
        
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4); // Remove todas as cores
    }
}

void RenderBotaoSelint()
{
    if (botaoselint) {
        // Cores para o botão com tema elétrico
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.2f, 0.9f));          // Fundo azul escuro
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.4f, 0.9f));   // Hover azul médio
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.6f, 0.9f));    // Active azul mais claro
        
        // Posição flutuante (canto superior esquerdo)
        ImGui::SetNextWindowPos(ImVec2(30, 30), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(160, 100)); // Um pouco maior
        
        // Janela com borda azul neon
        ImGui::PushStyleColor(ImGuiCol_Border, Theme::Border);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
        
        ImGui::Begin("BotaoSelint", nullptr, 
                     ImGuiWindowFlags_NoTitleBar | 
                     ImGuiWindowFlags_NoResize | 
                     ImGuiWindowFlags_NoScrollbar | 
                     ImGuiWindowFlags_NoBackground);
        
        // Texto do botão baseado no estado
        const char* texto_up = botao_selint ? "AIM SELINT" : "AIM SELINT";
        
        // Cor do texto baseado no estado - Azul neon quando ligado
        ImVec4 botao_selint, cor_texto;
cor_texto = botao_selint = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

        
        ImGui::PushStyleColor(ImGuiCol_Text, cor_texto);
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Fonte padrão
        
        // Botão com tamanho ajustado
        if (ImGui::Button(texto_up, ImVec2(140, 35)))
        {
            bool botao_selint = false;
            botao_selint = !botao_selint;

            AimSilent = !AimSilent;
        }
        
        ImGui::PopFont();
        ImGui::PopStyleColor(); // Remove a cor do texto
        ImGui::End();
        
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4); // Remove todas as cores
    }
}
