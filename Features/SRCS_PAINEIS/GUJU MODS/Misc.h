
#include "Nigga/Tools/Dobby/dobby.h"
#include "KittyMemory/KittyMemory.h"
#include "KittyMemory/KittyScanner.h"
#include "KittyMemory/MemoryPatch.h"

using KittyMemory::ProcMap;
using KittyScanner::RegisterNativeFn;



void hook(void *offset, void* ptr, void **orig)
{
    DobbyHook(offset, ptr, orig);
}
uintptr_t string2Offset(const char *c) {
    int base = 16;
    // See if this function catches all possibilities.
    // If it doesn't, the function would have to be amended
    // whenever you add a combination of architecture and
    // compiler that is not yet addressed.
    static_assert(sizeof(uintptr_t) == sizeof(unsigned long)
                  || sizeof(uintptr_t) == sizeof(unsigned long long),
                  "Please add string to handle conversion for this architecture.");

    // Now choose the correct function ...
    if (sizeof(uintptr_t) == sizeof(unsigned long)) {
        return strtoul(c, nullptr, base);
    }

    // All other options exhausted, sizeof(uintptr_t) == sizeof(unsigned long long))
    return strtoull(c, nullptr, base);
}

#define HOOK_IL2CPP(offset, ptr, orig) hook((void *)(_Libil2cpp.startAddress + string2Offset(OBFUSCATE(offset))), (void *)ptr, (void **)&orig)
#define HOOK_ANOGS(offset, ptr, orig) hook((void *)(_Libanogs.startAddress + string2Offset(OBFUSCATE(offset))), (void *)ptr, (void **)&orig)
