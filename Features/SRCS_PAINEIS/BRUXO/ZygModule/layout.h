#include <imports.h>
#include <imguiconfig.h>
#include "ESP.h"
#include "particles.h"
#include <string>
#include <chrono>

#ifndef OBFUSCATE
#define OBFUSCATE(str) str
#endif

// =========================================================================
// CONTROLE DE VISIBILIDADE E GESTOS (5 TAPS MENU / 15 TAPS PANIC)
// =========================================================================

static bool g_isMenuVisible = true;
static bool g_isModActive = true;

// Variáveis para controle dos 5/15 clicks
static int tapCount = 0;
static int panicTapCount = 0;
static double lastTapTime = 0.0;
static double lastPanicTapTime = 0.0;

static const double TAP_TIMEOUT = 1.0;    // 1 segundo para o tempo limite entre os taps
static const int REQUIRED_TAPS = 10;      // 5 taps para mostrar/esconder o menu
static const int REQUIRED_PANIC_TAPS = 50; // 15 taps para desativar o mod (Panic)


// Função principal para gerenciar Taps (Chamada em MenuRender)
void handleAllTaps() {
    // Apenas verifica se houve um clique (toque) na tela neste frame
    if (!ImGui::IsMouseClicked(0)) {
        return;
    }

    double currentTime = ImGui::GetTime();
    
    // --- Lógica de 5 Taps (Alternar Menu) ---
    // Se o tempo limite foi excedido, reinicia a contagem
    if (currentTime - lastTapTime > TAP_TIMEOUT) {
        tapCount = 0;
    }
    // Contagem de tap
    tapCount++;
    lastTapTime = currentTime;
    
    // Se a contagem atingir o necessário (5), alterna a visibilidade e reinicia
    if (tapCount >= REQUIRED_TAPS) {
        g_isMenuVisible = !g_isMenuVisible; // Alterna a visibilidade do menu
        tapCount = 0; // Reinicia a contagem
    }

    // --- Lógica de 15 Taps (Panic) ---
    // Se o tempo limite foi excedido, reinicia a contagem
    if (currentTime - lastPanicTapTime > TAP_TIMEOUT) {
        panicTapCount = 0;
    }
    // Contagem de tap
    panicTapCount++;
    lastPanicTapTime = currentTime;
    
    // Se a contagem atingir o necessário (15), desativa o mod e reinicia
    if (panicTapCount >= REQUIRED_PANIC_TAPS) {
        g_isModActive = false; // DESATIVA O MOD (PANIC)
        panicTapCount = 0;     // Reinicia a contagem
    }
}


// A função original UpdateTouchGestures é mantida, mas a lógica de tap será no FloatingButtonRender.
static inline void UpdateTouchGestures() {
    // Pode ser usado para a lógica de 4 toques se necessário, mas está vazio aqui
}

// =========================================================================
// VARIÁVEIS DE FEATURES (MANTIDAS)
// =========================================================================

// Aim (Mapeamento aproximado com a base fornecida)
static bool legitAimbot = false;
static bool aimbotToggle = false;
static int delayAimbot = 0; 
static bool aimCrouching = false;
static int aimFov = 999;
static int targetAimbot = 0;

// Draw (ESP)
static bool enableEsp = false;
static bool draw360 = false;
static bool drawInfo = false;
static bool drawHealthV2 = false;
static bool drawPlayerId = false;
static bool drawCount = false;
static int configColor = 0;
static int styleTypeBox = 0;
static int styleTypeLine = 0;

// Chams
static bool chamsTransparent = false;
static bool chamsWireframe = false;
static bool chamsGlow = false;
static bool chamsOutline = false;
static int chamsColor = 0;

// Variáveis adicionais usadas nas abas (simulação de ImGui::ColorEdit4 e Sliders)
// Se essas não existirem, o código irá falhar na compilação.
extern int Fov; 
extern int aimSmoothing;
extern int aimPosition; // Deve ser int para o Combo
extern bool EspCircle;
extern bool AimbotAim;
extern bool AimbotRage;
extern bool AimbotLegit;
extern bool Active; // Para Enable Aimbot
extern bool EnableESP;
extern bool EspLine;
extern bool EspBox;
extern bool EspVida;
extern bool EspSkeleton;
extern bool Inimigos;
extern bool Nick;
extern bool wireframe;
extern bool SpeedHack;
extern float rgb1, rgb2, rgb3; // Cores Chams (float para sliders)
extern void updateColors();
extern ImVec4 ThemeFov; // Adicionado para evitar erro de compilação
extern ImVec4 ThemeColer; // Adicionado para evitar erro de compilação
// Adicionando variáveis ausentes para a compilação:
extern bool AimbotTrigger;
namespace ESPConfig {
    extern bool EnableESP;
    extern bool EspLine;
    extern bool EspBox;
    extern bool EspVida;
}
extern bool AimbotEsp;


// =========================================================================
// TEMA PRETO & VERMELHO NEON (AGRESSIVO)
// =========================================================================

void ApplyTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Cores base escuras
    const ImVec4 black_deep   = ImVec4(0.06f, 0.06f, 0.06f, 0.98f);
    const ImVec4 black_dark   = ImVec4(0.03f, 0.03f, 0.03f, 0.99f);
    const ImVec4 black_medium = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);

    // Cores de destaque (Neon Red)
    const ImVec4 red_neon     = ImVec4(1.0f, 0.0f, 0.2f, 1.0f);
    const ImVec4 red_bright   = ImVec4(1.0f, 0.1f, 0.1f, 1.0f);
    const ImVec4 red_border   = ImVec4(0.8f, 0.0f, 0.0f, 0.7f);

    // Texto
    const ImVec4 text_white   = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);

    // Fundo
    colors[ImGuiCol_WindowBg]              = black_deep;
    colors[ImGuiCol_ChildBg]               = black_dark;
    colors[ImGuiCol_PopupBg]               = ImVec4(0.04f, 0.04f, 0.04f, 0.98f);

    // Bordas
    colors[ImGuiCol_Border]                = red_border;
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Títulos
    colors[ImGuiCol_TitleBg]               = black_deep;
    colors[ImGuiCol_TitleBgActive]         = black_deep;
    colors[ImGuiCol_TitleBgCollapsed]      = black_deep;

    // Botões (Abas Laterais)
    colors[ImGuiCol_Button]                = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_ButtonHovered]         = red_neon;
    colors[ImGuiCol_ButtonActive]          = red_bright;

    // Headers
    colors[ImGuiCol_Header]                = black_medium;
    colors[ImGuiCol_HeaderHovered]         = red_neon;
    colors[ImGuiCol_HeaderActive]          = red_bright;

    // Checkmarks & Sliders
    colors[ImGuiCol_CheckMark]             = red_bright;
    colors[ImGuiCol_SliderGrab]            = red_neon;
    colors[ImGuiCol_SliderGrabActive]      = red_bright;

    // Frames (Inputs, Checkboxes BG)
    colors[ImGuiCol_FrameBg]               = black_medium;
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);

    // Texto
    colors[ImGuiCol_Text]                  = text_white;
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = red_neon;
    colors[ImGuiCol_ScrollbarGrabActive]   = red_bright;

    // Estilo
    style.WindowRounding    = 8.0f;
    style.ChildRounding     = 6.0f;
    style.FrameRounding     = 4.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding      = 4.0f;
    style.PopupRounding     = 6.0f;

    style.WindowPadding     = ImVec2(0, 0);
    style.FramePadding      = ImVec2(8, 6);
    style.ItemSpacing       = ImVec2(10, 7);
    style.ItemInnerSpacing  = ImVec2(6, 6);
    style.IndentSpacing     = 22.0f;
    style.WindowBorderSize  = 1.0f;
}

// -------------------------------------------------------------------------
// NOTE: FloatingButtonRender() FOI REMOVIDA
// -------------------------------------------------------------------------

// =========================================================================
// MENU RENDER (COM CHAMADA PARA LÓGICA DE TAPS)
// =========================================================================

void MenuRender() {
    if (!g_isModActive) return;

    // 1. Verifica a Lógica de Taps em qualquer toque na tela (substituindo o botão flutuante)
    handleAllTaps();

    // 2. Se o menu principal estiver escondido, para a execução aqui
    if (!g_isMenuVisible) return;
    
    ApplyTheme();

    ImGuiIO& io = ImGui::GetIO();
    const ImVec2 windowSize = ImVec2(600, 620);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

    if (ImGui::Begin("GOAT XITERS - Mobilador", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) {

        // --- Layout em Duas Colunas (Abas e Conteúdo) ---

        static int activeTab = 0; // 0: Aim, 1: Draw, 2: Chams

        const float tabSectionHeight = ImGui::GetContentRegionAvail().y;

        // Painel de Abas (Lado Esquerdo)
        ImGui::BeginChild("##TabsPanel", ImVec2(100, tabSectionHeight), true, ImGuiWindowFlags_NoScrollbar);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));

        const char* tabNames[] = { "Aim", "Draw", "Chams" };
        const int tabCount = 3;
        const float tabButtonHeight = 40.0f;

        for (int i = 0; i < tabCount; i++) {
            bool selected = (activeTab == i);
            ImGui::PushStyleColor(ImGuiCol_Button, selected ? ImVec4(1.0f, 0.0f, 0.2f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.1f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.2f, 1.0f));

            if (ImGui::Button(tabNames[i], ImVec2(ImGui::GetContentRegionAvail().x, tabButtonHeight))) {
                activeTab = i;
            }

            ImGui::PopStyleColor(3);
            if (i < tabCount - 1) ImGui::Spacing();
        }

        ImGui::PopStyleVar(2);
        ImGui::EndChild();

        ImGui::SameLine();

        // Painel de Conteúdo (Lado Direito)
        ImGui::BeginChild("##ContentPanel", ImVec2(ImGui::GetContentRegionAvail().x, tabSectionHeight), false, ImGuiWindowFlags_NoScrollbar);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));
        
        // --- RENDERIZAÇÃO DO CONTEÚDO DA ABA ATIVA ---

        // =====================================
        // ABA AIM (0)
        // =====================================
        if (activeTab == 0) {
            ImGui::Text("Legit Aimbot");
            ImGui::Checkbox("Aimbot Colider", &AimbotLegit);

            ImGui::Text("Aimbot Rage");
            ImGui::Checkbox("Enable Aimbot", &Active); 
            ImGui::Checkbox("Aim Scope", &AimbotAim);
            ImGui::Checkbox("Aim Fire", &AimbotRage);
            ImGui::Checkbox("Exibir Fov", &EspCircle);

            ImGui::Text("Aim Fov");
            ImGui::SliderInt(OBFUSCATE("FOV"), &Fov, 0, 960, "%d.0");
            
			ImGui::Text("Deley Aimbot");
            ImGui::SliderInt(OBFUSCATE("Aim"), &aimSmoothing, 0, 960, "%d.0");
			
			
            ImGui::PushItemWidth(325.0f);				  
            const char* targetOptions[] = {"Cabeça", "Pescoço", "Peito"};    
            ImGui::Combo("Alvo", &aimPosition, targetOptions, IM_ARRAYSIZE(targetOptions));
			
        }

        // =====================================
        // ABA DRAW (1)
        // =====================================
        else if (activeTab == 1) {
            ImGui::Checkbox("Enable Esp", &EnableESP);
            ImGui::Checkbox("Draw - Line", &EspLine);
            ImGui::Checkbox("Draw - Box", &EspBox);
            ImGui::Checkbox("Draw - Health", &EspVida);
            ImGui::Checkbox("Draw - Skeleton", &EspSkeleton);
            ImGui::Checkbox("Draw - PlayerID", &Inimigos);
            ImGui::Checkbox("Draw - Name", &Nick);

            ImGui::Spacing();
            ImGui::Text("Config Color");
        	ImGui::ColorEdit4(OBFUSCATE("Cor do Fov"), (float*)&ThemeFov, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_PickerHueBar);    
            // ImGui::PopStyleColor(); // Removido para evitar erro
            ImGui::ColorEdit4(OBFUSCATE("Cor da Esp"), (float*)&ThemeColer, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_PickerHueBar);    
            // ImGui::PopStyleColor(); // Removido para evitar erro
        }

        // =PADRONIZAÇÃO DO CÓDIGO DO MENU ANTERIOR (2)
        // =====================================
        else if (activeTab == 2) {
            ImGui::Checkbox("Chams", &wireframe);  
            ImGui::Checkbox("Speed Hack", &SpeedHack);
			
            ImGui::Spacing();
            ImGui::Text("Chams Color");
            if (ImGui::SliderFloat("R", &rgb1, 0.0f, 255.0f, "%.0f")) updateColors();
            if (ImGui::SliderFloat("G", &rgb2, 0.0f, 255.0f, "%.0f")) updateColors();
            if (ImGui::SliderFloat("B", &rgb3, 0.0f, 255.0f, "%.0f")) updateColors();
        }

        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::End();
    }
}
