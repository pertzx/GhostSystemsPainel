#include <cstring>
#include <jni.h>
#include <pthread.h>
#include <thread>
#include <cstdio>
#include <unistd.h>
#include <dlfcn.h>
#include <list>
#include <vector>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <linux/ptrace.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <sys/system_properties.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <KittyMemory/KittyScanner.h>
#include <KittyMemory/KittyUtils.h>
#include <pthread.h>
#include <unistd.h>
#include "KittyMemory/KittyMemory.h"
#include "KittyMemory/MemoryPatch.h"
using namespace KittyMemory;
#include "oxorany/oxorany.h"
#include "Includes/obfuscate.h"
#include "Includes/Logger.h"
#include "Includes/Macros.h"
#include "Includes/Utils.h"
#include "xdl/xdl.h"
#include "curl/json.hpp"
#include "Nigga/fake_dlfcn.h"
#include "Nigga/Call_Me.h"
#include "Struct/Class.h"
#include "Struct/Unity.h"
#include "Misc.h"
#include "zygisk.hpp"
#include "Hook.h"
#include "Jni_Stuff.h"
#define ALPHA    ( ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder )
#define NO_ALPHA ( ImGuiColorEditFlags_NoTooltip    | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder )

JavaVM *jvm1 = nullptr;
jobject g_Activity = nullptr;

#include "ConstructMenu.h"
#include "ImGui/Call_ImGui.h"
#include "Includes/hide.h"

// Hex Auto 
#define FALSE64BIT "00 00 80 D2 C0 03 5F D6"
#define TRUE64BIT  "20 00 80 D2 C0 03 5F D6"
#define RETURN64BIT "C0 03 5F D6"
#define KLYUIM(assembly, namespaceName, className, methodName, paramCount, newFunc, oldFunc) \
DobbyHook((void*)Il2CppGetMethodOffset(OBFUSCATE(assembly), OBFUSCATE(namespaceName), OBFUSCATE(className), OBFUSCATE(methodName), paramCount), (void*)newFunc, (void**)&oldFunc)

void ResolveGameActivity(JNIEnv* env) {
    if (g_Activity != nullptr) return;

    jclass activityThreadClass = env->FindClass("android/app/ActivityThread");
    if (activityThreadClass) {
        jmethodID currentActivityThread = env->GetStaticMethodID(activityThreadClass, "currentActivityThread", "()Landroid/app/ActivityThread;");
        jobject at = env->CallStaticObjectMethod(activityThreadClass, currentActivityThread);
        if (at) {
            jmethodID getApplication = env->GetMethodID(activityThreadClass, "getApplication", "()Landroid/app/Application;");
            jobject app = env->CallObjectMethod(at, getApplication);
            if (app) {
                g_Activity = env->NewGlobalRef(app);
                return;
            }
        }
    }
    
    jclass unityPlayerClass = env->FindClass("com/unity3d/player/UnityPlayer");
    if (unityPlayerClass) {
        jfieldID currentActivityField = env->GetStaticFieldID(unityPlayerClass, "currentActivity", "Landroid/app/Activity;");
        jobject act = env->GetStaticObjectField(unityPlayerClass, currentActivityField);
        if (act) {
            g_Activity = env->NewGlobalRef(act);
        }
    }
}

bool g_IsGameReady = false;

/*bool (*_MPMatch)(void* _this);
bool MPMatch(void* _this) {
    if (func_ghost) {
        return false;
    }
    return _MPMatch(_this);
}*/
void patchLib(const char* libName, uintptr_t offset, const char* hex) {
ProcMap lib;
do {
lib = KittyMemory::getLibraryBaseMap(libName);
sleep(1);
} while (!lib.isValid());
MemoryPatch::createWithHex(lib, offset, hex).Modify();
}

bool (*ResetGuest)(void* _this);
bool _ResetGuest(void* _this) {
    if (Guest) {
        return true;
    }
    return ResetGuest(_this);
}
bool (*_ShowWeaponSkin)(void* _this);
bool ShowWeaponSkin(void* _this) {
        return true;
}
bool (*_ShowWeaponSkin1)(void* _this);
bool ShowWeaponSkin1(void* _this) {
        return false;
}
/*void (*_RET)(void *instance);
void RET(void *instance) {
    return;
}*/
bool (*_False)(void *Player);
bool False(void *Player) {
return false;
}
int (*_GetSystemPhysicalMemory)(void*_this, int value);
int GetSystemPhysicalMemory(void*_this, int value) {
return (int) 10000;
}
int (*_GetPhysXState)(void*_this, int value);
int GetPhysXState(void*_this, int value){
    if (_this != NULL) {        
        if (FlyUpBR) {
            return valueBr;
        }
        return _GetPhysXState(_this, value);
    }    
    return 0; 
}
void (*_EPhysXState)(void*_this, int value);

void EPhysXState(void*_this, int value){
    if (_this != NULL) {
        if(FlyUpBR) {
            _EPhysXState(_this, valueBr1); 
            return;
        }
        _EPhysXState(_this, value);
    }
}
bool (*orig_IsPoseFallingHigh)(void *Player);
bool hook_IsPoseFallingHigh(void *Player){
return true;
}

bool g_IsEmulatorReady = false;
bool IsEverythingLoad = false;

bool (*PCcheck) ();
bool _PCcheck() {
    IsEverythingLoad = true;
    
    return false;
}

ProcMap _Libil2cpp, _Libunity;

void *hack_thread(void *arg){
    while (!_Libil2cpp.isValid() || !_Libunity.isValid()) {
        _Libil2cpp = KittyMemory::getLibraryBaseMap("libil2cpp.so");
        _Libunity = KittyMemory::getLibraryBaseMap("libunity.so");
        sleep(1);
   }
   
   sleep(3); 

   Il2CppAttach();
   std::thread(SpinBotVoid).detach();
   std::thread(AimSilentVoid).detach();

   DobbyHook((void *)Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameFacade"), OBFUSCATE("IsEmulator"), 0), (void *) &_PCcheck, (void **) & PCcheck);

DobbyHook((void *) DobbySymbolResolver(OBFUSCATE("/system/lib/libEGL.so"), OBFUSCATE("eglSwapBuffers")), (void *) _eglSwapBuffers1, (void **) &orig_eglSwapBuffers1);
DobbyHook((void *) DobbySymbolResolver(OBFUSCATE("/system/lib/libandroid.so"), OBFUSCATE("ANativeWindow_getWidth")), (void *) _ANativeWindow_getWidth, (void **) &orig_ANativeWindow_getWidth);
DobbyHook((void *) DobbySymbolResolver(OBFUSCATE("/system/lib/libandroid.so"), OBFUSCATE("ANativeWindow_getHeight")), (void *) _ANativeWindow_getHeight, (void **) &orig_ANativeWindow_getHeight);
DobbyHook((void *) DobbySymbolResolver(OBFUSCATE("/system/lib/libandroid.so"), OBFUSCATE("AConfiguration_getDensity")), (void *) _AConfiguration_getDensity, (void **) &orig_AConfiguration_getDensity);
// KLEL-YUIM
KLYUIM("Assembly-CSharp.dll", "COW", "GameConfig", "get_ResetGuest", 0, _ResetGuest, ResetGuest);
KLYUIM("UnityEngine.dll", "UnityEngine", "Input", "GetTouch", 1, GetTouch, old_GetTouch);

KLYUIM("Assembly-CSharp.dll", "COW.GamePlay", "Player", "ShowWeaponSkin", 0, ShowWeaponSkin, _ShowWeaponSkin);
KLYUIM("Assembly-CSharp.dll", "COW.GamePlay", "Player", "IsWeaponSkinABStatusA", 0, ShowWeaponSkin1, _ShowWeaponSkin1);
KLYUIM("Assembly-CSharp.dll", "COW.GamePlay", "Player", "NeedWeaponSkinFireSound", 0, ShowWeaponSkin1, _ShowWeaponSkin1);
KLYUIM("Assembly-CSharp.dll", "COW.GamePlay", "Player", "UpdateBehavior", 2, _LateUpdate, LateUpdate);
//KLYUIM("Assembly-CSharp.dll", "COW.GamePlay", "Player", "UpdateBehavior", 2, _LateUpdateSilent, LateUpdateSilent);
// Fly Hack
KLYUIM("Assembly-CSharp.dll", "COW.GamePlay", "Player", "IsIgnoreHighFalling", 0, hook_IsPoseFallingHigh, orig_IsPoseFallingHigh);
KLYUIM("Assembly-CSharp.dll", "COW.GamePlay", "Player", "get_IsPoseFallingHigh", 0, hook_IsPoseFallingHigh, orig_IsPoseFallingHigh);
KLYUIM("Assembly-CSharp.dll", "COW.GamePlay", "Player", "OnStopCatapultFalling", 0, hook_IsPoseFallingHigh, orig_IsPoseFallingHigh);
KLYUIM("Assembly-CSharp.dll", "", "PhysXData", "get_IsJumpPadFalling", 0, hook_IsPoseFallingHigh, orig_IsPoseFallingHigh);
KLYUIM("Assembly-CSharp.dll", "", "PhysXData", "get_IsSkateboardJumping", 0, hook_IsPoseFallingHigh, orig_IsPoseFallingHigh);
KLYUIM("Assembly-CSharp.dll", "", "PhysXData", "get_IsSlideFalling", 0, hook_IsPoseFallingHigh, orig_IsPoseFallingHigh);
KLYUIM("Assembly-CSharp.dll", "COW.GamePlay", "Player", "GetPhysXState", 0, GetPhysXState, _GetPhysXState);

    if (offset_NeedSendMessage != 0) {
        DobbyHook(
            (void *)offset_NeedSendMessage,
            (void *)hook_NeedSendMessage,
            (void **)&orig_NeedSendMessage
        );
    }
    
    if (offset_GetScatterRate != 0) {
        DobbyHook(
            (void*)offset_GetScatterRate, 
            (void*)hook_GetScatterRate, 
            (void**)&orig_GetScatterRate
        );
    }
    
    //DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("GPBDEDFKJNA"), OBFUSCATE("BLAGCMCGEJG"), 1), (void *) BLAGCMCGEJG1,(void **) &old_BLAGCMCGEJG1);    
    //DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("PlayerNetwork"), OBFUSCATE("TakeDamage"), 9), (void *) &hook_PlayerNetwork_TakeDamage, (void **) &orig_PlayerNetwork_TakeDamage);    
    DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("GPBDEDFKJNA"), OBFUSCATE("BLAGCMCGEJG"), 1), (void *) BLAGCMCGEJG1kill,(void **) &old_BLAGCMCGEJG1kill);
    DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("PlayerNetwork"), OBFUSCATE("TakeDamage"), 9), (void *) &hook_PlayerNetwork_TakeDamagekill, (void **) &orig_PlayerNetwork_TakeDamagekill);    
    //DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("SceneGraphicsQuality"), OBFUSCATE("SetGraphicsQuality"), 1), (void *) _BypassESP, (void **) &BypassESP);
    //DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Screen"), OBFUSCATE("SetResolution"), 3), (void *) _BypassESP, (void **) &BypassESP);
    
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
    
    DobbyHook(
    (void *)Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("GetCurrentDashSpeed"), 0),
    (void *)hook_GetCurrentDashSpeed,
    (void **)&old_GetCurrentDashSpeed);
    
    DobbyHook(
        (void *)Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameSettingData"), OBFUSCATE("IsHighFPS120Open"), 0),
        (void *)hook_IsHighFPS120Open,
        (void **)&orig_IsHighFPS120Open
    );
    
    DobbyHook(
        (void *)Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("message"), OBFUSCATE("LLOABGDLMGK"), OBFUSCATE("LHGGPDLOPAH"), 1),
        (void *)hook_PackVector3,
        (void **)&orig_PackVector3
    );
    
    DobbyHook((void *)offset_UpdateLocalMark, (void *)hook_UpdateLocalMark, (void **)&orig_UpdateLocalMark);
    
    //DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("ServiceConnectionManager"), OBFUSCATE("SendMessageToLobby"), 5), (void *)_SendMessageToLobby, (void**)&orig_SendMessageToLobby);
    
    //DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_UserLevel"), 0), (void*) &_get_UserLevel, (void**) &get_UserLevel);
    //DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_UserExp"), 0), (void*) &_get_UserExp, (void**) &get_UserExp);
                                            
/*Pathghost = MemoryPatch::createWithHex(((uintptr_t)Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("MPMatch") , OBFUSCATE("NeedSendMessage"), 1)) ,"00 00 80 D2 C0 03 5F D6");*/

//DobbyHook((void *)PhimSetNhatban(0x4994188), (void *)EPhysXState, (void **)&_EPhysXState);

   return nullptr;
}
__attribute__((constructor))
void lib_main() {

}
REGISTER_ZYGISK_MODULE(MyModule)
REGISTER_ZYGISK_COMPANION(companion_handler)

extern "C" {
    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
        jvm1 = vm;
        JNIEnv *env;
        if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
            return JNI_FALSE;
        }

        ResolveGameActivity(env);

        pthread_t ptid;
        pthread_create(&ptid, nullptr, hack_thread, nullptr);
               
        return JNI_VERSION_1_6;
    }
}
