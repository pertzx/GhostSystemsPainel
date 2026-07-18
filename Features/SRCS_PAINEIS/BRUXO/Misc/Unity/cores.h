#pragma once

void SetTextColor(const ImVec4& color) {
ImGui::GetStyle().Colors[ImGuiCol_Text] = color;
}
void SetCheckboxColor(const ImVec4& color) {
ImGui::GetStyle().Colors[ImGuiCol_CheckMark] = color;
}

void SetMenuColor(const ImVec4& color) {
ImGuiStyle& style = ImGui::GetStyle();
style.Colors[ImGuiCol_WindowBg] = color;
style.Colors[ImGuiCol_PopupBg] = color;
//style.Colors[ImGuiCol_Border] = color;
//style.Colors[ImGuiCol_BorderShadow] = color;
style.Colors[ImGuiCol_ScrollbarBg] = color;
style.Colors[ImGuiCol_ScrollbarGrab] = color;
style.Colors[ImGuiCol_ScrollbarGrabHovered] = color;
style.Colors[ImGuiCol_SliderGrab] = color;
style.Colors[ImGuiCol_SliderGrabActive] = color;
style.Colors[ImGuiCol_TableRowBg] = color;
}

int rounding = 10;
void SetImGuiStyle(float rounding) {
ImGuiStyle& style = ImGui::GetStyle();
style.WindowRounding = rounding;
style.ChildRounding = rounding;
style.FrameRounding = rounding;
}

void TesteRadio(const ImVec4& allCores) {
ImGuiStyle& style = ImGui::GetStyle();
style.Colors[ImGuiCol_CheckMark] = allCores;
}


ImColor GetChangingColor() { 
float time = ImGui::GetTime(); 
int colorIndex = static_cast<int>(time / 3) % 7; 
switch (colorIndex) { 
case 0: return ImColor(255, 0, 0); 
case 1: return ImColor(0, 255, 0); 
case 2: return ImColor(0, 0, 255); 
case 3: return ImColor(255, 255, 0); 
case 4: return ImColor(255, 255, 255); 
case 5: return ImColor(255, 0, 255);
case 6: return ImColor(0, 255, 255);
default: return ImColor(255, 255, 255);
}
}

float x,y;
float enemy1 = 2.0f;

ImVec4 colorTextBackground = ImVec4(0, 0, 0, 255);
ImVec4 colorBox = ImVec4(allCores);
ImVec4 colorNick = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
ImVec4 colorHealth = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
ImVec4 colorDistance = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
ImVec4 colorLine = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
ImVec4 colorHexagono = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
ImVec4 colorMenuText = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
ImVec4 colorCheckbox = ImVec4(0.0f, 0.8f, 0.8f, 1.0f);
ImVec4 menucolor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
ImVec4 laserColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
ImVec4 shadowColor = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);
ImVec4 titulo_color = ImVec4(255, 0, 0, 255);



