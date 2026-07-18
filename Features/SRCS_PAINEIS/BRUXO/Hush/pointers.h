#pragma once
#include "importz.h"

void* current_Match = nullptr;
void* local_player = nullptr;
void* local_playerInMatch = nullptr;
void* bestEnemy = nullptr;
void* closestEnemy = nullptr;
void* localPlayerAttributes = nullptr;

void* mainCamera = nullptr;
Vector3 playerLocation = Vector3::zero();
Vector3 playerPosition = Vector3::zero();
Vector3 playerForward = Vector3::zero();

bool isFiring = false;
bool isScoped = false;
bool isAlive = false;
bool saved = false;

void* localTransform = nullptr;
void* enemyTransform = nullptr;
Vector3 enemyPosition = Vector3::zero();
float distanceToLocalPlayer = 0.0f;

bool inMatch = false;
int matchState = 0;

float savedScreenWidth = 0;
float savedScreenHeight = 0;
float active = 0.0f;
float desactive = 0.0f;

Vector3 ammoBase = Vector3::zero();
Vector3 enemyPos = Vector3::zero();
Vector3 dir = Vector3::zero();
void* hitObjInfo = nullptr;
