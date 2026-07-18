#pragma once
#include <stdint.h>

extern bool AimSilent;
extern bool AimbotRage;
extern bool AimSilent2;
extern bool AimBotLegit;
extern bool SilentAim;
extern bool eName;
extern bool Grendeline;
extern bool SilentKill;
extern bool eDistance;
extern bool SpeedHack;
extern bool GhostH;
extern bool EspGrenade;
extern bool Unlock120fps;
extern bool test666;
extern bool AimbotShoulder1;
extern ImVec4 espColorLine;
extern ImVec4 espColorBox;
extern int DrawScale;
extern bool TaTuPlayer;
extern bool PullEnemy;
extern bool TeleportMark1;
extern float TaTuValue;
extern int FlyCarValue;
extern bool SpinBot;
//extern bool AimSilent555;

bool ShowUpdate = false; // go to ImGui/Call_ImGui and search " UpdateExpireDate " if you want to change time
bool ShowKeySystem = false; 

#include "IMAGE/Logo.h" // if you want to change logo go https://t.me/p_diddy77/147
#include "IMAGE/Logo2.h" // if you want to change logo go https://t.me/p_diddy77/147
#include "IMAGE/Settings.h" // don't delete me
#include "IMAGE/GhostLogo.h" // don't delete me
#include "IMAGE/TatuLogo.h" // don't delete me

int32_t (*orig_ANativeWindow_getWidth)(ANativeWindow* window);
int32_t _ANativeWindow_getWidth(ANativeWindow* window) {
    screenWidth = orig_ANativeWindow_getWidth(window);
    return orig_ANativeWindow_getWidth(window);
}

int32_t (*orig_ANativeWindow_getHeight)(ANativeWindow* window);
int32_t _ANativeWindow_getHeight(ANativeWindow* window) {
    screenHeight = orig_ANativeWindow_getHeight(window);
    return orig_ANativeWindow_getHeight(window);
}

int32_t (*orig_AConfiguration_getDensity)(ANativeWindow* config);
int32_t _AConfiguration_getDensity(ANativeWindow* config) {
    density = orig_AConfiguration_getDensity(config);
    return orig_AConfiguration_getDensity(config);
}

#if defined(__aarch64__) 
    #define RETURN "CO 03 5F D6"
    #define NOP "1F 20 03 D5"
    #define TRUE "20 00 80 D2 CO 03 5F D6"
    #define FALSE "00 00 80 D2 CO 03 5F D6"
    auto window_name = OBFUSCATE("Zeri");
#else
    #define RETURN "1E FF 2F E1"
    #define NOP "00 F0 20 E3"
    #define TRUE "01 00 A0 E3 1E FF 2F E1"
    #define FALSE "00 00 A0 E3 1E FF 2F E1"
    auto window_name = OBFUSCATE("Zerri");
#endif

#include <list>
#include <vector>
#include <string.h>
#include <pthread.h>
#include <thread>
#include <cstring>
#include <iostream>
#include <chrono>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <dlfcn.h>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <unistd.h>
#include "Munodz/Munodz.hpp"
#include "ImGui/stb_image.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif

using namespace std::this_thread;
using namespace std::chrono;
static std::vector<OWM::Notification> g_Notifications;

void DrawESP(ImDrawList* draw, ImVec2 top, ImVec2 bottom, float health_val, float dist, const char* name) {
    // Agar koi bhi ESP toggle ON hai tabhi ye chalega
    if (aline || box1 || health || eName || eDistance || EspGrenade) {
        
        float height = bottom.y - top.y;
        float width = height / 2.0f;

        // 1. DRAW BOX
        if (box1) {
            draw->AddRect(ImVec2(top.x - width/2, top.y), ImVec2(top.x + width/2, bottom.y), IM_COL32(255, 255, 255, 255), 0.0f, 0, 1.5f);
        }

        // 2. DRAW LINE
        if (aline) {
            draw->AddLine(ImVec2(ImGui::GetIO().DisplaySize.x / 2, 0), top, IM_COL32(255, 0, 0, 255), 1.0f);
        }

        // 3. HEALTH BAR
        if (health) {
            float healthHeight = height * (health_val / 100.0f);
            draw->AddRectFilled(ImVec2(top.x - width/2 - 5, bottom.y - healthHeight), ImVec2(top.x - width/2 - 2, bottom.y), IM_COL32(0, 255, 0, 255));
        }

        // 4. NAME & DISTANCE
        char info[128];
        if (eName && eDistance) {
            sprintf(info, "%s [%.0fm]", name, dist);
            draw->AddText(ImVec2(top.x - width/2, top.y - 15), IM_COL32(255, 255, 0, 255), info);
        } else if (eName) {
            draw->AddText(ImVec2(top.x - width/2, top.y - 15), IM_COL32(255, 255, 0, 255), name);
        }
    }
}


static bool IsMenuVisible = false; 
static int TabMenu = 1;

//if (aline  box1  health  eName  eDistance|| EspGrenade AimKill camxa TeleportMark1) {

void DrawFloatingButton(float s, ImGuiIO& io) {
    ImGui::SetNextWindowSize(ImVec2(80 * s, 80 * s));
    ImGui::SetNextWindowPos(ImVec2(50 * s, 150 * s), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("##Float", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar)) {
        ImDrawList* fDraw = ImGui::GetWindowDrawList();
        ImVec2 fPos = ImGui::GetWindowPos();
        ImVec2 center = ImVec2(fPos.x + 40 * s, fPos.y + 40 * s);

        // Click Logic with Smooth Fix
        if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(0)) {
            if (io.MouseDragMaxDistanceSqr[0] < 5.0f) { 
                IsMenuVisible = !IsMenuVisible;
            }
        }

        // Blue Theme Design
        fDraw->AddCircleFilled(center, 30 * s, IM_COL32(0, 0, 0, 200)); 
        fDraw->AddCircle(center, 30 * s, IM_COL32(0, 150, 255, 255), 0, 3.0f * s); 
        
        const char* btnText = IsMenuVisible ? "CLOSE" : "OPEN";
        ImVec2 txtSz = ImGui::CalcTextSize(btnText);
        fDraw->AddText(ImVec2(center.x - txtSz.x/2, center.y - txtSz.y/2), IM_COL32(0, 150, 255, 255), btnText);
    }
    ImGui::End();
}



// --- Particle Structure for Spider Web Effect ---
struct MenuParticle {
    ImVec2 pos;
    ImVec2 vel;
};
static std::vector<MenuParticle> particles;
static bool particles_initialized = false;

struct TextureInfo {
    ImTextureID textureId = nullptr;
    int w = 0;
    int h = 0;
};

void DrawImage(int x, int y, int w, int h, ImTextureID Texture) {
    ImGui::GetForegroundDrawList()->AddImage(Texture, ImVec2(x, y), ImVec2(x + w, y + h));
}

/*struct TextureInfo {
    ImTextureID textureId = nullptr;
    int w = 0;
    int h = 0;
};

void DrawImage(int x, int y, int w, int h, ImTextureID Texture) {
    ImGui::GetForegroundDrawList()->AddImage(Texture, ImVec2(x, y), ImVec2(x + w, y + h));
}

TextureInfo CreateTexture(const unsigned char* buf, int len) {
    TextureInfo image;
    int n = 0;
    unsigned char* image_data = stbi_load_from_memory(buf, len, &image.w, &image.h, &n, 4);
    if (!image_data) {
        return {};
    }*/

    GLuint image_texture;
  /*  glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.w, image.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    image.textureId = (ImTextureID)(intptr_t)image_texture;
    return image;*/
//}

TextureInfo LogoTexture;
TextureInfo Logo2Texture; 
TextureInfo SettingsTexture;
TextureInfo GhostLogoTexture; 
TextureInfo TatuLogoTexture;

void InitTextures() {
  /*  if (sizeof(Logo_data) > 0) {
        LogoTexture = CreateTexture(Logo_data, sizeof(Logo_data));
    }
    if (sizeof(Logo2_data) > 0) {
        Logo2Texture = CreateTexture(Logo2_data, sizeof(Logo2_data));
    }
    if (sizeof(Settings_data) > 0) {
        SettingsTexture = CreateTexture(Settings_data, sizeof(Settings_data));
    }
    if (sizeof(GhostLogo_data) > 0) {
        GhostLogoTexture = CreateTexture(GhostLogo_data, sizeof(GhostLogo_data));
    }
    if (sizeof(TatuLogo_data) > 0) {
        TatuLogoTexture = CreateTexture(TatuLogo_data, sizeof(TatuLogo_data));
    }*/
}

void write_file (std::string path,std::string str) {
  std::ofstream myfile;
  myfile.open (path);
  myfile << str;
  myfile.close();
}

std::string float_to_string (float value) {
    std::string str; 
    str = std::to_string(value);
    return str;
}

char string_to_char(std::string str) {
    std::string convert = str;
    int get_len = convert.length();
    char char_array[get_len + 1];
    strcpy(char_array, convert.c_str());
    for (int i = 0; i < get_len; i++)
    return char_array[i];
}

std::string int_to_string2(int num) {
     std::string str = to_string(num);
     return str;
}

std::string getCurrentTime() {
    time_t now = time(nullptr);
    tm *ltm = localtime(&now);
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
    ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    return buffer;
}

void Concacvip1() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    InitTextures();
    KlelYuimimguivip1();
}

bool func_ghost = false;
bool Showghost = false;
bool M4A1 = false;
static bool ConstructWindows = false;
bool ShowMenu = false;
//int TabMenu = 1;
//static bool HideIcon;
//bool itsmk = true; 



//static bool IsMenuVisible = true;
static bool showSettings = false;
static bool showGhostFloat = false; 
static bool showTatuFloat = false;
static bool HideIcon;
bool itsmk = true; 

static bool EnableFunctions = false;

bool showteleportbr = false;
bool showteleportcs = false;
bool showfastdash = false;
bool showteleportmark = false;
static bool particles_init = false;

static bool isMenuCollapsed = false;

void DrawMenuornigaButton() {
    float s = ImGuiX::GScale;

  /*  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f * s, 8.0f * s)); 
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f * s); 
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0)); 
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0)); 

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.04f, 0.04f, 0.98f)); 
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.25f)); 

    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("##FloatingToggle", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        
        ImGui::SetWindowFontScale(s * 0.85f); 

        float logoH = 45.0f * s; 
        float logoW = 45.0f * s; 
        
        ImVec2 pStart = ImGui::GetCursorScreenPos();
        
        if (Logo2Texture.textureId) {
            ImGui::Image(Logo2Texture.textureId, ImVec2(logoW, logoH));
        } else if (LogoTexture.textureId) {
            ImGui::Image(LogoTexture.textureId, ImVec2(logoW, logoH));
        } else {
            ImGui::Dummy(ImVec2(logoW, logoH)); 
        }
        
        float textHeight = ImGui::GetTextLineHeight();
        float centerY = pStart.y + (logoH / 2.0f) - (textHeight / 2.0f);

        float spacing = 15.0f * s;
        float currentX = pStart.x + logoW + spacing;

        ImGui::SetCursorScreenPos(ImVec2(currentX, centerY));
        ImGui::TextColored(ImVec4(0.835f, 0.0f, 0.976f, 1.0f), "DRIP"); // change your name here
        
        float dripWidth = ImGui::CalcTextSize("DRIP").x; // change your name here
        float spacingText = 5.0f * s; 
        float nextX = currentX + dripWidth + spacingText;

        ImGui::SetCursorScreenPos(ImVec2(nextX, centerY));
        ImGui::Text("CLIENT | APKMOD"); // change your name here

        ImGui::SetCursorScreenPos(ImVec2(pStart.x, pStart.y + logoH));
        
        if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(0) && !ImGui::IsMouseDragging(0)) {
            IsMenuVisible = !IsMenuVisible;*/
        }
   /* }
    ImGui::End();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(5);
}*/

void DrawHitlerButton(const char* labelId, bool* state, float startX, float startY, ImTextureID customTex) {
    float s = ImGuiX::GScale;
   /* 
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f * s, 4.0f * s)); 
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f * s);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.05f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));

    ImGui::SetNextWindowPos(ImVec2(startX, startY), ImGuiCond_FirstUseEver);

    if (ImGui::Begin(labelId, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        
        float size = 175.0f * s; 
        
        ImVec4 tintColor = *state ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : ImVec4(1.0f, 0.2f, 0.2f, 1.0f);

        ImTextureID tex = customTex ? customTex : LogoTexture.textureId; 
        
        if (tex) {
            ImGui::Image(tex, ImVec2(size, size), ImVec2(0,0), ImVec2(1,1), tintColor);
        } else {
            ImGui::Dummy(ImVec2(size, size));
            ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 
                *state ? IM_COL32(50, 255, 50, 200) : IM_COL32(255, 50, 50, 200), 4.0f*s);
        }

        if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(0) && !ImGui::IsMouseDragging(0)) {
            *state = !(*state);*/
        }
    /*}
    ImGui::End();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
}*/

void DrawMenuHeader() {}

void KlelYuim() {
    float s = ImGuiX::GScale; 
    ImGuiIO& io = ImGui::GetIO();

    // Floating Button hamesha visible rahega
    DrawFloatingButton(s, io);

    if (!particles_init) {
        particles.clear();
        for (int i = 0; i < 45; i++) {
            particles.push_back({ImVec2((float)(rand() % 620), (float)(rand() % 480)), ImVec2((float)(rand() % 20 - 10) * 0.2f, (float)(rand() % 20 - 10) * 0.2f)});
        }
        particles_init = true;
    }

    if (!IsMenuVisible) return;

    // --- BLUE THEME SETUP ---
    ImGui::SetNextWindowSize(ImVec2(620 * s, 480 * s), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0, 0.6f, 1, 1));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0, 0.5f, 1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f * s);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 12.0f * s);

    if (ImGui::Begin("GUJJU MODS", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 m_pos = ImGui::GetWindowPos();
        ImVec2 m_size = ImGui::GetWindowSize();

        // Background Spider Web Particles
        for (auto& p : particles) {
            p.pos.x += p.vel.x; p.pos.y += p.vel.y;
            if (p.pos.x < 0 || p.pos.x > m_size.x) p.vel.x *= -1;
            if (p.pos.y < 0 || p.pos.y > m_size.y) p.vel.y *= -1;
            draw->AddCircleFilled(ImVec2(m_pos.x + p.pos.x, m_pos.y + p.pos.y), 1.2f * s, IM_COL32(0, 150, 255, 100));
        }

        // Header Text
        ImGui::SetCursorPos(ImVec2(20 * s, 18 * s));
        ImGui::TextColored(ImVec4(0, 0.6f, 1, 1), "=== GUJJU MODS V102 ===");

        // --- TABS SECTION ---
        float tw = 110 * s; ImGui::SetCursorPos(ImVec2(15 * s, 60 * s));
        auto TabBtn = [&](const char* name, int id) {
            if (TabMenu == id) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0.4f, 0.8f, 1.0f));
            else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
            if (ImGui::Button(name, ImVec2(tw, 40 * s))) TabMenu = id;
            ImGui::PopStyleColor();
        };
        TabBtn("[ AIM ]", 1); ImGui::SameLine();
        TabBtn("[ VISUAL ]", 2); ImGui::SameLine();
        TabBtn("[ BRUTAL ]", 3); ImGui::SameLine();
        TabBtn("[ FLY ]", 4); ImGui::SameLine();
        TabBtn("[ INFO ]", 5);

        ImGui::Separator();

        // --- SCROLLING CONTENT AREA ---
        ImGui::SetCursorPos(ImVec2(20 * s, 115 * s));
        if (ImGui::BeginChild("ScrollArea", ImVec2(580 * s, 345 * s), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            
            if (TabMenu == 1) { // AIMBOT
                ImGui::Checkbox("AIMBOT RAGE", &Aimbot);
                ImGui::Checkbox("AIMBOT LEGIT", &AimBotLegit);
                ImGui::Checkbox("AIM MAGNET", &PullEnemy);
                ImGui::Checkbox("SILENT AIM", &SilentAim);
              //  ImGui::Checkbox("AIMKILL", &AimKill);
                ImGui::SliderInt("FOV RANGE", &DrawScale, 0, 360);
            } 
            else if (TabMenu == 2) { // VISUALS
                ImGui::Checkbox("ENABLE NAME ESP", &eName);
                ImGui::Checkbox("DRAW LINE", &aline);
                ImGui::Checkbox("DRAW BOX", &box1);
                ImGui::Checkbox("HEALTH BAR", &health);
                ImGui::Checkbox("DISTANCE ESP", &eDistance);
             //   ImGui::SliderFloat("Spin Speed", &SpinSpeed, 30.0f, 1080.0f);
            } 
            else if (TabMenu == 3) { // BRUTAL
                ImGui::Checkbox("SPEED HACK", &SpeedHack);
            //    ImGui::Checkbox("FAST SWITCH", &FastSwitch);
                ImGui::Checkbox("TP TO MARK", &TeleportMark1);
                ImGui::Checkbox("120FPS UNLOCK", &Unlock120fps);
                ImGui::Checkbox("GHOST PLAYER", &GhostH);
                ImGui::Checkbox("SPIN BOT", &SpinBot);
                ImGui::Checkbox("FUCK GUEST", &Guest);
            } 
            else if (TabMenu == 4) { // FLY
          //      ImGui::Checkbox("BYPASS FLY", &bypass_india);
        //        ImGui::Checkbox("HIGH JUMP", &JumpHight);
            //    ImGui::SliderFloat("Jump Height", &JumpHeightx, 0.0f, 5.0f);
             //   ImGui::Checkbox("Camera View", &camxa);
                ImGui::Checkbox("Teleport", &TaTuPlayer);
                ImGui::SliderFloat("Teleport Velue", &TaTuValue, 4.0f, 20.0f);
                ImGui::SliderInt("Fly Speed", &FlyCarValue,  0, 2);
                ImGui::Checkbox("Fly Up", &FlyUpBR);
                ImGui::SliderInt("Fly Height", &flysaved.FlyBR, 0, 100); // 0 min hai, 100 max
                ImGui::SliderInt("Fly Speed", &flysaved.FlySpeedBR, 0, 50); // 0 min hai, 50 max
            }
            else if (TabMenu == 5) { // INFO
                ImGui::Text("OWNER: GUJJU MODS");
                ImGui::Text("TELEGRAM: @GUJJU_MODS");
                ImGui::Spacing();
                ImGui::TextDisabled("// CREDIT:- GUJJU MODS\n// DON'T REMOVE CREDIT");
            }
        }
        ImGui::EndChild();

        // Outer Glow Border
        draw->AddRect(m_pos, ImVec2(m_pos.x + m_size.x, m_pos.y + m_size.y), IM_COL32(0, 150, 255, 255), 12.0f * s, 0, 2.5f * s);
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

enum TouchType {
    Direct = 0,
    Indirect = 1,
    Stylus = 2
};

struct Touch {
    int m_FingerId; 
    Vector2 m_Position; 
    Vector2 m_RawPosition; 
    Vector2 m_PositionDelta; 
    float m_TimeDelta; 
    int m_TapCount; 
    TouchPhase m_Phase; 
    TouchType m_Type; 
    float m_Pressure; 
    float m_maximumPossiblePressure; 
    float m_Radius; 
    float m_RadiusVariance; 
    float m_AltitudeAngle; 
    float m_AzimuthAngle;
};

std::vector<TouchPhase> touch_phases;
static bool should_clear_mouse_pos = false;
static ImVec2 touch_pos;

Touch (*old_GetTouch)(...);
Touch GetTouch(int index) {
    Touch touch = old_GetTouch(index);
    if (index == 0) {
        if (ImGui::GetCurrentContext() == nullptr) {
            return touch;
        }
        ImGuiIO* io = &ImGui::GetIO();
        touch_pos = ImVec2(touch.m_Position.X, io->DisplaySize.y - touch.m_Position.Y);
        TouchPhase currentPhase = touch.m_Phase;
        io->MousePos = touch_pos;
        switch (currentPhase) {
            case TouchPhase::Began:
            case TouchPhase::Ended:
            case TouchPhase::Canceled: {
                touch_phases.push_back(currentPhase);
                break;
            }
        }
    }
    return touch;
}

bool clearMousePos = true;

inline EGLBoolean (*orig_eglSwapBuffers1)(EGLDisplay dpy, EGLSurface surface);
inline EGLBoolean _eglSwapBuffers1(EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &glWidth);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &glHeight);
    
    if (glWidth <= 0 || glHeight <= 0) {
        return eglSwapBuffers(dpy, surface);
    }
    
    if (!ConstructWindows) {
        Concacvip1();
       // ImGuiX::SetupDripStyle(3.5f);
        ConstructWindows = true;
    }
    ImGuiIO &io = ImGui::GetIO();

    if (!touch_phases.empty()) {
        auto phase = touch_phases.front();
        switch (phase) {
            case TouchPhase::Began:
                io.MouseDown[0] = true;
                break;
            case TouchPhase::Ended:
            case TouchPhase::Canceled:
                io.MouseDown[0] = false;
                should_clear_mouse_pos = true;
                break;
        }
        touch_phases.erase(touch_phases.begin());
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(glWidth, glHeight);
    ImGui::NewFrame();
    
    if (true) { 
        if (showteleportbr) {
            ImGui::SetNextWindowPos(ImVec2(300, 100), ImGuiCond_FirstUseEver); 
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));  
            ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.0f, 0.0f));  

            ImGui::Begin(OBFUSCATE("Teleport (BR)"), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize); 
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                ImGuiX::Switch(OBFUSCATE("Teleport (BR)"), &TeleportProBr);
                ImGui::PopStyleColor();
            }
            ImGui::End();
            ImGui::PopStyleColor(2); 
            ImGui::PopStyleVar(3);  
        }
        
        if (showteleportcs) {
            ImGui::SetNextWindowPos(ImVec2(300, 180), ImGuiCond_FirstUseEver); 
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.0f, 0.0f));

            ImGui::Begin(OBFUSCATE("Teleport (CS)"), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize); 
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                ImGuiX::Switch(OBFUSCATE("Teleport (CS)"), &TeleportProCs);
                ImGui::PopStyleColor();
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(3);  
        }
        
        if (showteleportmark) {
            ImGui::SetNextWindowPos(ImVec2(300, 180), ImGuiCond_FirstUseEver); 
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.0f, 0.0f));

            ImGui::Begin(OBFUSCATE("Mark"), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize); 
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                ImGuiX::Switch(OBFUSCATE("Mark"), &TeleportMark1);
                ImGui::PopStyleColor();
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(3);  
        }
        
        if (showfastdash) {
                ImGui::SetNextWindowPos(ImVec2(300, 180), ImGuiCond_FirstUseEver); 
                
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.0f, 0.0f));

                ImGui::Begin(OBFUSCATE("fast dash"), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize); 
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                    ImGuiX::Switch(OBFUSCATE("fast dash"), &fastdash);
                    ImGui::PopStyleColor();
                }
                ImGui::End();
                
                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(3);  
                }
        
        KlelYuim();
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return orig_eglSwapBuffers1(dpy, surface);
}
