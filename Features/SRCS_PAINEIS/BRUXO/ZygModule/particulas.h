#ifndef PARTICULAS_H
#define PARTICULAS_H
#include <vector>
#include <cmath>
#include <ctime>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_android.h>
#include <cstdlib>



ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) {
    return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) {
    return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y);
}

struct Particle {
    ImVec2 position;
    ImVec2 velocity;
};

const int NumParticles = 50;
std::vector<Particle> particles(NumParticles);
int ParticleSpeedLevel = 13;
const float MaxParticleSpeed = 2.0f;
const float ParticleRadius = 3.0f;
const float MaxDistance = 150.0f;
ImVec4 ParticleColor = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);

std::vector<ImVec4> colors = {
    ImVec4(0.0f, 0.0f, 1.0f, 1.0f), // Azul
    ImVec4(1.0f, 1.0f, 1.0f, 1.0f), // Branco
    ImVec4(1.0f, 0.0f, 0.0f, 1.0f), // Vermelho
    ImVec4(0.0f, 1.0f, 0.0f, 1.0f), // Verde
    ImVec4(1.0f, 0.0f, 1.0f, 1.0f), // Roxo
    ImVec4(1.0f, 0.5f, 0.0f, 1.0f), // Laranja
    ImVec4(1.0f, 1.0f, 0.0f, 1.0f)  // Amarelo
};

int currentColorIndex = 0;
float lastColorChangeTime = 0.0f;

void InitializeParticles() {
    for (auto& particle : particles) {
        particle.position = ImVec2(rand() % 800, rand() % 600);
        particle.velocity = ImVec2((rand() % 11 - 5) * MaxParticleSpeed, (rand() % 11 - 5) * MaxParticleSpeed);
    }
}

void MoveParticles(float width, float height) {
    if (ParticleSpeedLevel == 0) return;
    float actualSpeed = MaxParticleSpeed * (ParticleSpeedLevel > 10 ? (ParticleSpeedLevel - 10) / 10.0f : ParticleSpeedLevel / 10.0f);
    for (auto& particle : particles) {
        particle.position.x += particle.velocity.x * actualSpeed;
        particle.position.y += particle.velocity.y * actualSpeed;
        if (particle.position.x < 0) particle.position.x = width;
        else if (particle.position.x > width) particle.position.x = 0;
        if (particle.position.y < 0) particle.position.y = height;
        else if (particle.position.y > height) particle.position.y = 0;
    }
}

void UpdateParticleColor() {
    float currentTime = ImGui::GetTime();
    if (currentTime - lastColorChangeTime >= 3.0f) {
        lastColorChangeTime = currentTime;
        currentColorIndex = (currentColorIndex + 1) % colors.size();
    }
}

void DrawParticles(ImDrawList* draw_list, ImVec2 window_pos, ImVec2 window_size) {
    if (ParticleSpeedLevel == 0) return;

    ImVec4 actualColor = ParticleColor;
    if (ParticleSpeedLevel > 10) {
        UpdateParticleColor();
        actualColor = colors[currentColorIndex];
        actualColor.w = 0.0f;
        float timeSinceColorChange = ImGui::GetTime() - lastColorChangeTime;
        if (timeSinceColorChange < 0.5f) {
            actualColor.w = 2.0f * timeSinceColorChange;
        } else {
            actualColor.w = 1.0f;
        }
    }

    for (int i = 0; i < NumParticles; ++i) {
        for (int j = i + 1; j < NumParticles; ++j) {
            float distance = std::sqrt(std::pow(particles[j].position.x - particles[i].position.x, 2) +
                                       std::pow(particles[j].position.y - particles[i].position.y, 2));
            float opacity = 1.0f - (distance / MaxDistance);

            if (opacity > 0.0f) {
                ImU32 color = IM_COL32(actualColor.x * 255, actualColor.y * 255, actualColor.z * 255, static_cast<int>(opacity * 255));
                draw_list->AddLine(particles[i].position + window_pos, particles[j].position + window_pos, color);
            }
        }
        ImU32 color = IM_COL32(actualColor.x * 255, actualColor.y * 255, actualColor.z * 255, 255);
        draw_list->AddCircleFilled(particles[i].position + window_pos, ParticleRadius, color);
    }
}

void UpdateTextColor() {
    float currentTime = ImGui::GetTime();
    if (currentTime - lastColorChangeTime >= 3.0f) {
        lastColorChangeTime = currentTime;
        currentColorIndex = (currentColorIndex + 1) % colors.size();
    }
}






#endif
