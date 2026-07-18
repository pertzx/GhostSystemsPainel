#pragma once

#define targetLib OBFUSCATE("libil2cpp.so")
uintptr_t il2cppMap;

//ESP Player
bool Enable_ESP;
bool PlayerLine;
bool PlayerBox;
bool PlayerHealth;
bool PlayerName;
bool HeroName;
bool PlayerDistance;
bool PlayerAlert;
float droneView;
bool Drone;
bool HackMap;
//ESP Monster
bool MonsterBox;
bool MonsterHealth;

MemoryPatch CalcVis, SFSD, RV;

struct sConfig {
	struct sInitImGui {
		bool initImGui;
		bool bInitDone;
	};
	
	sInitImGui InitImGui;
	
	struct sOther {
		bool espLine;
		bool espBox;
		bool espDistance;
		float getFieldOfView;
		float setFieldOfView;
		bool mapHack;
		bool sup60FPs;
		bool showUntil;
		int wideView;
		
	};
	sOther OtherMenu;
};

sConfig Config{0};
