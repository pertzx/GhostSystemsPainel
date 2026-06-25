#pragma once

#include <sys/uio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <android/log.h>

#define LOG_TAG "GhostSystems"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace GhostSystems {

// Memory Cloak - Leitura/escrita via kernel (invisivel para hooks em libc)
class MemoryCloak {
public:
    // Le memoria de outro processo via process_vm_readv (syscall direto)
    static bool readMemory(pid_t pid, void* remoteAddr, void* localBuf, size_t size);
    
    // Escreve memoria em outro processo via process_vm_writev (syscall direto)
    static bool writeMemory(pid_t pid, void* remoteAddr, void* localBuf, size_t size);
    
    // Le memoria do proprio processo (mais seguro que memcpy)
    static bool readSelf(void* addr, void* buf, size_t size);
    
    // Escreve na memoria do proprio processo
    static bool writeSelf(void* addr, void* buf, size_t size);
    
    // Verifica se endereco eh valido antes de ler/escrever
    static bool isValidAddress(void* addr);
    
    // Template para leitura tipada
    template<typename T>
    static T read(void* addr) {
        T value = T();
        if (isValidAddress(addr)) {
            readSelf(addr, &value, sizeof(T));
        }
        return value;
    }
    
    // Template para escrita tipada
    template<typename T>
    static bool write(void* addr, T value) {
        if (isValidAddress(addr)) {
            return writeSelf(addr, &value, sizeof(T));
        }
        return false;
    }
};

} // namespace GhostSystems
