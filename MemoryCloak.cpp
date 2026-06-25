#include "MemoryCloak.h"
#include <fcntl.h>
#include <sys/mman.h>

namespace GhostSystems {

bool MemoryCloak::readMemory(pid_t pid, void* remoteAddr, void* localBuf, size_t size) {
    if (!isValidAddress(remoteAddr) || !localBuf || size == 0) {
        return false;
    }
    
    struct iovec local_iov;
    struct iovec remote_iov;
    
    local_iov.iov_base = localBuf;
    local_iov.iov_len = size;
    remote_iov.iov_base = remoteAddr;
    remote_iov.iov_len = size;
    
    ssize_t result = syscall(__NR_process_vm_readv, pid, &local_iov, 1, &remote_iov, 1, 0);
    
    if (result < 0) {
        LOGE("[MemoryCloak] readMemory falhou: %zd", result);
        return false;
    }
    
    return (size_t)result == size;
}

bool MemoryCloak::writeMemory(pid_t pid, void* remoteAddr, void* localBuf, size_t size) {
    if (!isValidAddress(remoteAddr) || !localBuf || size == 0) {
        return false;
    }
    
    struct iovec local_iov;
    struct iovec remote_iov;
    
    local_iov.iov_base = localBuf;
    local_iov.iov_len = size;
    remote_iov.iov_base = remoteAddr;
    remote_iov.iov_len = size;
    
    ssize_t result = syscall(__NR_process_vm_writev, pid, &local_iov, 1, &remote_iov, 1, 0);
    
    if (result < 0) {
        LOGE("[MemoryCloak] writeMemory falhou: %zd", result);
        return false;
    }
    
    return (size_t)result == size;
}

bool MemoryCloak::readSelf(void* addr, void* buf, size_t size) {
    return readMemory(getpid(), addr, buf, size);
}

bool MemoryCloak::writeSelf(void* addr, void* buf, size_t size) {
    return writeMemory(getpid(), addr, buf, size);
}

bool MemoryCloak::isValidAddress(void* addr) {
    if (!addr || addr == (void*)-1) {
        return false;
    }
    
    // Verifica se endereco esta em faixa valida (evita crash)
    uintptr_t ptr = (uintptr_t)addr;
    if (ptr < 0x1000 || ptr > 0x7FFFFFFFFFFF) {
        return false;
    }
    
    // Tenta verificar se pagina eh acessivel via mincore
    unsigned char vec[1];
    if (mincore(addr, 1, vec) != 0) {
        return false;
    }
    
    return true;
}

} // namespace GhostSystems
