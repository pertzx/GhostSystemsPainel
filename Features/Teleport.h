#pragma once
#include "IFeature.h"
#include <vector>
#include <string>
#include "../Entity.h"

struct NamedLocation {
    std::string name;
    GhostSystems::Vector3 pos;
};

class TeleportFeature : public IFeature {
public:
    TeleportFeature() {}
    ~TeleportFeature() override {}
    void Start() override {} // nothing to start
    void Stop() override {}
    void RenderUI() override {}
    const char* Name() const override { return "Teleport"; }

    // Carrega todas as localizações dos arquivos .txt
    std::vector<NamedLocation> LoadLocations();

    // Teleporta o player local para a posição fornecida
    void TeleportTo(const GhostSystems::Vector3& target);
};
