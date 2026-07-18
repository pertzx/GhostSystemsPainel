#include "particles.h"
#include <algorithm>

ParticleSystem::ParticleSystem(int maxParticles, float spawnRate) 
    : rng(std::random_device{}()), randomFloat(0.0f, 1.0f), spawnRate(spawnRate), timeSinceLastSpawn(0.0f) {
    particles.reserve(maxParticles);
}

void ParticleSystem::Update(float deltaTime, ImVec2 windowSize) {
    this->windowSize = windowSize;
    
    // Atualizar partÃƒÂ­culas existentes
    for (auto it = particles.begin(); it != particles.end();) {
        UpdateParticle(*it, deltaTime);
        
        // Remover partÃƒÂ­culas mortas
        if (it->life <= 0.0f) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }
    
    // Spawnar novas partÃƒÂ­culas
    timeSinceLastSpawn += deltaTime;
    if (timeSinceLastSpawn >= spawnRate && particles.size() < particles.capacity() && windowSize.x > 0 && windowSize.y > 0) {
        SpawnParticle();
        timeSinceLastSpawn = 0.0f;
    }
}

void ParticleSystem::Render(ImDrawList* drawList) {
    // Desenhar linhas entre partÃ­culas prÃ³ximas
    float connectionDistance = 100.0f; // DistÃ¢ncia mÃ¡xima para conectar partÃ­culas
    for (size_t i = 0; i < particles.size(); ++i) {
        for (size_t j = i + 1; j < particles.size(); ++j) {
            const Particle& p1 = particles[i];
            const Particle& p2 = particles[j];
            
            float distSq = (p1.position.x - p2.position.x) * (p1.position.x - p2.position.x) +
                           (p1.position.y - p2.position.y) * (p1.position.y - p2.position.y);
            
            if (distSq < connectionDistance * connectionDistance) {
                // Calcular transparÃªncia da linha baseada na distÃ¢ncia e aumentar a visibilidade
                float alpha = 1.0f - (sqrt(distSq) / connectionDistance);
                ImU32 lineColor = IM_COL32(255, 255, 255, (int)(alpha * 255 * 1.0f)); // Linha branca, alpha aumentado para 1.0f
                drawList->AddLine(p1.position, p2.position, lineColor, 1.0f);
            }
        }
    }

    for (const auto& particle : particles) {
        // Calcular cor com transparÃªncia baseada na vida
        float alpha = particle.life * particle.alpha;
        ImU32 color = GetParticleColor(particle.life);
        
        // Desenhar partÃƒÂ­cula como cÃƒÂ­rculo
        drawList->AddCircleFilled(particle.position, particle.size, color);
        
        // Opcional: adicionar um brilho sutil
        if (particle.life > 0.5f) {
            float glowAlpha = (particle.life - 0.5f) * 2.0f * 0.3f; // MÃƒÂ¡ximo 30% de transparÃƒÂªncia para o brilho
            ImU32 glowColor = IM_COL32(255, 255, 255, (int)(glowAlpha * 255));
            drawList->AddCircleFilled(particle.position, particle.size * 1.5f, glowColor);
        }
    }
}

void ParticleSystem::SpawnParticle(ImVec2 position) {
    Particle particle;
    
    // PosiÃƒÂ§ÃƒÂ£o inicial (aleatÃƒÂ³ria se nÃƒÂ£o especificada)
    if (position.x < 0 || position.y < 0) {
        particle.position.x = Random(0, windowSize.x);
        particle.position.y = Random(0, windowSize.y);
    } else {
        particle.position = position;
    }
    
    // Velocidade aleatÃƒÂ³ria
    float angle = Random(0, 2 * M_PI);
    float speed = Random(10.0f, 50.0f);
    particle.velocity.x = cos(angle) * speed;
    particle.velocity.y = sin(angle) * speed;
    
    // Propriedades da partÃƒÂ­cula
    particle.life = Random(3.0f, 8.0f);
    particle.maxLife = particle.life;
    particle.size = Random(1.0f, 4.0f);
    particle.alpha = Random(0.3f, 0.8f);
    
    particles.push_back(particle);
}

void ParticleSystem::UpdateParticle(Particle& particle, float deltaTime) {
    // Atualizar posiÃƒÂ§ÃƒÂ£o
    particle.position.x += particle.velocity.x * deltaTime;
    particle.position.y += particle.velocity.y * deltaTime;
    
    // Atualizar vida
    particle.life -= deltaTime;
    
    // Aplicar fÃƒÂ­sica simples (gravidade leve e resistÃƒÂªncia do ar)
    particle.velocity.y += 20.0f * deltaTime; // Gravidade leve
    particle.velocity.x *= 0.99f; // ResistÃƒÂªncia do ar
    particle.velocity.y *= 0.99f;
    
    // Fazer partÃƒÂ­culas "rebaterem" nas bordas da janela
    if (particle.position.x < 0 || particle.position.x > windowSize.x) {
        particle.velocity.x *= -0.8f; // Perda de energia no rebote
        particle.position.x = std::max(0.0f, std::min(windowSize.x, particle.position.x));
    }
    
    if (particle.position.y < 0 || particle.position.y > windowSize.y) {
        particle.velocity.y *= -0.8f; // Perda de energia no rebote
        particle.position.y = std::max(0.0f, std::min(windowSize.y, particle.position.y));
    }
}

ImU32 ParticleSystem::GetParticleColor(float life) {
    // Para deixar as partÃ­culas vermelhas e mais visÃ­veis
    float normalizedLife = life / 8.0f; // Assumindo vida mÃ¡xima de 8 segundos
    // Aumentamos o alpha para que as partÃ­culas fiquem mais visÃ­veis
    return IM_COL32(255, 0, 0, (int)(normalizedLife * 255 * 1.0f)); // Cor vermelha, alpha aumentado para 1.0f
}

float ParticleSystem::Random(float min, float max) {
    return min + randomFloat(rng) * (max - min);
}



