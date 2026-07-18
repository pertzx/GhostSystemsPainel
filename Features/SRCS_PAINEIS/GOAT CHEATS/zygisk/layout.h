#pragma once
#ifndef LAYOUT_H_INCLUDED
#define LAYOUT_H_INCLUDED

#include <imports.h>
#include "BatteryInfo.h"
#include <imguiconfig.h>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <string>

// ======= EXTERN (já existem no seu projeto) =======
extern bool AimbotEsp, aimbotAim, aimbotTrigger, Aimkil, DadosEnemy, EspLine, Inimigos, ResetGuest1, SpeedHack;
extern int  aimPosition;           // 0=cabeça, 1=pescoço, 2=peito, 3=quadril
extern int Fov_Aim, Aimdis;      // sliders
extern void addSwitch(const char* label, const char* icon, bool* v);

// Texturas externas (estruturas já existentes c/ .textureId)
extern decltype(logo)   logo;
extern decltype(aimbot) aimbot;
extern decltype(esp)    esp;

// ====== estado/ocultar ======
static bool g_hideUI = true; // começa escondido (3 dedos mostra/oculta)
static bool g_blockAllOnHide = true;

// ====== estado local ======
static bool   gspika      = false;
static bool   uiDragging  = false;
static ImVec2 uiButtonPos = ImVec2(50.f, 50.f);

static void DisableAllFeatures() {
    AimbotEsp=false; aimbotAim=false; aimbotTrigger=false; Aimkil=false;
    DadosEnemy=false; EspLine=false; Inimigos=false; ResetGuest1=false;
}

// ================== GESTO 3 DEDOS ==================
static inline int getTouchCount(){
    using Fn = int (*)();
    static Fn fn = nullptr;
    if (!fn) fn = (Fn)Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"),
                                            OBFUSCATE("UnityEngine"),
                                            OBFUSCATE("Input"),
                                            OBFUSCATE("get_touchCount"));
    return fn ? fn() : 0;
}
struct ThreeFingerState { int lastCount=0; bool toggled=false; double lastFlip=0.0; };
static ThreeFingerState g_tfState;
static inline void UpdateThreeFinger(ThreeFingerState& st, bool& hideFlag){
    st.toggled=false; const int tc=getTouchCount();
    if (tc>=3 && st.lastCount<4){
        double now=ImGui::GetTime();
        if (now-st.lastFlip>0.20){ hideFlag=!hideFlag; st.toggled=true; st.lastFlip=now; }
    }
    st.lastCount=tc;
}

// =================== TEMA ===================
static inline ImVec2 SZ(float x,float y){ return ImVec2(x,y); } // sem escala

struct Theme {
    ImU32 topBar, text, stripe, childBg;
    ImU32 btn, btnHover, btnActive;
    ImU32 particleCore, particleGlow, lineBase;
};

// ... (a tabela THEMES, THEME_NAMES e ApplyTheme exatamente como você já tem)

static const Theme THEMES[15] = {
    { IM_COL32(26,26,26,255), IM_COL32(255,245,235,255), IM_COL32(255,150,30,255), IM_COL32(10,10,10,255),
      IM_COL32(36,24,8,255),  IM_COL32(44,28,10,255),    IM_COL32(55,32,9,255),
      IM_COL32(255,140,0,235),IM_COL32(255,160,40,40),   IM_COL32(255,170,60,255) }, // Laranja
    { IM_COL32(18,24,32,255), IM_COL32(230,240,255,255), IM_COL32(70,150,255,255),  IM_COL32(12,14,18,255),
      IM_COL32(24,30,46,255), IM_COL32(30,38,58,255),    IM_COL32(36,46,70,255),
      IM_COL32(80,160,255,235),IM_COL32(120,170,255,40), IM_COL32(130,180,255,255) }, // Azul
    { IM_COL32(24,16,30,255), IM_COL32(245,235,255,255), IM_COL32(170,90,255,255),  IM_COL32(15,10,18,255),
      IM_COL32(34,22,50,255), IM_COL32(42,28,62,255),    IM_COL32(52,34,78,255),
      IM_COL32(190,110,255,235),IM_COL32(210,140,255,40),IM_COL32(210,150,255,255) }, // Roxo
    { IM_COL32(16,24,18,255), IM_COL32(230,255,240,255), IM_COL32(60,200,120,255),  IM_COL32(8,12,10,255),
      IM_COL32(18,36,24,255), IM_COL32(22,44,30,255),    IM_COL32(28,54,36,255),
      IM_COL32(40,220,140,235),IM_COL32(80,240,160,40),  IM_COL32(100,250,170,255) }, // Verde
    { IM_COL32(28,14,14,255), IM_COL32(255,235,235,255), IM_COL32(255,80,80,255),   IM_COL32(12,8,8,255),
      IM_COL32(46,20,20,255), IM_COL32(58,26,26,255),    IM_COL32(70,30,30,255),
      IM_COL32(255,90,90,235),IM_COL32(255,110,110,40),  IM_COL32(255,120,120,255) }, // Vermelho
    { IM_COL32(14,22,24,255), IM_COL32(220,245,245,255), IM_COL32(40,200,200,255),  IM_COL32(10,14,16,255),
      IM_COL32(18,36,38,255), IM_COL32(22,46,48,255),    IM_COL32(28,58,60,255),
      IM_COL32(50,210,210,235),IM_COL32(80,230,230,40),  IM_COL32(90,240,240,255) },  // Ciano
    { IM_COL32(26,14,22,255), IM_COL32(255,230,245,255), IM_COL32(255,60,160,255),  IM_COL32(12,8,12,255),
      IM_COL32(44,18,36,255), IM_COL32(56,22,44,255),    IM_COL32(70,28,56,255),
      IM_COL32(255,70,170,235),IM_COL32(255,120,200,40), IM_COL32(255,130,210,255) }, // Rosa neon
    { IM_COL32(26,22,14,255), IM_COL32(255,250,220,255), IM_COL32(255,190,60,255),  IM_COL32(12,10,8,255),
      IM_COL32(46,36,18,255), IM_COL32(58,44,22,255),    IM_COL32(72,54,28,255),
      IM_COL32(255,200,80,235),IM_COL32(255,210,120,40), IM_COL32(255,220,140,255) }, // Dourado
    { IM_COL32(22,24,26,255), IM_COL32(230,235,240,255), IM_COL32(140,160,180,255), IM_COL32(12,12,14,255),
      IM_COL32(34,38,44,255), IM_COL32(42,46,54,255),    IM_COL32(54,60,68,255),
      IM_COL32(150,170,190,235),IM_COL32(170,190,210,40),IM_COL32(180,200,220,255) }, // Cinza
    { IM_COL32(18,26,14,255), IM_COL32(235,255,230,255), IM_COL32(150,220,60,255),  IM_COL32(10,12,8,255),
      IM_COL32(24,38,18,255), IM_COL32(30,48,22,255),    IM_COL32(36,58,26,255),
      IM_COL32(160,230,70,235),IM_COL32(180,240,110,40), IM_COL32(190,250,120,255) }, // Lima
    { IM_COL32(22,12,24,255), IM_COL32(245,230,250,255), IM_COL32(200,60,220,255),  IM_COL32(10,6,12,255),
      IM_COL32(40,18,44,255), IM_COL32(52,22,56,255),    IM_COL32(64,28,70,255),
      IM_COL32(210,70,230,235),IM_COL32(230,120,245,40), IM_COL32(235,130,250,255) }, // Magenta
    { IM_COL32(14,22,20,255), IM_COL32(225,245,240,255), IM_COL32(60,200,160,255),  IM_COL32(8,12,10,255),
      IM_COL32(20,38,32,255), IM_COL32(26,48,40,255),    IM_COL32(32,58,48,255),
      IM_COL32(70,210,170,235),IM_COL32(120,230,200,40), IM_COL32(130,240,210,255) }, // Teal
    { IM_COL32(26,18,8,255),  IM_COL32(255,240,210,255), IM_COL32(255,170,40,255),  IM_COL32(12,8,4,255),
      IM_COL32(46,28,12,255), IM_COL32(58,34,16,255),    IM_COL32(72,42,20,255),
      IM_COL32(255,180,60,235),IM_COL32(255,200,110,40), IM_COL32(255,210,130,255) }, // Âmbar
    { IM_COL32(10,12,22,255), IM_COL32(220,230,255,255), IM_COL32(80,110,255,255),  IM_COL32(6,8,16,255),
      IM_COL32(18,24,46,255), IM_COL32(24,32,58,255),    IM_COL32(30,40,72,255),
      IM_COL32(100,130,255,235),IM_COL32(120,150,255,40),IM_COL32(130,160,255,255) }, // Azul elétrico
    { IM_COL32(240,240,245,255), IM_COL32(24,24,26,255), IM_COL32(80,110,255,255),  IM_COL32(245,245,248,255),
      IM_COL32(230,230,235,255), IM_COL32(220,220,230,255), IM_COL32(210,210,225,255),
      IM_COL32(100,130,255,235), IM_COL32(160,180,255,40),  IM_COL32(120,150,255,255) } // Clean
};

static const char* THEME_NAMES[15] = {
    "LARANJA","AZUL","ROXO","VERDE","VERMELHO","CIANO","ROSA NEON","DOURADO","CINZA",
    "LIMA","MAGENTA","TEAL","ÂMBAR","AZUL ELÉTRICO","CLEAN"
};

static int   gThemeIndex = 0;
static Theme gTheme      = THEMES[0];
static ImU32 gBgColorU32 = IM_COL32(0,0,0,255);

static inline void ApplyTheme(int idx){
    if (idx<0 || idx>14) idx=0;
    gThemeIndex=idx; gTheme=THEMES[idx];
    ImGuiStyle& st=ImGui::GetStyle();
    st.Colors[ImGuiCol_Text]         = ImGui::ColorConvertU32ToFloat4(gTheme.text);
    ImVec4 t = st.Colors[ImGuiCol_Text];
    st.Colors[ImGuiCol_TextDisabled] = ImVec4(t.x,t.y,t.z,0.62f);
    st.Colors[ImGuiCol_Button]        = ImGui::ColorConvertU32ToFloat4(gTheme.btn);
    st.Colors[ImGuiCol_ButtonHovered] = ImGui::ColorConvertU32ToFloat4(gTheme.btnHover);
    st.Colors[ImGuiCol_ButtonActive]  = ImGui::ColorConvertU32ToFloat4(gTheme.btnActive);
    st.Colors[ImGuiCol_SliderGrab]       = ImGui::ColorConvertU32ToFloat4(gTheme.btnHover);
    st.Colors[ImGuiCol_SliderGrabActive] = ImGui::ColorConvertU32ToFloat4(gTheme.btnActive);
    st.Colors[ImGuiCol_CheckMark]        = ImGui::ColorConvertU32ToFloat4(gTheme.btnActive);

    st.ScrollbarSize = 22.0f;
    st.Colors[ImGuiCol_ScrollbarBg]          = ImGui::ColorConvertU32ToFloat4(IM_COL32(40,0,0,140));
    st.Colors[ImGuiCol_ScrollbarGrab]        = ImGui::ColorConvertU32ToFloat4(IM_COL32(255,60,60,255));
    st.Colors[ImGuiCol_ScrollbarGrabHovered] = ImGui::ColorConvertU32ToFloat4(IM_COL32(255,80,80,255));
    st.Colors[ImGuiCol_ScrollbarGrabActive]  = ImGui::ColorConvertU32ToFloat4(IM_COL32(255,100,100,255));
}

// ===== partículas neon =====
struct P { ImVec2 p,v; float r; ImU32 c; };
static void DrawParticlesNeonInRect(const ImVec2& rect_min, const ImVec2& rect_max){
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 sz = ImVec2(rect_max.x-rect_min.x, rect_max.y-rect_min.y);
    if (sz.x<=0 || sz.y<=0) return;
    dl->PushClipRect(rect_min, rect_max, true);

    static std::vector<P> pts; static bool init=false;
    if (!init){ init=true; pts.resize(90);
        for (auto &e:pts){ e.p=ImVec2((float)(rand()%1000)/1000.f*sz.x, (float)(rand()%1000)/1000.f*sz.y);
            e.v=ImVec2(((rand()%200)-100)*0.04f, ((rand()%200)-100)*0.04f);
            e.r=2.0f+(rand()%100)/70.0f; e.c=gTheme.particleCore; }
    } else { for (auto &e:pts) e.c=gTheme.particleCore; }

    for (auto &e:pts){
        e.p.x+=e.v.x; e.p.y+=e.v.y;
        if (e.p.x<-10) e.p.x=sz.x+10; if (e.p.y<-10) e.p.y=sz.y+10;
        if (e.p.x>sz.x+10) e.p.x=-10; if (e.p.y>sz.y+10) e.p.y=-10;
        dl->AddCircleFilled(ImVec2(rect_min.x+e.p.x, rect_min.y+e.p.y), e.r*1.8f, gTheme.particleGlow, 12);
        dl->AddCircleFilled(ImVec2(rect_min.x+e.p.x, rect_min.y+e.p.y), e.r, e.c, 12);
    }

    float maxDist=140.0f;
    for (size_t i=0;i<pts.size();++i) for (size_t j=i+1;j<pts.size();++j){
        ImVec2 d=ImVec2(pts[i].p.x-pts[j].p.x, pts[i].p.y-pts[j].p.y);
        float dist2=d.x*d.x+d.y*d.y; if (dist2<maxDist*maxDist){
            float a=1.0f-(sqrtf(dist2)/maxDist); ImU32 lb=gTheme.lineBase;
            ImU32 col=IM_COL32((lb>>IM_COL32_R_SHIFT)&0xFF,(lb>>IM_COL32_G_SHIFT)&0xFF,(lb>>IM_COL32_B_SHIFT)&0xFF,(int)(a*220));
            dl->AddLine(ImVec2(rect_min.x+pts[i].p.x, rect_min.y+pts[i].p.y),
                        ImVec2(rect_min.x+pts[j].p.x, rect_min.y+pts[j].p.y), col, 1.2f);
        }
    }
    dl->PopClipRect();
}

// ---- Botões
static bool LabeledButton(const char* id, const char* label, const ImVec2& size, ImU32 textColor){
    ImDrawList* dl=ImGui::GetWindowDrawList();
    ImVec2 pos=ImGui::GetCursorScreenPos();
    bool pressed=ImGui::Button(id, size);
    ImGuiIO& io=ImGui::GetIO();
    ImFont* font=(io.Fonts->Fonts.size()>1 && io.Fonts->Fonts[1])? io.Fonts->Fonts[1]: ImGui::GetFont();
    float fs=ImGui::GetFontSize();
    ImVec2 tsize=font->CalcTextSizeA(fs, FLT_MAX, 0.f, label);
    ImVec2 center=ImVec2(pos.x+size.x*0.5f, pos.y+size.y*0.5f);
    ImVec2 textPos=ImVec2(center.x-tsize.x*0.5f, center.y-tsize.y*0.5f);
    dl->AddText(font, fs, textPos, textColor, label);
    return pressed;
}

static bool LeftLabelButton(const char* id, const char* label, const ImVec2& size, bool stripeActive){
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImGui::PushStyleColor(ImGuiCol_Button,        ImGui::ColorConvertU32ToFloat4(gTheme.btn));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::ColorConvertU32ToFloat4(gTheme.btnHover));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImGui::ColorConvertU32ToFloat4(gTheme.btnActive));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

    ImVec2 p = ImGui::GetCursorScreenPos();
    bool pressed = ImGui::Button(id, size);

    ImGuiIO& io = ImGui::GetIO();
    ImFont*  f  = (io.Fonts->Fonts.size()>1 && io.Fonts->Fonts[1]) ? io.Fonts->Fonts[1] : ImGui::GetFont();
    float fs = ImGui::GetFontSize();
    const float LEFT_PAD = 18.0f;
    float ty = p.y + (size.y - fs) * 0.5f - 1.0f;
    draw->AddText(f, fs, ImVec2(p.x + LEFT_PAD, ty), gTheme.text, label);

    if (stripeActive){
        draw->AddRectFilled(ImVec2(p.x - 6.0f, p.y),
                            ImVec2(p.x,        p.y + size.y),
                            gTheme.stripe, 6.0f, ImDrawFlags_RoundCornersLeft);
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    return pressed;
}

// ====== ESP STATE (inline, sem redefinition) ======
namespace ESPCFG {
inline ImU32 ColorU32    = IM_COL32(57,255,20,255); // padrão: verde neon
inline int   PosMode     = 1;                       // 0=Topo, 1=Meio, 2=Baixo
inline float ExtraOffset = 0.0f;                    // px (+baixo / -cima)
}

// Getters/Setters (use na renderização da ESP)
static inline ImU32 Esp_GetColorU32() { return ESPCFG::ColorU32; }
static inline float Esp_GetYOffset(float screenH) {
    const float anchor = (ESPCFG::PosMode==0 ? screenH*0.25f
                         : ESPCFG::PosMode==2 ? screenH*0.75f
                                              : screenH*0.50f);
    return (anchor - screenH*0.5f) + ESPCFG::ExtraOffset;
}
static inline void Esp_SetColorU32(ImU32 c)                 { ESPCFG::ColorU32 = c; }
static inline void Esp_SetColorRGBA(int r,int g,int b,int a=255) { ESPCFG::ColorU32 = IM_COL32(r,g,b,a); }
static inline void Esp_SetPosMode(int mode)                 { ESPCFG::PosMode = (mode<0||mode>2)?1:mode; }
static inline void Esp_SetYOffset(float px)                  { ESPCFG::ExtraOffset = px; }


// ============================ MENU ============================

void menu() {
    UpdateThreeFinger(g_tfState, g_hideUI);
    if (g_hideUI) return;

    static bool once=false; if(!once){ ApplyTheme(gThemeIndex); once=true; }

    // Botão flutuante
    {
        ImGui::SetNextWindowSize(SZ(180,180), ImGuiCond_Always);
        ImGui::SetNextWindowPos(uiButtonPos, ImGuiCond_Always);
        ImGui::Begin(OBFUSCATE("versaiiiofull"), nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoBackground);

        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) uiDragging = true;
        if (uiDragging && ImGui::IsMouseDown(0)){
            ImVec2 mp = ImGui::GetMousePos();
            uiButtonPos = ImVec2(mp.x - 90.f, mp.y - 90.f);
        }
        if (ImGui::IsMouseReleased(0)) uiDragging = false;

        static float clickStartTime = -1.0f;
        float rect_h=52.0f, circle_r=34.0f;
        if (ImGui::IsWindowHovered()){
            if (ImGui::IsMouseClicked(0)) clickStartTime = ImGui::GetTime();
            else if (ImGui::IsMouseReleased(0)){
                if (clickStartTime!=-1.0f && (ImGui::GetTime()-clickStartTime<=0.5f)) gspika=!gspika;
                clickStartTime=-1.0f;
            }
        }

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 img_pos  = ImVec2(pos.x + 8.0f, pos.y + rect_h/2.0f - circle_r);
        ImVec2 img_size = ImVec2(img_pos.x + circle_r*2, pos.y + rect_h/2.0f + circle_r);
        dl->AddImage(logo.textureId, img_pos, img_size);
        ImGui::Dummy(ImVec2(circle_r*2 + 16.0f, rect_h));
        ImGui::End();
    }

    if (gspika){
        static int activeTab = 0;
        ImGui::SetNextWindowSize(SZ(920,570), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(gBgColorU32));

        if (ImGui::Begin(OBFUSCATE("##aaa"), nullptr,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar))
        {
            ImDrawList* draw = ImGui::GetWindowDrawList();
            ImVec2 wp = ImGui::GetWindowPos(), ws = ImGui::GetWindowSize();

            DrawParticlesNeonInRect(wp, ImVec2(wp.x+ws.x, wp.y+ws.y));

            float barH=60.0f;
            draw->AddRectFilled(wp, ImVec2(wp.x+ws.x, wp.y+barH), gTheme.topBar, 18.0f, ImDrawFlags_RoundCornersTop);

            const char* title="            FFH4X  BRUXO CHEATS - APK MOD   ";
            ImGuiIO& io = ImGui::GetIO(); ImFont* font = io.Fonts->Fonts[1];
            float tSize=31.0f;
            ImVec2 t = font->CalcTextSizeA(tSize, FLT_MAX, 0.0f, title);
            ImVec2 tPos = ImVec2(wp.x+(ws.x-t.x)*0.5f, wp.y+(barH-t.y)*0.5f);
            draw->AddText(font, tSize, tPos, IM_COL32(255,255,255,255), title);

            // --------- Coluna esquerda ----------
            const float tabLeft = 24.0f;
            ImVec2 btnSize = SZ(180,56);

            ImGui::SetCursorPosX(tabLeft);
            if (LeftLabelButton("##tab_aimbot","AIMBOT", btnSize, activeTab==0)) activeTab=0;

            ImGui::SetCursorPosX(tabLeft);
            if (LeftLabelButton("##tab_esp","VISUAL", btnSize, activeTab==1)) activeTab=1;

			ImGui::SetCursorPosX(tabLeft);
            if (LeftLabelButton("##tab_extra","EXTRA", btnSize, activeTab==2)) activeTab=2;

			    ImGui::SetCursorPosX(tabLeft);
            if (LeftLabelButton("##tab_info","INFO", btnSize, activeTab==3)) activeTab=3;

            
            ImGui::Dummy(SZ(0,6));
            ImGui::SetCursorPosX(tabLeft);
            if (LeftLabelButton("##tema_btn","TEMA", SZ(180,34), false)) ImGui::OpenPopup(OBFUSCATE("##popup_tema"));
            if (ImGui::BeginPopup(OBFUSCATE("##popup_tema"))){
                for (int i=0;i<15;++i){ bool sel=(gThemeIndex==i);
                    if (ImGui::Selectable(THEME_NAMES[i], sel)) { gThemeIndex=i; ApplyTheme(i); }
                }
                ImGui::EndPopup();
            }

            ImGui::Dummy(SZ(0,8));
            ImGui::SetCursorPosX(tabLeft);
            if (LeftLabelButton("##hide_btn","BYPASS ADM", SZ(180,34), false)) g_hideUI = true;

            // --------- Conteúdo ----------
            ImGui::Dummy(ImVec2(0, barH + 4));
            ImGuiIO& io2 = ImGui::GetIO(); ImGui::PushFont(io2.Fonts->Fonts[1]);
            ImGui::SetCursorPos(ImVec2(20, 520)); ImGui::Text(OBFUSCATE("")); ImGui::PopFont();

            ImVec2 pos = ImGui::GetCursorScreenPos(); pos.x += 170; pos.y -= 540;
            ImGui::SetCursorScreenPos(pos);

            ImGui::BeginChild(OBFUSCATE("MenuContent"), SZ(700,500), true,
                              ImGuiWindowFlags_AlwaysVerticalScrollbar);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(gTheme.childBg));

            if (activeTab==0){
                addSwitch("ATIVAR AIMBOT", nullptr, &AimbotEsp);
				ImGui::SameLine();
				addSwitch("EXIBIR FOV",    nullptr, &ShowFov);
                addSwitch("AIM TIRO",     nullptr, &aimbotAim);
                addSwitch("Aim Mira",     nullptr, &aimbotTrigger);
				addSwitch("Aim Visible",     nullptr, &AimVisible);
           //     addSwitch("AIMKILL",      nullptr, &AimKill);

                ImGui::SliderInt(OBFUSCATE("AIM FOV"), &Fov_Aim, 0.0f, 360.0f, "%.0f°",
                                   ImGuiSliderFlags_AlwaysClamp|ImGuiSliderFlags_NoInput);
                ImGui::SliderInt(OBFUSCATE("AIM DIS"), &Aimdis, 0.0f, 360.0f, "%.1f m",
                                   ImGuiSliderFlags_AlwaysClamp|ImGuiSliderFlags_NoInput);

static const char* aimItems[] = {
                    OBFUSCATE("Cabeça"),
                    OBFUSCATE("Pescoço"),
                    OBFUSCATE("Peito"),
                    OBFUSCATE("Quadril")
                };
                ImGui::SetNextItemWidth(250.0f);
                ImGui::Combo(OBFUSCATE("##aimpos"), &aimPosition, aimItems, IM_ARRAYSIZE(aimItems));

                ImGui::PopStyleColor(5);
                ImGui::Spacing();

                ImGui::EndChild();
                ImGui::SameLine();

                ImGui::BeginChild("RightPanel_Misc", ImVec2(320, 0), true);

            } else if (activeTab==1){
				addSwitch("ESP TARGET",     nullptr, &EspTarget);
                addSwitch("ESP LINHA",     nullptr, &EspLine);
                addSwitch("ESP BOX",    nullptr, &EspBox);
                addSwitch("ESP DISTANCIA", nullptr, &Distancia);
			    addSwitch("ESP NOME", nullptr, &Nick);
				addSwitch("ESP VIDA", nullptr, &EspVida);
                
               
            } else if (activeTab==2){
                addSwitch("RESET GUEST", nullptr, &ResetGuest1);
				addSwitch("SPEED TIMER", nullptr, &SpeedHack);
				addSwitch("GHOST HACK", nullptr, &GhostHack);
				ImGui::Text("Desenvolvedor:");
                ImGui::BulletText("Voltrixff.wq");
				ImGui::BulletText("Voltrixff.wq");
				
				} else if (activeTab==3){
				ImGui::Text("Status: Online");
				
            }

            ImGui::Dummy(SZ(0, 140));
            ImGui::PopStyleColor(); // child bg
            ImGui::EndChild();
            ImGui::End();
        }
        ImGui::PopStyleColor(); // WindowBg
    }
}

#endif // LAYOUT_H_INCLUDED
