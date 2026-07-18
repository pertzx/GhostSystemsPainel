#pragma once
#include <unistd.h>
#include <sys/mman.h>
#include <cstdint>
#include <cstring>
#include <vector>
#include <cassert>

class MiniMemoryPatch {
public:
    MiniMemoryPatch(void* target, const void* patchBytes, size_t size)
        : target_(target), size_(size)
    {
        assert(target != nullptr && patchBytes != nullptr && size > 0);

        original_.resize(size_);
        std::memcpy(original_.data(), target_, size_);
        patch_.resize(size_);
        std::memcpy(patch_.data(), patchBytes, size_);
    }

    bool apply()  { return patchMemory(patch_.data());   }
    bool restore(){ return patchMemory(original_.data()); }

private:
    void* target_;
    size_t size_;
    std::vector<uint8_t> original_;
    std::vector<uint8_t> patch_;

    bool patchMemory(const void* bytes)
    {
        uintptr_t pageSize  = sysconf(_SC_PAGESIZE);
        uintptr_t pageStart = (uintptr_t)target_ & ~(pageSize - 1);

        if (mprotect((void*)pageStart, pageSize,
                     PROT_READ | PROT_WRITE | PROT_EXEC) != 0)
            return false;

        std::memcpy(target_, bytes, size_);

        mprotect((void*)pageStart, pageSize,
                 PROT_READ | PROT_EXEC);

        __builtin___clear_cache((char*)target_,
                                (char*)target_ + size_);
        return true;
    }
};
