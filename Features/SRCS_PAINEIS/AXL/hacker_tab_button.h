#pragma once
#include "imgui.h"


auto DrawToggleSwitch = [](const char* label, bool* v, ImTextureID icon = nullptr)
{
    ImGui::PushID(label);
    ImGui::BeginGroup();

    // Icon + Label with color based on toggle state
    if (icon)
        ImGui::Image(icon, ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.3f, 0.6f, 1.0f, 1.0f));
    ImGui::SameLine();

    ImVec4 label_color = *v ? ImVec4(1, 1, 1, 1) : ImVec4(0.5f, 0.5f, 0.5f, 1);
    ImGui::PushStyleColor(ImGuiCol_Text, label_color);
    ImGui::Text("%s", label);
    ImGui::PopStyleColor();

    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 70);

    // Custom toggle switch
    ImVec2 p = ImGui::GetCursorScreenPos();
    float height = 24.0f;
    float width = 50.0f;
    float radius = height * 0.5f;

    ImDrawList* draw = ImGui::GetWindowDrawList();

    // Background
    ImU32 bg_color = *v ? IM_COL32(0, 122, 255, 255) : IM_COL32(50, 50, 50, 255);
    draw->AddRectFilled(p, ImVec2(p.x + width, p.y + height), bg_color, radius);

    // Circle knob
    float circle_x = *v ? (p.x + width - radius) : (p.x + radius);
ImU32 circle_color = *v ? IM_COL32(200, 230, 255, 255) : IM_COL32(120, 120, 120, 255);
    draw->AddCircleFilled(ImVec2(circle_x, p.y + radius), radius - 3, circle_color);

    // Invisible button for toggling
    ImGui::InvisibleButton("toggle", ImVec2(width, height));
    if (ImGui::IsItemClicked())
        *v = !*v;

    ImGui::EndGroup();
    ImGui::PopID();
};

bool ToggleSwitch(const char* label, bool* v)
{
    ImGui::BeginGroup();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.8f;
    float corner = height * 0.3f;
    float knobWidth = height * 0.9f;

    ImGui::Text("%s", label);
    ImGui::SameLine(ImGui::GetWindowWidth() - width - ImGui::GetStyle().ItemSpacing.x * 2);

    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw = ImGui::GetWindowDrawList();

    ImGui::InvisibleButton(label, ImVec2(width, height));
    if (ImGui::IsItemClicked())
        *v = !*v;

    float t = *v ? 1.0f : 0.0f;
    ImU32 bg = *v ? IM_COL32(0, 180, 255, 255) : IM_COL32(90, 90, 90, 255);
    ImU32 knob = IM_COL32(240, 240, 240, 255);

    // Background
   draw->AddRectFilled(p, ImVec2(p.x + width, p.y + height), bg, corner);

    // Knob
    float knobX = p.x + t * (width - knobWidth - 2);
    draw->AddRectFilled(ImVec2(knobX + 1, p.y + 1), ImVec2(knobX + knobWidth, p.y + height - 1), knob, corner * 0.8f);

    // On/Off text
    ImVec2 textPos = ImVec2(p.x + width + 8, p.y + height * 0.15f);
    draw->AddText(textPos, IM_COL32(200, 200, 200, 220), *v ? "ON" : "OFF");

    ImGui::EndGroup();
    return *v;
}





bool TabButton(const char* label, bool active)
{
    if (active)
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 180, 255, 180));
    else
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(50, 50, 50, 200));

    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 180, 255, 220));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 180, 255, 255));

    bool pressed = ImGui::Button(label, ImVec2(130, 35));
    ImGui::PopStyleColor(3);
    return pressed;
}





bool PremiumButton(const char* label, ImVec2 size)
{
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(30, 144, 255, 200));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(30, 144, 255, 240));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 120, 220, 255));

    bool clicked = ImGui::Button(label, size);
    ImGui::PopStyleColor(3);
    return clicked;
}









bool ToggleSwitchLeftLabel(const char* label, bool* v, const char* icon = nullptr)
{
    ImGui::BeginGroup();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.8f;
    float corner = height * 0.25f;
    float knobWidth = height * 0.9f;

    // Label on left
    ImGui::Text("%s %s", icon ? icon : "", label);
    ImGui::SameLine(ImGui::GetWindowWidth() - width - ImGui::GetStyle().ItemSpacing.x * 2);

    // Toggle logic
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImGui::InvisibleButton(label, ImVec2(width, height));

    if (ImGui::IsItemClicked())
        *v = !*v;

    float t = *v ? 1.0f : 0.0f;
    ImU32 bg = *v ? IM_COL32(30, 144, 255, 255) : IM_COL32(70, 70, 70, 255); // Blue / Gray
    ImU32 knob = IM_COL32(240, 240, 240, 255); // Light knob

    // Background
    draw->AddRectFilled(p, ImVec2(p.x + width, p.y + height), bg, corner);

    // Knob
    float knobX = p.x + t * (width - knobWidth - 2);
 draw->AddRectFilled(ImVec2(knobX + 1, p.y + 1), ImVec2(knobX + knobWidth, p.y + height - 1), knob, corner * 0.8f);

    ImGui::EndGroup();
    return *v;
}


void DrawSectionHeader(const char* title) {
    ImVec2 curPos = ImGui::GetCursorScreenPos();
    float headerHeight = 40.0f;
    float headerWidth = ImGui::GetContentRegionAvail().x;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // === Background Bar (smooth black, rounded) ===
    drawList->AddRectFilled(
        curPos,
        ImVec2(curPos.x + headerWidth, curPos.y + headerHeight),
        IM_COL32(20, 20, 20, 255),  // Slightly softer black
        12.0f
    );

    // === Subtle Outline (for contrast) ===
    drawList->AddRect(
        curPos,
        ImVec2(curPos.x + headerWidth, curPos.y + headerHeight),
        IM_COL32(100, 100, 100, 60),  // faint border
        12.0f,
        0,
        1.2f
    );

    // === Shadow under text (optional for glow look) ===
    ImVec2 textSize = ImGui::CalcTextSize(title);
    float textX = curPos.x + (headerWidth - textSize.x) * 0.5f;
    float textY = curPos.y + (headerHeight - textSize.y) * 0.5f;

    drawList->AddText(ImVec2(textX + 1, textY + 1), IM_COL32(0, 0, 0, 120), title);  // shadow
    drawList->AddText(ImVec2(textX, textY), IM_COL32(255, 255, 255, 255), title);    // main text

    // === Space Below Header ===
    ImGui::SetCursorScreenPos(ImVec2(curPos.x, curPos.y + headerHeight + 8.0f));  // consistent spacing
}
auto DrawPremiumToggle = [](const char* label, bool* v)
{
    ImGui::PushID(label);
    ImGui::BeginGroup();

    ImVec4 textColor = *v
        ? ImVec4(0, 0, 0, 1)  
        : ImVec4(1, 1, 1, 1); 

    ImGui::PushStyleColor(ImGuiCol_Text, textColor);
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(15.0f, 0));  
    ImGui::SameLine();

    float switchWidth  = 70.0f;   
    float switchHeight = 32.0f;
    float radius = switchHeight * 0.5f;

    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw = ImGui::GetWindowDrawList();

  
    ImU32 bgColor = *v
        ? IM_COL32(255, 255, 255, 255) 
        : IM_COL32(0, 0, 0, 255);   

    draw->AddRectFilled(
        p,
        ImVec2(p.x + switchWidth, p.y + switchHeight),
        bgColor,
        radius
    );
    float knobX = *v
        ? (p.x + switchWidth - radius)
        : (p.x + radius);

    draw->AddCircleFilled(
        ImVec2(knobX, p.y + radius),
        radius - 4,
        IM_COL32(170, 170, 170, 255)
    );
    ImGui::InvisibleButton("##toggle", ImVec2(switchWidth, switchHeight));
    if (ImGui::IsItemClicked())
        *v = !*v;

    ImGui::EndGroup();
    ImGui::PopID();
};
