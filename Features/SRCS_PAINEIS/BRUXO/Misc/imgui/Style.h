void RedStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();

    // Definindo cores
    ImColor Black = ImColor(0, 0, 0, 255);

	
	//float rounding = 10.0f;
	
	
    // Configurando o estilo
    style.WindowRounding = 6.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.GrabMinSize = 20.0f;
    style.GrabRounding = 6.0f;
    style.FrameRounding = 3.0f;

    // Configurando as cores
style.Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f); // Cor do texto
style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Cor de fundo da janela
style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Cor da borda da janela
style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.6f); // Cor de fundo dos widgets
style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Cor de fundo do título da janela
style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Cor de fundo ativa do título da janela
style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.51f); // Cor de fundo colapsada do título da janela
style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f); // Cor da marca de verificação
style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f); // Cor da barra deslizante agarrar
style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f); // Cor da barra deslizante ativa agarrar
style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.0f, 0.0f, 0.6f); // Cor do botão
style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Cor do botão pairou
style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Cor do botão ativo
style.Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Cor do cabeçalho
style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f); // Cor do cabeçalho pairou
style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Cor do cabeçalho ativo
style.Colors[ImGuiCol_Separator] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Cor do separador
style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.78f); // Cor do separador pairou
style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Cor do separador ativo
style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.0f, 0.0f, 0.0f, 0.3f); // Cor do aperto de redimensionamento
style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.67f); // Cor do aperto de redimensionamento pairou
style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.95f); // Cor do aperto de redimensionamento ativo
style.Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Cor das linhas de plotagem
style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Cor das linhas de plotagem pairou
style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Cor do histograma de plotagem
style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Cor do histograma de plotagem pairou
style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.35f); // Cor de fundo do texto selecionado
style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.0f, 0.0f, 0.0f, 0.35f); // Cor da escurecimento da janela modal
style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.0f, 0.0f, 0.0f, 0.9f); // Cor do alvo de arrastar e soltar    ImGui::Begin("Editor de tema personalizado");

    
