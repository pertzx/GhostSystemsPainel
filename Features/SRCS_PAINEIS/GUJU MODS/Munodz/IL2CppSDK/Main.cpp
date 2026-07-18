//change 18/8/2024 by Khánh Mods
#include <cstring>
#include <jni.h>
#include <pthread.h>
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
#include "KhanhMods/fake_dlfcn.h"
#include "KhanhMods/Call_Me.h"
#include "Struct/Class.h"
#include "Struct/Unity.h"
#include "Misc.h"
#include "zygisk.hpp"
#include "Hook.h"
#include "Jni_Stuff.h"
#define ALPHA    ( ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder )
#define NO_ALPHA ( ImGuiColorEditFlags_NoTooltip    | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder )
#include "ConstructMenu.h"
#include "FoxCheats/ScanEngine.hpp"
using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *genv) override {
        this->api = api;
        this->env = env;
		env = genv;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) {
            LOGD(OBFUSCATE("Skip unknown process"));
            return;
        }
        enable_hack = isGame(env, args->app_data_dir);
    }
private:
    Api *api;
    JNIEnv *env;

    void preSpecialize(const char *process) {
        unsigned r = 0;
        int fd = api->connectCompanion();
        read(fd, &r, sizeof(r));
        close(fd);
        LOGD(OBFUSCATE("process=[%s], r=[%u]\n"), process, r);
        api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    }

};

static int urandom = -1;

static void companion_handler(int i) {
    if (urandom < 0) {
        urandom = open(OBFUSCATE("/dev/urandom"), O_RDONLY);
    }
    unsigned r;
    read(urandom, &r, sizeof(r));
    LOGD(OBFUSCATE("companion r=[%u]\n"), r);
    write(i, &r, sizeof(r));
}


int isGame(JNIEnv *env, jstring appDataDir) {
    if (!appDataDir)
        return 0;
    const char *app_data_dir = env->GetStringUTFChars(appDataDir, nullptr);
    int user = 0;
    static char package_name[256];
    if (sscanf(app_data_dir, OBFUSCATE("/data/%*[^/]/%d/%s"), &user, package_name) != 2) {
        if (sscanf(app_data_dir, OBFUSCATE("/data/%*[^/]/%s"), package_name) != 1) {
            package_name[0] = '\0';
            LOGW("can't parse %s", app_data_dir);
            return 0;
        }
    }
    if (strcmp(package_name, OBFUSCATE("com.dts.freefireth")) == 0) {
        LOGI(OBFUSCATE("detect game: %s"), package_name);
        game_data_dir = new char[strlen(app_data_dir) + 1];
        strcpy(game_data_dir, app_data_dir);
        env->ReleaseStringUTFChars(appDataDir, app_data_dir);
        return 1;
    } else {
        env->ReleaseStringUTFChars(appDataDir, app_data_dir);
        return 0;
    }
}
int (*old_Level)(void *instance);
int Level(void *instance) {
    if (instance != NULL && spin360) {
        return (int) 999999;
    }
    return old_Level(instance);
}
bool lohhgg = true;
int (*get_ClanID)(void *instance);
int ClanID(void *instance) {
    if (instance != NULL && spin360) {
        return (int) 993621639;
    }
    return get_ClanID(instance);
}
bool lohhgg1 = true;
int (*get_UserCoins)(void *instance);
int UserCoins(void *instance) {
    if (instance != NULL && spin360) {
        return (int) 999999;
    }
    return get_UserCoins(instance);
}
bool lohhgg3 = true;
int (*get_AccountID)(void *instance);
int AccountID(void *instance) {
    if (instance != NULL && spin360) {
        return (int) idgame;
    }
    return get_AccountID(instance);
}
ProcMap _Libil2cpp, _Libunity;
void *hack_thread(void *arg){
        while (!_Libil2cpp.isValid() || !_Libunity.isValid()) {
        _Libil2cpp = KittyMemory::getLibraryBaseMap("libil2cpp.so");
        _Libunity = KittyMemory::getLibraryBaseMap("libunity.so");
        sleep(1); // tránh CPU loop
   }
   ProcMap anogs;
    do {
        anogs = KittyMemory::getLibraryBaseMap("libanogs.so");
        sleep(1);
    } while (!anogs.isValid());
    Logs = Logs+" \n Anogs has Loaded";
    
   MemoryPatch ghost = MemoryPatch::createWithHex(_Libil2cpp, 0x354c5bc, "00 00 80 D2 C0 03 5F D6");

   Il2CppAttach();
sleep(1);
DobbyHook((void *) DobbySymbolResolver(OBFUSCATE("/system/lib/libEGL.so"), OBFUSCATE("eglSwapBuffers")), (void *) _eglSwapBuffers, (void **) &orig_eglSwapBuffers);
OpenURL = (void (*)(String*))Il2CppGetMethodOffset("UnityEngine.CoreModule.dll", "UnityEngine", "Application", "OpenURL", 1);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Input"), OBFUSCATE("GetTouch"), 1), (void*)GetTouch, (void**)&old_GetTouch);
/*
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameConfig"), OBFUSCATE("get_ResetGuest"), 0), (void *)_ResetGuest, (void **)&ResetGuest);

DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("GCommon"), OBFUSCATE("PlatformUtility"), OBFUSCATE("GetSystemPhysicalMemory"), 0), (void *)_Bypass21, (void **)& Bypass21);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("LobbyServiceConnectionHandler"), OBFUSCATE("CheckIsInBatchRoom"), 0), (void *)_Bypass21, (void **)& Bypass21);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("LobbyServiceConnectionHandler"), OBFUSCATE("CheckIsInRoom"), 0), (void *)_Bypass21, (void **)& Bypass21);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("LobbyServiceConnectionHandler"), OBFUSCATE("CheckIsUGCRoomMatchMaking"), 0), (void *)_Bypass21, (void **)& Bypass21);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("LobbyServiceConnectionHandler"), OBFUSCATE("IntConvertToEnum"), 1), (void *)_Bypass21, (void **)& Bypass21);
//Fix Game
/*
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("mscorlib.dll"), OBFUSCATE("System"), OBFUSCATE("SerializableAttribute"), OBFUSCATE(".ctor"), 0), (void *) &_FixGame, (void **) &FixGame);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("GarenaMSDK"), OBFUSCATE("GarenaMSDKMgr"), OBFUSCATE("IsPlatformInstalled"), 0), (void *) &_FixGame, (void **) &FixGame);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("System.dll"), OBFUSCATE("System.Net"), OBFUSCATE("WebProxy"), OBFUSCATE("IsBypassed"), 0), (void *) &_FixGame, (void **) &FixGame);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("System.dll"), OBFUSCATE("System.Net"), OBFUSCATE("IWebProxy"), OBFUSCATE("IsBypassed"), 0), (void *) &_FixGame, (void **) &FixGame);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("System.dll"), OBFUSCATE("System.Net"), OBFUSCATE("WebProxy"), OBFUSCATE(".ctor"), 0), (void *) &_FixGame, (void **) &FixGame);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("System.dll"), OBFUSCATE("System.Net"), OBFUSCATE("WebProxy"), OBFUSCATE("CheckBypassList"), 0), (void *) &_FixGame, (void **) &FixGame);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("EventLogger"), OBFUSCATE("LogReportCheatInHistory"), 0), (void *) &_FixGame, (void **) &FixGame);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("EventLogger"), OBFUSCATE("LogReportCheat"), 0), (void *) &_FixGame, (void **) &FixGame);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("MobileReplayManager"), OBFUSCATE("GetGameTimeMS"), 0), (void *) &_FixGame, (void **) &FixGame);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("CreditPunishManager"), OBFUSCATE("ShowPunishWindow"), 0), (void *) &_FixGame, (void **) &FixGame);
// Bypass Esp
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("SceneGraphicsQuality"), OBFUSCATE("SetGraphicsQuality"), 0), (void *) _BypassESP, (void **) &BypassESP);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Screen"), OBFUSCATE("SetResolution"), 0), (void *) _BypassESP, (void **) &BypassESP);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("QualitySettings"), OBFUSCATE("GetQualityLevel"), 0), (void *) _BypassESP, (void **) &BypassESP);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("SceneGraphicsQuality"), OBFUSCATE("SetGraphicsQuality"), 0), (void *) _BypassESP, (void **) &BypassESP);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("GCommon"), OBFUSCATE("PlatformUtility_Android"), OBFUSCATE("IsPackageInstalled"), 0), (void *) _Set_Aim, (void **) &Set_Aim);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("GCommon"), OBFUSCATE("PlatformUtility_Android"), OBFUSCATE("CheckFileExists"), 0), (void *) _Set_Aim, (void **) &Set_Aim);
// Bypass (cũ)
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIHudInGameShopBaseController"), OBFUSCATE("ClearData"), 0), (void *) &_FixGame, (void **) &FixGame);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIHudTakeTurnsShopGrandBaseController"), OBFUSCATE("ClearData"), 0), (void *) &_FixGame, (void **) &FixGame);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIPreviewNavigationController"), OBFUSCATE("ClearData"), 0), (void *) &_FixGame, (void **) &FixGame);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIBaseController"), OBFUSCATE("ClearData"), 0), (void *) &_FixGame, (void **) &FixGame);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("MonoBehaviour"), OBFUSCATE("ClearData"), 0), (void *) &_FixGame, (void **) &FixGame);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIEasyListItemController"), OBFUSCATE("ClearData"), 0), (void *) &_FixGame, (void **) &FixGame);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("CSVBaseData"), OBFUSCATE("GetRankName"), 0), (void *) &_FixGame, (void **) &FixGame);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIBaseController"), OBFUSCATE("UpdateGamePlay"), 0), (void *) &_FixGame, (void **) &FixGame);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIBaseController"), OBFUSCATE("SetViewData"), 0), (void *) &_FixGame, (void **) &FixGame);

DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("NetworkAIPawn"), OBFUSCATE("UpdateBehavior"), 0), (void *) &_AntiCrash, (void **) &AntiCrash);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("LobbyServiceConnectionHandler"), OBFUSCATE("OnMsgGin"), 0), (void *) &_AntiCrash, (void **) &AntiCrash);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("LobbyServiceConnectionHandler"), OBFUSCATE("OnMsgAno"), 0), (void *) &_AntiCrash, (void **) &AntiCrash);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("LobbyServiceConnectionHandler"), OBFUSCATE("OnSendDataToClientNtf"), 0), (void *) &_AntiCrash, (void **) &AntiCrash);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE(""), OBFUSCATE("GameVoiceFacade"), OBFUSCATE("Init"), 0), (void *) &_AntiCrash, (void **) &AntiCrash);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.Gameplay.UGC"), OBFUSCATE("UGCSoundEntity"), OBFUSCATE("GetFlag"), 0), (void *) &_AntiCrash, (void **) &AntiCrash);
// Fix crash V1
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("AnoSDKNamespace"), OBFUSCATE("IOPOOHPNCKH"), OBFUSCATE("LJGJMOPAEFB"), 0), (void *) &_Klel_Yuim, (void **) &Klel_Yuim); // 0x51adda8 Ob44 Đuôi 94
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("KPDMJKOEHEE"), OBFUSCATE("LGINFBPMLAI"), 0), (void *) &_Klel_Yuim, (void **) &Klel_Yuim); // 0x11ef43c Ob44 Đuôi 94
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("ffano"), OBFUSCATE("FNFOLKEMMBG"), OBFUSCATE("AOCFOHFIAAM"), 0), (void *) &_Klel_Yuim, (void **) &Klel_Yuim); // 0x42974b4 Ob44 Đuôi 94
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("ffano"), OBFUSCATE("FNFOLKEMMBG"), OBFUSCATE("CCNEAFOPMIH"), 0), (void *) &_Klel_Yuim, (void **) &Klel_Yuim); // 0x4297604 Ob44 Đuôi 94
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelLogin"), OBFUSCATE("DetectAndroidApplications"), 0), (void *) &_Klel_Yuim, (void **) &Klel_Yuim);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("JPPGAJBAAKK"), OBFUSCATE("IsOnlineGame"), 2), (void *) &_Klel_Yuim, (void **) &Klel_Yuim);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("AnoSDKNamespace"), OBFUSCATE("IOPOOHPNCKH"), OBFUSCATE("LBABEMNOJAJ"), 8), (void *) &_Klel_Yuim, (void **) &Klel_Yuim);
    
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("GCommon"), OBFUSCATE("PlatformUtility_Android"), OBFUSCATE("GetLowMemoryThreshold"), 0), (void *) &Bypass, (void **) &Bypass);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Exception"), OBFUSCATE("get_processorFrequency"), 0), (void *) &Bypass, (void **) &Bypass);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Exception"), OBFUSCATE("get_processorCount"), 0), (void *) &Bypass, (void **) &Bypass);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Exception"), OBFUSCATE("get_systemMemorySize"), 0), (void *) &Bypass, (void **) &Bypass);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Exception"), OBFUSCATE("get_graphicsMemorySize"), 0), (void *) &Bypass, (void **) &Bypass);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Exception"), OBFUSCATE("get_graphicsDeviceID"), 0), (void *) &Bypass, (void **) &Bypass);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Exception"), OBFUSCATE("get_graphicsDeviceVendorID"), 0), (void *) &Bypass, (void **) &Bypass);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Exception"), OBFUSCATE("get_graphicsShaderLevel"), 0), (void *) &Bypass, (void **) &Bypass);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Exception"), OBFUSCATE("get_supportedRenderTargetCount"), 0), (void *) &Bypass, (void **) &Bypass);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Exception"), OBFUSCATE("get_maxTextureSize"), 0), (void *) &Bypass, (void **) &Bypass);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Exception"), OBFUSCATE("get_version"), 0), (void *) &Bypass, (void **) &Bypass);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Exception"), OBFUSCATE("get_targetFrameRate"), 0), (void *) &Bypass, (void **) &Bypass);
*/
// Bypass path + Hex

/*
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_UserGems"), 0), (void *)Level, (void **)&old_Level);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_ClanID"), 0), (void *)ClanID, (void **)&get_ClanID);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_UserCoins"), 0), (void *)UserCoins, (void **)&get_UserCoins);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_BadgeItemCount"), 0), (void *)UserCoins, (void **)&get_UserCoins);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_EmulatorScore"), 0), (void *)UserCoins, (void **)&get_UserCoins);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_BadgeItemId"), 0), (void *)UserCoins, (void **)&get_UserCoins);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_UserExp"), 0), (void *)UserCoins, (void **)&get_UserCoins);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_AgeState"), 0), (void *)UserCoins, (void **)&get_UserCoins);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_UserLevel"), 0), (void *)UserCoins, (void **)&get_UserCoins);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_PaidLevel"), 0), (void *)UserCoins, (void **)&get_UserCoins);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_Role"), 0), (void *)UserCoins, (void **)&get_UserCoins);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_NeedShowLevelUpWindow"), 0), (void *)UserCoins, (void **)&get_UserCoins);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_CloseLevelUpWindow"), 0), (void *)UserCoins, (void **)&get_UserCoins);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("GetUserGameDays"), 0), (void *)UserCoins, (void **)&get_UserCoins);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_CsRank"), 0), (void *)UserCoins, (void **)&get_UserCoins);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_RecommendAvatarID"), 0), (void *)UserCoins, (void **)&get_UserCoins);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelLadderMatch"), OBFUSCATE("GetCurrentBRRankSort"), 0), (void *)UserCoins, (void **)&get_UserCoins);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_AccountID"), 0), (void *)AccountID, (void **)&get_AccountID);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_Nickname"), 0), (void *)AccountID, (void **)&get_AccountID);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_ClanName"), 0), (void *)AccountID, (void **)&get_AccountID);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_ClanName"), 0), (void *)AccountID, (void **)&get_AccountID);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_ClanName"), 0), (void *)AccountID, (void **)&get_AccountID);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIModelUser"), OBFUSCATE("get_ClanName"), 0), (void *)AccountID, (void **)&get_AccountID);*/
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player") , OBFUSCATE("UpdateBehavior"), 2), (void *) _LateUpdate, (void **) &LateUpdate);
DobbyHook(Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("PlayerAttributes"), OBFUSCATE("PCFAIFBHFFO"), 0), (void *)_Speed, (void **)&Speed);

   return nullptr;
}
__attribute__((constructor))
void lib_main() {
    cobeyuim();
    DobbyHook((void *) DobbySymbolResolver(OBFUSCATE("/system/lib/libandroid.so"), OBFUSCATE("ANativeWindow_getWidth")), (void *) _ANativeWindow_getWidth, (void **) &orig_ANativeWindow_getWidth);
    DobbyHook((void *) DobbySymbolResolver(OBFUSCATE("/system/lib/libandroid.so"), OBFUSCATE("ANativeWindow_getHeight")), (void *) _ANativeWindow_getHeight, (void **) &orig_ANativeWindow_getHeight);
    DobbyHook((void *) DobbySymbolResolver(OBFUSCATE("/system/lib/libandroid.so"), OBFUSCATE("AConfiguration_getDensity")), (void *) _AConfiguration_getDensity, (void **) &orig_AConfiguration_getDensity);
}
REGISTER_ZYGISK_MODULE(MyModule)
REGISTER_ZYGISK_COMPANION(companion_handler)
extern "C" {
    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
        jvm = vm;
        JNIEnv *env;
        if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
            return JNI_FALSE;
        }
        pthread_t ptid;
    pthread_create(&ptid, nullptr, hack_thread, nullptr);
        return JNI_VERSION_1_6;
    }
}
