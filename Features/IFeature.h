#pragma once

#include <string>

// Interface base para todas as features do cheat.
// Cada feature deve implementar Start(), Stop() e RenderUI().
// RenderUI() é chamado a cada frame no menu para exibir os controles da feature.
// As classes devem garantir thread‑safety ao iniciar e parar seus workers.

class IFeature {
public:
    virtual ~IFeature() = default;
    // Inicializa a feature (cria thread, aplica hooks, etc.)
    virtual void Start() = 0;
    // Finaliza a feature (para thread, remove hooks, etc.)
    virtual void Stop() = 0;
    // Renderiza os controles ImGui da feature na aba "Extras"
    virtual void RenderUI() = 0;
    // Nome da feature (usado no UI)
    virtual const char* Name() const = 0;
};
