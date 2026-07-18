#include <esp.h>
#include <layout.h>
#include <imports.h>
#include <jni.h>
#include "../KittyMemory/MemoryPatch.hpp"
#include "../KittyMemory/MemoryPatch.hpp"
#include <telaonline.h>
class _BYTE; class _QWORD;
class _DWORD; class _WORD;
#define _QWORD long 
#define _DWORD long
#define _BYTE long 
#define _WORD long
#include <EGL/egl.h>

 EGLBoolean(*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
 eglQuerySurface(dpy, surface, EGL_WIDTH, &egl.width);
 eglQuerySurface(dpy, surface, EGL_HEIGHT, &egl.height);
 if (!egl.setup) {
  ImGui::CreateContext();
  SetupImgui(); InitTexture();
  egl.setup = true;
 }
 ImGuiIO& io = ImGui::GetIO(); ImGui_ImplOpenGL3_NewFrame();

static bool show_login_window = false;
static bool three_finger_touch_active = false;

 ImGui_ImplAndroid_NewFrame(egl.width, egl.height);
 ImGui::NewFrame();

 int touchCount = (((int (*)())(Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Input"), OBFUSCATE("get_touchCount"))))());

 if (touchCount == 4) {
  if (!three_finger_touch_active) {
   show_login_window = !show_login_window; // Alterna a visibilidade da janela
   three_finger_touch_active = true;
  }
 } else {
  three_finger_touch_active = false;
 }

 // Processar o primeiro toque para o ImGui (simulando mouse)
 if (touchCount > 0) {
  UnityEngine_Touch_Fields touch = ((UnityEngine_Touch_Fields(*)(int))(Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Input"), OBFUSCATE("GetTouch"), 1)))(0);
  float reverseY = io.DisplaySize.y - touch.m_Position.fields.y;

  if (touch.m_Phase == TouchPhase::Began) {
   io.MousePos = ImVec2(touch.m_Position.fields.x, reverseY);
    io.MouseDown[0] = true;
   } else if (touch.m_Phase == TouchPhase::Ended || touch.m_Phase == TouchPhase::Canceled) {
    io.MouseDown[0] = false;
    clearMousePos = true;
   } else if (touch.m_Phase == TouchPhase::Moved || touch.m_Phase == TouchPhase::Stationary) {
    io.MousePos = ImVec2(touch.m_Position.fields.x, reverseY);
   }
 }

 if (show_login_window) {
    menu();
    DrawESP(egl.width, egl.height);
    Aimbott();
 }
	ImGui::EndFrame(); ImGui::Render(); 
	glViewport(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	return old_eglSwapBuffers(dpy, surface);
}
/*
int64_t (*hsub_419EA0)(int64_t a1, int64_t a2);
int64_t sub_419EA0(int64_t a1, int64_t a2) { 
    return 0; 
}

int64_t (*hsub_50CFB60)(int64_t a1, int64_t a2);
int64_t osub_50CFB60(int a1, int64_t a2) {
    printf("[HOOK] sub_50CFB60 called: a1=%d, a2=0x%llx\n", a1, a2);

    int new_a1 = a1;
    if (a1 == 18) new_a1 = 99;

    int64_t result = hsub_50CFB60(new_a1, a2);

    printf("[HOOK] sub_50CFB60 result=0x%llx\n", result);

    return 0;
}
*/
void *FreeFire(const char *) {
//while (!getLibraryLoaded(OBFUSCATE("libil2cpp.so")));
sleep(7);
void* libEGL = dlopen(OBFUSCATE("libEGL.so"), RTLD_NOW | RTLD_GLOBAL);
if (!libEGL) {return nullptr;}
A64HookFunction((void *) dlsym(dlopen(OBFUSCATE("libEGL.so"), 4), OBFUSCATE("eglSwapBuffers")), (void *) hook_eglSwapBuffers, (void **) &old_eglSwapBuffers);
Il2CppAttach();
/*
A64HookFunction((void *)getAbsoluteAddress("libil2cpp.so", 0x50cf13c), (void *)&osub_50CFB60, (void **)&hsub_50CFB60);
A64HookFunction((void *)getAbsoluteAddress("libil2cpp.so", 0x50cf1d8), (void *)&osub_50CFB60, (void **)&hsub_50CFB60);
A64HookFunction((void *)getAbsoluteAddress("libil2cpp.so", 0x50cf9cc), (void *)&osub_50CFB60, (void **)&hsub_50CFB60);
A64HookFunction((void *)getAbsoluteAddress("libil2cpp.so", 0x50cea6c), (void *)&osub_50CFB60, (void **)&hsub_50CFB60);
A64HookFunction((void *)getAbsoluteAddress("libil2cpp.so", 0x50cfb60), (void *)&osub_50CFB60, (void **)&hsub_50CFB60);
A64HookFunction((void *)getAbsoluteAddress("libil2cpp.so", 0x23a7), (void *)&sub_419EA0, (void **)&hsub_419EA0);
*/
if (mlovinit()){
//setShader(OBFUSCATE("_AlphaMask"));
//LogShaders(); Wallhack();
}
}


void *pLibRealUnity = 0;
typedef jint(JNICALL *CallJNI_OnLoad_t)(JavaVM *vm, void *reserved);
typedef void(JNICALL *CallJNI_OnUnload_t)(JavaVM *vm, void *reserved);
CallJNI_OnLoad_t RealJNIOnLoad = 0; CallJNI_OnUnload_t RealJNIOnUnload = 0;
JNIEXPORT jint JNICALL CallJNIOL(
JavaVM *vm, void *reserved) {
if (!pLibRealUnity)
pLibRealUnity = dlopen(OBFUSCATE("libmainn.so"), RTLD_NOW);
if (!RealJNIOnLoad)
RealJNIOnLoad = reinterpret_cast<CallJNI_OnLoad_t>(dlsym(pLibRealUnity, OBFUSCATE("JNI_OnLoad")));
return RealJNIOnLoad(vm, reserved);
}
JNIEXPORT void JNICALL CallJNIUL(
JavaVM *vm, void *reserved) {
if (!pLibRealUnity)
pLibRealUnity = dlopen(OBFUSCATE("libmainn.so"), RTLD_NOW);
if (!RealJNIOnUnload)
RealJNIOnUnload = reinterpret_cast<CallJNI_OnUnload_t>(dlsym(pLibRealUnity, OBFUSCATE("JNI_OnUnload")));
RealJNIOnUnload(vm, reserved);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
jvm = vm;
return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
CallJNIUL(vm, reserved);
}

__attribute__((constructor))
void lib_main() {
std::thread(FreeFire, nullptr).detach();
}
