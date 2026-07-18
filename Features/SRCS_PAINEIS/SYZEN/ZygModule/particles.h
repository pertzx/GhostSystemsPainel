#pragma once
#include <vector>
#include <random>
#include <cmath> // Adicionado para M_PI
#include <imgui.h>

// Estrutura para uma partÃ­cula individual
struct Particle {
    ImVec2 position;    // PosiÃ§Ã£o atual da partÃ­cula
    ImVec2 velocity;    // Velocidade da partÃ­cula
    float life;         // Tempo de vida restante (0.0 a 1.0)
    float maxLife;      // Tempo de vida mÃ¡ximo
    float size;         // Tamanho da partÃ­cula
    ImU32 color;        // Cor da partÃ­cula
    float alpha;        // TransparÃªncia
    
    Particle() : position(0, 0), velocity(0, 0), life(1.0f), maxLife(1.0f), size(2.0f), color(IM_COL32(255, 255, 255, 255)), alpha(1.0f) {}
};

// Classe para gerenciar o sistema de partÃ­culas
class ParticleSystem {
private:
    std::vector<Particle> particles;
    std::mt19937 rng;
    std::uniform_real_distribution<float> randomFloat;
    ImVec2 windowSize;
    float spawnRate;
    float timeSinceLastSpawn;
    
public:
    ParticleSystem(int maxParticles = 100, float spawnRate = 0.1f);
    
    void Update(float deltaTime, ImVec2 windowSize);
    void Render(ImDrawList* drawList);
    void SpawnParticle(ImVec2 position = ImVec2(-1, -1));
    void SetWindowSize(ImVec2 size) { windowSize = size; }
    
private:
    void UpdateParticle(Particle& particle, float deltaTime);
    ImU32 GetParticleColor(float life);
    float Random(float min = 0.0f, float max = 1.0f);
};



