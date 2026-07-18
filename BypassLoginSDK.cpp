/*
 * GhostSystems - BypassLoginSDK v4.0 (Safe + Funcional)
 *
 * Estrategia:
 *  - __attribute__((constructor)) roda na injecao da lib
 *  - Pre-popula um memfd com /proc/self/maps filtrado (sem libghostsystems.so)
 *  - Inline hook via A64HookFunction em libc.so: open(), openat(), fopen()
 *    (page size dinâmico corrigido em And64InlineHook.cpp)
 *  - Quando o anti-cheat le /proc/self/maps, recebe o fd filtrado
 *  - Logs visiveis no painel ImGui + logcat
 */
#include "BypassLoginSDK.h"
#include <android/log.h>
#include <dlfcn.h>
#include <string.h>
#include <link.h>
#include <elf.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdarg.h>
#include <string>
#include <deque>
#include <mutex>
#include <vector>
#include <chrono>
#include <thread>

namespace GhostSystems {

#define LOG_TAG_BS "Ghost_Bypass"

static std::deque<BypassLogEntry> g_Logs;
static std::mutex g_LogsMutex;
static constexpr int MAX_LOGS = 100;

static void LogEntry(const char* hook, const char* action, const char* details) {
    std::lock_guard<std::mutex> lock(g_LogsMutex);
    BypassLogEntry e;
    e.hookName = hook;
    e.action = action;
    e.details = details;
    e.color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    char buf[32];
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&t));
    char final[64];
    snprintf(final, sizeof(final), "%s.%03d", buf, (int)ms.count());
    e.timestamp = final;
    g_Logs.push_back(e);
    if (g_Logs.size() > MAX_LOGS) g_Logs.pop_front();
}

// Not real callbacks, still needed interface
static std::vector<HookInfo> g_HookList;

// Module to hide
static const char* HIDE_MODULE = "libghostsystems.so";

// Forked file descriptors for filtered /proc/self/maps
static int g_maps_fd = -1;
static std::mutex g_maps_fd_mutex;

/*
 * build_filtered_maps_content()
 * Reads original /proc/self/maps via raw syscall (avoid our own hook recursion)
 * Returns string with lines that do NOT contain HIDE_MODULE.
 */
static std::string build_filtered_maps_content() {
    std::string filtered;
    int orig_fd = (int)syscall(__NR_openat, AT_FDCWD, "/proc/self/maps", O_RDONLY, 0);
    if (orig_fd < 0) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG_BS, "Falha openat /proc/self/maps: errno=%d", errno);
        return filtered;
    }

    std::string raw;
    char buf[8192];
    int n;
    while ((n = (int)read(orig_fd, buf, sizeof(buf))) > 0) {
        raw.append(buf, n);
    }
    close(orig_fd);

    size_t pos = 0;
    while (pos < raw.size()) {
        size_t eol = raw.find('\n', pos);
        if (eol == std::string::npos) eol = raw.size();
        std::string line = raw.substr(pos, eol - pos);
        if (line.find(HIDE_MODULE) == std::string::npos) {
            filtered.append(line);
            filtered.push_back('\n');
        }
        pos = eol + 1;
    }
    return filtered;
}

/*
 * rebuild_filtered_maps_fd()
 * (Re)creates the memfd with fresh filtered content.
 * Thread-safe. Safe to call multiple times (replaces fd).
 */
static int rebuild_filtered_maps_fd() {
    std::lock_guard<std::mutex> lock(g_maps_fd_mutex);

    std::string content = build_filtered_maps_content();
    if (content.empty()) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG_BS, "filtered content vazio");
        return -1;
    }

    int memfd = (int)syscall(__NR_memfd_create, "maps", 0);
    if (memfd < 0) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG_BS, "memfd_create falhou: errno=%d", errno);
        return -1;
    }

    ssize_t total = 0;
    const char* p = content.c_str();
    size_t remaining = content.size();
    while (remaining > 0) {
        ssize_t w = write(memfd, p + total, remaining);
        if (w <= 0) break;
        total += w;
        remaining -= (size_t)w;
    }
    lseek(memfd, 0, SEEK_SET);

    if (g_maps_fd >= 0) close(g_maps_fd);
    g_maps_fd = memfd;

    __android_log_print(ANDROID_LOG_INFO, LOG_TAG_BS, "memfd recriado: fd=%d size=%zd", memfd, content.size());
    return memfd;
}

static int get_or_build_maps_fd() {
    {
        std::lock_guard<std::mutex> lock(g_maps_fd_mutex);
        if (g_maps_fd >= 0) {
            lseek(g_maps_fd, 0, SEEK_SET);
            return g_maps_fd;
        }
    }
    return rebuild_filtered_maps_fd();
}

//============================================================================
// HOOKS - libc open / openat / fopen
//============================================================================

using real_open_fn = int(*)(const char*, int, ...);
using real_openat_fn = int(*)(int, const char*, int, ...);
using real_fopen_fn = FILE* (*)(const char*, const char*);

static real_open_fn   rl_open   = nullptr;
static real_openat_fn rl_openat = nullptr;
static real_fopen_fn  rl_fopen  = nullptr;

static void* trampoline_open   = nullptr;
static void* trampoline_openat = nullptr;
static void* trampoline_fopen  = nullptr;

static int my_open(const char* path, int flags, ...) {
    if (path && (strstr(path, "/proc/self/maps") || strstr(path, "/proc/self/smaps"))) {
        LogEntry("open", "FILTRADO", path);
        int fd = get_or_build_maps_fd();
        if (fd >= 0) return fd;
    }
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap; va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
    return trampoline_open ? ((real_open_fn)trampoline_open)(path, flags, mode)
                          : (int)syscall(__NR_openat, AT_FDCWD, path, flags, mode);
}

static int my_openat(int dirfd, const char* path, int flags, ...) {
    if (path && (strstr(path, "/proc/self/maps") || strstr(path, "/proc/self/smaps"))) {
        LogEntry("openat", "FILTRADO", path);
        int fd = get_or_build_maps_fd();
        if (fd >= 0) return fd;
    }
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list ap; va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
    return trampoline_openat ? ((real_openat_fn)trampoline_openat)(dirfd, path, flags, mode)
                           : (int)syscall(__NR_openat, dirfd, path, flags, mode);
}

static FILE* my_fopen(const char* path, const char* mode) {
    if (path && (strstr(path, "/proc/self/maps") || strstr(path, "/proc/self/smaps"))) {
        LogEntry("fopen", "FILTRADO", path);
        int fd = get_or_build_maps_fd();
        if (fd >= 0) {
            FILE* f = fdopen(fd, mode);
            if (f) return f;
            close(fd);
        }
    }
    return trampoline_fopen ? ((real_fopen_fn)trampoline_fopen)(path, mode) : nullptr;
}

//============================================================================
// APLICA HOOKS via A64HookFunction (inline hook, page size corrigido)
//============================================================================

static bool install_inline_hook(void* target, void* hook, void** orig, const char* name) {
    if (!target || !hook) return false;
    A64HookFunction(target, hook, orig);
    bool ok = (*orig != nullptr);
    if (ok) {
        __android_log_print(ANDROID_LOG_INFO, LOG_TAG_BS, "[Hook] %s inline OK (orig=%p)", name, *orig);
        LogEntry(name, "APLICADO", "inline hook");
    } else {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG_BS, "[Hook] %s falhou", name);
        LogEntry(name, "ERRO", "A64HookFunction retornou orig=null");
    }
    return ok;
}

static bool apply_libc_hooks() {
    void* libc = dlopen("libc.so", RTLD_NOW);
    if (!libc) {
        LogEntry("SYSTEM", "ERRO", "libc.so nao abriu");
        return false;
    }

    rl_open   = (real_open_fn)  dlsym(libc, "open");
    rl_openat = (real_openat_fn)dlsym(libc, "openat");
    rl_fopen  = (real_fopen_fn) dlsym(libc, "fopen");
    dlclose(libc);

    int ok = 0;
    if (rl_open)   ok += install_inline_hook((void*)rl_open,   (void*)my_open,   &trampoline_open,   "open")   ? 1 : 0;
    if (rl_openat) ok += install_inline_hook((void*)rl_openat, (void*)my_openat, &trampoline_openat, "openat") ? 1 : 0;
    if (rl_fopen)  ok += install_inline_hook((void*)rl_fopen,  (void*)my_fopen,  &trampoline_fopen,  "fopen")  ? 1 : 0;

    __android_log_print(ANDROID_LOG_INFO, LOG_TAG_BS, "[v4.0] Inline hooks libc: %d/3 aplicados", ok);
    LogEntry("SYSTEM", "HOOKS", "libc hooks aplicados");
    return ok > 0;
}

//============================================================================
// CONSTRUCTOR - roda automaticamente na injecao da lib
//============================================================================

static bool g_bypass_auto_initialized = false;

static void auto_init_bypass() {
    if (g_bypass_auto_initialized) return;
    g_bypass_auto_initialized = true;

    __android_log_print(ANDROID_LOG_INFO, LOG_TAG_BS, "[v4.0] auto_init_bypass: criando fd filtrado");
    LogEntry("SYSTEM", "INIT", "auto_init v4.0");

    int fd = rebuild_filtered_maps_fd();
    if (fd < 0) {
        LogEntry("SYSTEM", "ERRO", "memfd inicial falhou");
    } else {
        LogEntry("SYSTEM", "OK", "memfd pronto (hooks deferidos p/ Initialize)");
    }
    // NOTA: hooks em libc sao aplicados via Initialize() apos A64 estaticamente pronto
}

__attribute__((constructor))
static void BypassLoginSDK_Constructor() {
    auto_init_bypass();
}

//============================================================================
// SINGLETON & PUBLIC INTERFACE
//============================================================================

BypassLoginSDK& BypassLoginSDK::Instance() {
    static BypassLoginSDK inst;
    return inst;
}

BypassLoginSDK::BypassLoginSDK() {}
BypassLoginSDK::~BypassLoginSDK() {}

const std::deque<BypassLogEntry>& BypassLoginSDK::GetLogs() { return g_Logs; }

void BypassLoginSDK::ClearLogs() {
    std::lock_guard<std::mutex> lock(g_LogsMutex);
    g_Logs.clear();
}

std::mutex& BypassLoginSDK::GetLogsMutex() { return g_LogsMutex; }

bool BypassLoginSDK::Initialize(const LoginSDKConfig& cfg) {
    (void)cfg;
    if (initialized) return true;
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG_BS, "[v4.0] Initialize: aplicando hooks libc");
    LogEntry("SYSTEM", "INIT", "Initialize v4.0");

    // Aplica inline hooks no libc (A64 estatico ja pronto neste ponto)
    hooksActive = apply_libc_hooks();
    initialized = true;
    return hooksActive;
}

bool BypassLoginSDK::ApplyHooks() {
    if (hooksActive) {
        LogEntry("SYSTEM", "INFO", "hooks ja ativos, refresh fd");
        rebuild_filtered_maps_fd();
        return true;
    }
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG_BS, "[v4.0] ApplyHooks (aplicando agora)");
    LogEntry("SYSTEM", "REAPPLY", "ApplyHooks manual");

    rebuild_filtered_maps_fd();
    hooksActive = apply_libc_hooks();
    return hooksActive;
}

void BypassLoginSDK::RemoveHooks() {
    for (auto& h : hookList) h.isApplied = false;
    hooksActive = false;
    LogEntry("SYSTEM", "REMOVE", "hooks marcados removidos");
}

void BypassLoginSDK::UpdateConfig(const LoginSDKConfig& newConfig) {
    config = newConfig;
}

bool BypassLoginSDK::HasAnyHookApplied() const {
    for (const auto& h : hookList) if (h.isApplied) return true;
    return false;
}

bool BypassLoginSDK::SetupHook(const std::string&, const std::string&, void*, void*, void**, bool) {
    return true;
}

void* BypassLoginSDK::ResolveAddress(uintptr_t rva) const {
    if (!baseAddress) return nullptr;
    return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(baseAddress) + rva);
}

// Stubs
void* BypassLoginSDK::Hook_LoginInit(void*, int) { return nullptr; }
bool BypassLoginSDK::Hook_VerifySession(void*) { return true; }
const char* BypassLoginSDK::Hook_GetDeviceID() { return "fake-id"; }
bool BypassLoginSDK::Hook_CheckRoot() { return false; }
void BypassLoginSDK::Hook_SendTelemetry(void*, void*) {}
void BypassLoginSDK::Hook_AntiCheatInit(void*) {}
bool BypassLoginSDK::Hook_ValidateToken(void*, const char*) { return true; }
int BypassLoginSDK::Hook_GetLoginType() { return 0; }
void BypassLoginSDK::Hook_IntegrityCheck(void*) {}

} // namespace GhostSystems
