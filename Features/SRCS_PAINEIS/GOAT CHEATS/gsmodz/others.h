bool clearMousePos = true;
bool initImGui = false;
bool isLoggedIn = false;
//char password[128] = "";
std::string userInput = "";
std::string passInput = "";
static double fps = 0;

static bool configmenu = false;
static int tab;
static int subtab;
bool ativar_desativar = true;
bool nova_layout = true;
bool buceta = true;
bool DarkStyle = false;
bool DarkStyle2 = false;
bool Switch = false;
bool nova_layout2 = false;
bool buceta2 = false;
bool Botao = false;
bool closeButtonActive2 = false;
static float alpha = 0.0f;
bool isLoading = false;
std::chrono::time_point<std::chrono::steady_clock> loadingStartTime;
bool testerage = false;
bool Urubu = false;
ImVec2 top_left_pos = ImVec2(0.0f, 0.0f);
static float clickStartTime = -1.0f;
const float clickThreshold = 0.2f;
static ImVec2 gsButtonPos = ImVec2(0, 0);
static bool isDragging = false;
static bool isDragging1 = false;
static bool isDragging2 = false;
static bool isDragging3 = false;
const float gsButtonWidth = 300.0f; 
const float gsButtonHeight = 100.0f;
const float textScale = 2.5f;
const float borderWidth = 4.0f;

const float textScale1 = 2.5f;
const float borderWidth1 = 4.0f;
static float clickStartTime1 = -1.0f;
const float clickThreshold1 = 0.2f;
static ImVec2 gsButtonPos1 = ImVec2(0, 0);

const float textScale2 = 2.5f;
const float borderWidth2 = 4.0f;
static float clickStartTime2 = -1.0f;
const float clickThreshold2 = 0.5f;
static ImVec2 gsButtonPos2 = ImVec2(0, 0);

const float textScale3 = 2.5f;
const float borderWidth3 = 4.0f;
static float clickStartTime3 = -1.0f;
const float clickThreshold3 = 0.2f;
static ImVec2 gsButtonPos3 = ImVec2(0, 0);

// Alterado: Amarelo -> Vermelho escuro
ImVec4 allCores = ImVec4(0.5f, 0.0f, 0.0f, 1.0f);
ImVec4 allCores2 = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
ImVec4 DerrubadosColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
ImU32 colorU32 = ImGui::ColorConvertFloat4ToU32(allCores);
bool closeButtonActive = false;
bool isScrolling;
static int curtab;

bool opened = false;

void drawCategory(const char* text) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;
    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;
    const ImVec2 text_size = ImGui::CalcTextSize(text);
    ImVec2 pos = window->DC.CursorPos;
    const float padding = 4.0f;
    ImVec2 text_size_with_padding = ImVec2(text_size.x + padding * 2, text_size.y + padding * 2);
    ImVec2 end_pos = ImVec2(pos.x + text_size_with_padding.x, pos.y + text_size_with_padding.y);
    ImVec2 layout_end = ImVec2(window->Pos.x + window->Size.x, end_pos.y);
    ImRect bb(pos, layout_end);
    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, 0)) return;
    ImVec4 bg_color = allCores; // sua cor principal (vermelho escuro)
    ImVec4 text_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    float rounding = 5.0f;
    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(bg_color), rounding);
    ImVec2 text_pos = ImVec2(bb.Min.x + (bb.Max.x - bb.Min.x - text_size.x) * 0.5f, bb.Min.y + (bb.Max.y - bb.Min.y - text_size.y) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    ImGui::RenderText(text_pos, text);
    ImGui::PopStyleColor();
}

bool CustomSwitch(const char* label, bool* v) {
    ImGui::PushID(label);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float height = 22.0f;
    float width = 40.0f;
    float radius = height * 0.5f;

    ImGui::InvisibleButton(label, ImVec2(width, height));
    bool clicked = ImGui::IsItemClicked();
    if (clicked)
        *v = !*v;

    float t = *v ? 1.0f : 0.0f;

    // Cores do switch
    ImU32 bg_color = *v ? ImColor(0.6f, 0.0f, 0.0f, 1.0f) : ImColor(0.12f, 0.12f, 0.12f, 1.0f); // Vermelho escuro ou fundo escuro
    ImU32 knob_color = *v ? ImColor(0.9f, 0.0f, 0.0f, 1.0f) : ImColor(0.5f, 0.5f, 0.5f, 1.0f);  // Círculo vermelho ou cinza

    // Fundo arredondado
    draw_list->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), bg_color, radius);

    // Círculo do toggle
    float knob_x = pos.x + (t * (width - height));
    draw_list->AddCircleFilled(ImVec2(knob_x + radius, pos.y + radius), radius - 3.0f, knob_color);

    // Texto ao lado, com tamanho aumentado
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); // Ajuste vertical fino
    ImGui::SetWindowFontScale(1.2f);  // Aumenta só o texto desse switch
    ImGui::TextUnformatted(label);
    ImGui::SetWindowFontScale(1.0f);  // Volta ao normal

    ImGui::PopID();
    return clicked;
}
void addSwitch(const char* label, const char* desc, bool* v) {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 6));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

    if (ImGui::Checkbox(label, v)) {
        // Aqui você pode colocar alguma ação quando o switch for ativado/desativado
    }

    ImGui::PopStyleVar(2);

    if (desc && strlen(desc) > 0) {
        ImGui::SameLine();
        ImGui::TextDisabled(desc);
    }
}

bool customRadioButtonCrosshair(const char* label, int* v, int v_button, ImU32 allCores) {
    ImGui::PushID(label);
    ImGui::BeginGroup();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    float radius = 10.0f;
    ImVec2 center = ImVec2(p.x + radius, p.y + radius);
    ImU32 borderColor = allCores;   // vermelho escuro
    ImU32 lineColor = allCores;     // vermelho escuro

    draw_list->AddCircle(center, radius, borderColor, 32, 2.0f);
    draw_list->AddLine(ImVec2(center.x - radius / 2, center.y), ImVec2(center.x + radius / 2, center.y), lineColor, 2.0f);
    draw_list->AddLine(ImVec2(center.x, center.y - radius / 2), ImVec2(center.x, center.y + radius / 2), lineColor, 2.0f);
    draw_list->AddCircle(center, radius / 4, IM_COL32(0, 0, 0, 0), 32, 2.0f);

    bool clicked = ImGui::InvisibleButton(label, ImVec2(radius * 2, radius * 2));
    if (clicked) {
        *v = v_button;
    }
    if (*v == v_button) {
        draw_list->AddCircleFilled(center, radius / 4, allCores, 32);
    }
    ImGui::TextUnformatted(label);
    ImGui::EndGroup();
    ImGui::PopID();
    return clicked;
}

bool CustomCheckbox(const char* label, bool* v) {
    ImGui::PushID(label);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();

    float size = 12.0f; // ← Tamanho fixo pequeno
    float radius = 3.0f;

    ImGui::InvisibleButton(label, ImVec2(size, size));
    if (ImGui::IsItemClicked())
        *v = !*v;

    ImGuiContext& g = *ImGui::GetCurrentContext();
    float ANIM_SPEED = 0.1f;
    float t = *v ? 1.0f : 0.0f;
    if (g.LastActiveId == g.CurrentWindow->GetID(label)) {
        float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
        t = *v ? (t_anim) : (1.0f - t_anim);
    }

    ImU32 col_red_dark = ImGui::GetColorU32(ImVec4(0.5f, 0.0f, 0.0f, 1.0f)); // vermelho escuro
    ImU32 col_bg = ImGui::GetColorU32(*v ? ImVec4(0.5f, 0.0f, 0.0f, 1.0f) : ImVec4(0.06f, 0.06f, 0.06f, 1.0f));
    ImU32 col_check = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    draw_list->AddRectFilled(p, ImVec2(p.x + size, p.y + size), col_bg, radius);

    if (*v) {
        draw_list->AddRect(p, ImVec2(p.x + size, p.y + size), col_red_dark, radius, ImDrawFlags_RoundCornersAll, 2.0f);
        draw_list->AddRectFilled(p, ImVec2(p.x + size, p.y + size), col_check, radius);
    }

    ImGui::PopID();
    return *v;
}
void addCheckbox(const char* label, bool* v) {
    ImGui::Text(label);
    ImGui::SameLine(300);
    CustomCheckbox(label, v);
}

void addLabeledCheckbox(const char* label, bool* v) {
    addCheckbox(label, v);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5); 
}

void DrawCustomSlider(int& value, int minValue, int maxValue, float sliderWidth, float sliderHeight, float circleRadius) {
ImGui::PushID(&value);
ImVec2 startPos = ImGui::GetCursorScreenPos();
ImVec2 endPos(startPos.x + sliderWidth, startPos.y + sliderHeight);
ImDrawList* drawList = ImGui::GetWindowDrawList();
float normalizedValue = (static_cast<float>(value - minValue) / (maxValue - minValue));
float circleX = startPos.x + normalizedValue * sliderWidth;
drawList->AddRectFilled(startPos, endPos, ImColor(30, 30, 30));
drawList->AddRectFilled(startPos, ImVec2(circleX, endPos.y), ImColor(allCores));
ImVec2 circlePos(circleX, startPos.y + sliderHeight / 2.0f);
drawList->AddCircleFilled(circlePos, circleRadius, ImColor(255, 255, 255));
ImGui::SetCursorScreenPos(startPos);
ImGui::InvisibleButton(OBFUSCATE("##customSlider"), ImVec2(sliderWidth, sliderHeight));
if (ImGui::IsItemActive()) {
ImVec2 mousePos = ImGui::GetMousePos();
float mouseX = mousePos.x - startPos.x;
float clampedX = ImClamp(mouseX, 0.0f, sliderWidth);
value = minValue + static_cast<int>((clampedX / sliderWidth) * (maxValue - minValue));
}
ImGui::PopID();
}
void Spinner(float radius, int thickness, ImU32 color) {
ImDrawList* draw_list = ImGui::GetWindowDrawList();
ImVec2 window_pos = ImGui::GetWindowPos();
ImVec2 window_size = ImGui::GetWindowSize();
ImVec2 center = ImVec2(window_pos.x + window_size.x * 0.5f, window_pos.y + window_size.y * 0.5f + 20.0f);
float start_angle = ImGui::GetTime() * 3.0f;
radius *= 1.5f;
for (int i = 0; i < 8; i++) {
float angle = start_angle + (i * IM_PI * 2.0f / 8.0f);
draw_list->AddCircleFilled(ImVec2(center.x + ImCos(angle) * radius, center.y + ImSin(angle) * radius), thickness, color);
}
}





void ConfigureScrollingParams(float& scrollPos, float contentHeight, float viewportHeight) {
if (isScrolling) {
float delta = ImGui::GetIO().MouseDelta.y;
scrollPos += delta;
scrollPos = ImClamp(scrollPos, 0.0f, contentHeight - viewportHeight);
ImGui::SetScrollY(scrollPos);
}
}

void addButton(const char* label, bool& isActive) {
ImGuiWindow* window = ImGui::GetCurrentWindow();
if (window->SkipItems) return;
ImGuiContext& g = *ImGui::GetCurrentContext();
const ImGuiStyle& style = g.Style;
const ImGuiID id = window->GetID(label);
const ImVec2 label_size = ImGui::CalcTextSize(label);
ImVec2 pos = window->DC.CursorPos;
ImVec2 button_size = ImVec2(ImGui::GetContentRegionAvail().x, 55);
const ImRect bb(pos, ImVec2(pos.x + button_size.x, pos.y + button_size.y));
ImGui::ItemSize(bb, style.FramePadding.y);
if (!ImGui::ItemAdd(bb, id)) return;
bool hovered, held;
bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
if (hovered && ImGui::IsMouseDragging(0)) {
isScrolling = true;
} else if (ImGui::IsMouseReleased(0)) {
isScrolling = false;
}

ImVec4 bg_color = isActive ? allCores : ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
ImVec4 border_color = allCores;
ImVec4 text_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
float border_thickness = 3.0f;
float rounding = 8.0f;
window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(bg_color), rounding);
window->DrawList->AddRect(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(border_color), rounding, ImDrawFlags_RoundCornersAll, border_thickness);
ImVec2 label_pos = ImVec2(bb.Min.x + (button_size.x - label_size.x) * 0.5f, bb.Min.y + (button_size.y - label_size.y) * 0.5f);
ImGui::PushStyleColor(ImGuiCol_Text, text_color);
ImGui::RenderText(label_pos, label);
ImGui::PopStyleColor();
if (pressed && !isScrolling) isActive = !isActive;
}

void close(const char* label, bool& isActive) {
ImGuiWindow* window = ImGui::GetCurrentWindow();
if (window->SkipItems) return;
ImGuiContext& g = *ImGui::GetCurrentContext();
const ImGuiStyle& style = g.Style;
const ImGuiID id = window->GetID(label);
const ImVec2 label_size = ImGui::CalcTextSize(label);
ImVec2 pos = window->DC.CursorPos;
ImVec2 button_size = ImVec2(ImGui::GetContentRegionAvail().x, 70);
const ImRect bb(pos, ImVec2(pos.x + button_size.x, pos.y + button_size.y));
ImGui::ItemSize(bb, style.FramePadding.y);
if (!ImGui::ItemAdd(bb, id)) return;

bool hovered, held;
bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

if (hovered && ImGui::IsMouseDragging(0)) {
isScrolling = true;
} else if (ImGui::IsMouseReleased(0)) {
isScrolling = false;
}

ImVec4 bg_color = allCores;
ImVec4 border_color = allCores;
ImVec4 text_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
float border_thickness = 3.0f;
float rounding = 8.0f;
window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(bg_color), rounding);
window->DrawList->AddRect(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(border_color), rounding, ImDrawFlags_RoundCornersAll, border_thickness);
ImVec2 label_pos = ImVec2(bb.Min.x + (button_size.x - label_size.x) * 0.5f, bb.Min.y + (button_size.y - label_size.y) * 0.5f);
ImGui::PushStyleColor(ImGuiCol_Text, text_color);
ImGui::RenderText(label_pos, label);
ImGui::PopStyleColor();
if (pressed && !isScrolling) isActive = !isActive;
}






