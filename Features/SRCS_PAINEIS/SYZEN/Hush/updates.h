#pragma once
#include <cstdint>
#include <codecvt>
#include <locale>
#include <cmath>
#include "structs.h"
#include "memory.h"


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

Vector3 GetForwardAdjusted(void* player, float adjustAngle = 0.0f) {
    Vector3 forward = GetForward(player);

    float len = std::sqrt(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
    if (len > 0.0001f)
        forward = forward / len;

    if (std::abs(adjustAngle) > 0.0001f) {
        float rad = adjustAngle * (M_PI / 180.0f);
        float cosA = std::cos(rad);
        float sinA = std::sin(rad);

        float newX = forward.x * cosA - forward.z * sinA;
        float newZ = forward.x * sinA + forward.z * cosA;

        forward.x = newX;
        forward.z = newZ;
    }

    return forward;
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

#define m_PlayerAttributes (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("m_Attributes"))

#define Class_Transform__Rotation Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Transform"), OBFUSCATE("get_rotation"), 0)
static Quaternion GetRotation(void* player) {
Quaternion (*_GetRotation)(void* players) = (Quaternion(*)(void *))(Class_Transform__Rotation);
return _GetRotation(player);
}

#define Class_Camera__WorldToScreenPoint (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Camera"), OBFUSCATE("WorldToScreenPoint"), 1)
static Vector3 WorldToScreenPoint(void* WorldCam, Vector3 WorldPos) {
    Vector3(*_WorldToScreenScene)(void* Camera, Vector3 position) = (Vector3(*)(void*, Vector3)) (Class_Camera__WorldToScreenPoint);
    return _WorldToScreenScene(WorldCam, WorldPos);
}

#define ListPlayer (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Match"), OBFUSCATE("m_Players"))

#define EnemyUpdate (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("LateUpdate"), 0)

#define MainCam (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("MainCameraTransform"))
static Vector3 CameraMain(void* player) {
    return get_position(*(void**)((uint64_t)player + MainCam));
}

#define Match (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameFacade"), OBFUSCATE("CurrentMatch"), 0)
static void* Curent_Match() {
    void* (*_Curent_Match) (void* nuls) = (void* (*)(void*))(Match);
    return _Curent_Match(NULL);
}

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

#define Aim (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("SetAimRotation"), 2)
static void set_aim(void* player, Quaternion look) {
    using fnSetAim = void(*)(void*, Quaternion, bool);
    auto _set_aim = reinterpret_cast<fnSetAim>(Aim);

    _set_aim(player, look, false);
}

#define m_Aim2 (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("set_m_AimRotation"), 1)
static void set_aim2(void* player, Quaternion look) {
    using fnSetAim = void(*)(void*, Quaternion);
    auto _set_aim = reinterpret_cast<fnSetAim>(m_Aim2);

    _set_aim(player, look);
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

#define m_setRun (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("PlayerAttributes"), OBFUSCATE("SetSpecialRunSpeedScale"), 2)
inline void SetRunSpeed(void* localAttributes, int32_t type, float value)
{
    auto fn = reinterpret_cast<void(*)(void*, int32_t, float)>(m_setRun);
    fn(localAttributes, type, value);
}

#define u3str (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("mscorlib.dll"), OBFUSCATE("System"), OBFUSCATE("String"), OBFUSCATE("CreateString"), 1)
static monoString *U3DStr(const char *str) {
    monoString *(*String_CreateString)(void *_this, const char *str) = (monoString * (*)(void *, const char *))(u3str);
    return String_CreateString(NULL, str);
}

#define Name (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_NickName"), 0)
static monoString* get_NickName(void* player) {
    monoString* (*_get_NickName)(void* players) = (monoString * (*)(void*))(Name);
    return _get_NickName(player);
}

#define CharGet (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("mscorlib.dll"), OBFUSCATE("System"), OBFUSCATE("String"), OBFUSCATE("get_Chars"), 1)
char get_Chars(monoString* str, int index) {
    char (*_get_Chars)(monoString * str, int index) = (char (*)(monoString*, int))(CharGet);
    return _get_Chars(str, index);
}

std::string MonoStringToUtf8(monoString* str) {
    if (!str) return "";

    int len = str->getLength();
    std::u16string utf16;

    utf16.reserve(len);
    for (int i = 0; i < len; i++) {
        utf16.push_back(get_Chars(str, i));
    }

    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    return convert.to_bytes(utf16);
}

/*#define FakeName (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("set_NickName"), 1)
static void spofNick(void *players) {
void (*_spof_nick)(void *player, monoString *nick) = (void (*)(void *, monoString *))(FakeName);
_spof_nick(players, U3DStr((OBFUSCATE("[FFFFFF]TIKTOK[FFFFFF]:[FF001848]C4DEVOFC[FFFFFF]"))));
}*/

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

#define m_highFps (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameSettingData"), OBFUSCATE("SetHighFPSSetting"), 1)
inline void SetHighFPS(int32_t value)
{
    auto fn = reinterpret_cast<void(*)(int32_t)>(m_highFps);
    fn(value);
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

#define Esp2 (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("UGCLevelMiniSentry"), OBFUSCATE("PlayGunTrace"), 2)
static void set_esp2(void* imo, Vector3 x, Vector3 y) {
    void (*_SetEsp2)(void* imo, Vector3 X, Vector3 Y) = (void (*)(void*, Vector3, Vector3))Esp2;
    _SetEsp2(imo, x, y);
}

#define CenterWS (uintptr_t)Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("GetAttackableCenterWS"))
static Vector3 GetAttackableCenterWS(void* player) {
    Vector3 (*_GetAttackableCenterWS)(void*) = (Vector3 (*)(void*))CenterWS;
    return _GetAttackableCenterWS(player);
}

#define Raycast (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("PhysicsUtil"), OBFUSCATE("SingleLineCheck"), 4)
static bool Physics_Raycast(Vector3 camLocation, Vector3 headLocation, unsigned int LayerID, HitObjectInfo* outInfo) {
    using fnRaycast = bool(*)(Vector3, Vector3, unsigned int, HitObjectInfo*);
    auto _Physics_Raycast = reinterpret_cast<fnRaycast>(Raycast);

    return _Physics_Raycast(camLocation, headLocation, LayerID, outInfo);
}

#define m_cris_ray (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("PhysicsUtil"), OBFUSCATE("SingleLineCheck"), 4)
static bool Cristiano_RayVery(Vector3 camLocation, Vector3 headLocation, unsigned int LayerID, void* collider) {
    bool (*_Cristiano_RayVery)(Vector3 camLocation, Vector3 headLocation, unsigned int LayerID, void* collider) = (bool(*)(Vector3, Vector3, unsigned int, void*))(m_cris_ray);
    return _Cristiano_RayVery(camLocation, headLocation, LayerID, collider);
}

#define TranGetPosition (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.CoreModule.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Transform"), OBFUSCATE("get_position_Injected"), 1)
static Vector3 Transform_INTERNAL_GetPosition(void *player) {
    Vector3 out = Vector3::zero();
    void (*_Transform_INTERNAL_GetPosition)(void *transform, Vector3 * out) = (void (*)(void *, Vector3 *))(TranGetPosition);
    _Transform_INTERNAL_GetPosition(player, &out);
    return out;
}

#define m_State (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Match"), OBFUSCATE("m_State"))
#define m_isClientBot (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("IsClientBot"))
#define m_LocalPlayer (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Match"), OBFUSCATE("m_LocalPlayer"))

#define Lvl (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("LevelProjectile"), OBFUSCATE("get_HitObjectInfo3"), 0)
static void* Get_HitObject() {
    void* (*_Get_HitObject) (void* nuls) = (void* (*)(void*))(Lvl);
    return _Get_HitObject(NULL);
}

#define LAimCollider (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("AttackableEntity"), OBFUSCATE("set_LockedAimingCollider"), 1)
static void SetAimCollider(void* enemy, void* collider) {
    using fnSetCollider = void(*)(void*, void*);
    auto _set_collider = reinterpret_cast<fnSetCollider>(LAimCollider);
    _set_collider(enemy, collider);
}

#define m_get_transform (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Component"), OBFUSCATE("get_transform"), 0)
static auto get_transform(void *player) {
    auto (*_get_transform)(void *player) = (void *(*)(void *))(m_get_transform);
    return _get_transform(player);
}

#define m_get_pos (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Transform"), OBFUSCATE("get_position_Injected"), 1)
static Vector3 get_position_Injected(void* transform) {
    Vector3 ret{};
    auto func = (void(*)(void*, Vector3*))(m_get_pos);
    func(transform, &ret);
    return ret;
}

#define m_set_pos (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Transform"), OBFUSCATE("set_position_Injected"), 1)
static void set_position_Injected(void* transform, Vector3& position) {
    auto func = (void(*)(void*, Vector3*))(m_set_pos);
    func(transform, &position);
}


#define m_UpdateRotation Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("UpdateRotation"), 2)
static void UpdateRotation(void* player, float gameTime, float deltaTime) {
    using fnUpdateRotation = void(*)(void*, float, float);
    auto _UpdateRotation = reinterpret_cast<fnUpdateRotation>(m_UpdateRotation);

    _UpdateRotation(player, gameTime, deltaTime);
}

/*
#define m_tryRemove Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Match"), OBFUSCATE("TryRemovePlayer"), 1)
static void TryRemovePlayer(void* matchInstance, PlayerID playerId) {
    using fnTryRemove = void(*)(void*, PlayerID);
    auto _tryRemove = reinterpret_cast<fnTryRemove>(m_tryRemove);
    _tryRemove(matchInstance, playerId);
}

#define m_Remove Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Match"), OBFUSCATE("RemovePlayer"), 1)
static void RemovePlayer(void* matchInstance, PlayerID playerId) {
    using fnRemove = void(*)(void*, PlayerID);
    auto _Remove = reinterpret_cast<fnRemove>(m_Remove);
    _Remove(matchInstance, playerId);
}
*/

#define m_currentUi Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameFacade"), OBFUSCATE("CurrentInGameUIScene"), 0)
static void* CurrentInGameUIScene() {
    using fnCurrentUIScene = void* (*)();
    auto _CurrentUIScene = reinterpret_cast<fnCurrentUIScene>(m_currentUi);
    return _CurrentUIScene();
}

static monoString *U3DStrFormat(float distance, float vida) {
    char buffer[128] = {0};
    sprintf(buffer, "DIST %.f M | PLAYER BOT | %.f HP | [FF0000]TIKTOK[FFFFFF]:[DBE60B]HUSHZINNSL[FFFFFF]", distance,vida);
    return U3DStr(buffer);
}

static monoString *U3DStrPlayer2(float distance, float vida) {
    char buffer[128] = {0};
    sprintf(buffer, "DIST %.f M | PLAYER REAL | %.f HP | [FF0000]TIKTOK[FFFFFF]:[DBE60B]HUSHZINNSL[FFFFFF]", distance,vida);
    return U3DStr(buffer);
}

#define m_addTeamHud Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("UIInGameScene"), OBFUSCATE("ShowAssistantText"), 2)
static void ShowAssistantText(void* uiInstance, monoString* playerName, monoString* line) {
    using fnShowAssistantText = void(*)(void*, monoString*, monoString*);
    auto _ShowAssistantText = reinterpret_cast<fnShowAssistantText>(m_addTeamHud);
    _ShowAssistantText(uiInstance, playerName, line);
}

#define m_takeDamage (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("TakeDamage"), 9)
#define m_HitColliderType (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("DamageInfo"), OBFUSCATE("HitColliderType"))



//AimKill2
#define m_Send (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameFacade"), OBFUSCATE("Send"), 4)
bool GameFacade_Send(uint32_t messageID, void* msg, uint8_t sendOption, bool cacheMsgAnyWay)
{
    return ((bool (*)(uint32_t, void*, uint8_t, bool))(m_Send))(messageID, msg, sendOption, cacheMsgAnyWay);
}

#define m_get_player_ID (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("get_PlayerID"), 0)
PlayerID GetplayerID(void* _this)
{
    return ((PlayerID(*)(void*))getRealOffset(m_get_player_ID))(_this);
}

#define m_gen_chk_params (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Weapon"), OBFUSCATE("GenerateCheckParams"), 4)
monoList<float*>* LCLHHHKFCFP(void* Weapon, void* CAGCICACKCF, void* HFBDJJDICLN, bool LDGHPOPPPNL, DamageInfo2_o* DamageInfo)
{
    return ((monoList<float*> *(*)(void*, void*, void*, bool, DamageInfo2_o*))(m_gen_chk_params))(Weapon, CAGCICACKCF, HFBDJJDICLN, LDGHPOPPPNL, DamageInfo);
}

#define m_cur_simulation_timer (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("GameFacade"), OBFUSCATE("CurrentGameSimulationTimer"), 0)
void* CurrentGameSimulationTimer()
{
    return ((void* (*)(void*))(m_cur_simulation_timer))(NULL);
}

#define m_op_implicit (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("PlayerID"), OBFUSCATE("op_Implicit"), 1)
uint32_t CFFPIACECIG(PlayerID IDNEFEOPGIF)
{
    return ((uint32_t(*)(PlayerID))(m_op_implicit))(IDNEFEOPGIF);
}

#define m_GetAimingAttackableEntity (uintptr_t)Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("GetAimingAttackableEntity"), 1)
void* GKHECDLGAJA(void* pthis, void* a1)
{
    return ((void* (*)(void*, void*))(m_GetAimingAttackableEntity))(pthis, a1);
}

#define m_DamageCount (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Weapon"), OBFUSCATE("get_Damage"), 0)
static int GetDamage(void* pthis)
{
    return ((int (*)(void*))(m_DamageCount))(pthis);
}

static int get_DamageCount(void* pthis)
{
    return ((int (*)(void*))(m_DamageCount))(pthis);
}

#define m_DamageRange (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Weapon"), OBFUSCATE("get_Range"), 0)
static float get_Range(void* pthis)
{
    return ((float (*)(void*))(m_DamageRange))(pthis);
}

#define m_gwoh (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("GetWeaponOnHand"), 0)
static void* GetWeaponOnHand(void* player) {
    using fnGetWeaponOnHand = void* (*)(void*);
    auto _GetWeaponOnHand = reinterpret_cast<fnGetWeaponOnHand>(m_gwoh);
    return _GetWeaponOnHand(player);
}

#define m_weapon_hand1 (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("GetWeaponOnHand"), 0)
static void *GetWeaponOnHand1(void *local) {
    void *(*_GetWeaponOnHand1)(void *local) = (void *(*)(void *))(m_weapon_hand1);
    return _GetWeaponOnHand1(local);
}


#define m_swap_Weapon (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("PlayerNetwork"), OBFUSCATE("SwapWeapon"), 3)
#define m_phy_X_state (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("GetPhysXState"), 0)
#define m_PlayerID (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("m_PlayerID"))

#define m_recoil (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("WeaponFireComponent"), OBFUSCATE("GenerateRandomDirection"), 3)

#define m_PlayerLevel (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW"), OBFUSCATE("BaseProfileInfo"), OBFUSCATE("Level"))
#define m_PlayerProfs (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("PlayerNetwork"), OBFUSCATE("get_Profile"))
inline void* GetProfile(void* player)
{
    using fnGetProfile = void*(*)(void*);
    auto fn = reinterpret_cast<fnGetProfile>(m_PlayerProfs);
    return fn(player);
}

#define m_itransformNode (uintptr_t) Il2CppGetMethodOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("GCommon"), OBFUSCATE("TransformNode"), OBFUSCATE("get_transform"), 0)
static auto get_itransform(void *player) {
    auto (*_get_itransform)(void *player) = (void *(*)(void *))(m_itransformNode);
    return _get_itransform(player);
}

#define m_HeadNode (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("HeadNode"))
#define m_Neck (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("m_BloodEffectNode"))
#define m_HipNode (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("HipNode"))
#define m_LeftShoulder (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("m_LeftArmNode"))
#define m_RightShoulder (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("m_RightArmNode"))
#define m_LeftElbow (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("m_LeftForeArmNode"))
#define m_RightElbow (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("m_RightForeArmNode"))
#define m_LeftHand (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("m_LeftWeaponNode"))
#define m_RightHand (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("m_RightHandNode"))
#define m_LeftKnee (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("m_LeftAnkleNode"))
#define m_RightKnee (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("m_RightAnkleNode"))
#define m_LeftFoot (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("m_LeftToeNode"))
#define m_RightFoot (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("m_RightToeNode"))
#define m_Root (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("m_RootNode"))

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

#define m_HawkerStartPos (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("HitObjectInfo"), OBFUSCATE("StartPosition"))
#define m_HawkrtEndPos (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("HitObjectInfo"), OBFUSCATE("RayDir"))
#define m_HawkerHitObject (uintptr_t) Il2CppGetFieldOffset(OBFUSCATE("Assembly-CSharp.dll"), OBFUSCATE("COW.GamePlay"), OBFUSCATE("Player"), OBFUSCATE("m_LastAimingInfoFromWeapon"))

