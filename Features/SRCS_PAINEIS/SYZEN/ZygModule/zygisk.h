#include <x.h>
#include <layout.h>


class _BYTE; class _QWORD;
class _DWORD; class _WORD;
#define _QWORD long 
#define _DWORD long
#define _BYTE long 
#define _WORD long

JavaVM *jvm;
JNIEnv *genv;


using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
eglQuerySurface(dpy, surface, EGL_WIDTH, &egl.width);
eglQuerySurface(dpy, surface, EGL_HEIGHT, &egl.height);
if (!egl.setup) {
ImGui::CreateContext();
SetupImgui();InitTexture();
egl.setup = true;
}
ImGuiIO &io = ImGui::GetIO();ImGui_ImplOpenGL3_NewFrame();
ImGui_ImplAndroid_NewFrame(egl.width, egl.height);ImGui::NewFrame();
menu();DrawESP(ImGui::GetBackgroundDrawList(), egl.width, egl.height, true);
ImGui::EndFrame();ImGui::Render();
glViewport(0, 0, io.DisplaySize.x, io.DisplaySize.y);
ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
return old_eglSwapBuffers(dpy, surface);
}

void *FreeFire(const char *) {
while (!getLibraryLoaded(OBFUSCATE("libanogs.so")));
sleep(1);Il2CppAttach();
void* libAndroid = dlopen(OBFUSCATE("libandroid.so"), RTLD_NOW | RTLD_GLOBAL);
if (!libAndroid) {return nullptr;}
void* libInput = dlopen(OBFUSCATE("libinput.so"), RTLD_NOW | RTLD_GLOBAL);
if (!libInput) {return nullptr;}
void* libEGL = dlopen(OBFUSCATE("libEGL.so"), RTLD_NOW | RTLD_GLOBAL);
if (!libEGL) {return nullptr;}
A64HookFunction((void *) dlsym(dlopen(OBFUSCATE("libandroid.so"), 4), OBFUSCATE("ANativeWindow_getWidth")), (void *) hook_getWidth, (void **) &old_getWidth);
A64HookFunction((void *) dlsym(dlopen(OBFUSCATE("libandroid.so"), 4), OBFUSCATE("ANativeWindow_getHeight")), (void *) hook_getHeight, (void **) &old_getHeight);
A64HookFunction((void *) dlsym(dlopen(OBFUSCATE("libinput.so"), 4), OBFUSCATE("_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE")), (void *) hook_input, (void **) &old_input);
A64HookFunction((void *) dlsym(dlopen(OBFUSCATE("libEGL.so"), 4), OBFUSCATE("eglSwapBuffers")), (void *) hook_eglSwapBuffers, (void **) &old_eglSwapBuffers);
A64HookFunction((void *)Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("PlayerAttributes"), OBFUSCATE("PBDHFOEOIBH"), 1), (void *) _EXEMPLO, (void **) &EXEMPLO);
setShader(OBFUSCATE("_AlphaMask"));
if (mlovinit()) {
LogShaders();Wallhack();
}
}




class GsModz : public zygisk::ModuleBase {
public:
void onLoad(Api *api, JNIEnv *env) override {
this->api = api; 
this->env = env;
}
void preAppSpecialize(AppSpecializeArgs *args) override {
auto package_name = env->GetStringUTFChars(args->nice_name, nullptr);
auto app_data_dir = env->GetStringUTFChars(args->app_data_dir, nullptr);
preSpecialize(package_name, app_data_dir);
env->ReleaseStringUTFChars(args->nice_name, package_name);
env->ReleaseStringUTFChars(args->app_data_dir, app_data_dir);
}
void postAppSpecialize(const AppSpecializeArgs *) override {
if (enable_hack) {
if (isFreeFire) {
std::thread hack_thread(FreeFire, game_data_dir);
hack_thread.detach();}}}
private:
Api *api;
JNIEnv *env;
bool enable_hack; bool isFreeFire; char *game_data_dir;
std::vector<std::string> targetPackagesFF = {
OBFUSCATE("com.dts.freefireth"), OBFUSCATE("com.dts.freefiremax"), OBFUSCATE("com.dts.freefireadv")};
void preSpecialize(const char *package_name, const char *app_data_dir) {
for (const auto &targetPackage : targetPackagesFF) {
if (strcmp(package_name, targetPackage.c_str()) == 0) {
enable_hack = true;isFreeFire = true;
game_data_dir = new char[strlen(app_data_dir) + 1];
strcpy(game_data_dir, app_data_dir);
return;}}}};
REGISTER_ZYGISK_MODULE(GsModz)
