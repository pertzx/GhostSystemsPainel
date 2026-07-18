#include <android/log.h>
#include <unistd.h>
#include <thread>
#include <limits>
#include <KittyMemory/KittyMemory.h>
#include <KittyMemory/MemoryPatch.h>
#include <KittyMemory/KittyScanner.h>
#include <KittyMemory/KittyUtils.h>
#include "oxorany/oxorany.h"
#include <xdl.h>
#include <KittyUtils.h>
#include <KittyMemory.h>
#include <Il2Cpp.h>
#include <SubstrateHook.h>
#include <CydiaSubstrate.h>
#include "AxlMods/gui.hpp"
//#include "fonts/FontAwesome6_solid.h"
#include "AXL.MODS.hpp"
#include <ImguiPP.h>
#include <Themes.h>
//#include "Firewall.h"
#include "exptime.h"
#include "AXL.MODS.h"
#include "Vampire_Wars_Italic.h"
#include "Chams.h"
#include "hacker_tab_button.h"
#include "Icon.h"
#include "autofont.h"

bool bypass;
bool Combo;
bool selectedLang;
#include <fstream>

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

void hack();
void writeLog(const std::string& logMessage, const std::string& filename = "/storage/emulated/0/Android/data/com.dualspace.multispace.androidx/files/log.txt");
JavaVM *jvm;
JNIEnv *genv;


class MyModule : public zygisk::ModuleBase {
 public:
  void onLoad(Api *api, JNIEnv *env) override {
    this->api_ = api;
    this->env_ = env;
    genv = env;
   // gEnv = env;

  }

  void preAppSpecialize(AppSpecializeArgs *args) override {
    static constexpr const char *packages[] = {
        "com.dts.freefireth"
    };
    const char *process = env_->GetStringUTFChars(args->nice_name, nullptr);
    for (const auto *package: packages) {
      is_game_ = (strcmp(process, package) == 0);
      if (is_game_) {
        break;
      }
    }
    env_->ReleaseStringUTFChars(args->nice_name, process);
  }

  void postAppSpecialize(const AppSpecializeArgs *args) override {
    if (is_game_) {
       genv->GetJavaVM(&jvm);
     //  genv->GetJavaVM(&gJvm);
      std::thread{hack}.detach();
    }
  }

 private:
  Api *api_ = nullptr;
  JNIEnv *env_ = nullptr;
  bool is_game_ = false;
};


uintptr_t il2cpp_base = 0;
void *getRealAddr(ulong offset) {
    return reinterpret_cast<void*>(il2cpp_base + offset);
};


static int Tab = 1;
/*
std::string GetCurrentTime() {
    std::time_t currentTime = std::time(nullptr);
    std::tm* timeInfo = std::localtime(&currentTime);

    std::ostringstream oss;
    oss << std::put_time(timeInfo, "%H:%M:%S");
    return oss.str();
}*/
#include <cstdint>

#define HOOK(offset, target, name) target = (decltype(target))(baseAddress + offset)






extern "C" void OnLoad() {
    }
bool clearMousePos = true;
bool ImGuiOK = false;
bool initImGui = false;
void OpenURL(Il2CppString* url) {
    typedef void (*OpenURL_t)(Il2CppString*);
    static OpenURL_t openURL = nullptr;

    if (!openURL) {
        openURL = (OpenURL_t)Il2CppGetMethodOffset( 
            OBFUSCATE("UnityEngine.CoreModule.dll"),
            OBFUSCATE("UnityEngine"),
            OBFUSCATE("Application"),
            OBFUSCATE("OpenURL"),
            1
        );
    }

    if (openURL) {
        openURL(url);
    }
}
 
inline EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
inline EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    
        eglQuerySurface(dpy, surface, EGL_WIDTH, &g_GlWidth);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &g_GlHeight);
    

    if (!g_IsSetup) {
        prevWidth = g_GlWidth;
        prevHeight = g_GlHeight;
		
		
        SetupImgui();
        
        g_IsSetup = true;
    }
    
    

// Setup window size and style (call once before your UI)
//ImGui::SetNextWindowSize(ImVec2(600, 450), ImGuiCond_Always);
ImGuiIO& io = ImGui::GetIO();
ImGuiStyle& style = ImGui::GetStyle();
ImVec4* colors = style.Colors;
colors[ImGuiCol_WindowBg]           = ImColor(18, 18, 18, 255);
colors[ImGuiCol_Border]             = ImColor(60, 60, 60, 150);
colors[ImGuiCol_FrameBg]            = ImColor(28, 28, 28, 220);
colors[ImGuiCol_FrameBgHovered]     = ImColor(50, 50, 50, 255);
colors[ImGuiCol_FrameBgActive]      = ImColor(70, 70, 70, 255);
colors[ImGuiCol_TitleBg]            = ImColor(40, 40, 40, 255);
colors[ImGuiCol_TitleBgActive]      = ImColor(40, 40, 40, 255);
colors[ImGuiCol_Button]             = ImColor(100, 100, 100, 150);
colors[ImGuiCol_ButtonHovered]      = ImColor(40, 130, 255, 230);
colors[ImGuiCol_ButtonActive]       = ImColor(90, 90, 90, 255);
colors[ImGuiCol_CheckMark]          = ImColor(0, 180, 255, 255);
style.WindowRounding    = 8.0f;
style.FrameRounding     = 6.0f;
style.ScrollbarRounding = 8.0f;
 style.GrabRounding      = 5.0f;
style.FramePadding      = ImVec2(12, 8);
style.ItemSpacing       = ImVec2(12, 8);
style.WindowPadding     = ImVec2(14, 14);
style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
style.ButtonTextAlign = ImVec2(0.5f, 0.5f);



ImFontConfig font_cfg;
    font_cfg.SizePixels = 45.0f;
 //   io.Fonts->AddFontDefault(&font_cfg);

  //  ImGui_ImplOpenGL3_Init("#version 300 es");

  //  ImFontConfig font_cfg;
    font_cfg.SizePixels = 28.0f;
   // ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;

    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    static const ImWchar icons_ranges[] = {0xf000, 0xf3ff, 0};

    io.DisplaySize = ImVec2((float)g_GlWidth, (float)g_GlHeight);

 //   io.Fonts->AddFontFromMemoryTTF((void *)Custom_data, Custom_size, 30.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
  io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 30.0f, &icons_config, icons_ranges);
    // === UI Style Settings ===
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(g_GlWidth, g_GlHeight);
    ImGui::NewFrame();
	if (ImGuiOK) {
	    int touchCount = (((int (*)())(Class_Input__get_touchCount))());
    if (touchCount > 0) {
        UnityEngine_Touch_Fields touch = ((UnityEngine_Touch_Fields(*)(int))(Class_Input__GetTouch))(0);
        float reverseY = io.DisplaySize.y - touch.m_Position.fields.y;
        switch (touch.m_Phase) {
            case TouchPhase::Began:
            case TouchPhase::Stationary:
                io.MousePos = ImVec2(touch.m_Position.fields.x, reverseY);
                io.MouseDown[0] = true;
				UpdateSpeedhack();
                break;
            case TouchPhase::Ended:
            case TouchPhase::Canceled:
                io.MouseDown[0] = false;
                clearMousePos = true;
                break;
            case TouchPhase::Moved:
                io.MousePos = ImVec2(touch.m_Position.fields.x, reverseY);
                break;
            default:
                break;
        }
    }
	}

    ExpireDate exp;
    exp.setExpirationDate(11, 2, 2026);
    if (exp.isExpired()) {
        ImGui::SetNextWindowSize(ImVec2((float) g_GlWidth * 0.3f, (float) g_GlHeight * 0.2f), ImGuiCond_Once);
        ImGui::Begin(OBFUSCATE(" This Mode Expiry Join Telegram "), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text(OBFUSCATE("New Version Link"));
    OpenURL(Il2CppString::Create("https://t.me/axl_modsx"));
        ImGui::End();
       
    
}else{

static bool show;
DrawESP(g_GlWidth, g_GlHeight);
static int selectedLang = 0; 


    static bool showESP = true;
 static bool showAimbot = false;
    static int activeTab = 0;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f)); // Gray background for both BeginChild
    ImGui::SetNextWindowSize(ImVec2(650, 505), ImGuiCond_Always);
//	if (ImGui::Begin(ICON_FA_USER_CIRCLE " AXL MODS LODER NEW UI !!" , &WORTERSYPORNO, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus)) {
    ImGui::Begin(ICON_FA_USER_CIRCLE "  AXL MODS BY SRC OB52 ", nullptr, ImGuiWindowFlags_NoResize);

// White Laser Particle Network 
// Declare globally or at top of render function
static const int numParticles = 250;
static ImVec2 particlePositions[numParticles];
static ImVec2 particleVelocities[numParticles];
static bool initialized = false;

// Inside your render/draw function
ImDrawList* pDrawList = ImGui::GetWindowDrawList();
ImVec2 winPos = ImGui::GetWindowPos();
ImVec2 winSize = ImGui::GetWindowSize();
ImVec2 winMax = ImVec2(winPos.x + winSize.x, winPos.y + winSize.y);
ImVec2 cursorPos = ImGui::GetIO().MousePos;
float deltaTime = ImGui::GetIO().DeltaTime;

if (!initialized) {
    for (int i = 0; i < numParticles; ++i) {
        particlePositions[i] = ImVec2(
            winPos.x + winSize.x * (float(rand()) / RAND_MAX),
            winPos.y + winSize.y * (float(rand()) / RAND_MAX)
        );
        particleVelocities[i] = ImVec2(
            float((rand() % 100) - 50),
            float((rand() % 100) - 50)
        );
    }
    initialized = true;
}

for (int i = 0; i < numParticles; ++i) {
    for (int j = i + 1; j < numParticles; ++j) {
        ImVec2 diff = ImVec2(
            particlePositions[j].x - particlePositions[i].x,
            particlePositions[j].y - particlePositions[i].y
        );
        float dist = ImLengthSqr(diff);
        if (dist < 6400.0f) {
            float opacity = 1.0f - (dist / 6400.0f);
            ImU32 glow = IM_COL32(255, 255, 255, (int)(opacity * 50));
            pDrawList->AddLine(particlePositions[i], particlePositions[j], glow, 2.0f);
            pDrawList->AddLine(particlePositions[i], particlePositions[j], IM_COL32(255, 255, 255, 255), 1.2f);
        }
    }

    ImVec2 diffCursor = ImVec2(
        cursorPos.x - particlePositions[i].x,
        cursorPos.y - particlePositions[i].y
    );
    float distCursor = ImLengthSqr(diffCursor);
    if (distCursor < 6400.0f) {
        float opacity = 1.0f - (distCursor / 6400.0f);
        ImU32 glow = IM_COL32(255, 255, 255, (int)(opacity * 50));
        pDrawList->AddLine(cursorPos, particlePositions[i], glow, 2.0f);
        pDrawList->AddLine(cursorPos, particlePositions[i], IM_COL32(255, 255, 255, 255), 1.2f);
    }

    // Update particle position
    particlePositions[i].x += particleVelocities[i].x * deltaTime;
    particlePositions[i].y += particleVelocities[i].y * deltaTime;

    // Bounce on window edges
    if (particlePositions[i].x < winPos.x || particlePositions[i].x > winMax.x)
        particleVelocities[i].x *= -1;
    if (particlePositions[i].y < winPos.y || particlePositions[i].y > winMax.y)
        particleVelocities[i].y *= -1;

    // Draw particle
    ImU32 white = IM_COL32(255, 255, 255, 255);
    ImU32 soft = IM_COL32(255, 255, 255, 30);
    for (int g = 2; g > 0; --g)
        pDrawList->AddCircleFilled(particlePositions[i], 3.0f + g, soft);
    pDrawList->AddCircleFilled(particlePositions[i], 2.5f, white);
}
    // Left Sidebar style (Tabs as buttons)
    ImGui::BeginChild("Sidebar", ImVec2(150, 0), true);

    static int selectedTab = 0;
  

ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.039f, 0.039f, 0.039f, 1.0f));
ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.392f, 0.392f, 1.0f, 0.78f));
ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.5f);
ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
ImGui::PopStyleVar(2);
ImGui::PopStyleColor(2);
ImVec4 selectedColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
ImVec4 normalColor   = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); 

    ImGui::PushStyleColor(ImGuiCol_Text, selectedTab == 0 ? selectedColor : normalColor);
    if (ImGui::Button(ICON_FA_HOME" Aim", ImVec2(-1, 50)))
        selectedTab = 0;
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Text, selectedTab == 1 ? selectedColor : normalColor);
    if (ImGui::Button(ICON_FA_GLOBE" Esp", ImVec2(-1, 50)))
        selectedTab = 1;
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Text, selectedTab == 2 ? selectedColor : normalColor);
    if (ImGui::Button(ICON_FA_EXCLAMATION_TRIANGLE" Chms", ImVec2(-1, 50)))
        selectedTab = 2;
   ImGui::PopStyleColor();

       ImGui::PushStyleColor(ImGuiCol_Text, selectedTab == 3 ? selectedColor : normalColor);
    if (ImGui::Button(ICON_FA_PAPER_PLANE" Butl", ImVec2(-1, 50)))
        selectedTab = 3;
   ImGui::PopStyleColor();
   
    ImGui::PushStyleColor(ImGuiCol_Text, selectedTab == 4 ? selectedColor : normalColor);
    if (ImGui::Button(ICON_FA_BOLT" Run", ImVec2(-1, 50)))
        selectedTab = 4;
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Text, selectedTab == 5 ? selectedColor : normalColor);
    if (ImGui::Button(ICON_FA_EDIT" Clur", ImVec2(-1, 50)))
        selectedTab = 5;
   ImGui::PopStyleColor();
   ImGui::PushStyleColor(ImGuiCol_Text, selectedTab == 6 ? selectedColor : normalColor);
    if (ImGui::Button(ICON_FA_FILTER" Mode", ImVec2(-1, 50)))
        selectedTab = 6;
   ImGui::PopStyleColor();
    ImGui::EndChild();

    ImGui::SameLine();


    ImGui::BeginChild("Content", ImVec2(0, 0), true);

    if (selectedTab == 0) // General
    {
	         ImGui::Separator();
			 ImGui::Text("           [ AIM - MENU ]" );
			 ImGui::Separator();
			 				ImGui::Dummy(ImVec2(0, 12));
        DrawToggleSwitch(ICON_FA_CROSSHAIRS " Enable Aimbot", &Aimbot);
		ImGui::Dummy(ImVec2(0, 12));
		DrawToggleSwitch(ICON_FA_CROSSHAIRS " Enable Esp", &Enable);
        ImGui::Dummy(ImVec2(0, 12));
        DrawToggleSwitch(ICON_FA_CROSSHAIRS " AimVisible", &VisibltyDone);
		ImGui::Dummy(ImVec2(0, 12));
		DrawToggleSwitch(ICON_FA_CROSSHAIRS " Aim Maganet", &AimMagnet);
        ImGui::Dummy(ImVec2(0, 12));
        DrawToggleSwitch(ICON_FA_CROSSHAIRS " Aimsilent", &AimSilent);
		ImGui::SliderFloat(OBFUSCATE("Aim Fov"),&Fov_Aim,0.0f,890.0f,"%.f");
		        ImGui::Separator();
    }
    else if (selectedTab == 1) // Display
    {
    //    static bool darkMode = true;
	        ImGui::Separator();
			 ImGui::Text("           [ ESP - MENU ]" );
			 ImGui::Separator();
							ImGui::Dummy(ImVec2(0, 12));
        DrawToggleSwitch(ICON_FA_CREDIT_CARD" Esp Line", &Config.ESP.Line);
		ImGui::Dummy(ImVec2(0, 12));
		DrawToggleSwitch(ICON_FA_CREDIT_CARD" Esp Granade", &EspGrenade);
		ImGui::Dummy(ImVec2(0, 12));
		DrawToggleSwitch(ICON_FA_CREDIT_CARD" Esp Health", &Config.ESP.Health);
		ImGui::Dummy(ImVec2(0, 12));
		DrawToggleSwitch(ICON_FA_CREDIT_CARD" Esp Info", &Config.ESP.Info);
		ImGui::Dummy(ImVec2(0, 12));
		DrawToggleSwitch(ICON_FA_CREDIT_CARD" Esp Box", &Config.ESP.Box);
		        ImGui::Separator();
    }
    else if (selectedTab == 2) // Advanced
    {
		        ImGui::Separator();
			 ImGui::Text("           [ CHAMS - MENU ]" );
			 ImGui::Separator();
								ImGui::Dummy(ImVec2(0, 12));
      //  static bool location = false;
        DrawToggleSwitch(ICON_FA_PAPER_PLANE" Default Chams", &chams);
				ImGui::Dummy(ImVec2(0, 12));
		DrawToggleSwitch(ICON_FA_PAPER_PLANE" Glow Chams", &glow);
				ImGui::Dummy(ImVec2(0, 12));
		DrawToggleSwitch(ICON_FA_PAPER_PLANE" Shading Chams", &shading);
				ImGui::Dummy(ImVec2(0, 12));
		DrawToggleSwitch(ICON_FA_PAPER_PLANE" WireFrame Chams", &wireframe);
		        ImGui::Separator();
    }
else if (selectedTab == 3) // Display
    {
		
    //    static bool darkMode = true;
	        ImGui::Separator();
			 ImGui::Text("           [ BRUTAL - MENU ]" );
			 ImGui::Separator();
							ImGui::Dummy(ImVec2(0, 12));
        DrawToggleSwitch(ICON_FA_HEART" No Scop Awm", &Awm);
				ImGui::Dummy(ImVec2(0, 12));
		DrawToggleSwitch(ICON_FA_HEART" Fast Switch", &FastSwitch);
				ImGui::Dummy(ImVec2(0, 12));
		DrawToggleSwitch(ICON_FA_HEART" No Recoil", &norecoil);	
		ImGui::Dummy(ImVec2(0, 12));
		DrawToggleSwitch(ICON_FA_HEART" High Jump", &JumpHight);	
		ImGui::Dummy(ImVec2(0, 12));
		DrawToggleSwitch(ICON_FA_HEART" Pc Logo", &PCLOGO);
		ImGui::Separator();
ImGui::Checkbox("Enable Spin", &SpinEnable);
ImGui::SliderFloat("Spin bot", &SpinSpeed, 30.0f, 1080.0f, "%.0f deg/s");
		        ImGui::Separator();

    }
    
    else if (selectedTab == 4) // Display
    {
		
    //    static bool darkMode = true;
	        ImGui::Separator();
			 ImGui::Text("         [ Running - MENU 4 ]" );
			 ImGui::Separator();
			 ImGui::Dummy(ImVec2(0, 12));
		DrawToggleSwitch(ICON_FA_SACK" Fish Rining Test", &fishRining);	
ImGui::RadioButton("Speed Timer", &SpeedMode, 1);
ImGui::SameLine();
ImGui::RadioButton("Speed hack", &SpeedMode, 2);
ImGui::Separator();
ImGui::Separator();
ImGui::Text(" YT : AXL MODS OFFICIAL ");
ImGui::Text(" Telegram : Axl_Modsx ");
ImGui::Text(" Subscribe My all Channels ");
ImGui::Separator();
			 }
			else if (selectedTab == 5) // Display
    {
		
    //    static bool darkMode = true;
	        ImGui::Separator();
			 ImGui::Text("           [ COLOURS - MENU 5 ]" );
			 ImGui::Separator();
			 ImGui::SliderFloat("ESP Size", &ESP_LineThickness, 1.0f, 3.0f, "%.1f");
ImGui::Text("ESP Line Color");
ImGui::SameLine();

if (ImGui::ColorButton(
        "##LineBtn",
        ESP_LineColor,
        ImGuiColorEditFlags_NoTooltip,
        ImVec2(20, 20)))
{
    ImGui::OpenPopup("LinePicker");
}

if (ImGui::BeginPopup("LinePicker"))
{
    ImGui::ColorPicker4(
        "##LinePick",
        (float*)&ESP_LineColor,
        ImGuiColorEditFlags_NoInputs |
        ImGuiColorEditFlags_NoSidePreview |
        ImGuiColorEditFlags_NoLabel |
        ImGuiColorEditFlags_NoOptions
    );
    ImGui::EndPopup();
}
ImGui::Text("ESP Box Color");
ImGui::SameLine();

if (ImGui::ColorButton(
        "##BoxBtn",
        ESP_BoxColor,
        ImGuiColorEditFlags_NoTooltip,
        ImVec2(20, 20)))
{
    ImGui::OpenPopup("BoxPicker");
}

if (ImGui::BeginPopup("BoxPicker"))
{
    ImGui::ColorPicker4(
        "##BoxPick",
        (float*)&ESP_BoxColor,
        ImGuiColorEditFlags_NoInputs |
        ImGuiColorEditFlags_NoSidePreview |
        ImGuiColorEditFlags_NoLabel |
        ImGuiColorEditFlags_NoOptions
    );
    ImGui::EndPopup();
}
			 ImGui::Separator();
}
else if (selectedTab == 6)
    {
		
	        ImGui::Separator();
			 ImGui::Text("           [ MODE - MENU ]" );
			 ImGui::Separator();
			 ImGui::Dummy(ImVec2(0, 12));
        DrawToggleSwitch(ICON_FA_PLUS" Stream Mode", &stream_mode);
				ImGui::Dummy(ImVec2(0, 12));
		DrawToggleSwitch(ICON_FA_PLUS" Chams Mode", &FastSwitch);
				ImGui::Dummy(ImVec2(0, 12));
		DrawToggleSwitch(ICON_FA_PLUS" Ghost Mode", &Ghost);
		ImGui::Dummy(ImVec2(0, 12));
		bool GuestResetOn = true;
		DrawToggleSwitch(ICON_FA_PLUS" Guest Reset", &GuestResetOn);
		        ImGui::Separator();

}
    ImGui::EndChild();
   
	
	
      }
     ImGui::End();
	 
if (SpeedMode == 1)
{
    ImVec2 screenCenter = ImGui::GetMainViewport()->GetCenter();
    ImVec2 screenSize = ImGui::GetMainViewport()->Size;
    float posY = screenCenter.y + (screenSize.y * 0.25f);
    ImVec2 pos = ImVec2(screenCenter.x, posY);
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.0f); 
    ImGui::Begin("Speed Hack Floating", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoMove);
    DrawPremiumToggle("SPEED", &SpeedHack1);

    ImGui::End();
}
	ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        return old_eglSwapBuffers(dpy, surface);
}



typedef unsigned long DWORD;
static uintptr_t libBase;

uintptr_t string2Offset(const char *c) {
    int base = 16;
    // See if this function catches all possibilities.
    // If it doesn't, the function would have to be amended
    // whenever you add a combination of architecture and
    // compiler that is not yet addressed.
    static_assert(sizeof(uintptr_t) == sizeof(unsigned long)
                  || sizeof(uintptr_t) == sizeof(unsigned long long),
                  "Please add string to handle conversion for this architecture.");

    // Now choose the correct function ...
    if (sizeof(uintptr_t) == sizeof(unsigned long)) {
        return strtoul(c, nullptr, base);
    }

    // All other options exhausted, sizeof(uintptr_t) == sizeof(unsigned long long))
    return strtoull(c, nullptr, base);
}


inline void hack_injec();
inline void StartGUI() {
    void *ptr_eglSwapBuffer = DobbySymbolResolver("/system/lib/libEGL.so", "eglSwapBuffers");
    if (NULL != ptr_eglSwapBuffer) {
        DobbyHook((void *)ptr_eglSwapBuffer, (void*)hook_eglSwapBuffers, (void**)&old_eglSwapBuffers);
            LOGD("Gui Started");
			hack_injec();
        }
    }

bool libLoaded = false;
bool(*camcao0)(void*_this, int value);
bool _camcao0(void*_this, int value){
    if (_this != NULL && Camcao) {
                return 2;
            }
        return camcao0(_this, value);
    }

/*
static int (*LEBIPIGPEEP)(...);
static int _LEBIPIGPEEP(void* GLDLCOBLGNF)
{
	if (Speedrun)
	{
		return 1;
	}
	return LEBIPIGPEEP(GLDLCOBLGNF);
}

bool (*IsFoldWingGliding)(void *_CrazyMod);
bool _IsFoldWingGliding(void *_CrazyMod)
{
	if (_CrazyMod != NULL && Speedrun)
	{
		return true;
	}
	return IsFoldWingGliding(_CrazyMod);
}*/
bool (*GJCHEHNJIAD)(void *_CrazyMod);
bool _GJCHEHNJIAD(void *_CrazyMod)
{
	if (_CrazyMod != NULL && Awm)
	{
		return false;
	}
	return GJCHEHNJIAD(_CrazyMod);
}
DWORD findLibrary(const char *library) {
    char filename[0xFF] = {0},
            buffer[1024] = {0};
    FILE *fp = NULL;
    DWORD address = 0;

    sprintf(filename, OBFUSCATE("/proc/self/maps"));

    fp = fopen(filename, OBFUSCATE("rt"));
    if (fp == NULL) {
        perror(OBFUSCATE("fopen"));
        goto done;
    }
	
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strstr(buffer, library)) {
            address = (DWORD) strtoul(buffer, NULL, 16);
            goto done;
        }
    }

    done:

    if (fp) {
        fclose(fp);
    }

    return address;
}

DWORD getAbsoluteAddress(const char *libraryName, DWORD relativeAddr) {
    libBase = findLibrary(libraryName);
    if (libBase == 0)
        return 0;
    return (reinterpret_cast<DWORD>(libBase + relativeAddr));
}
ProcMap unityMap, anogsMap, il2cppMap;
using KittyScanner::RegisterNativeFn;



void hack() {
     LOGD("Inject Ok");
    //std::thread thread_hack(hack_thread, getpid());
    //thread_hack.detach();
    
}
uintptr_t get_symbol_addr_in_pid(pid_t pid, const char* libname, uintptr_t offset_in_lib) {
    char maps_path[64];
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);

    FILE* fp = fopen(maps_path, "r");
    if (!fp) return 0;

    char line[512];
    uintptr_t base = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, libname)) {
            sscanf(line, "%lx-%*lx", &base);
            break;
        }
    }
    fclose(fp);

    if (base == 0) return 0;
    return base + offset_in_lib;
}

pid_t get_pid_by_name(const char* process_name) {
    DIR* proc_dir = opendir("/proc");
    if (!proc_dir) return -1;

    struct dirent* entry;
    while ((entry = readdir(proc_dir)) != NULL) {
        if (entry->d_type != DT_DIR) continue;

        pid_t pid = atoi(entry->d_name);
        if (pid <= 0) continue;

        char cmdline_path[256];
        snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%d/cmdline", pid);

        FILE* fp = fopen(cmdline_path, "r");
        if (!fp) continue;

        char cmdline[256];
        fgets(cmdline, sizeof(cmdline), fp);
        fclose(fp);

        if (strstr(cmdline, process_name)) {
            closedir(proc_dir);
            return pid;
        }
    }

    closedir(proc_dir);
    return -1;
}

void writeLog(const std::string& logMessage, const std::string& filename) {
    std::ofstream outFile(filename, std::ios::app); // Mở file ở chế độ append (thêm)
    if (outFile.is_open()) {
        outFile << logMessage << std::endl;
        outFile.close();
    } else {
        std::cerr << "Không thể mở file log: " << filename << std::endl;
    }
}

bool is_current_process(const char* target_name) {
    char cmdline_path[64];
    snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%d/cmdline", getpid());

    FILE* fp = fopen(cmdline_path, "r");
    if (!fp) return false;

    char cmdline[256] = {0};
    fgets(cmdline, sizeof(cmdline), fp);
    fclose(fp);

    return strcmp(cmdline, target_name) == 0;
}
bool (*origpclogo)(void* _this);
bool _hookpclogo(void* _this) {
 if (PCLOGO) {
     return true;

}}
bool g_IsEmulatorReady = false;
bool IsEverythingLoad = false;

bool (*PCcheck) ();
bool _PCcheck() {
    IsEverythingLoad = true;
    
    return false;
}
void hack_injec() {
	while (!unityMap.isValid()) {
        unityMap = KittyMemory::getLibraryBaseMap("libunity.so");
		anogsMap = KittyMemory::getLibraryBaseMap("libanogs.so");
		il2cppMap = KittyMemory::getLibraryBaseMap("libil2cpp.so");
		
        sleep(6);
	}
    
 sleep(5);
    Il2CppAttach();

    if (mlovinit()) {
        setShader(OBFUSCATE("_AlphaMask"));
        LogShaders();
        Wallhack();
    }


DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("UpdateBehavior"), 2), (void *) _LateUpdate, (void **) &LateUpdate);
	//aimsilent
//	DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("OJOHLBJEIJO"), 2), (void *) hook_mrAimSilent, (void **)&orig_mrSilentAim);
	 
        // Speed Run
   DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("IsFoldWingGliding"), 0), (void *) _IsFoldWingGliding, (void **)& IsFoldWingGliding);
	//DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("message"), OBFUSCATE("LLOABGDLMGK"), OBFUSCATE("LEBIPIGPEEP"), 1), (void *) _LEBIPIGPEEP, (void **)& LEBIPIGPEEP);

    // ResetGuest
//hexPatches.Guest = MemoryPatch::createWithHex(getAbsoluteAddress("libil2cpp.so", 0x5ed140c),"20 00 80 D2 C0 03 5F D6");
    		   
//--+++++--- PC LOGO ________///

     DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE(""), OBFUSCATE("GGP"), OBFUSCATE("getEVData"), 0), (void *)_hookpclogo, (void **)& origpclogo);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE(""), OBFUSCATE("GGP"), OBFUSCATE("IsEmulator"), 0),  (void *)_hookpclogo, (void **)& origpclogo);
// Dummy base address

     DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIHudSuicideController"), OBFUSCATE("OnPreparationCancel"), 0), (void *) _OnPreparationCancel, (void **)& OnPreparationCancel);
	  DobbyHook(
        (void *)Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GrenadeLine"), OBFUSCATE("Update"), 0), 
        (void *)_UpdateGranada, 
        (void **)&UpdateGranada
    );

    DobbyHook(
        (void *)Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GrenadeLine"), OBFUSCATE("OnShowGrenadeLineChanged"), 0), 
        (void *)_UpdateGranada, 
        (void **)&UpdateGranada
    );
    
	  //DoubleGun
 DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Weapon"), OBFUSCATE("GetCombineType"), 0), (void *) _GetCombineType, (void **) &GetCombineType);

//FOR DOUBLE GUN
  /*void * ADBBMDMEFNO(){
       void * (*_ADBBMDMEFNO)(void* thiz) = (void * (*)(void*))getAddress("libil2cpp.so", None);
      return _ADBBMDMEFNO(this);
  }*/
  //fast switch
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_InSwapWeaponCD"), 0), (void *) _get_InSwapWeaponCD, (void **) &get_InSwapWeaponCD);
              



DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameVarDef"), OBFUSCATE("EnableCheckWaterWhileSwimming"), 0), (void *) _EnableCheckWaterWhileSwimming, (void **) &EnableCheckWaterWhileSwimming);
              


	   



	      DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("PlayerAttributes"), OBFUSCATE("GetScatterRate"), 0), (void *) _GetScatterRate, (void **) &GetScatterRate); // 0x15e799c ob44 đuôi 94
    
   

    //Fix Game
	DobbyHook(
    (void *)Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("GetCurrentDashSpeed"), 0),
    (void *)hook_GetCurrentDashSpeed,
    (void **)&old_GetCurrentDashSpeed);
	DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_MaxJumpHeight"), 0), (void*)my_get_MaxJumpHeight, (void**)&orig_get_MaxJumpHeight);
    DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_CustomGravity"), 0), (void*)my_get_CustomGravity, (void**)&orig_get_CustomGravity);
    DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_RisingGravity"), 0), (void*)my_get_RisingGravity, (void**)&orig_get_RisingGravity);
    DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_FallingGravity"), 0), (void*)my_get_FallingGravity, (void**)&orig_get_FallingGravity);
    DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameConfig") , OBFUSCATE("get_ResetGuest"), 0), (void *) ResetGuest, (void **) &_ResetGuest);
if (Ghostplayer != 0) {
        DobbyHook(
            (void *)Ghostplayer,
            (void *)hook_NeedSendMessage,
            (void **)&orig_NeedSendMessage
        );
    }
    
	
    DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("GCommon"), OBFUSCATE("PlatformUtility_Android"), OBFUSCATE("IsPackageInstalled"), 0), (void *) _Set_Aim, (void **) &Set_Aim);
    DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("GCommon"), OBFUSCATE("PlatformUtility_Android"), OBFUSCATE("CheckFileExists"), 0), (void *) _Set_Aim, (void **) &Set_Aim);
DobbyHook((void *) (uintptr_t)Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("UpdateRotation"), 2), (void *) hook_mrAimSilent, (void **)&orig_mrSilentAim);


//MemoryPatch::createWithHex((uintptr_t)Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameConfig"), OBFUSCATE("get_ResetGuest"), 0), OBFUSCATE("20 00 80 D2 C0 03 5F D6")).Modify();
	// DobbyHook((void *) (uintptr_t)Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("UpdateBehavior"), 2), (void *) hook_LateUpdate, (void **) &orig_LateUpdate);
  MemoryPatch::createWithHex((uintptr_t)Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameConfig"), OBFUSCATE("get_ResetGuest"), 0), OBFUSCATE("20 00 80 D2 C0 03 5F D6")).Modify();
	// DobbyHook((void *) (uintptr_t)Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("UpdateBehavior"), 2), (void *) hook_LateUpdate, (void **) &orig_LateUpdate);
  //  hexPatches.Guest = MemoryPatch::createWithHex(getAbsoluteAddress("libil2cpp.so", 0x5ed140c),"20 00 80 D2 C0 03 5F D6");
	

    ImGuiOK = true;
    
}



void hack_thread(pid_t pid) {
	
	StartGUI();
	while(pid == -1){pid = get_pid_by_name("com.dts.freefireth");} 
	remote_inject(pid);
	writeLog(to_string(pid));
    
}
__attribute__((constructor))
void lib_main()
{
    std::thread thread_hack(hack_thread, get_pid_by_name("com.dts.freefireth"));
    thread_hack.detach();
}


