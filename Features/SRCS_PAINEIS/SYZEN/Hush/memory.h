#pragma once
//#include "MemoryPatch.h"

const float MAX_DISTANCE = 99999.0f;
const float AIM_ANGLE_MULTIPLIER = 70.0f;
static int aimPosition = 0;
const float NECK_OFFSET = -0.15f;
const float CHEST_OFFSET = -0.35f;
static int aimSmoothing = 0;
static bool ignoreKnockedEnemies = false;
static int aimMode = 0;

bool AimbotRage = false;
bool Headshot = false;
bool AimbotLegit = false;
bool AimbotAim = false;
bool AimbotAuto = false;
bool lineStyles = 0;

int Fov = 0;
bool Startgs = true;
bool Funcoes = true;
bool Active = false;
bool HidePanel = false;
bool AimSilent = false;
bool EspLine = false;
bool EspBox = false;
bool Nick = false;
bool Inimigos = false;
bool EspVida = false;
bool EspCircle = false;
bool EspSkeleton = false;
bool ResetGuest = false;
bool SpeedHack = false;
bool telegram = false;
bool UpPlayer = false;
bool TPPlayer = false;
bool NoRecoil = false;
bool EsnapLine = false;
bool g_FixLogin = false;
bool g_DestroyHackBypass = false;
bool patchesApplied = false;
const int TOP_LEFT = 0;
const int TOP = 1;
const int BOTTOM = 2;
const int CENTER = 3;
const int DISABLED = 4;
int currentLinePosition = TOP;
int tempLineStyle = 0;
int tempBoxStyle = 0;
int tempLifeStyle = 0;
int tempNameStyle = 0;

const int BOX_TYPE_1 = 0;
const int BOX_TYPE_2 = 1;
const int BOX_TYPE_3 = 2;
const int BOX_TYPE_4 = 3;
const int BOX_DISABLED = 4;
int currentBoxType = BOX_TYPE_1;

int selectedStyle = 1;
static int currentStyleIndex = 0;

bool Camera = true;
bool Aaa = false;


//AimKill
bool AimKill = false;
static int TimerTakeDamageinit = 0;
float maxDistanceAimkill = 100.0f;

