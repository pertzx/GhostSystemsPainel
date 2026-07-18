#pragma once

#include <cstdint>

extern bool g_IsGameReady;
extern bool g_IsEmulatorReady;
extern bool IsEverythingLoad;

#define PhimSetNhatban(offset) AgetAbsoluteAddress("libil2cpp.so",offset)
long AfindLibrary(const char *library) {
    char filename[0xFF] = {0},
    buffer[1024] = {0};
    FILE *fp = nullptr;
    long address = 0;

    sprintf(filename, "/proc/self/maps");

    fp = fopen( filename, "rt" );
    if( fp == nullptr ){
        perror("fopen");
        goto done;
    }

    while( fgets( buffer, sizeof(buffer), fp ) ) {
        if(strstr( buffer, library ) ){
            address = (long)strtoul( buffer, NULL, 16 );
            goto done;
        }
    }

    done:

    if(fp){
        fclose(fp);
    }
    return address;
}

long ClibBase;

long AgetAbsoluteAddress(const char* libraryName, long relativeAddr) {
    if (ClibBase == 0) {
        ClibBase = AfindLibrary(libraryName);
        if (ClibBase == 0) {
            ClibBase = 0;
        }
    }
    return ClibBase + relativeAddr;
}

template <typename T>
struct monoArray
{
    void* klass;
    void* monitor;
    void* bounds;
    int   max_length;
    void* vector [1];
    int getLength()
    {
        return max_length;
    }
    T getPointer()
    {
        return (T)vector;
    }
};

template <typename T>
struct monoList {
    void *unk0;
    void *unk1;
    monoArray<T> *items;
    int size;
    int version;

    T getItems(){
        return items->getPointer();
    }

    int getSize(){
        return size;
    }

    int getVersion(){
        return version;
    }
};

template <typename K, typename V>
struct monoDictionary {
    void *unk0;
    void *unk1;
    monoArray<int **> *table;
    monoArray<void **> *linkSlots;
    monoArray<K> *keys;
    monoArray<V> *values;
    int touchedSlots;
    int emptySlot;
    int size;

    K getKeys(){
        return keys->getPointer();
    }

    V getValues(){
        return values->getPointer();
    }

    int getNumKeys(){
        return keys->getLength();
    }

    int getNumValues(){
        return values->getLength();
    }

    int getSize(){
        return size;
    }
};
union intfloat {
    int i;
    float f;
};

typedef struct _monoString
{
    void* klass;
    void* monitor;
    int length;    
    char chars[1];   
    int getLength()
    {
      return length;
    }
    char* getChars()
    {
        return chars;
    }
}monoString;

class ExpireDate {
    public:
    ExpireDate() {
        std::time_t now = std::time(nullptr);
        expirationTime = std::chrono::system_clock::from_time_t(now);
    }
    void setExpirationDate(int day, int month, int year) {
        std::tm tm_struct = {};
        tm_struct.tm_mday = day;
        tm_struct.tm_mon = month - 1;
        tm_struct.tm_year = year - 1900;
        std::time_t expirationTime_t = std::mktime(&tm_struct);
        expirationTime = std::chrono::system_clock::from_time_t(expirationTime_t);
    }
    bool isExpired() {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        return now >= expirationTime;
    }
    private:
    std::chrono::system_clock::time_point expirationTime;
};
bool PatchOffset(uintptr_t address, const void *buffer, size_t length) {
	unsigned long page_size = sysconf(_SC_PAGESIZE);
    unsigned long size = page_size * sizeof(uintptr_t);
    return mprotect((void *)(address - (address % page_size) - page_size), (size_t) size, PROT_EXEC | PROT_READ | PROT_WRITE) == 0 && memcpy((void *)address, (void *)buffer, length) != 0;
}
bool IsValidEnemy(void *player) {
    static int fd = -1;
    if (fd == -1) {
        fd = open(OBFUSCATE("/dev/random"), O_WRONLY);
    }
    return write(fd, player, 4) >= 0;
}
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

#define Class_Camera__get_main (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Camera"), OBFUSCATE("get_main"))

void* get_main() {
    return reinterpret_cast<void* (__fastcall*)()>(Class_Camera__get_main)();
}

#define Class_Input__get_touchCount (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Input"), OBFUSCATE("get_touchCount"))

#define Class_Input__GetTouch (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Input"), OBFUSCATE("GetTouch"), 1)

#define Class_Input__get_mousePosition (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Input"), OBFUSCATE("get_mousePosition"))

#define Class_Screen__get_width (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Screen"), OBFUSCATE("get_width"))
int get_width() {
    return reinterpret_cast<int(__fastcall*)()>(Class_Screen__get_width)();
}

#define Class_Screen__get_height (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Screen"), OBFUSCATE("get_height"))
int get_height() {
    return reinterpret_cast<int(__fastcall*)()>(Class_Screen__get_height)();
}

#define Class_Screen__get_density (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Screen"), OBFUSCATE("get_dpi"))

#define Camera_get_fieldOfView (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Camera"), OBFUSCATE("get_fieldOfView"))
float get_fieldOfView() {
    return reinterpret_cast<float(__fastcall*)(void*)>(Camera_get_fieldOfView)(get_main());
}

#define Camera_set_fieldOfView (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Camera"), OBFUSCATE("set_fieldOfView"), 1)
void* set_fieldOfView(float value) {
    return reinterpret_cast<void* (__fastcall*)(void*, float)>(Camera_set_fieldOfView)(get_main(), value);
}

#define ForWard (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Transform"), OBFUSCATE("get_forward"), 0)
static Vector3 GetForward(void* player) {
    Vector3(*_GetForward)(void* players) = (Vector3(*)(void*))(ForWard);
    return _GetForward(player);
}

#define Class_Transform__GetPosition Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Transform"), OBFUSCATE("get_position_Injected"), 1)
static Vector3 Transform_GetPosition(void* player) {
    Vector3 out = Vector3::zero();
    void (*_Transform_GetPosition)(void* transform, Vector3 * out) = (void (*)(void*, Vector3*))(Class_Transform__GetPosition);
    _Transform_GetPosition(player, &out);
    return out;
}

#define Class_Transform__SetPosition Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Transform"), OBFUSCATE("set_position_Injected"), 1)
static void Transform_INTERNAL_SetPosition(void* player, Vvector3 inn) {
    void (*Transform_INTERNAL_SetPosition)(void* transform, Vvector3 in) = (void (*)(void*, Vvector3))(Class_Transform__SetPosition);
    Transform_INTERNAL_SetPosition(player, inn);
}

#define Class_Transform__Position Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Transform"), OBFUSCATE("get_position"), 0)
Vector3 get_position(void* player) {
    Vector3(*_get_position)(void* players) = (Vector3(*)(void*))(Class_Transform__Position);
    return _get_position(player);
}

#define Class_Transform__Rotation Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Transform"), OBFUSCATE("get_rotation"), 0)
static Quaternion GetRotation(void* player) {
Quaternion (*_GetRotation)(void* players) = (Quaternion(*)(void *))(Class_Transform__Rotation);
return _GetRotation(player);
}

void SetRotation(void* transform, Quaternion rot) {
    void (*set_rotation)(void*, Quaternion) = (void (*)(void*, Quaternion))Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Transform"), OBFUSCATE("set_rotation"), 1);
    set_rotation(transform, rot);
}

#define Class_Camera__WorldToScreenPoint (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Camera"), OBFUSCATE("WorldToScreenPoint"), 1)
static Vector3 WorldToScreenPoint(void* WorldCam, Vector3 WorldPos) {
    Vector3(*_WorldToScreenScene)(void* Camera, Vector3 position) = (Vector3(*)(void*, Vector3)) (Class_Camera__WorldToScreenPoint);
    return _WorldToScreenScene(WorldCam, WorldPos);
}


#define ListPlayer (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("NFJPHMKKEBF"), OBFUSCATE("HOOCHDLKOOG"))

#define EnemyUpdate (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("LateUpdate"), 0)

#define MainCam (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("MainCameraTransform"))
static Vector3 CameraMain(void* player) {
    return get_position(*(void**)((uint64_t)player + MainCam));
}

/*#define Match (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameFacade"), OBFUSCATE("CurrentMatch"), 0)
static void* Curent_Match() {
    void* (*_Curent_Match) (void* nuls) = (void* (*)(void*))(Match);
    return _Curent_Match(NULL);
}*/

#define Local (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIHudDetectorController"), OBFUSCATE("GetLocalPlayer"), 0)
static void* GetLocalPlayer(void* Game) {
    void* (*_GetLocalPlayer)(void* match) = (void* (*)(void*))(Local);
    return _GetLocalPlayer(Game);
}

#define Visible (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("IsVisible"), 0)
static bool get_isVisible(void* player) {
    bool (*_get_isVisible)(void* players) = (bool (*)(void*))(Visible);
    return _get_isVisible(player);
}

#define Team (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("IsLocalTeammate"), 1)
static bool get_isLocalTeam(void* player) {
    using fnGetIsLocalTeam = bool(*)(void*, bool);
    auto _get_isLocalTeam = reinterpret_cast<fnGetIsLocalTeam>(Team);
    return _get_isLocalTeam(player, false);
}

#define Die (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_IsDieing"), 0)
static bool get_IsDieing(void* player) {
    bool (*_get_die)(void* players) = (bool (*)(void*))(Die);
    return _get_die(player);
}

#define KnockDown (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_IsKnockDownKeepForwardAsAiming"), 0)
static bool get_IsKnockDownKeepForwardAsAiming(void* player) {
    bool (*_get_knock)(void* players) = (bool (*)(void*))(KnockDown);
    return _get_knock(player);
}

#define CurHP (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_CurHP"), 0)
static int GetHp(void* player) {
    int (*_GetHp)(void* players) = (int(*)(void*))(CurHP);
    return _GetHp(player);
}

#define MaxHP (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_MaxHP"), 0)
static int get_MaxHP(void* enemy) {
    int (*_get_MaxHP)(void* player) = (int(*)(void*))(MaxHP);
    return _get_MaxHP(enemy);
}

#define Name (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_NickName"), 0)
static monoString* get_NickName(void* player) {
    monoString* (*_get_NickName)(void* players) = (monoString * (*)(void*))(Name);
    return _get_NickName(player);
}

#define Aim (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("SetAimRotation"), 2) //0
static void set_aim(void* player, Quaternion look) {
    using fnSetAim = void(*)(void*, Quaternion, bool);
    auto _set_aim = reinterpret_cast<fnSetAim>(Aim);

    _set_aim(player, look, false);
}

#define Scope (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_IsSighting"),0 )
static bool get_IsSighting(void* player) {
    bool (*_get_IsSighting)(void* players) = (bool (*)(void*))(Scope);
    return _get_IsSighting(player);
}

#define Fire (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("IsFiring"), 0)
static bool get_IsFiring(void* player) {
    bool (*_get_IsFiring)(void* players) = (bool (*)(void*))(Fire);
    return _get_IsFiring(player);
}

#define Head (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("GetHeadTF"), 0)
static void* GetHeadPositions(void* player) {
    void* (*_GetHeadPositions)(void* players) = (void* (*)(void*))(Head);
    return _GetHeadPositions(player);
}

#define CharGet (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("mscorlib.dll"), OBFUSCATE("System"), OBFUSCATE("String"), OBFUSCATE("get_Chars"), 1) //0
char get_Chars(monoString* str, int index) {
    char (*_get_Chars)(monoString * str, int index) = (char (*)(monoString*, int))(CharGet);
    return _get_Chars(str, index);
}



#define HeadColider (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_HeadCollider"))
static void* Player_GetHeadCollider(void* player) {
    void* (*_Player_GetHeadCollider)(void* players) = (void* (*)(void*))(HeadColider);
    return _Player_GetHeadCollider(player);
}

#define Hip (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("GetHipTF"), 0)
static void* GetHipPositions(void* player) {
void* (*_GetHipPositions)(void* players) = (void*(*)(void*))(Hip);
return _GetHipPositions(player);
}

#define Class_Compent__Transform Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Component"), OBFUSCATE("get_transform"), 0)
static void *Component_GetTransform(void *player) {
    void *(*_Component_GetTransform)(void *component) = (void *(*)(void *))(Class_Compent__Transform);
    return _Component_GetTransform(player);
}


static void *Camera_main() {
void *(*_Camera_main)(void *nuls) = (void *(*)(void *))(Class_Camera__get_main);
return _Camera_main(nullptr);
}

Vector3 getPosition(void *transform) {
return get_position(Component_GetTransform(transform));
}
static Vector3 GetHeadPosition(void* player) {
return get_position(GetHeadPositions(player));
}
static Vector3 GetHipPosition(void* player) {
return get_position(GetHipPositions(player));
}

float get_density() {
return reinterpret_cast<float(__fastcall *)()>(Class_Screen__get_density)();
}


#define Imo (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("GetActiveWeapon"))
static void* get_imo(void* player) {
    void* (*_GetImo)(void* players) = (void* (*)(void*))Imo;
    return _GetImo(player);
}


#define Esp2 (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("LevelMiniSentry"), OBFUSCATE("CPBCGAKODII"), 2)
static void set_esp2(void* imo, Vector3 x, Vector3 y) {
    void (*_SetEsp2)(void* imo, Vector3 X, Vector3 Y) = (void (*)(void*, Vector3, Vector3))Esp2;
    _SetEsp2(imo, x, y);
}

#define CenterWS (uintptr_t)Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("GetAttackableCenterWS"))

static Vector3 GetAttackableCenterWS(void* player) {
    Vector3 (*_GetAttackableCenterWS)(void*) = (Vector3 (*)(void*))CenterWS;
    return _GetAttackableCenterWS(player);
}

#define TranGetPosition (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Transform"), OBFUSCATE("get_position_Injected"), 1)
static Vector3 Transform_INTERNAL_GetPosition(void *player) {
    Vector3 out = Vector3::zero();
    void (*_Transform_INTERNAL_GetPosition)(void *transform, Vector3 * out) = (void (*)(void *, Vector3 *))(TranGetPosition);
    _Transform_INTERNAL_GetPosition(player, &out);
    return out;
}

#define SetEulerAddr (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Transform"), OBFUSCATE("set_eulerAngles"), 1)
void SetEuler(void* transform, Vector3 rot) {
    if (!transform) return;
    void (*_set)(void*, Vector3) = (void(*)(void*, Vector3))SetEulerAddr;
    _set(transform, rot);
}

#define WeaponOnHand (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("GetWeaponOnHand"))
void *GetWeaponOnHand(void *player)
{
    return ((void* (*)(void*))(WeaponOnHand))(player);
}

#define GameObject Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Component"), OBFUSCATE("get_gameObject"), 0)
void *get_gameObject(void *player)
{
    return ((void *(*)(void *))(GameObject))(player);
}

#define HeadTF (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("GetHeadTF"), 0)
void *GetHeadTF(void *player)
{
    return ((void* (*)(void*))(HeadTF))(player);
}
#define offset_isGod (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_IsGod"), 0)

static bool get_God(void *player) {
    bool (*_get_God)(void *players) = (bool (*)(void *))(offset_isGod);
    return _get_God(player);
}

#define Raycast (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("JEAGCMACNNC"), OBFUSCATE("PLDCHDBCOBF"), 4)
static bool Physics_Raycast(Vector3 start, Vector3 end, unsigned int flag, void* hitInfo) {
    auto _Physics_Raycast = (bool(*)(Vector3, Vector3, unsigned int, void*))(Raycast);
    return _Physics_Raycast(start, end, flag, hitInfo);
}

#define offset_StopFiring (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("GPBDEDFKJNA"), OBFUSCATE("EJONAOOEOJH"), 0)
#define offset_StartFiringer (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("GPBDEDFKJNA"), OBFUSCATE("MPGJOKFGLGK"), 0)
#define offset_GetDelay (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("GPBDEDFKJNA"), OBFUSCATE("LMGGNHPNMNI"), 0)
#define m_DamageRange (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("GPBDEDFKJNA"), OBFUSCATE("JDGGIFMKIKF"), 0)

static float get_Range(void* pthis)
{
    return ((float (*)(void*))(m_DamageRange))(pthis);
}
static float get_Delay(void *pthis)
{
    return ((float (*)(void *))(offset_GetDelay))(pthis);
}
static void *StopFiring(void *weapon) {
    void *(*_StopFiring)(void *Weapon) = (void *(*)(void *))(offset_StopFiring);
    return _StopFiring(weapon);
}
static void *StartFiring(void *weapon) {
    void *(*_StartFiring)(void *Weapon) = (void *(*)(void *))(offset_StartFiringer);
    return _StartFiring(weapon);
}


#define m_isClientBot (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("IsClientBot"))

#define offset_JMKMBNIBFNA (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("MADMMIICBNN"), OBFUSCATE("JMKMBNIBFNA")) //HitOpject
#define offset_GHACJPMCEDK (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("MADMMIICBNN"), OBFUSCATE("GHACJPMCEDK")) // HitCollider
#define offset_OJKBBAOPPIN (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("MADMMIICBNN"), OBFUSCATE("OJKBBAOPPIN")) //HitGroup

#define Match (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameFacade"), OBFUSCATE("CurrentMatch"), 0)

static void *Curent_Match() {
    if (!IsEverythingLoad) {
        return NULL;
    }

    void *(*_Curent_Match) (void *nuls) = (void *(*)(void *))(Match);
    return _Curent_Match(NULL);
}

Vector3 getPosition1(void *transform) {
    return get_position(Component_GetTransform(transform));
}

#define offset_set_positionCount (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("LineRenderer"), OBFUSCATE("set_positionCount"), 1)
#define offset_SetPosition (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("LineRenderer"), OBFUSCATE("SetPosition"), 2)
#define offset_DrawLine2 (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GrenadeLine"), OBFUSCATE("DrawLine2"), 3)
#define offset_GrenadeLine (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GrenadeLine"), OBFUSCATE("m_GrenadeLine"))
#define offset_ShowGrenadeLine (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GrenadeLine"), OBFUSCATE("m_ShowGrenadeLine"))
#define offset_ShowThrowSkillLine (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GrenadeLine"), OBFUSCATE("m_ShowThrowSkillLine"))
#define offset_set_startColor (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("LineRenderer"), OBFUSCATE("set_startColor"), 1)
#define offset_set_endColor (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("LineRenderer"), OBFUSCATE("set_endColor"), 1)

struct UnityColor {
    float r, g, b, a;
};

static void LineRenderer_SetPosition(void *Render, int value, Vector3 Location){
    void (*_LineRenderer_SetPosition)(void *Render, int value, Vector3 Location) = (void (*)(void*, int, Vector3))(offset_SetPosition);
    return _LineRenderer_SetPosition(Render, value, Location);
}

static void LineRenderer_Set_PositionCount(void *Render, int value){
    void (*_LineRenderer_Set_PositionCount)(void *Render, int value) = (void (*)(void*, int))(offset_set_positionCount);
    return _LineRenderer_Set_PositionCount(Render, value);
}

static void LineRenderer_SetColor(void *Render, UnityColor color){
    void (*_set_startColor)(void *Render, UnityColor color) = (void (*)(void*, UnityColor))(offset_set_startColor);
    void (*_set_endColor)(void *Render, UnityColor color) = (void (*)(void*, UnityColor))(offset_set_endColor);
    _set_startColor(Render, color);
    _set_endColor(Render, color);
}

static void GrenadeLine_DrawLine(void *instance, Vector3 start, Vector3 end, Vector3 position) {
    void (*_GrenadeLine_DrawLine)(void *clz, Vector3 throwPos, Vector3 throwVel, Vector3 gravity) = (void (*)(void*, Vector3, Vector3,Vector3)) (offset_DrawLine2);
    return _GrenadeLine_DrawLine(instance, start, end, position);
}

void *LineGrenade = nullptr;
void *RenderLine = nullptr;

void (*UpdateGranada)(void *_this);
void _UpdateGranada(void *_this) {
    if (_this != NULL) {
        LineGrenade = _this;
        *(bool *)((long)_this + 0x28) = true;
        RenderLine = *(void **)((long)_this + 0x30);
    }
    UpdateGranada(_this);
}

#define u3str (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("mscorlib.dll"), OBFUSCATE("System"), OBFUSCATE("String"), OBFUSCATE("CreateString"), 1)

static monoString *U3DStr(const char *str) {
    monoString *(*String_CreateString)(void *_this, const char *str) = (monoString * (*)(void *, const char *))(u3str);
    return String_CreateString(NULL, str);
}

#define m_currentUi Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameFacade"), OBFUSCATE("CurrentInGameUIScene"), 0)
static void* CurrentInGameUIScene() {
    using fnCurrentUIScene = void* (*)();
    auto _CurrentUIScene = reinterpret_cast<fnCurrentUIScene>(m_currentUi);
    return _CurrentUIScene();
}

#define m_addTeamHud Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIInGameScene"), OBFUSCATE("ShowAssistantText"), 2)
static void ShowAssistantText(void* uiInstance, monoString* playerName, monoString* line) {
    using fnShowAssistantText = void(*)(void*, monoString*, monoString*);
    auto _ShowAssistantText = reinterpret_cast<fnShowAssistantText>(m_addTeamHud);
    
    if (uiInstance != nullptr) {
        _ShowAssistantText(uiInstance, playerName, line);
    }
}

static monoString *U3DStrFormat(float distance, float vida) {
    char buffer[128] = {0};
    sprintf(buffer, "distance: %.2fm [DLxsCheatz]", distance);
    return U3DStr(buffer);
}

static monoString *U3DStrPlayer2(float distance, float vida) {
    char buffer[128] = {0};
    sprintf(buffer, "distance: %.2fm [DLxsCheatz]", distance);
    return U3DStr(buffer);
}

#define offset_NeedSendMessage (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("EGKODKEJIAD"), OBFUSCATE("EAGIPMCDBIF"), 1)

static auto set_position_Injected(void *player, Vvector3 position) {
    auto (*_set_position_Injected)(void *player, Vvector3 position) = (void (*)(void *, Vvector3))Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Transform"), OBFUSCATE("set_position_Injected"), 1);
    return _set_position_Injected(player, position);
}

#define offset_TakeDamage_New Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("TakeDamage"), 4)

static auto get_position11(void *player) {
    auto (*_get_position11)(void *player) = (Vector3 (*)(void *))Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Transform"), OBFUSCATE("get_position"), 0);
    return _get_position11(player);
}

static auto get_transform(void *player) {
    auto (*_get_transform)(void *player) = (void *(*)(void *))Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Component"), OBFUSCATE("get_transform"), 0);
    return _get_transform(player);
}

#define m_highFps (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameSettingData"), OBFUSCATE("SetHighFPSSetting"), 1)
inline void SetHighFPS(int32_t value)
{
    auto fn = reinterpret_cast<void(*)(int32_t)>(m_highFps);
    fn(value);
}

#define offset_GetHeadCollider (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_HeadCollider"), 0)
static void *GetHeadCollider(void* player){
    void *(*_GetHeadCollider)(void *players) = (void *(*)(void *))(offset_GetHeadCollider);
    return _GetHeadCollider(player);
}

#define offset_LocalPlayer (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameFacade"), OBFUSCATE("CurrentLocalPlayer"), 0)
static void *Current_Local_Player() {
    void *(*_Local_Player)(void *players) = (void *(*)(void *))(offset_LocalPlayer);
    return _Local_Player(NULL);
}

#define m_PlayerAttributes (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("JKPFFNEMJIF"))
#define m_setFireSpeed (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("PlayerAttributes"), OBFUSCATE("set_FireIntervalScale"), 1)

inline void SetFireSpeed(void* localAttributes, float value)
{
    if (m_setFireSpeed == 0) return;
    auto fn = reinterpret_cast<void(*)(void*, float)>(m_setFireSpeed);
    fn(localAttributes, value);
}

#define m_Fixed (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("GCommon"), OBFUSCATE("TimeService"), OBFUSCATE("m_FixedDeltaTime"))
#define m_Simulation (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("GCommon"), OBFUSCATE("BaseGame"), OBFUSCATE("m_SimulationTimer"))
#define m_Timer (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameFacade"), OBFUSCATE("CurrentGameSimulationTimer"), 0)
static void* GetSimulationTimer()
{
    void* (*_GetSimulationTimer) () = reinterpret_cast<void* (*)()>(m_Timer);
    return _GetSimulationTimer();
}

#define m_GetTimer (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("GCommon"), OBFUSCATE("TimeService"), OBFUSCATE("get_FixedDeltaTime"), 0)
static float GetTimer(void* timeServiceInstance)
{
    using fnGetFixedDelta = float(*)(void*);
    auto _GetTimer = reinterpret_cast<fnGetFixedDelta>(m_GetTimer);
    return _GetTimer(timeServiceInstance);
}

#define m_SetTimer (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("GCommon"), OBFUSCATE("TimeService"), OBFUSCATE("UseFixedDeltaTime"), 1)
static void SetTimer(void* timeServiceInstance, float fixedDelta)
{
    using fnSetFixed = void(*)(void*, float);
    auto _set_fixed = reinterpret_cast<fnSetFixed>(m_SetTimer);
    _set_fixed(timeServiceInstance, fixedDelta);
}

static auto set_position_Injected555(void *player, Vvector3 position) {
    auto (*_set_position_Injected)(void *player, Vvector3 position) = (void (*)(void *, Vvector3))Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Transform"), OBFUSCATE("set_position_Injected"), 1);
    return _set_position_Injected(player, position);
}

#define offset_GetScatterRate (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("PlayerAttributes"), OBFUSCATE("GetScatterRate"), 0)

#define offset_UpdateLocalMark Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIMapContentController"), OBFUSCATE("UpdateLocalMark"), 3)

#define offset_GetLocalCar (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("VehicleIAmIn"), 0)
#define offset_get_IsPassenger (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_IsPassenger"), 0)
#define offset_get_IsDriver (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_IsDriver"), 0)

void* GetLocalCar(void* player) {
    if (!player) return nullptr;
    return ((void* (*)(void*))offset_GetLocalCar)(player);
}

bool get_IsPassenger(void* player) {
    if (!player) return false;
    return ((bool (*)(void*))offset_get_IsPassenger)(player);
}

bool get_IsDriver(void* player) {
    if (!player) return false;
    return ((bool (*)(void*))offset_get_IsDriver)(player);
}

//#define offset_PackVector3 (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("message"), OBFUSCATE("LLOABGDLMGK"), OBFUSCATE("LHGGPDLOPAH"), 1)

#define m_HawkerStartPos (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("MADMMIICBNN"), OBFUSCATE("BOGOIAMJFDN"))
#define m_HawkrtEndPos (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("MADMMIICBNN"), OBFUSCATE("NHKKHPLFMNG"))
#define m_HawkerHitObject (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("GEGFCFDGGGP"))

#define Class_Entity__SetRotation Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("GCommon"), OBFUSCATE("Entity"), OBFUSCATE("set_Rotation"), 1)
static void Entity_SetRotation(void* entity, Quaternion rotation) {
    void (*_SetRotation)(void* entity, Quaternion rotation) = (void(*)(void*, Quaternion))(Class_Entity__SetRotation);    
    _SetRotation(entity, rotation);
}
