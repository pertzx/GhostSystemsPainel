#include "Menu.h"
#include "Entity.h"
#include "Il2CppHelper.h"
#include <imgui.h>

// ===== CHAMS OVERLAY REMOVIDO =====
// Chams agora é feito via hook de OpenGL (glDrawElements/glDrawArrays)
// Este arquivo existe apenas para compatibilidade com a declaracao no Menu.h

void GhostSystems::Menu::renderChamsOverlay(ImDrawList* drawList, float screenW, float screenH) {
    // Funcao vazia - chams agora e renderizado via hook OpenGL
    // Nao desenhamos boxes 2D, usamos chams 3D reais
    if (!drawList || !chamsEnabled) return;
    if (!masterSwitch) return;
    
    // Chams reais sao renderizados pelo hook glDrawElements
    // Nada para fazer aqui
}