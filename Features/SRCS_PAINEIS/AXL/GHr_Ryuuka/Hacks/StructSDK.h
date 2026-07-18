#pragma once

class Camera {
	public:
	
	static Camera *get_main() {
		Camera *(*get_main_) () = (Camera *(*)()) (IL2Cpp::Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Camera"), OBFUSCATE("get_main"), 0));
		return get_main_();
	}
	
	Vector3 WorldToScreenPoint(Vector3 position) {
		Vector3 (*WorldToScreenPoint_)(Camera *camera, Vector3 position) = (Vector3 (*)(Camera *, Vector3)) (IL2Cpp::Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Camera"), OBFUSCATE("WorldToScreenPoint"), 1));
		return WorldToScreenPoint_(this, position);
	}
	void set_fieldOfView(float view) {
		if(this == nullptr) return;
		void (*set_fieldOfView_)(Camera *camera, float value) = (void (*)(Camera *, float)) (IL2Cpp::Il2CppGetMethodOffset(OBFUSCATE("UnityEngine.dll"), OBFUSCATE("UnityEngine"), OBFUSCATE("Camera"), OBFUSCATE("set_fieldOfView"), 1));
	     set_fieldOfView_(this, view);
	}
};

class ValueLinkerComponent {
	public:
	
	int get_actorHp() {
		int (*get_actorHp_)(ValueLinkerComponent *objLinkerWrapper) = (int (*)(ValueLinkerComponent *)) (IL2Cpp::Il2CppGetMethodOffset(OBFUSCATE("Project_d.dll"), OBFUSCATE("Kyrios.Actor"), OBFUSCATE("ValueLinkerComponent"), OBFUSCATE("get_actorHp"), 0));
		return get_actorHp_(this);
	}
	
	int get_actorHpTotal() {
		int (*get_actorHpTotal_)(ValueLinkerComponent *objLinkerWrapper) = (int (*)(ValueLinkerComponent *)) (IL2Cpp::Il2CppGetMethodOffset(OBFUSCATE("Project_d.dll"), OBFUSCATE("Kyrios.Actor"), OBFUSCATE("ValueLinkerComponent"), OBFUSCATE("get_actorHpTotal"), 0));
		return get_actorHpTotal_(this);
	}	
};

class CActorInfo {
	public:
	
	String *ActorName() {
		return *(String **) ((uintptr_t) this + IL2Cpp::Il2CppGetFieldOffset(OBFUSCATE("Project_d.dll"), OBFUSCATE("Assets.Scripts.GameLogic"), OBFUSCATE("CActorInfo"), OBFUSCATE("ActorName")));
	}
};

class ActorLinker {
	public:
	
	ValueLinkerComponent *ValueComponent() {
		return *(ValueLinkerComponent **) ((uintptr_t) this + IL2Cpp::Il2CppGetFieldOffset(OBFUSCATE("Project_d.dll"), OBFUSCATE("Kyrios.Actor"), OBFUSCATE("ActorLinker"), OBFUSCATE("ValueComponent")));
	}
	
	Vector3 get_position() {
		Vector3 (*get_position_)(ActorLinker *linker) = (Vector3 (*)(ActorLinker *)) (IL2Cpp::Il2CppGetMethodOffset(OBFUSCATE("Project_d.dll"), OBFUSCATE("Kyrios.Actor"), OBFUSCATE("ActorLinker"), OBFUSCATE("get_position"), 0));
		return get_position_(this);
	}
	
	CActorInfo *CharInfo() {
		return *(CActorInfo **) ((uintptr_t) this + IL2Cpp::Il2CppGetFieldOffset(OBFUSCATE("Project_d.dll"), OBFUSCATE("Kyrios.Actor"), OBFUSCATE("ActorLinker"), OBFUSCATE("CharInfo")));
	}
	
	bool IsHostCamp() {
		bool (*IsHostCamp_)(ActorLinker *linker) = (bool (*)(ActorLinker *)) (IL2Cpp::Il2CppGetMethodOffset(OBFUSCATE("Project_d.dll"), OBFUSCATE("Kyrios.Actor"), OBFUSCATE("ActorLinker"), OBFUSCATE("IsHostCamp"), 0));
		return IsHostCamp_(this);
	}
};

class ActorManager {
	public:
	
	List<ActorLinker *> *GetAllHeros() {
		List<ActorLinker *> *(*_GetAllHeros)(ActorManager *actorManager) = (List<ActorLinker *> *(*)(ActorManager *)) (IL2Cpp::Il2CppGetMethodOffset(OBFUSCATE("Project_d.dll"), OBFUSCATE("Kyrios.Actor"), OBFUSCATE("ActorManager"), OBFUSCATE("GetAllHeros"), 0));
		return _GetAllHeros(this);
	}
};

class KyriosFramework {
	public:
	
	static ActorManager *get_actorManager() {
		auto get_actorManager_ = (ActorManager *(*)()) (IL2Cpp::Il2CppGetMethodOffset(OBFUSCATE("Project_d.dll"), OBFUSCATE("Kyrios"), OBFUSCATE("KyriosFramework"), OBFUSCATE("get_actorManager"), 0));
		return get_actorManager_();
	}
};
