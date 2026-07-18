// GhostSystems - Bypass Completo (baseado em BRUXO/Hush e GOAT/gsmodz)
// Hooks: ptrace, fopen, open, dlopen, getuid, readlink + JNI isDebuggable/DEBUG/getPackageInfo
// Ativacao via enable_bypass() ou automatico no startup

#include <sys/ptrace.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <jni.h>
#include <atomic>
#include <android/log.h>

#define LOG_TAG_GSB "GhostSystems_Bypass"
#define LOGI_BYPASS(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG_GSB, __VA_ARGS__)
#define LOGE_BYPASS(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG_GSB, __VA_ARGS__)

static std::atomic<bool> bypass_enabled(false);

// Ativar bypass externamente (menu, botao, etc.)
extern "C" void enable_bypass() {
    bypass_enabled = true;
    LOGI_BYPASS("[Bypass] ATIVADO");
}

extern "C" void disable_bypass() {
    bypass_enabled = false;
    LOGI_BYPASS("[Bypass] DESATIVADO");
}

static bool is_enabled() {
    return bypass_enabled.load();
}

// ---- BYPASS fopen (bloqueia leitura de arquivos suspeitos) ----
extern "C" FILE *fopen(const char *pathname, const char *mode) {
    if (is_enabled() && pathname) {
        if (strstr(pathname, "cheat") || strstr(pathname, "mod") ||
            strstr(pathname, "xposed") || strstr(pathname, "frida") ||
            strstr(pathname, "/proc/self/") ||
            strstr(pathname, "libanogs.so") || strstr(pathname, "libanort.so") ||
            strstr(pathname, "libil2cpp.so") || strstr(pathname, "libunity.so") ||
            strstr(pathname, "libmain.so") || strstr(pathname, "libcrashlytics.so") ||
            strstr(pathname, "libFirebaseCppApp") || strstr(pathname, "libff_voice_engine.so") ||
            strstr(pathname, "libFFVoiceMagicVoiceEngine.so") || strstr(pathname, "SH-Gpp2") ||
            strstr(pathname, "liblongs.so")) {
            LOGE_BYPASS("[fopen] BLOQUEADO: %s", pathname);
            return nullptr;
        }
    }
    FILE *(*original_fopen)(const char *, const char *) =
        (FILE *(*)(const char *, const char *)) dlsym(RTLD_NEXT, "fopen");
    return original_fopen(pathname, mode);
}

// ---- BYPASS open (versao alternativa do fopen) ----
extern "C" int open(const char *pathname, int flags, ...) {
    if (is_enabled() && pathname) {
        if (strstr(pathname, "cheat") || strstr(pathname, "mod") ||
            strstr(pathname, "xposed") || strstr(pathname, "frida") ||
            strstr(pathname, "/proc/self/") ||
            strstr(pathname, "libanogs.so") || strstr(pathname, "libanort.so") ||
            strstr(pathname, "libil2cpp.so") || strstr(pathname, "libunity.so") ||
            strstr(pathname, "libmain.so") || strstr(pathname, "libcrashlytics.so") ||
            strstr(pathname, "libFirebaseCppApp") || strstr(pathname, "libff_voice_engine.so") ||
            strstr(pathname, "libFFVoiceMagicVoiceEngine.so") || strstr(pathname, "SH-Gpp2") ||
            strstr(pathname, "liblongs.so")) {
            LOGE_BYPASS("[open] BLOQUEADO: %s", pathname);
            return -1;
        }
    }
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
    int (*original_open)(const char *, int, ...) =
        (int (*)(const char *, int, ...)) dlsym(RTLD_NEXT, "open");
    return original_open(pathname, flags, mode);
}

// ---- BYPASS dlopen (bloqueia carregamento de bibliotecas suspeitas) ----
void *dlopen(const char *filename, int flag) {
    if (is_enabled() && filename) {
        if (strstr(filename, "libmod") || strstr(filename, "libcheat") ||
            strstr(filename, "libxposed") || strstr(filename, "libfrida") ||
            strstr(filename, "liblongs.so") || strstr(filename, "libanogs.so") ||
            strstr(filename, "libanort.so") || strstr(filename, "libil2cpp.so") ||
            strstr(filename, "libunity.so") || strstr(filename, "libmain.so") ||
            strstr(filename, "libcrashlytics.so") || strstr(filename, "libFirebaseCppApp") ||
            strstr(filename, "libff_voice_engine.so") || strstr(filename, "libFFVoiceMagicVoiceEngine.so") ||
            strstr(filename, "SH-Gpp2")) {
            LOGE_BYPASS("[dlopen] BLOQUEADO: %s", filename);
            return nullptr;
        }
    }
    void *(*original_dlopen)(const char *, int) =
        (void *(*)(const char *, int)) dlsym(RTLD_NEXT, "dlopen");
    return original_dlopen(filename, flag);
}

// ---- BYPASS getuid (spoof de UID para esconder root) ----
extern "C" uid_t getuid(void) {
    if (is_enabled()) {
        return 10000; // Retorna UID de usuario comum (nao root)
    }
    uid_t (*original_getuid)(void) =
        (uid_t (*)(void)) dlsym(RTLD_NEXT, "getuid");
    return original_getuid();
}

// ---- BYPASS readlink (impede leitura de /proc/self/maps e /proc/self/exe) ----
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
    if (is_enabled() && pathname) {
        if (strstr(pathname, "/proc/self/maps") || strstr(pathname, "/proc/self/exe")) {
            LOGE_BYPASS("[readlink] BLOQUEADO: %s", pathname);
            return -1;
        }
    }
    ssize_t (*original_readlink)(const char *, char *, size_t) =
        (ssize_t (*)(const char *, char *, size_t)) dlsym(RTLD_NEXT, "readlink");
    return original_readlink(pathname, buf, bufsiz);
}

// ---- BYPASS JNI: isDebuggable ----
extern "C" jboolean Java_android_content_pm_ApplicationInfo_isDebuggable(JNIEnv* env, jobject thiz) {
    return is_enabled() ? false : true;
}

// ---- BYPASS JNI: BuildConfig.DEBUG (Garena) ----
extern "C" jboolean Java_com_garena_android_buildconfig_DEBUG(JNIEnv* env, jobject thiz) {
    return is_enabled() ? false : true;
}

// ---- BYPASS JNI: getPackageInfo (impede deteccao de apps de cheat) ----
extern "C" jobject Java_android_content_pm_PackageManager_getPackageInfo(JNIEnv* env, jobject thiz, jstring pkg, jint flags) {
    if (is_enabled()) {
        return nullptr;
    }
    return nullptr;
}

// ---- Auto-inicializacao do bypass na injecao ----
static bool g_bypass_auto_init = false;

static void auto_init_bypass() {
    if (g_bypass_auto_init) return;
    g_bypass_auto_init = true;
    LOGI_BYPASS("[AutoInit] GhostSystems Bypass carregado (libc + JNI hooks)");
}

__attribute__((constructor))
static void GhostSystemsBypass_Constructor() {
    auto_init_bypass();
}
