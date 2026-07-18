#pragma once
#include "Entity.h"
#include <vector>
#include <string>
#include <cmath>

namespace GhostSystems {

// Offsets baseados nos paineis BRUXO/SYZEN que funcionam em qualquer rig
// (nao depende de GetBoneTransform do Animator — le diretamente do Transform)
namespace BoneOffsets {
    constexpr uintptr_t m_HeadNode = 0x20;       // cabeca
    constexpr uintptr_t m_Neck = 0x28;           // pescoco
    constexpr uintptr_t m_HipNode = 0x38;        // quadril
    constexpr uintptr_t m_LeftShoulder = 0x40;   // ombro esquerdo
    constexpr uintptr_t m_RightShoulder = 0x48;  // ombro direito
    constexpr uintptr_t m_LeftElbow = 0x50;      // cotovelo esquerdo
    constexpr uintptr_t m_RightElbow = 0x58;     // cotovelo direito
    constexpr uintptr_t m_LeftHand = 0x60;       // mao esquerda
    constexpr uintptr_t m_RightHand = 0x68;      // mao direita
    constexpr uintptr_t m_LeftKnee = 0x70;       // joelho esquerdo
    constexpr uintptr_t m_RightKnee = 0x78;      // joelho direito
    constexpr uintptr_t m_LeftFoot = 0x80;       // pe esquerdo
    constexpr uintptr_t m_RightFoot = 0x88;      // pe direito
    constexpr uintptr_t m_Root = 0x10;           // root/transform base
}

struct BonePos {
    std::string name;
    Vector3 pos;
    bool valid;
};

// Obtem posicao de um bone pelo offset do transform (estilo BRUXO)
inline Vector3 GetBonePosByOffset(void* playerObj, uintptr_t transformOffset) {
    if (!playerObj) return {0, 0, 0};
    void* iTransformNode = *(void**)((uintptr_t)playerObj + transformOffset);
    if (!iTransformNode) return {0, 0, 0};

    // Tenta obter o transform direto (BRUXO usa get_itransform)
    // Se nao tiver a funcao get_itransform, acessa diretamente a matriz
    void* transform = iTransformNode;

    // Tenta extrair posicao do Transform
    // Unity Transform: m_LocalPosition = 0x10, m_LocalRotation = 0x1C, m_LocalScale = 0x2C
    // Ou m_position = offset variavel dependendo da versao
    // Usamos o metodo get_position do Il2Cpp como fallback mais seguro
    static void* getPositionMethod = nullptr;
    static bool positionCached = false;

    if (!positionCached) {
        void* transformKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Transform");
        if (!transformKlass) transformKlass = Il2Cpp::GetClass("UnityEngine.CoreModule", "UnityEngine", "Transform");
        if (transformKlass) {
            getPositionMethod = Il2Cpp::GetMethodRecursively(transformKlass, "get_position", 0);
        }
        positionCached = true;
    }

    if (getPositionMethod) {
        void* exc = nullptr;
        void* posObj = Il2Cpp::runtime_invoke(getPositionMethod, transform, nullptr, &exc);
        if (posObj && !exc) {
            Vector3Args* pos = (Vector3Args*)((uintptr_t)posObj + 0x10);
            return Vector3{pos->x, pos->y, pos->z};
        }
    }

    // Fallback: leitura direta da estrutura (Unity 2020+)
    // m_LocalPosition fica em 0x10 do Transform
    Vector3* localPos = (Vector3*)((uintptr_t)transform + 0x10);
    return Vector3{localPos->x, localPos->y, localPos->z};
}

// Retorna todos os bones do jogador (estilo GetAllNodePositions do BRUXO)
inline std::vector<BonePos> GetAllNodePositions(void* playerObj) {
    std::vector<BonePos> result;

    struct BoneEntry { const char* name; uintptr_t offset; };
    static const BoneEntry bones[] = {
        {"Head",         BoneOffsets::m_HeadNode},
        {"Neck",         BoneOffsets::m_Neck},
        {"Hip",          BoneOffsets::m_HipNode},
        {"LeftShoulder", BoneOffsets::m_LeftShoulder},
        {"RightShoulder",BoneOffsets::m_RightShoulder},
        {"LeftElbow",    BoneOffsets::m_LeftElbow},
        {"RightElbow",   BoneOffsets::m_RightElbow},
        {"LeftHand",     BoneOffsets::m_LeftHand},
        {"RightHand",    BoneOffsets::m_RightHand},
        {"LeftKnee",     BoneOffsets::m_LeftKnee},
        {"RightKnee",    BoneOffsets::m_RightKnee},
        {"LeftFoot",     BoneOffsets::m_LeftFoot},
        {"RightFoot",    BoneOffsets::m_RightFoot},
    };

    result.reserve(sizeof(bones) / sizeof(BoneEntry));

    for (auto& b : bones) {
        Vector3 pos = GetBonePosByOffset(playerObj, b.offset);
        result.push_back({b.name, pos, (pos.x != 0 || pos.y != 0 || pos.z != 0)});
    }

    return result;
}

// Desenha esqueleto conectando os bones (DrawBones do BRUXO adaptado para ImGui)
inline void DrawSkeleton(const std::vector<BonePos>& bones, ImDrawList* drawList, float distance, float screenH, ImU32 color) {
 if (bones.empty() || distance <= 0.0f) return;

 const float scaleFactor = std::max(0.5f, 50.0f / distance);
 const float lineThickness = 0.3f * scaleFactor;

 auto findIdx = [&](const char* name) -> int {
 for (int i = 0; i < (int)bones.size(); ++i)
 if (bones[i].name == name) return i;
 return -1;
 };

 std::vector<ImVec2> screen(bones.size());
 std::vector<bool> visible(bones.size());

 static void* mainCamera = nullptr;
 static void* worldToScreenMethod = nullptr;
 static bool methodsCached = false;
 if (!methodsCached) {
 void* cameraKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Camera");
 if (cameraKlass) worldToScreenMethod = Il2Cpp::GetMethodRecursively(cameraKlass, "WorldToScreenPoint", 1);
 mainCamera = Il2Cpp::runtime_invoke(Il2Cpp::GetMethodRecursively(cameraKlass, "get_main", 0), nullptr, nullptr, nullptr);
 methodsCached = true;
 }

 for (size_t i = 0; i < bones.size(); ++i) {
 if (!bones[i].valid) { visible[i] = false; continue; }
 Vector3Args* pos3 = (Vector3Args*)&bones[i].pos;
 void* args[1] = { pos3 };
 void* exc = nullptr;
 void* w2s = Il2Cpp::runtime_invoke(worldToScreenMethod, mainCamera, args, &exc);
 Vector3Args sPos = *(Vector3Args*)((uintptr_t)w2s + 0x10);
 visible[i] = (sPos.z > 0.0f);
 screen[i] = ImVec2(sPos.x, screenH - sPos.y);
 }

 auto DrawLineIfVisible = [&](const char* a, const char* b) {
 int ia = findIdx(a), ib = findIdx(b);
 if (ia >= 0 && ib >= 0 && visible[ia] && visible[ib])
 drawList->AddLine(screen[ia], screen[ib], color, lineThickness);
 };

 DrawLineIfVisible("Neck", "Hip");
 DrawLineIfVisible("Neck", "LeftShoulder");
 DrawLineIfVisible("Neck", "RightShoulder");
 DrawLineIfVisible("LeftShoulder", "LeftElbow");
 DrawLineIfVisible("RightShoulder", "RightElbow");
 DrawLineIfVisible("LeftElbow", "LeftHand");
 DrawLineIfVisible("RightElbow", "RightHand");
 DrawLineIfVisible("Hip", "LeftKnee");
 DrawLineIfVisible("Hip", "RightKnee");
 DrawLineIfVisible("LeftKnee", "LeftFoot");
 DrawLineIfVisible("RightKnee", "RightFoot");
}

}