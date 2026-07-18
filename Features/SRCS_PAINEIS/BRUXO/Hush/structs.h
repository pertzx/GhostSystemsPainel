#pragma once
#include <cstdint>
#include <codecvt>
#include <locale>
#include <cmath>

#define getRealOffset(offset) AgetAbsoluteAddress("libil2cpp.so",offset)
static uintptr_t libBase;
bool isGameLibLoaded = false;

long AfindLibrary(const char *library) {
    char filename[0xFF] = {0},
    buffer[1024] = {0};
    FILE *fp = NULL;
    long address = 0;
    sprintf(filename, OBFUSCATE("/proc/self/maps"));
    fp = fopen(filename, OBFUSCATE("rt"));
    if (fp == NULL) {
        perror(OBFUSCATE("fopen"));
        goto done;
    }
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strstr(buffer, library)) {
            address = (long) strtoul(buffer, NULL, 16);
            goto done;
        }
    }
    done:
    if (fp) {
        fclose(fp);
    }
    return address;
}

long AClibBase;
long AgetAbsoluteAddress(const char* libraryName, long relativeAddr) {
    if (AClibBase == 0) {
        AClibBase = AfindLibrary(libraryName);
        if (AClibBase == 0) {
            AClibBase = 0;
        }
    }
    return AClibBase + relativeAddr;
}

__attribute__((visibility("hidden")))
uintptr_t string2Offset(const char* s) {
    using conv_fn_t = unsigned long (*)(const char*, char**, int);
    conv_fn_t conv_fn = reinterpret_cast<conv_fn_t>(dlsym(RTLD_DEFAULT, OBFUSCATE("strtoul")));
    if constexpr (sizeof(uintptr_t) == sizeof(unsigned long)) {
        return conv_fn(s, nullptr, 16);
    } else {
        using conv64_fn_t = unsigned long long (*)(const char*, char**, int);
        auto conv64_fn = reinterpret_cast<conv64_fn_t>(dlsym(RTLD_DEFAULT, OBFUSCATE("strtoull")));
        return conv64_fn(s, nullptr, 16);
    }
}


uintptr_t basePtr12,basePtr13,basePtr14,basePtr15;

__attribute__((visibility("hidden")))
auto LocalizarInderecoBase(const char* lib)
{
    uintptr_t InderecoBase = 0;
    char line[1024];
    char filename[0xFF] = {0};
    sprintf(filename,OBFUSCATE("/proc/self/maps"));
    FILE* fp = fopen(filename, OBFUSCATE("re"));
    if(fp) {
        while(fgets(line, sizeof line, fp)) {
            if(strstr(line, lib)) {
                InderecoBase = std::stoul(line, nullptr, 16);
                return InderecoBase;
            }
        }
    }
    return InderecoBase;
}


__attribute__((visibility("hidden")))
void* getAddressIL2CPP(uintptr_t relativeAddr2, bool recheck2 = false)
{
    while(basePtr13 == 0)
    {
        basePtr13 = LocalizarInderecoBase(OBFUSCATE("libil2cpp.so"));
        //LOGD(WRAPPER_MARCO("basePtr1: %p"), basePtr1);
    }
    if(recheck2)
        basePtr13 = LocalizarInderecoBase(OBFUSCATE("libil2cpp.so"));
    return (void*)(basePtr13 + relativeAddr2);
}





struct HitObjectInfo {
    char _pad0[0x18];            // herança + static pool
    void* HitObject;             // 0x18
    void* HitCollider;           // 0x20
    Vector3 HitLocation;         // 0x28
    Vector3 HitNormal;           // 0x34
    Vector3 RayDir;              // 0x40
    Vector3 StartPosition;       // 0x4c
    int Damage;                  // 0x58
    float Distance;              // 0x5c
    int ActorLayer;              // 0x60
    int HitGroup;                // 0x64
    void* HitPhysicMaterial;     // 0x68
    bool IgnoreHappens;          // 0x70
    bool ViewBlocked;            // 0x71
    char _pad1[2];               // alinhamento
    Vector3 OrigStartPosition;   // 0x74
    short SpecialHitType;        // 0x80
    char _pad2[6];               // alinhar até 0x88
    void* SpecialHitDic;         // 0x88
    void* UGCLogicEntityID;      // 0x90
};

struct PlayerID {
    uint32_t m_Value;     // 0x00
    uint32_t m_ID;        // 0x04
    uint8_t  m_TeamID;    // 0x08
    uint8_t  m_ShortID;   // 0x09
    uint8_t  _padding[6]; // 0x0A
    uint64_t m_IDMask;    // 0x10
};

struct DamageInfo2_o {
    void *klass;
    void *monitor;
    int32_t DBLBLKADCNP;
    int32_t KENBMOOEHBG;
    monoString* JANPNJIFOJJ;
    bool NNNADMOFPIE;
    PlayerID DHGCIEKPBFA;
    void* GPBDEDFKJNA;
    int32_t PIAMIOFEBKF;
    Vector3 CNEICNJFGLM;
    Vector3 HECJHKEDFEB;
    Vector3 JNLGFLFLBHO;
    uint8_t ACAKHEABPEJ;
    bool MJIHLDJNHLF;
    int32_t LOKIMAEAPCB;
    monoDictionary<uint8_t*, void **> *FHLFLAHCIBN;
};


class Vvector3 {
public:
float X;
float Y;
float Z;
Vvector3() : X(0), Y(0), Z(0) {}
Vvector3(float x1, float y1, float z1) : X(x1), Y(y1), Z(z1) {}
Vvector3(const Vvector3 &v);
~Vvector3();
};

Vvector3::Vvector3(const Vvector3 &v) : X(v.X), Y(v.Y), Z(v.Z) {}
Vvector3::~Vvector3() {}

Vector3 Normalize(const Vector3& v) {
    float mag = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (mag > 0.0f)
        return Vector3(v.x / mag, v.y / mag, v.z / mag);
    return Vector3(0, 0, 0);
}
