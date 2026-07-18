bool clearMousePos = true;
bool initImGui = false;
bool isLoggedIn = false;
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
const float textScale = 1.5f;
const float borderWidth = 4.0f;

const float textScale1 = 1.5f;
const float borderWidth1 = 4.0f;
static float clickStartTime1 = -1.0f;
const float clickThreshold1 = 0.2f;
static ImVec2 gsButtonPos1 = ImVec2(0, 0);

const float textScale2 = 1.5f;
const float borderWidth2 = 4.0f;
static float clickStartTime2 = -1.0f;
const float clickThreshold2 = 0.5f;
static ImVec2 gsButtonPos2 = ImVec2(0, 0);

const float textScale3 = 1.5f;
const float borderWidth3 = 4.0f;
static float clickStartTime3 = -1.0f;
const float clickThreshold3 = 0.2f;
static ImVec2 gsButtonPos3 = ImVec2(0, 0);

ImVec4 allCores = ImVec4(1.0, 1.0, 0.0, 1.0);
ImVec4 allCores2 = ImVec4(0.0, 0.0, 1.0, 1.0);
ImVec4 DerrubadosColor = ImVec4(1.0, 1.0, 1.0, 1.0);

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
    const float padding = 6.0f;
    const float fixed_width = 250.0f;

    ImVec2 box_size = ImVec2(fixed_width, text_size.y + padding * 2);
    ImVec2 end_pos = ImVec2(pos.x + box_size.x, pos.y + box_size.y);

    ImRect bb(pos, end_pos);
    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, 0)) return;

    // 🟣 Fundo roxo claro (igual imagem)
    ImVec4 bg_color = ImVec4(0.6f, 0.4f, 1.0f, 1.0f);
    ImVec4 text_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    // 🔄 Arredondamento menor (4.0f)
    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(bg_color), 4.0f);

    // ✍️ Texto centralizado
    ImVec2 text_pos = ImVec2(
        bb.Min.x + (box_size.x - text_size.x) * 0.5f,
        bb.Min.y + (box_size.y - text_size.y) * 0.5f
    );

    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    ImGui::RenderText(text_pos, text);
    ImGui::PopStyleColor();
}


bool CustomSwitch(const char* label, bool* v) {
    ImGui::PushID(label);
    float height = ImGui::GetFrameHeight() * 0.5f;
    float width = height * 2.0f;
    float radius = height * 0.50f;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton(label, ImVec2(width, height));
    if (ImGui::IsItemClicked())
        *v = !*v;
    float t = *v ? 1.0f : 0.0f;
    ImGuiContext& g = *ImGui::GetCurrentContext();
    float ANIM_SPEED = 0.1f;
    if (g.LastActiveId == g.CurrentWindow->GetID(label)) {
        float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
        t = *v ? (t_anim) : (1.0f - t_anim);
    }
    ImU32 col_switch = ImGui::GetColorU32(ImVec4(allCores));
    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_switch, height * 0.5f);
    draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
    ImGui::PopID();
    return *v;
}

void addSwitch(const char* label, const char* /*description*/, bool* v) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float rect_width = 270;
    float rect_padding_x = 10;
    float rect_padding_y = 6;
    float label_height = ImGui::CalcTextSize(label).y;
    float total_height = label_height + rect_padding_y * 2;

    // Texto (label)
    ImGui::SetCursorScreenPos(ImVec2(pos.x + rect_padding_x, pos.y + rect_padding_y));
    ImGui::Text(label);

    // Switch (estilo personalizado)
    ImGui::SetCursorScreenPos(ImVec2(pos.x + rect_width - 60 - rect_padding_x, pos.y + rect_padding_y));
    ImGui::PushID(label);

    ImVec2 switchPos = ImGui::GetCursorScreenPos();
    float width = 60.0f;
    float height = 35.0f;
    float radius = height / 2.0f;

    ImGui::InvisibleButton("##switch", ImVec2(width, height));
    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemClicked();
    if (clicked)
        *v = !(*v);

    // Cores do switch
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImU32 color_off = IM_COL32(40, 40, 40, 255);
    ImU32 color_on = IM_COL32(168, 108, 214, 255); // roxinho
    ImU32 knob_color = IM_COL32(210, 210, 210, 255);

    // Fundo do switch
    draw_list->AddRectFilled(switchPos, ImVec2(switchPos.x + width, switchPos.y + height), *v ? color_on : color_off, radius);

    // Botão (círculo)
    float circle_x = *v ? (switchPos.x + width - radius) : (switchPos.x + radius);
    draw_list->AddCircleFilled(ImVec2(circle_x, switchPos.y + radius), radius - 2.0f, knob_color);

    ImGui::PopID();

    // Move cursor pra próxima posição
    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + total_height + 6));
}

bool customRadioButtonCrosshair(const char* label, int* v, int v_button, ImU32 allCores) {
ImGui::PushID(label);
ImGui::BeginGroup();
ImVec2 p = ImGui::GetCursorScreenPos();
ImDrawList* draw_list = ImGui::GetWindowDrawList();
float radius = 10.0f;
ImVec2 center = ImVec2(p.x + radius, p.y + radius);
ImU32 borderColor = allCores;
ImU32 lineColor = allCores;
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


bool CustomCheckbox(const char* label, bool* v)
{
ImGui::PushID(label);
ImGuiStyle& style = ImGui::GetStyle();
float size = ImGui::GetFrameHeight() * 0.65f;
float radius = size * 0.25f;
ImDrawList* draw_list = ImGui::GetWindowDrawList();
ImVec2 p = ImGui::GetCursorScreenPos();
ImGui::InvisibleButton(label, ImVec2(size, size));
if (ImGui::IsItemClicked())
*v = !*v;
ImGuiContext& g = *ImGui::GetCurrentContext();
float ANIM_SPEED = 0.1f;
float t = *v ? 1.0f : 0.0f;
if (g.LastActiveId == g.CurrentWindow->GetID(label))
{
float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
t = *v ? (t_anim) : (1.0f - t_anim);
}
ImU32 col_blue = ImGui::GetColorU32(ImVec4(0.0f, 0.5f, 1.0f, 1.0f));
ImU32 col_bg = ImGui::GetColorU32(*v ? ImVec4(0.0f, 0.5f, 1.0f, 1.0f) : ImVec4(0.06f, 0.06f, 0.06f, 1.0f));
ImU32 col_check = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); 
draw_list->AddRectFilled(p, ImVec2(p.x + size, p.y + size), col_bg, radius);
if (*v) {
draw_list->AddRect(p, ImVec2(p.x + size, p.y + size), col_blue, radius, ImDrawFlags_RoundCornersAll, 2.0f);
draw_list->AddRectFilled(p, ImVec2(p.x + size, p.y + size), col_check, radius);
}
ImGui::PopID();
return *v;
}
void addCheckbox(const char* label, bool* v)
{
ImGui::Text(label);
ImGui::SameLine(300);
CustomCheckbox(label, v);
}
void addLabeledCheckbox(const char* label, bool* v)
{
addCheckbox(label, v);
ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5); 
}

void DrawCustomSlider(int& value, int minValue, int maxValue, float sliderWidth, float sliderHeight, float circleRadius, const char* labelText) {
    ImGui::PushID(&value);

    ImVec2 startPos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Texto "AimFov Scale: xxx"
    char text[64];
    snprintf(text, sizeof(text), "%s:  %d", labelText, value);
    ImVec2 textSize = ImGui::CalcTextSize(text);
    drawList->AddText(startPos, IM_COL32(255, 255, 255, 255), text);

    // Posição da barra (abaixo do texto)
    float spacingY = 10.0f;
    ImVec2 barPos = ImVec2(startPos.x, startPos.y + textSize.y + spacingY);
    ImVec2 barSize = ImVec2(sliderWidth, sliderHeight);

    // Linha de fundo do slider
    drawList->AddRectFilled(barPos, ImVec2(barPos.x + barSize.x, barPos.y + barSize.y), IM_COL32(150, 150, 255, 100), sliderHeight / 2.0f);

    // Calcula posição do círculo com base no valor
    float t = (float)(value - minValue) / (float)(maxValue - minValue);
    float circleX = barPos.x + t * barSize.x;
    float circleY = barPos.y + barSize.y / 2.0f;

    // Círculo (ponteiro)
    drawList->AddCircleFilled(ImVec2(circleX, circleY), circleRadius, IM_COL32(200, 200, 255, 255));

    // Área invisível para interação
    ImGui::SetCursorScreenPos(barPos);
    ImGui::InvisibleButton("##customSlider", barSize);
    if (ImGui::IsItemActive()) {
        ImVec2 mousePos = ImGui::GetMousePos();
        float clampedX = ImClamp(mousePos.x - barPos.x, 0.0f, barSize.x);
        value = minValue + static_cast<int>((clampedX / barSize.x) * (maxValue - minValue));
    }

    ImGui::SetCursorScreenPos(ImVec2(barPos.x, barPos.y + barSize.y + 10)); // Ajuste final da posição
    ImGui::PopID();
}




void ConfigureScrollingParams(float& scrollPos, float contentHeight, float viewportHeight) {
if (isScrolling) {
float delta = ImGui::GetIO().MouseDelta.y;
scrollPos += delta;
scrollPos = ImClamp(scrollPos, 10.0f, contentHeight - viewportHeight);
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
    ImVec4 white_border = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 text_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    float border_thickness = 3.0f;
    float outer_border = 1.5f;
    float rounding = 8.0f;

    // Preenchimento
    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(bg_color), rounding);

    // Borda principal (colorida)
    window->DrawList->AddRect(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(border_color), rounding, ImDrawFlags_RoundCornersAll, border_thickness);

    // Borda branca externa
    window->DrawList->AddRect(ImVec2(bb.Min.x - 1, bb.Min.y - 1), ImVec2(bb.Max.x + 1, bb.Max.y + 1),
        ImGui::ColorConvertFloat4ToU32(white_border), rounding + 1.0f, ImDrawFlags_RoundCornersAll, outer_border);

    // Texto centralizado
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
    ImVec4 white_border = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 text_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    float border_thickness = 3.0f;
    float outer_border = 1.5f;
    float rounding = 8.0f;

    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(bg_color), rounding);
    window->DrawList->AddRect(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(border_color), rounding, ImDrawFlags_RoundCornersAll, border_thickness);
    window->DrawList->AddRect(ImVec2(bb.Min.x - 1, bb.Min.y - 1), ImVec2(bb.Max.x + 1, bb.Max.y + 1),
        ImGui::ColorConvertFloat4ToU32(white_border), rounding + 1.0f, ImDrawFlags_RoundCornersAll, outer_border);

    ImVec2 label_pos = ImVec2(bb.Min.x + (button_size.x - label_size.x) * 0.5f, bb.Min.y + (button_size.y - label_size.y) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    ImGui::RenderText(label_pos, label);
    ImGui::PopStyleColor();

    if (pressed && !isScrolling) isActive = !isActive;
}
