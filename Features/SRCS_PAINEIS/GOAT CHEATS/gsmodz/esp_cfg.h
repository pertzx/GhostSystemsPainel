#pragma once
#include <imgui.h>

// ================== Config & helpers globais das ESP ==================
// Use só este header em todos os .cpp/.h que precisarem dos helpers.
// (NÃO declare versões alternativas desses helpers em outros arquivos.)

// Estado default (pode mudar em runtime se você expor sliders/botões)
inline ImU32 gEspColorU32     = IM_COL32(57, 255, 20, 255); // verde neon
inline int   gEspPosMode      = 1;    // 0=Topo, 1=Meio, 2=Baixo
inline float gEspExtraOffset  = 0.0f; // px (+baixo / -cima)

// Cor atual das ESP
inline ImU32 GetEspColor() {
    return gEspColorU32;
}

// Deslocamento vertical global para todos os elementos de ESP
inline float GetEspYOffset(float screenHeight) {
    float anchor = (gEspPosMode == 0 ? screenHeight * 0.25f
                  : gEspPosMode == 2 ? screenHeight * 0.75f
                                     : screenHeight * 0.50f);
    return (anchor - screenHeight * 0.5f) + gEspExtraOffset;
}