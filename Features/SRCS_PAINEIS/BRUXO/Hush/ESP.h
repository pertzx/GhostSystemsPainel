#pragma once
#include "importz.h"
#include "aimbot.h"
#include "pointers.h"
#include "THEME_CONFIG.h"
#include <algorithm>  // Para std::clamp, std::min

extern ImVec4 ThemeFov;
extern ImVec4 ThemeColer;

struct BonePos {
    std::string name;
    Vector3 pos;
};

std::vector<BonePos> GetAllNodePositions(void* enemy)
{
    static const struct { const char* name; uintptr_t offset; } bones[] = {
        {"Head",          m_HeadNode},
        {"Neck",          m_Neck},
        {"Hip",           m_HipNode},
        {"LeftShoulder",  m_LeftShoulder},
        {"RightShoulder", m_RightShoulder},
        {"LeftElbow",     m_LeftElbow},
        {"RightElbow",    m_RightElbow},
        {"LeftHand",      m_LeftHand},
        {"RightHand",     m_RightHand},
        {"LeftKnee",      m_LeftKnee},
        {"RightKnee",     m_RightKnee},
        {"LeftFoot",      m_LeftFoot},
        {"RightFoot",     m_RightFoot},
        {"Root",          m_Root},
    };

    std::vector<BonePos> result;
    result.reserve(std::size(bones));

    for (auto& b : bones)
    {
        void* iTransformNode = *(void**)((uintptr_t)enemy + b.offset);
        if (!iTransformNode) continue;

        void* transform = get_itransform(iTransformNode);
        if (!transform) continue;

        Vector3 pos = Transform_GetPosition(transform);
        result.push_back({ b.name, pos });
    }
    return result;
}

void DrawBones(const std::vector<BonePos>& bones, ImDrawList* drawList, float distance, float screenHeight)
{
    if (distance <= 0.0f) return;

    const float scaleFactor   = std::max(0.5f, 50.0f / distance);
    const float lineThickness = 0.3f * scaleFactor;  // SKELETON FINO
    // === ALTERADO PARA BRANCO ===
    const ImU32 boneColor     = IM_COL32(255, 255, 255, 255); // BRANCO
    // ===========================

    auto findIdx = [&](const char* name) -> int {
        for (int i = 0; i < (int)bones.size(); ++i)
            if (bones[i].name == name) return i;
        return -1;
    };

    std::vector<ImVec2> screen(bones.size());
    std::vector<bool>   visible(bones.size());

    for (size_t i = 0; i < bones.size(); ++i) {
        Vector3 s3 = WorldToScreenPoint(mainCamera, bones[i].pos);
        visible[i] = (s3.z > 0.0f);
        screen[i]  = ImVec2(s3.x, screenHeight - s3.y);
    }

    auto DrawLineIfVisible = [&](const char* a, const char* b) {
        int ia = findIdx(a), ib = findIdx(b);
        if (ia >= 0 && ib >= 0 && visible[ia] && visible[ib])
            drawList->AddLine(screen[ia], screen[ib], boneColor, lineThickness);
    };

    // === CORPO (SEM CABEÇA) ===
    // Tronco
    DrawLineIfVisible("Neck", "Hip");

    // Braços
    DrawLineIfVisible("Neck", "LeftShoulder");
    DrawLineIfVisible("Neck", "RightShoulder");
    DrawLineIfVisible("LeftShoulder", "LeftElbow");
    DrawLineIfVisible("RightShoulder", "RightElbow");
    DrawLineIfVisible("LeftElbow", "LeftHand");
    DrawLineIfVisible("RightElbow", "RightHand");

    // Pernas
    DrawLineIfVisible("Hip", "LeftKnee");
    DrawLineIfVisible("Hip", "RightKnee");
    DrawLineIfVisible("LeftKnee", "LeftFoot");
    DrawLineIfVisible("RightKnee", "RightFoot");

    // Mãos e pés (agora em BRANCO)
    const float limbRadius = 0.4f * scaleFactor;
    auto DrawLimb = [&](const char* name) {
        int idx = findIdx(name);
        if (idx >= 0 && visible[idx])
            drawList->AddCircleFilled(screen[idx], limbRadius, boneColor);
    };

    DrawLimb("LeftHand");
    DrawLimb("RightHand");
    DrawLimb("LeftFoot");
    DrawLimb("RightFoot");
}

ImU32 ColorToUint32(ImVec4 color) {
    return ImGui::ColorConvertFloat4ToU32(color);
}

ImVec4 InterpolateColor(const ImVec4& a, const ImVec4& b, float t) {
    return ImVec4(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t
    );
}

void DrawHealthBar(ImDrawList* drawList, short health, short maxHealth, float x, float y, float height, float boxWidth)
{
    if (maxHealth <= 0) maxHealth = 100;

    float healthPct = std::clamp(static_cast<float>(health) / maxHealth, 0.0f, 1.0f);
    float healthWidth = boxWidth * healthPct;

    static const ImVec4 RED = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    static const ImVec4 YELLOW = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    static const ImVec4 GREEN = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    static const ImVec4 BG = ImVec4(0.39f, 0.0f, 0.0f, 1.0f);
    static const ImVec4 BLACK = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

    ImVec4 healthColor;
    if (healthPct >= 0.5f) {
        float t = (healthPct - 0.5f) * 2.0f;
        healthColor = InterpolateColor(YELLOW, GREEN, t);
    }
    else {
        float t = healthPct * 2.0f;
        healthColor = InterpolateColor(RED, YELLOW, t);
    }

    ImU32 colBG = ColorToUint32(BG);
    ImU32 colHealth = ColorToUint32(healthColor);
    ImU32 colBorder = ColorToUint32(BLACK);

    drawList->AddRectFilled(
        ImVec2(x, y - height),
        ImVec2(x + boxWidth, y),
        colBG
    );

    drawList->AddRectFilled(
        ImVec2(x, y - height),
        ImVec2(x + healthWidth, y),
        colHealth
    );

    drawList->AddRect(
        ImVec2(x, y - height),
        ImVec2(x + boxWidth, y),
        colBorder,
        0.0f, 0, 1.0f
    );
}

void DrawESP(float screenWidth, float screenHeight) {
    savedScreenWidth = screenWidth; savedScreenHeight = screenHeight;
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    if (!draw || !Active) return;
    
    if (!local_player || !current_Match) return;

    monoDictionary<uint8_t*, void**>* players = *(monoDictionary<uint8_t*, void**>**)((long)current_Match + ListPlayer);
    if (!players || !mainCamera) return;

    int numVisibleEnemies = 0;
    int enemyIndex = 0;

    for (int i = 0; i < players->getNumValues(); ++i) {
        void* enemy = players->getValues()[i];
        if (!enemy || enemy == local_player || get_isLocalTeam(enemy) || !get_isVisible(enemy)) continue;

        ++numVisibleEnemies;
        ++enemyIndex;
        bool isKnocked = get_IsDieing(enemy);
        Vector3 toePos = getPosition(enemy);
        Vector3 toeScreen = WorldToScreenPoint(mainCamera, toePos);
        if (toeScreen.z < 1) continue;

        Vector3 headPos = isKnocked ? toePos + Vector3(0, 0.20f, 0) : toePos + Vector3(0, 1.32f, 0);
        Vector3 headScreen = WorldToScreenPoint(mainCamera, headPos);
        if (headScreen.z < 1) continue;

        float distance = Vector3::Distance(getPosition(local_player), headPos);

        float height = abs(headScreen.y - toeScreen.y) * (1.2f / 1.1f);
        float width = height * 0.5f;
        Rect rect(headScreen.x - width / 2.f, screenHeight - headScreen.y, width, height);

        if (EspLine) {
            ImVec2 startPoint = ImVec2(0.0f, 0.0f);
            if (tempLineStyle == 0) {
                startPoint = ImVec2(screenWidth / 2.0f, 10.0f);
            } else if (tempLineStyle == 1) {
                startPoint = ImVec2(screenWidth / 2.0f, screenHeight - 10.0f);
            }

            ImColor lineColor = isKnocked ? ImColor(255, 0, 0, 255) : ImColor(ThemeColer);

            if (!isKnocked) {
                int segments = 100;
                for (int j = 0; j < segments; ++j) {
                    float t = j / float(segments);
                    ImVec2 segmentStart = ImLerp(startPoint, ImVec2(headScreen.x, screenHeight - headScreen.y), t);
                    ImVec2 segmentEnd = ImLerp(startPoint, ImVec2(headScreen.x, screenHeight - headScreen.y), t + 2.5f / segments);
                    draw->AddLine(segmentStart, segmentEnd, lineColor, 2.5f);
                }
            }
            else {
                draw->AddLine(startPoint, ImVec2(headScreen.x, screenHeight - headScreen.y), lineColor, 2.5f);
            }
        }

        if (EspBox) {
            ImColor boxColor = isKnocked ? ImColor(255, 0, 0, 255) : ImColor(ThemeColer);

            float lineW = 1.5f;
            float radius = rect.w * 0.20f;
            radius = ImMin(radius, ImMin(rect.w, rect.h) * 0.5f);

            ImVec2 min = ImVec2(rect.x, rect.y);
            ImVec2 max = ImVec2(rect.x + rect.w, rect.y + rect.h);

            if (tempBoxStyle == 0) {
                draw->PathLineTo(ImVec2(min.x + radius, min.y));
                draw->PathLineTo(ImVec2(max.x - radius, min.y));
                draw->PathArcTo(ImVec2(max.x - radius, min.y + radius), radius, -M_PI / 2.0f, 0.0f);
                draw->PathLineTo(ImVec2(max.x, max.y - radius));
                draw->PathArcTo(ImVec2(max.x - radius, max.y - radius), radius, 0.0f, M_PI / 2.0f);
                draw->PathLineTo(ImVec2(min.x + radius, max.y));
                draw->PathArcTo(ImVec2(min.x + radius, max.y - radius), radius, M_PI / 2.0f, M_PI);
                draw->PathLineTo(ImVec2(min.x, min.y + radius));
                draw->PathArcTo(ImVec2(min.x + radius, min.y + radius), radius, M_PI, 3.0f * M_PI / 2.0f);
                draw->PathStroke(boxColor, false, lineW);
            }
            else if (tempBoxStyle == 1) {
                draw->AddRect(min, max, boxColor, 2.5f, 0, lineW);
            }
        }

        if (Nick) {
            monoString* EspNameStr = get_NickName(enemy);
            std::string enemyName = MonoStringToUtf8(EspNameStr);
            if (enemyName.size() > 12) 
                enemyName = enemyName.substr(0, 12) + "...";

            ImFont* font = ImGui::GetFont();
            float scaledFontSize = font->FontSize * 0.65f;
            ImVec2 nameSize = font->CalcTextSizeA(scaledFontSize, 1e9f, 0.0f, enemyName.c_str());

            ImVec2 headScreenPos(rect.x + rect.w * 0.5f, rect.y);

            if (tempNameStyle == 0) {
                draw->AddText(
                    font,
                    scaledFontSize,
                    ImVec2(headScreenPos.x - (nameSize.x * 0.5f),
                           headScreenPos.y - nameSize.y - 2.0f),
                    ImColor(ThemeColer),
                    enemyName.c_str()
                );
            }
            else if (tempNameStyle == 1) {
                draw->AddRectFilled(
                    ImVec2(headScreenPos.x - (nameSize.x * 0.5f) - 3.0f,
                           headScreenPos.y - nameSize.y - 4.0f),
                    ImVec2(headScreenPos.x + (nameSize.x * 0.5f) + 3.0f,
                           headScreenPos.y - 0.0f),
                    IM_COL32(0, 0, 0, 120),
                    2.0f
                );

                draw->AddText(
                    font,
                    scaledFontSize,
                    ImVec2(headScreenPos.x - (nameSize.x * 0.5f),
                           headScreenPos.y - nameSize.y - 2.0f),
                    ImColor(ThemeColer),
                    enemyName.c_str()
                );
            }
        }

        if (EspVida) {
            float maxHealth = get_MaxHP(enemy);
            float currentHealth = GetHp(enemy);
            float healthPercentage = (maxHealth > 0.0f) ? currentHealth / maxHealth : 0.0f;
            healthPercentage = std::clamp(healthPercentage, 0.0f, 1.0f);

            ImVec4 colorGreen  = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
            ImVec4 colorYellow = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
            ImVec4 colorRed    = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
            ImVec4 healthColor;

            if (healthPercentage > 0.5f) {
                float t = (healthPercentage - 0.5f) / 0.5f;
                healthColor = ImLerp(colorYellow, colorGreen, t);
            } else {
                float t = healthPercentage / 0.5f;
                healthColor = ImLerp(colorRed, colorYellow, t);
            }

            float barWidth = 3.0f;
            float spacing = 3.0f;

            auto DrawHealthBar = [&](float xOffset) {
                ImVec2 barPos(xOffset, rect.y);
                ImVec2 barSize(barWidth, rect.h * healthPercentage);

                draw->AddRectFilled(
                    ImVec2(barPos.x, barPos.y),
                    ImVec2(barPos.x + barWidth, barPos.y + rect.h),
                    ImColor(40, 40, 40, 150)
                );

                draw->AddRectFilled(
                    ImVec2(barPos.x, barPos.y + rect.h - barSize.y),
                    ImVec2(barPos.x + barWidth, barPos.y + rect.h),
                    ImColor(healthColor)
                );

                draw->AddRect(
                    ImVec2(barPos.x, barPos.y),
                    ImVec2(barPos.x + barWidth, barPos.y + rect.h),
                    ImColor(0, 0, 0, 255)
                );
            };

            if (tempLifeStyle == 0) {
                DrawHealthBar(rect.x - barWidth - spacing);
            } 
            else if (tempLifeStyle == 1) {
                DrawHealthBar(rect.x + rect.w + spacing);
            }
        }

        if (EspSkeleton) {
            std::vector<BonePos> bonePositions = GetAllNodePositions(enemy);
            DrawBones(bonePositions, draw, distance, screenHeight);
        }
    }


    if (Inimigos) {
        if (numVisibleEnemies > 0) { // Alterado para > 0 para incluir o primeiro inimigo (o seu código usava > 1, corrigi para ser inclusivo se necessário)
            int enemiesToShowOnTop = std::min(numVisibleEnemies, 60); // Contagem de inimigos
            
            // --- Componentes de texto e cores ---
            std::string countText = std::to_string(enemiesToShowOnTop);
            std::string playerText = " PLAYERS"; 
            
            float textHeight = 40.0f;
            
            // Calculo de tamanho para centralização
            ImFont* font = ImGui::GetFont();
            ImVec2 countSize = font->CalcTextSizeA(textHeight, 1e9f, 0.0f, countText.c_str());
            ImVec2 playerSize = font->CalcTextSizeA(textHeight, 1e9f, 0.0f, playerText.c_str());
            
            float totalWidth = countSize.x + playerSize.x;
            ImVec2 basePos(screenWidth / 2.0f - totalWidth / 2.0f, 10);
            
            float backgroundPadding = 20.0f;
            ImVec2 rectMin(basePos.x - backgroundPadding, basePos.y - 10);
            ImVec2 rectMax(basePos.x + totalWidth + backgroundPadding, basePos.y + textHeight + 10);
            
            // Cores
            ImU32 countColor = IM_COL32(255, 0, 0, 255); // VERMELHO
            ImU32 playersColor = IM_COL32(255, 255, 255, 255); // BRANCO

            // 1. Desenha o Background (mantido transparente)
            draw->AddRectFilled(rectMin, rectMax, ImColor(0, 0, 0, 0), 10.0f);
            draw->AddRect(rectMin, rectMax, ImColor(0, 0, 0, 0), 2.0f);

            // 2. Desenha a Contagem (VERMELHO)
            draw->AddText(font, textHeight, basePos, countColor, countText.c_str());

            // 3. Desenha o Texto "PLAYERS" (BRANCO)
            ImVec2 playerPos(basePos.x + countSize.x, basePos.y);
            draw->AddText(font, textHeight, playerPos, playersColor, playerText.c_str());
        }

        if (EsnapLine && closestEnemy != nullptr) {
            Vector3 enemyPos = GetHeadPosition(closestEnemy);
            Vector3 headScreen = WorldToScreenPoint(mainCamera, enemyPos);

            if (headScreen.z >= 0) {
                ImVec2 screenCenter(
                    ImGui::GetIO().DisplaySize.x / 2.0f,
                    ImGui::GetIO().DisplaySize.y / 2.0f
                );

                ImVec2 endPoint(
                    headScreen.x,
                    screenHeight - headScreen.y
                );

                draw->AddLine(
                    screenCenter,
                    endPoint,
                    ImColor(ThemeColer),
                    1.0f
                );
            }
        }
    }
    
	  if (EspCircle) {
        ImVec2 screenCenter = ImVec2(ImGui::GetIO().DisplaySize.x / 2.0f, ImGui::GetIO().DisplaySize.y / 2.0f);
        draw->AddCircle(screenCenter, Fov, ImColor(ThemeFov), 64, 1.0f);
    }
}

