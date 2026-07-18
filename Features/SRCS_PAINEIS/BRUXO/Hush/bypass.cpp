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

// Variável global para controlar todos os bypasses
std::atomic<bool> bypass_enabled(false);
std::atomic<bool> offset_bypass_enabled(false);

// Função para ativar os bypasses externamente (ex: por um menu)
extern "C" void enable_bypass() {
    bypass_enabled = true;
}

extern "C" void enable_offset_bypass() {
    offset_bypass_enabled = true;
}

// Função interna para verificar se os bypasses estão ativados
bool is_enabled() {
    return bypass_enabled.load();
}

bool is_offset_bypass_enabled() {
 return offset_bypass_enabled.load();
}

uintptr_t get_lib_base_addr(const char* libName);

// --- BYPASSES DE FUNÇÕES NATIVAS ---

// Bypass para fopen (impede a abertura de arquivos de log, cheat, e bibliotecas de anticheat)
extern "C" FILE *fopen(const char *pathname, const char *mode) {
    if (is_enabled() && pathname &&
        (strstr(pathname, "cheat") || strstr(pathname, "mod") || strstr(pathname, "xposed") || strstr(pathname, "frida") ||
         strstr(pathname, "/proc/self") || // Bloqueia acesso geral ao diretório do processo
         // --- Início dos bypasses de biblioteca baseados nos seus logs ---
         strstr(pathname, "libanogs.so") ||
         strstr(pathname, "libanort.so") ||
         strstr(pathname, "libil2cpp.so") ||
         strstr(pathname, "libunity.so") ||
         strstr(pathname, "libmain.so") ||
         strstr(pathname, "libcrashlytics.so") ||
         strstr(pathname, "libcrashlytics-common.so") ||
         strstr(pathname, "libFirebaseCppApp-11_8_1.so") ||
         strstr(pathname, "libff_voice_engine.so") ||
         strstr(pathname, "libFFVoiceMagicVoiceEngine.so") ||
         strstr(pathname, "SH-Gpp2") ||
         strstr(pathname, "liblongs.so")
         // --- Fim dos bypasses de biblioteca ---
        )) {
        return nullptr; // Simula que o arquivo não existe
    }
    FILE *(*original_fopen)(const char *, const char *) = (FILE *(*)(const char *, const char *)) dlsym(RTLD_NEXT, "fopen");
    return original_fopen(pathname, mode);
}

// Bypass para open (versão alternativa do fopen com a mesma lógica)
extern "C" int open(const char *pathname, int flags, ...) {
    if (is_enabled() && pathname &&
        (strstr(pathname, "cheat") || strstr(pathname, "mod") || strstr(pathname, "xposed") || strstr(pathname, "frida") ||
         strstr(pathname, "/proc/self") ||
         // --- Início dos bypasses de biblioteca baseados nos seus logs ---
         strstr(pathname, "libanogs.so") ||
         strstr(pathname, "libanort.so") ||
         strstr(pathname, "libil2cpp.so") ||
         strstr(pathname, "libunity.so") ||
         strstr(pathname, "libmain.so") ||
         strstr(pathname, "libcrashlytics.so") ||
         strstr(pathname, "libcrashlytics-common.so") ||
         strstr(pathname, "libFirebaseCppApp-11_8_1.so") ||
         strstr(pathname, "libff_voice_engine.so") ||
         strstr(pathname, "libFFVoiceMagicVoiceEngine.so") ||
         strstr(pathname, "SH-Gpp2") ||
         strstr(pathname, "liblongs.so")
         // --- Fim dos bypasses de biblioteca ---
        )) {
        return -1; // Simula erro ao abrir
    }
    int (*original_open)(const char *, int, ...) = (int (*)(const char *, int, ...)) dlsym(RTLD_NEXT, "open");
    return original_open(pathname, flags);
}

// Bypass para dlopen (impede o carregamento de bibliotecas de cheat)
void *dlopen(const char *filename, int flag) {
    if (is_enabled() && filename &&
        (strstr(filename, "libmod") || strstr(filename, "libcheat") || strstr(filename, "libxposed") || strstr(filename, "libfrida") ||
         strstr(filename, "liblongs.so") ||
         strstr(filename, "libanogs.so") ||
         strstr(filename, "libanort.so") ||
         strstr(filename, "libil2cpp.so") ||
         strstr(filename, "libunity.so") ||
         strstr(filename, "libmain.so") ||
         strstr(filename, "libcrashlytics.so") ||
         strstr(filename, "libcrashlytics-common.so") ||
         strstr(filename, "libFirebaseCppApp-11_8_1.so") ||
         strstr(filename, "libff_voice_engine.so") ||
         strstr(filename, "libFFVoiceMagicVoiceEngine.so") ||
         strstr(filename, "SH-Gpp2"))
        ) {
        return nullptr;
    }
    void *(*original_dlopen)(const char *, int) = (void *(*)(const char *, int)) dlsym(RTLD_NEXT, "dlopen");
    return original_dlopen(filename, flag);
}

// Bypass para getuid (esconde o root)
extern "C" uid_t getuid(void) {
    if (is_enabled()) {
        return 10000; // Retorna um ID de usuário comum
    }
    uid_t (*original_getuid)(void) = (uid_t (*)(void)) dlsym(RTLD_NEXT, "getuid");
    return original_getuid();
}

// Bypass para readlink (impede a inspeção de memória e do executável)
ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
    if (is_enabled() && pathname &&
       (strstr(pathname, "/proc/self/maps") || // Impede o jogo de ler os próprios mapas de memória
        strstr(pathname, "/proc/self/exe"))) {
        return -1; // Retorna um erro, impedindo a leitura dos endereços
    }
    ssize_t (*original_readlink)(const char *, char *, size_t) = (ssize_t (*)(const char *, char *, size_t)) dlsym(RTLD_NEXT, "readlink");
    return original_readlink(pathname, buf, bufsiz);
}


// --- BYPASSES DE FUNÇÕES JNI (ANDROID) ---

// Bypass para isDebuggable (esconde o modo de depuração)
extern "C" jboolean Java_android_content_pm_ApplicationInfo_isDebuggable(JNIEnv* env, jobject thiz) {
    return is_enabled() ? false : true;
}

// Bypass para a flag DEBUG do Garena
extern "C" jboolean Java_com_garena_android_buildconfig_DEBUG(JNIEnv* env, jobject thiz) {
    return is_enabled() ? false : true;
}

// Bypass para getPackageInfo (pode ser usado para detectar apps de cheat instalados)
extern "C" jobject Java_android_content_pm_PackageManager_getPackageInfo(JNIEnv* env, jobject thiz, jstring pkg, jint flags) {
    if (is_enabled()) {
        return nullptr;
    }
    // Apenas retornamos nulo para bloquear a verificação
    return nullptr;
}

// --- BYPASSES DE OFFSETS (INLINE HOOKING) ---

// Estrutura para armazenar informações do hook
struct HookInfo {
    const char* libName;
    uintptr_t offset;
    void* original_func;
    void* bypass_func;
};

// Função de bypass genérica (padrão seguro)
// Esta função pode ser adaptada para retornar valores específicos ou manipular argumentos
// Por enquanto, ela simplesmente não faz nada, neutralizando a função original.
void generic_bypass_func() {
    // Exemplo: se a função original retornasse um booleano, poderíamos retornar false
    // return false;
}

// Lista de todos os hooks a serem aplicados
HookInfo hooks[] = {
    // Offsets da libanogs.so
    { "libanogs.so", 0x21C, nullptr, (void*)generic_bypass_func },
    { "libanogs.so", 0x994, nullptr, (void*)generic_bypass_func },
    { "libanogs.so", 0x584f24, nullptr, (void*)generic_bypass_func },
    { "libanogs.so", 0x592f40, nullptr, (void*)generic_bypass_func },
    { "libanogs.so", 0x597074, nullptr, (void*)generic_bypass_func },

    // Offsets da libil2cpp.so
    { "libil2cpp.so", 0x7663b8, nullptr, (void*)generic_bypass_func },

    // Offsets da libunity.so
    { "libunity.so", 0x8c6aa4, nullptr, (void*)generic_bypass_func },
    { "libunity.so", 0x8c6aa8, nullptr, (void*)generic_bypass_func },
    { "libunity.so", 0x8d9de4, nullptr, (void*)generic_bypass_func },
    { "libunity.so", 0x8d9eac, nullptr, (void*)generic_bypass_func },
    { "libunity.so", 0x8da12c, nullptr, (void*)generic_bypass_func },

    // Offsets da libFirebaseCppApp-11_8_1.so
    { "libFirebaseCppApp-11_8_1.so", 0x3ff874, nullptr, (void*)generic_bypass_func },
    { "libFirebaseCppApp-11_8_1.so", 0x400c2c, nullptr, (void*)generic_bypass_func },
    { "libFirebaseCppApp-11_8_1.so", 0x4015fc, nullptr, (void*)generic_bypass_func },

    // Offsets da libcrashlytics-common.so
    { "libcrashlytics-common.so", 0x20dc0, nullptr, (void*)generic_bypass_func },

    // Offsets da SH-Gpp2
    { "SH-Gpp2", 0xa6d0, nullptr, (void*)generic_bypass_func },

    // Offsets da libFFVoiceMagicVoiceEngine.so
    { "libFFVoiceMagicVoiceEngine.so", 0x17c9b0, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x17c9b8, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x17cb10, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x17cb14, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x17cb18, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x17cb20, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x17cb24, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x17cb28, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d5b3c, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7670, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7674, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7678, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d767c, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7680, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7684, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7688, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d768c, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7690, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7694, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d76ac, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d76cc, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d76ec, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d76f0, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d76f4, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d76f8, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d76fc, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7700, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7704, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7708, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d770c, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7710, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7714, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7718, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d771c, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7720, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7724, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7728, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d772c, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7730, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7734, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7738, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d773c, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7740, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7744, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7748, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d774c, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7750, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7754, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7758, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d775c, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7760, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7764, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7768, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d776c, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7c30, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1d7c34, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1da4e4, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1da590, nullptr, (void*)generic_bypass_func },
    { "libFFVoiceMagicVoiceEngine.so", 0x1da5a8, nullptr, (void*)generic_bypass_func },

    // Offsets da libff_voice_engine.so
    { "libff_voice_engine.so", 0x255b00, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x255b08, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x255c60, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x255c64, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x255c68, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x255c70, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x255c74, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x255c78, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b15d0, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b15d4, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b15d8, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b15dc, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b15e0, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b15e4, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b15e8, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b15ec, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b15f0, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b15f4, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b160c, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b162c, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b164c, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b1650, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b1654, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b1658, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b165c, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b1660, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b1664, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b1668, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b166c, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b1670, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b1674, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b1678, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b167c, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b1680, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b1684, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b1688, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b168c, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b1690, nullptr, (void*)generic_bypass_func },
    { "libff_voice_engine.so", 0x2b1694, nullptr, (void*)generic_bypass_func },
};

// Função para aplicar todos os hooks
void apply_hooks() {
    if (!is_offset_bypass_enabled()) return;

    for (auto& hook : hooks) {
        // Encontra o endereço base da biblioteca
        uintptr_t base_addr = get_lib_base_addr(hook.libName);
        if (base_addr == 0) continue; // Biblioteca não encontrada

        // Calcula o endereço absoluto da função
        uintptr_t absolute_addr = base_addr + hook.offset;

        // TODO: Substituir por uma biblioteca de hooking real (ex: Dobby, Substrate)
        // Exemplo conceitual:
        // DobbyHook((void*)absolute_addr, hook.bypass_func, &hook.original_func);
        // LOGD("Hook aplicado em %s + 0x%lx", hook.libName, hook.offset);
    }
}

// Função para obter o endereço base de uma biblioteca (exemplo simplificado)
uintptr_t get_lib_base_addr(const char* libName) {
    uintptr_t base_addr = 0;
    char line[512];
    FILE* maps = fopen("/proc/self/maps", "r");
    if (maps) {
        while (fgets(line, sizeof(line), maps)) {
            if (strstr(line, libName)) {
                base_addr = (uintptr_t)strtoul(line, NULL, 16);
                break;
            }
        }
        fclose(maps);
    }
    return base_addr;
}
