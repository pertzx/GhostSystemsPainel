#pragma once

#include <imgui_internal.h>
#include <imgui.h>

namespace ImGui {
    void CenteredText(ImColor color, const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        const char *text, *text_end;
        ImFormatStringToTempBufferV(&text, &text_end, fmt, args);
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(text, text_end).x) * 0.5);
        ImGui::TextColoredV(color, fmt, args);
        va_end(args);
    }
}
