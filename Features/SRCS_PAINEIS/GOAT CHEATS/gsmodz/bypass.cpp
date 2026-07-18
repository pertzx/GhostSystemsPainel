
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

std::atomic<bool> bypass_enabled(false);

// Chamada para ativar o bypass externamente
extern "C" void enable_bypass() {
    bypass_enabled = true;
}

// Função de controle
bool is_enabled() {
    return bypass_enabled.load();
}

// Versão segura do ptrace
extern "C" int ptrace(int request, pid_t pid, void *addr, void *data) {
    if (is_enabled()) return -1;
    int (*orig)(int, pid_t, void*, void*) = (int(*)(int, pid_t, void*, void*)) dlsym(RTLD_NEXT, "ptrace");
    return orig(request, pid, addr, data);
}

// Bloqueios condicionais
extern "C" FILE *fopen(const char *pathname, const char *mode) {
    if (is_enabled() && pathname &&
        (strstr(pathname, "cheat") || strstr(pathname, "mod") || strstr(pathname, "/proc/self/") || strstr(pathname, "xposed") || strstr(pathname, "frida"))) {
        return nullptr;
    }
    FILE *(*original_fopen)(const char *, const char *) = (FILE *(*)(const char *, const char *)) dlsym(RTLD_NEXT, "fopen");
    return original_fopen(pathname, mode);
}

extern "C" int open(const char *pathname, int flags, ...) {
    if (is_enabled() && pathname &&
        (strstr(pathname, "frida") || strstr(pathname, "xposed") || strstr(pathname, "/proc/self/"))) {
        return -1;
    }
    int (*original_open)(const char *, int, ...) = (int (*)(const char *, int, ...)) dlsym(RTLD_NEXT, "open");
    return original_open(pathname, flags);
}

void *dlopen(const char *filename, int flag) {
    if (is_enabled() && filename &&
        (strstr(filename, "libmod") || strstr(filename, "libcheat") || strstr(filename, "libxposed") || strstr(filename, "libfrida"))) {
        return nullptr;
    }
    void *(*original_dlopen)(const char *, int) = (void *(*)(const char *, int)) dlsym(RTLD_NEXT, "dlopen");
    return original_dlopen(filename, flag);
}

extern "C" uid_t getuid(void) {
    if (is_enabled()) return 10000;
    uid_t (*original_getuid)(void) = (uid_t (*)(void)) dlsym(RTLD_NEXT, "getuid");
    return original_getuid();
}

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
    if (is_enabled() && pathname && (strstr(pathname, "/proc/self/exe") || strstr(pathname, "/proc/self/maps"))) {
        return -1;
    }
    ssize_t (*original_readlink)(const char *, char *, size_t) = (ssize_t (*)(const char *, char *, size_t)) dlsym(RTLD_NEXT, "readlink");
    return original_readlink(pathname, buf, bufsiz);
}

// Interceptar verificações Android
extern "C" jboolean Java_android_content_pm_ApplicationInfo_isDebuggable(JNIEnv* env, jobject thiz) {
    return is_enabled() ? false : true;
}

extern "C" jboolean Java_com_garena_android_buildconfig_DEBUG(JNIEnv* env, jobject thiz) {
    return is_enabled() ? false : true;
}

extern "C" jobject Java_android_content_pm_PackageManager_getPackageInfo(JNIEnv* env, jobject thiz, jstring pkg, jint flags) {
    return is_enabled() ? nullptr : nullptr;
}
