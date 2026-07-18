#include "imgui_impl_opengl3.h"
#include "imgui_impl_android.h"
#include "palatino.h"
#include <imgui.h>
#include <memory.h>
bool Speed = true;
float (*EXEMPLO) (void* as);
float _EXEMPLO(void* as){
if (Speed){
return 1.9f;
}
return EXEMPLO(as);
}



struct {
    bool setup;
    int width;
    int height;
    int screenWidth;
    int screenHeight;
} egl;

void (*old_input)(void *event, void *exAb, void *exAc);
void hook_input(void *event, void *exAb, void *exAc) {
old_input(event, exAb, exAc);
ImGui_ImplAndroid_HandleTouchEvent((AInputEvent *) event, {(float) egl.screenWidth / (float) egl.width, (float) egl.screenHeight / (float) egl.height});
return;
}
int (*old_getWidth)(ANativeWindow* window);
int hook_getWidth(ANativeWindow* window) {
egl.screenWidth = old_getWidth(window);
return old_getWidth(window);
}
int (*old_getHeight)(ANativeWindow* window);
int hook_getHeight(ANativeWindow* window) {
egl.screenHeight = old_getHeight(window);
return old_getHeight(window);
}

float density = -1;
ImFont *font;


inline static bool g_IsSetup = false;
inline int prevWidth, prevHeight;
ImFontConfig config;
void SetupImgui() {
IMGUI_CHECKVERSION();
ImGui::CreateContext();
ImGuiIO& io = ImGui::GetIO();
ImGuiStyle* style = &ImGui::GetStyle();
io.DisplaySize = ImVec2(egl.width, egl.height);
ImGui::StyleColorsDark();
style->WindowRounding = 4.0f;
style->FrameRounding = 2.0f;
style->ScrollbarRounding = 0;
style->WindowTitleAlign = ImVec2(0.5, 0.5);
style->FramePadding = ImVec2(8, 6);
style->ScaleAllSizes(1.0f);
style->ScrollbarSize /= 1;
ImGui_ImplOpenGL3_Init(OBFUSCATE("#version 300 es"));
ImGui_ImplAndroid_Init(NULL);
ImFontConfig font_cfg;
io.Fonts->AddFontFromMemoryTTF((void *)khoanguyen_data, khoanguyen_size, 22.0f, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
//io.Fonts->AddFontFromMemoryTTF((void *)&ImpactRegular_data, sizeof(ImpactRegular_data), 40.0f, &font_cfg, io.Fonts->GetGlyphRangesDefault());
font_cfg.SizePixels = (float)density / 25.0f; 
ImGui::GetStyle().ScaleAllSizes(3.0f);
}

struct UnityEngine_Vector2_Fields {
float x;
float y;
};

struct UnityEngine_Vector2_o {
UnityEngine_Vector2_Fields fields;
};

enum TouchPhase {
Began = 0,
Moved = 1,
Stationary = 2,
Ended = 3,
Canceled = 4
};

struct UnityEngine_Touch_Fields {
int32_t m_FingerId;
struct UnityEngine_Vector2_o m_Position;
struct UnityEngine_Vector2_o m_RawPosition;
struct UnityEngine_Vector2_o m_PositionDelta;
float m_TimeDelta;
int32_t m_TapCount;
int32_t m_Phase;
int32_t m_Type;
float m_Pressure;
float m_maximumPossiblePressure;
float m_Radius;
float m_RadiusVariance;
float m_AltitudeAngle;
float m_AzimuthAngle;
};




