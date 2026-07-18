#pragma once
#include "importz.h"
#include "aimbot.h"
#include "BatteryInfo.h"
#include <cstdint>   // uintptr_t

extern "C" uintptr_t getRealOffset(uintptr_t off);
extern "C" int DobbyHook(void* function, void* replace, void** origin) __attribute__((weak));
extern bool (*Pclogo)(void* self);

// ----- Compat: IM_PI em versões antigas do ImGui -----
#ifndef IM_PI
#define IM_PI 3.14159265358979323846f
#endif

// ======= COR/POSIÇÃO DAS ESP (compartilhado) =======
// NÃO defina ESP_CFG_DEFINE neste TU.
#include "esp_cfg.h"   // fornece GetEspColor() e GetEspYOffset()

void DrawESP(float screenWidth, float screenHeight) {
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    if (!draw) return;

    const ImU32 ESP_COLOR  = GetEspColor();                 // cor global das ESP
    const float ESP_YOFF   = GetEspYOffset(screenHeight);   // offset vertical global
    const ImU32 NEON_GREEN = IM_COL32(57, 255, 20, 255);    // se ainda for usado em algo

    // ===== FOV (roxinho -> rosa) =====
    if (Fov_Aim > 0.0f) {
        ImVec2 screenSize   = ImGui::GetIO().DisplaySize;
        ImVec2 screenCenter = ImVec2(screenSize.x * 0.5f, screenSize.y * 0.5f);

        const float radius    = Fov_Aim * 1.6f;
        const float thickness = 4.5f;
        const int   segs      = 120;

       const ImU32 FOV_PURPLE = IM_COL32(255, 0, 0, 255); // vermelho (substituindo o roxinho)
       const ImU32 FOV_PINK   = IM_COL32(255, 0, 0, 255); // vermelho (substituindo o rosa)

        auto LerpColor = [](ImU32 a, ImU32 b, float t)->ImU32 {
            ImColor ca(a), cb(b);
            float r = ca.Value.x + (cb.Value.x - ca.Value.x) * t;
            float g = ca.Value.y + (cb.Value.y - ca.Value.y) * t;
            float bl= ca.Value.z + (cb.Value.z - ca.Value.z) * t;
            float al= ca.Value.w + (cb.Value.w - ca.Value.w) * t;
            return ImColor(r, g, bl, al);
        };

        for (int i = 0; i < segs; ++i) {
            float t0 = (float)i / segs;
            float t1 = (float)(i + 1) / segs;

            float a0 = t0 * 2.0f * IM_PI;
            float a1 = t1 * 2.0f * IM_PI;

            ImVec2 p0 = ImVec2(screenCenter.x + cosf(a0) * radius, screenCenter.y + sinf(a0) * radius);
            ImVec2 p1 = ImVec2(screenCenter.x + cosf(a1) * radius, screenCenter.y + sinf(a1) * radius);

            ImU32 col = LerpColor(FOV_PURPLE, FOV_PINK, t0);
            draw->AddLine(p0, p1, col, thickness);
        }
    }

    // ===== Crosshair =====
    if (Crosshair) {
        ImVec2 center(screenWidth * 0.5f, screenHeight * 0.5f);
        draw->AddCircleFilled(center, 3.0f, ImColor(255,255,255,255));
        float lineLength = 15.0f;
        float lineThickness = 2.0f;
        draw->AddLine(ImVec2(center.x - lineLength - 3.0f, center.y), ImVec2(center.x - 3.0f, center.y), ImColor(255,255,255,255), lineThickness);
        draw->AddLine(ImVec2(center.x + 3.0f, center.y), ImVec2(center.x + lineLength + 3.0f, center.y), ImColor(255,255,255,255), lineThickness);
        draw->AddLine(ImVec2(center.x, center.y - lineLength - 3.0f), ImVec2(center.x, center.y - 3.0f), ImColor(255,255,255,255), lineThickness);
        draw->AddLine(ImVec2(center.x , center.y + 3.0f), ImVec2(center.x, center.y + lineLength + 3.0f), ImColor(255,255,255,255), lineThickness);
    }

    // ===== Hexágono central =====
    if (Hexagono) {
        ImVec2 center(screenWidth * 0.5f, screenHeight * 0.5f);
        ImVec2 vertices[6];
        float radius = 30.0f, angleOffset = IM_PI * 0.5f;
        for (int i = 0; i < 6; ++i) {
            float angle = angleOffset + i * (2 * IM_PI / 6);
            vertices[i] = ImVec2(center.x + cosf(angle) * radius, center.y + sinf(angle) * radius);
        }
        draw->AddPolyline(vertices, 6, ImColor(colorHexagono), true, 2.0f);
    }

    // ===== Guardas =====
    if (!AimbotEsp || !Funcoes) return;

    void* current_Match = Curent_Match();
    void* local_player  = GetLocalPlayer(current_Match);
    int inimigo_num = 0;

    if (local_player && current_Match) {
        monoDictionary<uint8_t *, void **>* players =
            *(monoDictionary<uint8_t*, void **> **)((long)current_Match + ListPlayer);
        void* camera = Camera_main();

        if (players && camera) {
            for (int u = 0; u < players->getNumValues(); u++) {
                void* closestEnemy = players->getValues()[u];
                if (closestEnemy && closestEnemy != local_player && !get_isLocalTeam(closestEnemy) && get_isVisible(closestEnemy)) {
                    inimigo_num++;

                    Vector3 Toepos = getPosition(closestEnemy);
                    Vector3 HeadPos = Toepos + Vector3(0,1.7f,0);
                    Vector3 Toeposi = WorldToScreenPoint(camera, Toepos);
                    Vector3 HeadPosition = WorldToScreenPoint(camera, HeadPos);
                    if (Toeposi.z < 1 || HeadPosition.z < 1) continue;

                    float distance = Vector3::Distance(getPosition(local_player), HeadPos);
                    float Hight = fabsf(HeadPosition.y - Toeposi.y) * (1.2f / 1.1f);
                    float Width = Hight * 0.50f;

                    // y de tela (invertido) + offset global de ESP
                    float headY = (screenHeight - HeadPosition.y) + ESP_YOFF;
                    float toeY  = (screenHeight - Toeposi.y)    + ESP_YOFF; // (toeY está aqui caso use futuramente)

                    Rect rect(HeadPosition.x - Width*0.5f, headY, Width, Hight);

                    // ===== BOX =====
                    if (currentBoxType != BOX_DISABLED) {
                        float boxThickness = 3.2f;
                        switch (currentBoxType) {
                            case BOX_TYPE_1:
                                draw->AddLine(ImVec2(rect.x, rect.y), ImVec2(rect.x + rect.w*0.25f, rect.y), ESP_COLOR, boxThickness);
                                draw->AddLine(ImVec2(rect.x + rect.w*0.75f, rect.y), ImVec2(rect.x + rect.w, rect.y), ESP_COLOR, boxThickness);
                                draw->AddLine(ImVec2(rect.x + rect.w, rect.y), ImVec2(rect.x + rect.w, rect.y + rect.h*0.25f), ESP_COLOR, boxThickness);
                                draw->AddLine(ImVec2(rect.x + rect.w, rect.y + rect.h*0.75f), ImVec2(rect.x + rect.w, rect.y + rect.h), ESP_COLOR, boxThickness);
                                draw->AddLine(ImVec2(rect.x + rect.w, rect.y + rect.h), ImVec2(rect.x + rect.w*0.75f, rect.y + rect.h), ESP_COLOR, boxThickness);
                                draw->AddLine(ImVec2(rect.x + rect.w*0.25f, rect.y + rect.h), ImVec2(rect.x, rect.y + rect.h), ESP_COLOR, boxThickness);
                                draw->AddLine(ImVec2(rect.x, rect.y + rect.h), ImVec2(rect.x, rect.y + rect.h*0.75f), ESP_COLOR, boxThickness);
                                draw->AddLine(ImVec2(rect.x, rect.y + rect.h*0.25f), ImVec2(rect.x, rect.y), ESP_COLOR, boxThickness);
                                break;
                            case BOX_TYPE_2:
                            case BOX_TYPE_3:
                            case BOX_TYPE_4:
                                draw->AddRect(ImVec2(rect.x, rect.y), ImVec2(rect.x + rect.w, rect.y + rect.h), ESP_COLOR, 0.0f, 0, boxThickness);
                                break;
                            default: break;
                        }
                    }

                    // ===== VIDA =====
                    if (EspVida) {
                        float maxHealth = get_MaxHP(closestEnemy);
                        float currentHealth = GetHp(closestEnemy);
                        float healthPercentage = maxHealth > 0 ? (currentHealth / maxHealth) : 0.0f;
                        float barHeight = rect.h * healthPercentage;
                        draw->AddRectFilled(ImVec2(rect.x - 10.0f, rect.y + (rect.h-barHeight)*0.5f),
                                            ImVec2(rect.x - 6.0f, rect.y + (rect.h+barHeight)*0.5f),
                                            IM_COL32(0,150,255,255));
                    }

                    // ===== TRACERS =====
                    if (EspLine)
                        draw->AddLine(ImVec2(screenWidth*0.5f, 10.0f),
                                      ImVec2(HeadPosition.x, headY),
                                      ESP_COLOR, 3.0f);

                    // ===== DISTÂNCIA =====
                    if (Distancia) {
                        std::string distanceText = "[" + std::to_string((int)distance) + "M]";
                        ImVec2 textSize = ImGui::CalcTextSize(distanceText.c_str(), NULL, 0.8f*ImGui::GetFontSize());
                        ImVec2 textPos(rect.x + rect.w*0.5f - textSize.x*0.5f, rect.y + rect.h + 20);
                        draw->AddRectFilled(ImVec2(textPos.x-5,textPos.y-2),
                                            ImVec2(textPos.x + textSize.x + 5, textPos.y + textSize.y + 2),
                                            IM_COL32(0,0,0,200), 4.0f);
                        draw->AddText(textPos, ImColor(colorDistance), distanceText.c_str());
                    }

                    // ===== ESP NOME =====
                    if (Nick) {
                        monoString* NickStr = get_NickName(closestEnemy);
                        std::string names;
                        if (NickStr) {
                            for (int i = 0; i < NickStr->getLength(); i++)
                                names += get_Chars(NickStr, i);
                        }
                        ImVec2 textSize = ImGui::CalcTextSize(names.c_str(), NULL, 0.8f*ImGui::GetFontSize());
                        ImVec2 textPos(rect.x + rect.w*0.5f - textSize.x*0.5f, rect.y - textSize.y - 5);
                        draw->AddRectFilled(ImVec2(textPos.x-5,textPos.y-2),
                                            ImVec2(textPos.x + textSize.x + 5, textPos.y + textSize.y + 2),
                                            IM_COL32(0,0,0,200), 4.0f);
                        draw->AddText(textPos, ImColor(colorNick), names.c_str());
                    }

                } // visible
            } // for players
        } // players & camera
    } // local & match
}
