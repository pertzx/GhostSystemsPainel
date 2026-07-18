
    // ===== BYPASS MANAGER =====
    void GhostSystems::Menu::drawBypassManager() {
        if (!bypassManagerInitialized) {
            initBypassManager();
        }
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Sistema de Protecao Anti-Ban");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "40 tecnicas de bypass implementadas");
        ImGui::Separator();

        bool allProtected = bypassManager.IsFullyProtected();
        size_t applied = bypassManager.GetAppliedCount();
        size_t total = bypassManager.GetTotalCount();

        if (allProtected) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "PROTECAO TOTAL ATIVA");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "PROTECAO PARCIAL (%zu/%zu)", applied, total);
        }

        ImGui::Separator();

        if (ImGui::Button("Aplicar Todas as Tecnicas", ImVec2(200, 35))) {
            bypassManager.ApplyAll();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reverter Todas", ImVec2(150, 35))) {
            bypassManager.RevertAll();
        }

        ImGui::Separator();
        ImGui::Text("Tecnicas de Protecao:");

        auto& entries = bypassManager.GetEntries();

        if (ImGui::BeginTable("BypassTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Tecnica", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 100);
            ImGui::TableSetupColumn("Acao", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableHeadersRow();

            for (auto& entry : entries) {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("[%s] %s", entry.id.c_str(), entry.name.c_str());
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("%s", entry.description.c_str());
                }

                ImGui::TableSetColumnIndex(1);
                switch (entry.status) {
                    case BypassStatus::APPLIED:
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Ativo");
                        break;
                    case BypassStatus::APPLYING:
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Aplicando...");
                        break;
                    case BypassStatus::FAILED:
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Falhou");
                        break;
                    default:
                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Inativo");
                }

                ImGui::TableSetColumnIndex(2);
                if (!entry.isApplied) {
                    if (ImGui::Button((std::string("Aplicar##") + entry.id).c_str())) {
                        entry.applyFunc();
                    }
                } else {
                    if (ImGui::Button((std::string("Reverter##") + entry.id).c_str())) {
                        entry.revertFunc();
                    }
                }
            }
            ImGui::EndTable();
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
            "Nota: Estas tecnicas sao PASSIVAS e nao injetam codigo no anticheat.");
    }

    // ===== BYPASS SDK (ffantihack hooks) =====
    void GhostSystems::Menu::drawBypassSDK() {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "=== Bypass Login SDK (ffantihack) ===");
        ImGui::Separator();

        if (bypassSDKActive) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: ATIVO");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Status: INATIVO");
        }

        if (ImGui::Button(bypassSDKActive ? "Desativar Bypass SDK" : "Ativar Bypass SDK")) {
            bypassSDKActive = !bypassSDKActive;
            BypassLoginSDK::Instance().UpdateConfig({bypassSDKActive, true, true, true, true});
        }

        ImGui::SameLine();
        if (ImGui::Button("Reinicializar")) {
            bypassSDKInitialized = false;
            initBypassSDK();
        }

        ImGui::Separator();
        ImGui::Text("Hooks Ativos:");

        const auto& hooks = BypassLoginSDK::Instance().GetHookList();
        if (ImGui::BeginTable("BypassSDKHooks", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Hook", ImGuiTableColumnFlags_WidthFixed, 180.0f);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Descricao", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for (const auto& hook : hooks) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", hook.name.c_str());
                ImGui::TableSetColumnIndex(1);
                if (hook.isApplied) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "ATIVO");
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "OFF");
                }
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", hook.description.c_str());
            }
            ImGui::EndTable();
        }

        if (hooks.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Nenhum hook aplicado ainda.");
            if (ImGui::Button("Tentar Aplicar Hooks")) {
                BypassLoginSDK::Instance().ApplyHooks();
            }
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Logs: adb logcat -s GhostSDK_Bypass:D");
    }

    // ===== DUMPER =====
    void GhostSystems::Menu::drawDumper() {
        static bool dumpInProgress = false;
        static std::string dumpStatus = "Clique para iniciar o dump";
        static int dumpCount = 0;

        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "IL2CPP Dumper");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Gera offsets.h automatico");
        ImGui::Separator();

        if (dumpInProgress) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "DUMANDO...");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", dumpStatus.c_str());
        }

        ImGui::Separator();

        if (ImGui::Button("Dump IL2Cpp Classes", ImVec2(200, 35))) {
            dumpInProgress = true;
            dumpStatus = "Iniciando dump...";
            IL2CppDumper::DumpAll();
            dumpStatus = "Dump concluido! Verifique /sdcard/Android/data/com.dts.freefireth/files/";
            dumpInProgress = false;
            dumpCount++;
            LOGI("[Dumper] IL2Cpp dump concluido");
        }
    }

} // namespace GhostSystems
