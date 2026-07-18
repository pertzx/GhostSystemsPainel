
#pragma once

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <Il2Cpp.h>
#include <Tools.h>
#include <Vector3.hpp>
#include <Color.h>
//#include "HUNTERMODS/huntermod.h"
         
#include "obfuscate.h"
#include "Unity/unity.h"
#include "Class.h"
#include "Rect.h"
//inline static int g_GlHeight, g_GlWidth;
#include "Bools.h"
#include <math.h>
#include <stdio.h>
inline static int g_GlHeight, g_GlWidth;
typedef char PACKAGENAME;
//#include "MemoryPatch.h"
// Khai Báo
bool DoubleGun = false;
bool Blk = false;
bool Noscop = false;
bool FstRlod = false;
bool Mgbullet = false;
bool Car = false;
bool FastSwitch = false;
bool norecoil = false;
bool isSpeedSafe = false;
bool Fastrelod = false;
bool AimSilent = false;
	bool All = false;
bool EspName = false;
bool PCLOGO = false;
// Your ESP configuration struct
struct cfg {
	bool Enable = false;
	
bool myPlayer = false;
	
bool RecoFitur = false;
bool FPcount = false;
bool SpeedHack = false;
bool Camera = false;
bool NoRecoil = false;
bool Fish = false;
bool ResetGuest = false;
bool isSpeedSafe = false;
bool Aimbot = false;
bool Aimauto = false;
    bool aimbot;
    int aim_type = 0;
    int aim_target = 1;
float Fov_Aim = 0.0f;
float Aimdis = 0.0f;
float aimSmoothness = 0.0f;
int AimCheck = 0;
int AimType = 0;
int AimWhen = 1;
bool VisibltyDone = true;
bool AimActivar,AimCrouch;
float AimDistance;
bool esp;
float Size = 1000;
float AimDis;
float MoveSpeed;
bool Tele;
bool check_move = false;
int countEnemy = 0;
bool AimFire = false;
bool Socpe = false;
bool FastSwitch = false;
float AimSmoothFactor = 5.0f;
//int AimWhen = 0; // 0 = head, 1 = hip

    struct _esp {
	//	bool inforohan = false
        bool Box = false;
		bool Nick = false;
        bool Box3D = false;
        bool Line = false;
		bool Line2 = false;
		bool Info = false;
        bool Health = false;
        bool Distance = false;
        bool IsName = false;
        bool Path = false;
        bool Esp360 = false;
        bool Target = false;

		bool SniperAlert = false;
    }ESP;
}inline Config;
bool AimWhen3 = false;
bool AimWhen2 = false;
bool box = false;
bool AimWhen1 = false;
bool SniperAlert = false;
bool showbox1 = false;
bool showline1 = false;
bool showline2 = false;

bool camcao = false;
bool showline = false;
bool speedrun = false;
bool Firewall = false;
bool health = false;
int isSpeed = 0;  
int Speedx = 0;  
//int showline = 0;
const char* line[] = {OBFUSCATE("Esp Line 1"),OBFUSCATE("Esp Line 2"),OBFUSCATE("Esp Line 3")};

static void* magnetLastEnemy = nullptr;
//codes
// ESP Line settings
float ESP_LineThickness = 1.0f;              // default motaai
ImVec4 ESP_LineColor = ImVec4(1,1,1,1);
ImVec4 ESP_BoxColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);


static int g_AimMode = 0;

enum AimMode {    AIM_VISIBLE = 0,
    AIM_FIRING = 1,
    AIM_SCOPE = 2
};







int showbox = 1;
//MemoryPatch  Resetguest,DouleGun,nightmod,WallHack2;

//const char* box[] = {OBFUSCATE("Esp Box"),OBFUSCATE("Esp Box 3D")};

struct sColor {
        float circle[4] = {0 / 255.0f, 255 / 255.0f, 0 / 255.0f, 255 / 255.0f};

		float box[4] = {255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 255 / 255.0f};
        float line1[4] = {255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 255 / 255.0f};
        float box1[4] = {255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 255 / 255.0f};
        float box3d1[4] = {255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 255 / 255.0f};
        float info1[4] = {0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 255 / 255.0f};

        float line2[4] = {255 / 255.0f, 0 / 255.0f, 0 / 255.0f, 255 / 255.0f};
        float box2[4] = {255 / 255.0f, 0 / 255.0f, 0 / 255.0f, 255 / 255.0f};
        float box3d2[4] = {255 / 255.0f, 0 / 255.0f, 0 / 255.0f, 255 / 255.0f};
        float info2[4] = {255 / 255.0f, 0 / 255.0f, 0 / 255.0f, 255 / 255.0f};
};
sColor Color;
bool Reset = false;



bool isOutsideScreen(ImVec2 pos, ImVec2 screen) {
    if (pos.y < 0) {
        return true;
    }
    if (pos.x > screen.x) {
        return true;
    }
    if (pos.y > screen.y) {
        return true;
    }
    return pos.x < 0;
}

ImVec2 pushToScreenBorder(ImVec2 Pos, ImVec2 screen, int offset) {
    int x = (int) Pos.x;
    int y = (int) Pos.y;
    
    if (Pos.y < 0) {
        y = -offset;
    }
    
    if (Pos.x > screen.x) {
        x = (int) screen.x + offset;
    }
    
    if (Pos.y > screen.y) {
        y = (int) screen.y + offset;
    }
    
    if (Pos.x < 0) {
        x = -offset;
    }
    return ImVec2(x, y);
}

static float tm = 127 / 255.f;

void(*Set_Aim)(void* _this);
void _Set_Aim(void* _this) {
    return;
    return Set_Aim(_this);
}


void DrawCircleHealth(ImVec2 position, int health, int max_health, float radius) {
    float a_max = ((3.14159265359f * 2.0f));
    ImU32 healthColor = IM_COL32(45, 180, 45, 255);
    if (health <= (max_health * 0.6)) {
        healthColor = IM_COL32(180, 180, 45, 255);
    }
    if (health < (max_health * 0.3)) {
        healthColor = IM_COL32(180, 45, 45, 255);
    }
    ImGui::GetForegroundDrawList()->PathArcTo(position, radius, (-(a_max / 4.0f)) + (a_max / max_health) * (max_health - health), a_max - (a_max / 4.0f));
    ImGui::GetForegroundDrawList()->PathStroke(healthColor, ImDrawFlags_None, 4);
}

void drawText2(int x, int y, long color, const char *str, float size) {
    ImGui::GetBackgroundDrawList()->AddText(NULL, size, ImVec2(x, y), color, str);
}

void DrawRectFilled(int x, int y, int w, int h, ImColor color) {
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), color, 0, 0);
}

void DrawRectFilledHealth(int x, int y, int w, int h, ImVec4 color) {
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(color), 0, 0);
}

void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int color, int T) {
    ImGui::GetBackgroundDrawList()->AddTriangleFilled(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), color);
}

void DrawHealths(Rect box, float entityHealth, float maxHealth, int TeamID, const char* name, long color) {
    float x = box.x - (140 - box.w) / 2;
    float y = box.y;
    char dis[20];
    // sprintf(dis, "%0.fM", d);

    char TeamText[20];
    sprintf(TeamText, "%d", (int)TeamID);
    ImVec4 HPColor = ImVec4{1, 1, 0, 127 / 255.f}; // Màu mặc định cho máu

    if (entityHealth < maxHealth) {
        float healthPercentage = entityHealth / maxHealth;

        if (healthPercentage < 0.3f) {
            HPColor = ImVec4{0.5f, 0.0f, 0.0f, 127 / 255.f}; // Màu đỏ cho máu thấp
        } else if (healthPercentage < 0.6f) {
            HPColor = ImVec4{1, 0, 0, 127 / 255.f}; // Màu cam cho máu trung bình
        }
    }

    tm = 100.f / 255.f;
    DrawRectFilled(x - strlen(name), y - 41, 120 + strlen(name) * 2, 20, color); // Đám mây nền cho tên người chơi
    tm = 255.f / 255.f;
    DrawRectFilled(x - strlen(name), box.y - 41, 25, 20, color);
    if (strlen(TeamText) < 2) {
        drawText2(x + 6 - strlen(name), box.y - 42, ImColor(255,255,255), TeamText, 18.943);
    } else {
        drawText2(x - strlen(name), box.y - 42, ImColor(255,255,255), TeamText, 18.943);
    }
    drawText2(x + 28 - strlen(name), y - 43, ImColor(255,255,255), name, 18.943);
    float maxWidth = 120; // Độ dài tối đa của thanh máu
    float healthBarWidth = entityHealth * maxWidth / maxHealth; // Độ dài thanh máu dựa trên tỷ lệ máu hiện tại và máu tối đa
    DrawRectFilledHealth(x - maxWidth / 2, y - 18, healthBarWidth, 8, HPColor);// cập nhật vị trí và kích thước của thanh máu
    drawText2(x + 125 + strlen(name), y - 43, ImColor(255,255,255), dis,18.943);
    DrawTriangle(box.x + box.w / 2 - 10, y - 8, box.x + box.w / 2 + 15 - 10, y - 8, box.x + box.w / 2 - 2, y, ImColor(255,255,255), 1);
}

static float SetFieldOfView = 0, GetFieldOfView = 0;
void UpdateSpeedhack() {
    if (!Enable) return;

    void* Simulation = GetSimulationTimer();
    if (!Simulation) return;

    float FixedDeltaTime = GetTimer(Simulation);

    if (!saved) {
        active     = FixedDeltaTime * 1.82f; 
        desactive  = FixedDeltaTime;        
        saved = true;
    }
    if (SpeedHack1) {  
        if (FixedDeltaTime != active) {
            SetTimer(Simulation, active);
        }
    }
    else {
        if (FixedDeltaTime != desactive) {
            SetTimer(Simulation, desactive);
        }
    }
}
float (*old_GetCurrentDashSpeed)(void *instance);

float hook_GetCurrentDashSpeed(void *instance) {
    void* localPlayer = GetLocalPlayer(Curent_Match());

    if (instance && instance == localPlayer) {
        if (SpeedMode == 2) {
            return 9.0f;  
        }
    }

    return old_GetCurrentDashSpeed(instance);
}

float (*orig_get_MaxJumpHeight)(void *instance);
float (*orig_get_CustomGravity)(void *instance);
float (*orig_get_RisingGravity)(void *instance);
float (*orig_get_FallingGravity)(void *instance);
float JumpHeightx = 1.4f;

float my_get_MaxJumpHeight(void *instance) {
    if (instance != NULL) {
        if (Enable) {
            if (JumpHight) {
                return JumpHeightx;
            }
        }
    }
    return orig_get_MaxJumpHeight(instance);
}

float my_get_CustomGravity(void *instance) {
    if (instance != NULL) {
        if (Enable) {
           if (JumpHight) {
               return -4.0f;
           }
        }
    }
    return orig_get_CustomGravity(instance);
}



float my_get_RisingGravity(void *instance) {
    if (instance != NULL) {
        if (Enable) {
           if (JumpHight) {
               return -4.0f;
           }
        }
    }
    return orig_get_RisingGravity(instance);
}
float my_get_FallingGravity(void *instance) {
    if (instance != NULL) {
        if (Enable) {
           if (JumpHight) {
               return -4.0f;
           }
        }
    }
    return orig_get_FallingGravity(instance);
}
float (*get_deltaTime)();
float smoothSpeedAIM = 5; 
void *MyPlayer = NULL;
bool Drone;

const float RADAR_SIZE = 200.0f; // kích thước của radar
const ImVec2 RADAR_CENTER(100.0f, 100.0f); // tọa độ trung tâm của radar

float x,y;
float enemy1 = 2.0f;

inline void DrawAIMBOT(float screenWidth, float screenHeight) { 
}

void DroneView() {
    if (GetFieldOfView == 0) {
        GetFieldOfView = get_fieldOfView();
    }
    if (SetFieldOfView > 0 && GetFieldOfView != 0) {
        set_fieldOfView((float)GetFieldOfView + SetFieldOfView);
    }
    if (SetFieldOfView == 0 && GetFieldOfView != 0) {
        set_fieldOfView((float)GetFieldOfView);
    }
}


Quaternion GetRotationToTheLocation(Vector3 Target, float Height, Vector3 MyEnemy) {
return Quaternion::LookRotation((Target + Vector3(0, Height, 0)) - MyEnemy, Vector3(0, 1, 0));
}


void *EnemyVisible(void *match) {
    if(!match || !ActiveFeature) {
        return nullptr;
    }
    float shortestDistance = 99999.0f;
    float maxAngle=Fov_Aim; 
    void* closestEnemy = NULL;
    void* LocalPlayer = GetLocalPlayer(match);

    if(LocalPlayer != nullptr) {

        monoDictionary<uint8_t *, void **> *players = *(monoDictionary<uint8_t*, void **> **)((long)match + m_ShortIDToPlayers);        
        
        for(int u = 0; u < players->getNumValues(); u++) {
            void* Player = players->getValues()[u];
            
            if(Player != NULL && !get_isLocalTeam(Player) && !get_IsDieing(Player) && get_MaxHP(Player)) {
               
               Vector3 PlayerPos = GetHeadPosition(Player);
               Vector3 LocalPlayerPos = GetHeadPosition(LocalPlayer);              
               Vector3 targetDir = Vector3::Normalized(PlayerPos - LocalPlayerPos);
               
               float angle = Vector3::Angle(targetDir, GetForward(Component_GetTransform(Camera_main()))) * 100.0;
               
               if (!get_God(Player)) {
                    if(angle <= maxAngle) {
                        if(angle < shortestDistance) {
                            shortestDistance = angle;
                            closestEnemy = Player;
                        }
                    }
               }
             }
         }
     }
    return closestEnemy;
}


bool isVisible(void *enemy)  {
    if(enemy != NULL)  {
         void *hitObj = NULL;
         auto Camera = Transform_GetPosition(Component_GetTransform(Camera_main()));
         auto Target = Transform_GetPosition(Component_GetTransform(Player_GetHeadCollider(enemy)));
         return !Physics_Raycast(Camera, Target, 12, &hitObj);
    }
    return false;
}
void (*orig_mrSilentAim)(void *, float, float);
void hook_mrAimSilent(void *_this, float a1, float a2) {
if (_this != nullptr) {
if(Aimbot) {
if (AimSilent) {
void *CurrentMatch = Curent_Match();
if (CurrentMatch != NULL) {
void* LocalPlayer = GetLocalPlayer(CurrentMatch);
if (LocalPlayer != NULL)  {
bool visible = isVisible(LocalPlayer);
if (visible) {
if (get_IsFiring(GetLocalPlayer(CurrentMatch))) {
return;
} else {
orig_mrSilentAim(_this, a1, a2);
}
}
}
}
}
}
orig_mrSilentAim(_this, a1, a2);
}
}



void *GetClosestEnemy() {
    float shortestDistance = 9999;
    void* closestEnemy = NULL;   
     
    void* get_MatchGame = Curent_Match();
    void* LocalPlayer = GetLocalPlayer(get_MatchGame);

    if (LocalPlayer != NULL && get_MatchGame != NULL && Enable && get_MatchGame) {
        monoDictionary<uint8_t *, void **> *players = *(monoDictionary<uint8_t*, void **> **)((long)get_MatchGame + ListPlayer);
        
        for (int u = 0; u < players->getNumValues(); u++) {
            void* Player = players->getValues()[u]; 
            if (Player != NULL && !get_isLocalTeam(Player) && !get_IsDieing(Player) && get_isVisible(Player) && get_MaxHP(Player)) {    
                Vector3 PlayerPos = getPosition(Player);
                Vector3 LocalPlayerPos = getPosition(LocalPlayer);
                Vector3 pos2 = WorldToScreenPoint(Camera_main(), PlayerPos);
                bool isFov1 = isFov(Vector3(pos2.x, pos2.y), Vector3(g_GlWidth / 2, g_GlHeight / 2), Fov_Aim);
            
                
                
                
                float distance = Vector3::Distance(LocalPlayerPos, PlayerPos);
                if (distance < 1000) {
                    Vector3 targetDir = Vector3::Normalized(PlayerPos - LocalPlayerPos);
                    float angle = Vector3::Angle(targetDir, GetForward(Component_GetTransform(Camera_main()))) * 100.0;             
            
                    if (angle <= Fov_Aim && isFov1 && angle < shortestDistance) {
                     if(isVisible(Player)) {
                        shortestDistance = angle;
                        closestEnemy = Player;
                    }
                   }
                }
            }
        }
    }
    return closestEnemy;
}


             
static Quaternion MakeSpinRotation(float angleDeg)
{
    float rad  = angleDeg * 0.017453292f;
    float half = rad * 0.5f;

    Quaternion q;
    q.x = 0.0f;
    q.y = sinf(half);
    q.z = 0.0f;
    q.w = cosf(half);
    return q;
}



void (*LateUpdate)(void *Player);
void _LateUpdate(void *Player)
{
    if (!Player) {
        LateUpdate(Player);
        return;
    }
    if (!IsLocal(Player)) {
        LateUpdate(Player);
        return;
    }

    float dt = reinterpret_cast<float(*)()>(Time)();
    if (dt <= 0.0f) {
        LateUpdate(Player);
        return;
    }
    if (SpinEnable)
    {
        static float spinAngle = 0.0f;
        spinAngle += SpinSpeed * dt;

        if (spinAngle >= 360.0f)
            spinAngle -= 360.0f;

        Quaternion spinRot = MakeSpinRotation(spinAngle);
        void* transform = Component_GetTransform(Player);
        if (transform)
        {
            Transform_SetRotation(transform, spinRot);
        }
    }

    if (Aimbot)
    {
        void* CurrentMatch = Curent_Match();
        if (CurrentMatch)
        {
            void* LocalPlayer   = GetLocalPlayer(CurrentMatch);
            void* closestEnemy = GetClosestEnemy();

            if (LocalPlayer && closestEnemy)
            {
                Vector3 EnemyLocation  = GetHeadPosition(closestEnemy);
                Vector3 PlayerLocation = CameraMain(LocalPlayer);

                Quaternion look =
                    GetRotationToTheLocation(
                        EnemyLocation,
                        0.1f,
                        PlayerLocation
                    );

                bool firing = get_IsFiring(LocalPlayer);
                bool scope  = get_IsSighting(LocalPlayer);

                if (AimWhen == 0 ||
                   (AimWhen == 1 && firing) ||
                   (AimWhen == 2 && scope))
                {
                    set_aim(LocalPlayer, look);
                }
            }
        }
   
if (AimMagnet) {
    void* match = Curent_Match();
    void* local = GetLocalPlayer(match);

    if (match && local) {
        float shortestAngle = 15.0f; 
        void* target = nullptr;
        Vector3 localPos = getPosition(local);
        void* camTrans = Component_GetTransform(Camera_main());
        Vector3 camPos = Transform_GetPosition(camTrans);  // กล้อง pos
        Vector3 camFwd = Vector3::Normalized(GetForward(camTrans));

        // หาศัตรูใน FOV (บายพาส visibility สำหรับ magnet)
        auto players = *(monoDictionary<uint8_t*, void**>**) ((long)match + ListPlayer);
        if (players) {
            for (int i = 0; i < players->getNumValues(); i++) {
                void* enemy = players->getValues()[i];
                if (!enemy || get_isLocalTeam(enemy) || get_IsDieing(enemy)) continue;

                Vector3 enemyPos = getPosition(enemy);
                float angle = Vector3::Angle(Vector3::Normalized(enemyPos - localPos), camFwd);

                if (angle < shortestAngle) {
                    if (EnemyVisible && !get_isVisible(enemy) && !AimMagnet) continue;
                    shortestAngle = angle;
                    target = enemy;
                }
            }
        }

        if (target && get_IsFiring(local)) {
    void* tr = Component_GetTransform(target);
    if (tr) {
        Vector3 head = GetHeadPosition(target);
        float headOffset = 1.5f;
        Vector3 root = getPosition(target);
        float dist = Vector3::Distance(camPos, head);
        
        if (dist <= AimMagnetMaxDistance) {
            Vector3 targetPoint = camPos + camFwd * dist;
            Vector3 rootTarget = targetPoint - Vector3(0, headOffset, 0);
            
            Vector3 newRoot = Vector3::Lerp(root, rootTarget, AimMagnetStrength);
            
            Transform_INTERNAL_SetPosition(tr, Vvector3(newRoot.x, newRoot.y, newRoot.z));
            magnetLastEnemy = target;
        }
    }
}
        else if (magnetLastEnemy) {
            void* tr = Component_GetTransform(magnetLastEnemy);
            if (tr) {
                Vector3 original = getPosition(magnetLastEnemy);
                Transform_INTERNAL_SetPosition(tr, Vvector3(original.x, original.y, original.z));
            }
            magnetLastEnemy = nullptr;
        }
    }
	}
}
// --------------------------------------------------------
// UP PLAYER
// --------------------------------------------------------
/*
if (upPlayer) {
    void* enemy = GetClosestEnemy();
    if (enemy && enemy != LocalPlayer) {
        if (get_IsDieing(enemy)) {
            upPlayer = false;
            return;
        }
        if (!get_isVisible(closestEnemy)) {
                return;  // Skip ถ้าไม่ visible
            }
        void* enemyTF = get_transform(enemy);
        if (enemyTF) {
            Vector3 pos = get_position11(enemyTF);
            Vector3 localPos = get_position11(get_transform(LocalPlayer));
            
            float dist = Vector3::Distance(localPos, pos);
            if (dist > 50.0f) {
                return;  // Skip ถ้าไกลเกิน
            }
            
            pos.y += UpPlayerHeight;
            
            Transform_INTERNAL_SetPosition(enemyTF, Vvector3(pos.x, pos.y, pos.z));
        }
    } else {
        
    }
}*/
    LateUpdate(Player);
}


inline void DrawESP(float screenWidth, float screenHeight) {
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
	/*
    if (!draw) return;

    std::string title = "AXL MODS V1 || x64";
    float size = 20.0f; // text size
    ImVec2 textSize = ImGui::CalcTextSize(title.c_str());

    float padding = 8.0f;
    float x = (screenWidth - textSize.x) * 0.5f;
    float y = 20.0f; // top padding

    ImVec2 boxMin = ImVec2(x - padding, y - padding);
    ImVec2 boxMax = ImVec2(x + textSize.x + padding, y + textSize.y + padding);

    // Draw black filled box
    draw->AddRectFilled(
        boxMin,
        boxMax,
        IM_COL32(0, 192, 192, 192) // semi-transparent black
    );

    // Draw white border
    draw->AddRect(
        boxMin,
        boxMax,
        IM_COL32(0, 192, 192, 192), // white
        4.0f, // corner rounding
        0,
        2.0f // border thickness
    );

    // Draw green text
    draw->AddText(
        ImVec2(x, y),
        IM_COL32(0, 0, 0, 255), // green
        title.c_str()
    );
/*
  if (isSpeedSafe) {
        isSpeed = 4;
    } else {
        isSpeed = 0;
    }
*/

           


 if (Enable && !stream_mode) {
void* current_Match = Curent_Match();
void* local_player = GetLocalPlayer(current_Match);

if (local_player && current_Match) {  
        auto* players = *(monoDictionary<uint8_t*, void**>**)((long)current_Match + ListPlayer);  
        void* camera = Camera_main();  

        if (players && camera) {  
            for (int u = 0; u < players->getNumValues(); u++) {  
                void* closestEnemy = players->getValues()[u];  
                if (closestEnemy != local_player && closestEnemy && get_isVisible(closestEnemy) && !get_isLocalTeam(closestEnemy)) {  
                    Vector3 Toepos = getPosition(closestEnemy);  
                    Vector3 Toeposi = WorldToScreenPoint(camera, Toepos);  
                    if (Toeposi.z < 1) continue;  

                    Vector3 HeadPos = getPosition(closestEnemy) + Vector3(0, 1.9f, 0);  
                    Vector3 HeadPosition = WorldToScreenPoint(camera, HeadPos);  
                    if (HeadPosition.z < 1) continue;  

                    draw->AddCircle(ImVec2(screenWidth / 2, screenHeight / 2), Fov_Aim, ImColor(255, 255, 255), 0, 1.5f);  

                    float distance = Vector3::Distance(getPosition(local_player), Toepos);  
                    float Height = abs(HeadPosition.y - Toeposi.y) * (1.2 / 1.1);  
                    float Width = Height * 0.50f;  
                   
				Rect rect = Rect(HeadPosition.x - Width / 2.f, screenHeight - HeadPosition.y, Width, Height);

if (Aimbot && closestEnemy && EspGrenade) {
Vector3 LocalHead = GetHeadPosition(local_player);
Vector3 EnemyHead = GetHeadPosition(closestEnemy);

GrenadeLine_DrawLine(LineGrenade, LocalHead, LocalHead, Vector3(0, 0.1f, 0) * 0.1);  
     if (RenderLine) {  
         UnityColor RedColor = {1.0f, 0.0f, 0.0f, 1.0f};  
         LineRenderer_SetColor(RenderLine, RedColor);  

         LineRenderer_Set_PositionCount(RenderLine, 0x2);  
         LineRenderer_SetPosition(RenderLine, 0, LocalHead);  
         LineRenderer_SetPosition(RenderLine, 1, EnemyHead);  
     }  
}


if (Config.ESP.Line2) {
ImVec2 start = ImVec2(screenWidth / 2.0f, screenHeight / 2.0f);
ImVec2 end = ImVec2(rect.x + rect.w / 2.0f, rect.y + rect.h / 2.0f);
bool isDown = get_IsDieing(closestEnemy);

static float hue = 0.0f;  
hue += 0.01f;  
if (hue > 1.0f) hue = 0.0f;  
ImColor rgbColor = ImColor::HSV(hue, 1.0f, 1.0f);  
draw->AddLine(start, end, ImColor(0, 0, 0, 180), 10.0f);  
draw->AddLine(start, end, ImColor::HSV(hue, 1.0f, 1.0f, 0.3f), 8.0f);  
ImColor lineColor = isDown ? ImColor(255, 0, 0, 255) : rgbColor;  
draw->AddLine(start, end, lineColor, 3.5f);  

float pulse = sin(ImGui::GetTime() * 8) * 2.0f + 10.0f;  
draw->AddCircleFilled(end, pulse + 5.0f, ImColor(255, 0, 0, 100), 32);  
draw->AddCircleFilled(end, pulse, rgbColor, 32);  

if (!isDown) {  
    const char* label = "";  
    ImVec2 textSize = ImGui::CalcTextSize(label);  

ImVec2 textPos = ImVec2(end.x - textSize.x / 2, end.y - 20);
draw->AddText(textPos, IM_COL32(255, 255, 255, 220), label);
}
}
if (Config.ESP.Line && closestEnemy)
{
ImVec2 start(screenWidth * 0.5f, 10.0f);

ImVec2 end(  
    rect.x + rect.w * 0.5f, 
    rect.y             
);  
bool isDown = get_IsDieing(closestEnemy);  

ImColor lineColor = isDown  
    ? ImColor(255, 0, 0, 255)  
    : ImColor(ESP_LineColor);  

draw->AddLine(  
    start,  
    end,  
    lineColor,  
    ESP_LineThickness  
);

}
if (Config.ESP.Box)
{
    float x = rect.x;
    float y = rect.y;
    float w = rect.w;
    float h = rect.h;

    float rounding  = visual_esp_box;
    float thickness = ESP_LineThickness;

    ImVec2 topLeft(x, y);
    ImVec2 bottomRight(x + w, y + h);
    ImVec4 c = ESP_BoxColor;
    draw->AddRect(
        ImVec2(topLeft.x - 2, topLeft.y - 2),
        ImVec2(bottomRight.x + 2, bottomRight.y + 2),
        ImColor(c.x, c.y, c.z, 0.15f),
        rounding,
        0,
        thickness + 2.0f
    );
    draw->AddRect(
        ImVec2(topLeft.x - 1, topLeft.y - 1),
        ImVec2(bottomRight.x + 1, bottomRight.y + 1),
        ImColor(c.x, c.y, c.z, 0.30f),
        rounding,
        0,
        thickness + 1.0f
    );
    draw->AddRect(
        topLeft,
        bottomRight,
        ImColor(c.x, c.y, c.z, 1.0f),
        rounding,
        0,
        thickness
    );
    draw->AddRect(
        ImVec2(topLeft.x + 1, topLeft.y + 1),
        ImVec2(bottomRight.x - 1, bottomRight.y - 1),
        ImColor(c.x, c.y, c.z, 0.45f),
        rounding,
        0,
        1.0f
    );
}

int xx = rect.x + rect.w + 2;
int yy = rect.y;

if (Config.ESP.Health) {
    float hp    = (float)GetHp(closestEnemy);
    float maxHp = (float)get_MaxHP(closestEnemy);
    if (maxHp <= 0.0f) return;

    float healthPercent = hp / maxHp;
    if (healthPercent < 0.0f) healthPercent = 0.0f;
    if (healthPercent > 1.0f) healthPercent = 1.0f;
    int barWidth  = 6;               
    int barHeight = rect.h;           
    int barX = rect.x - barWidth - 4;  
    int barY = rect.y;
    ImColor barColor = ImColor(0, 255, 0);
    if (healthPercent <= 0.6f) barColor = ImColor(255, 255, 0);
    if (healthPercent <= 0.3f) barColor = ImColor(255, 0, 0);
    draw->AddRectFilled(
        ImVec2(barX, barY),
        ImVec2(barX + barWidth, barY + barHeight),
        ImColor(0, 0, 0, 180),
        2.0f
    );
    draw->AddRect(
        ImVec2(barX, barY),
        ImVec2(barX + barWidth, barY + barHeight),
        IM_COL32(255, 255, 255, 120),
        2.0f,
        0,
        1.0f
    );
    float filledHeight = (barHeight - 2) * healthPercent;

    draw->AddRectFilled(
        ImVec2(barX + 1, barY + barHeight - 1 - filledHeight),
        ImVec2(barX + barWidth - 1, barY + barHeight - 1),
        barColor,
        2.0f
    );
}
if (Config.ESP.Info)
{
    monoString* Nick = get_NickName(closestEnemy);
    const char* name = "Bot";
    char nameBuf[32];

    if (Nick)
    {
        int len = Nick->getLength();
        int j = 0;
        for (int i = 0; i < len && j < 31; i++)
        {
            char ch = get_Chars(Nick, i);
            nameBuf[j++] = isascii(ch) ? ch : '?';
        }
        nameBuf[j] = 0;
        name = nameBuf;
    }
    static void* ep[4] = {0};
    static int   en[4] = {1,2,3,4};
    int num = 1;
    for (int i = 0; i < 4; i++)
    {
        if (ep[i] == closestEnemy) { num = en[i]; break; }
        if (!ep[i]) { ep[i] = closestEnemy; num = en[i]; break; }
    }
    char nTxt[2] = { char('0' + num), 0 };

    ImVec2 ts = ImGui::CalcTextSize(name);
    ImVec2 ns = ImGui::CalcTextSize(nTxt);

    float h = ts.y + 1.0f;

    float x = rect.x + rect.w * 0.5f - (ts.x + ns.x + 5.0f) * 0.5f;
    float y = rect.y - h - 2.0f;

    ImVec2 a(x, y);
    ImVec2 b(x + ns.x + 2.5f, y + h);

    draw->AddRectFilled(a, b, ImColor(180, 40, 40, 140), 0.5f);
    draw->AddText(ImVec2(a.x + 1.2f, a.y), IM_COL32(255,255,255,220), nTxt);

    ImVec2 c(b.x + 1.5f, y);
    ImVec2 d(c.x + ts.x + 2.5f, y + h);

    draw->AddRectFilled(c, d, ImColor(0,0,0,90), 0.5f);
    draw->AddText(ImVec2(c.x + 1.2f, c.y), IM_COL32(255,255,255,220), name);
}
}}}}}}

bool (*EnableCheckWaterWhileSwimming)(void* thiz);
bool _EnableCheckWaterWhileSwimming(void* thiz) {
  if (fishRining) {
        return true;
    }
    return EnableCheckWaterWhileSwimming(thiz);
}
bool (*IsFoldWingGliding)(void *player);
bool _IsFoldWingGliding(void *player) {
	if (player != NULL) {
	    if(Enable) {
	        if (speedrun) {
		        return true;
            }
        }
	}
	return IsFoldWingGliding(player);
}

static int (*MappingFromPhysXState)(...);
static int _MappingFromPhysXState(void* state) {
 if(Enable) {
     if (speedrun) {
      return 1;
     }
    }
 return MappingFromPhysXState(state);
}
	
	
bool (*OnPreparationCancel)(void *thiz);
bool _OnPreparationCancel(void *thiz) {
 if (thiz != NULL) {
     if(Enable) {
         if (DoubleGun) {
          return true;
            }
        }
 }
 return _OnPreparationCancel(thiz);
}
bool (*GetCombineType)(void *thiz);
bool _GetCombineType(void *thiz) {
 if (thiz != NULL) {
     if(Enable) {
         if (DoubleGun) {
          return true;
            }
        }
 }
 return _GetCombineType(thiz);
}
bool (*OnStartChangeWeapon)(void *thiz);
bool _OnStartChangeWeapon(void *thiz) {
 if (thiz != NULL) {
     if(Enable) {
         if (FastSwitch) {
          return true;
            }
        }
 }
 return _OnStartChangeWeapon(thiz);
}

bool(*GetScatterRate)(void*_this, int value);
bool _GetScatterRate(void*_this, int value){
    if (_this != NULL) {
	    	if(norecoil) {
                return true;
            }
        return _GetScatterRate(_this, value);
    }
}


bool (*orig_NeedSendMessage)(void* _this, int Value);

bool hook_NeedSendMessage(void* _this, int Value) {
    if (_this != nullptr) {
        if (Ghost) {
            return true; 
        }
    }
    return orig_NeedSendMessage(_this, Value);
}

bool (*get_InSwapWeaponCD)(void* thiz);
bool _get_InSwapWeaponCD(void* thiz) {
  if (FastSwitch) {
        return false;
    }
    return get_InSwapWeaponCD(thiz);
}
	   

bool(*_ResetGuest)(void* _this);
bool ResetGuest(void* _this){
    if (_this != NULL) {
        if (Reset) {
            return true; 
            remove(OBFUSCATE("/storage/emulated/0/com.garena.msdk/guest100067.dat"));
        }
    }
}



