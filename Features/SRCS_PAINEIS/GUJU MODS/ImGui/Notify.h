#pragma once
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include "imgui.h"
#include "imgui_internal.h"
#include <algorithm>

// Font Awesome icons
#define ICON_FA_CHECK_CIRCLE           u8"\uf058"
#define ICON_FA_EXCLAMATION_TRIANGLE  u8"\uf071"
#define ICON_FA_TIMES_CIRCLE          u8"\uf057"
#define ICON_FA_INFO_CIRCLE           u8"\uf05a"
#define ICON_FA_SERVER                u8"\uf233"

#define OWM_API
#define OWM_INLINE inline
#define WHITE(a) ImColor(255, 255, 255, a)
#define RED(a) ImColor(255, 0, 0, a)
#define YELLOW(a) ImColor(255, 255, 0, a)
#define LAWNGREEN(a) ImColor(124, 252, 0, a)
#define CEK_STR(str) (!str.empty())

#define NF_MAX_MSG_LENGTH 4096        
#define NF_PADDING_X 20.f            
#define NF_PADDING_Y 20.f            
#define NF_PADDING_MESSAGE_Y 10.f    
#define NF_FADE_IN_OUT_TIME 150      
#define NF_DEFAULT_DISMISS 4000      
#define NF_OPACITY 0.7f              
#define NF_FLAGS ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing
#define NF_USE_SEPARATOR

using namespace std::chrono;

namespace OWM {

    OWM_INLINE void wait() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    enum struct NotifyType : int {
        NotifyType_None,
        NotifyType_Success,
        NotifyType_Warning,
        NotifyType_Error,
        NotifyType_Info,
        NotifyType_Server,
        NotifyType_COUNT
    };

    enum struct NotifyPhase : int {
        NotifyPhase_FadeIn,
        NotifyPhase_Wait,
        NotifyPhase_FadeOut,
        NotifyPhase_Expired,
        NotifyPhase_COUNT
    };

    OWM_INLINE int GetTickCount() {
        return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    }

    struct OWM_API Notification {
        NotifyType type;
        std::string title;
        std::string content;
        int dismiss_time;
        int creation_time;
        bool bDone;

        Notification() : type(NotifyType::NotifyType_None), dismiss_time(0), creation_time(0), bDone(false) {}
        Notification(NotifyType _type, int _dismiss_time = NF_DEFAULT_DISMISS)
            : type(_type), dismiss_time(_dismiss_time), creation_time(OWM::GetTickCount()), bDone(false) {}

        void set_title(const char* format) { title = format; }
        void set_content(const char* format) { content = format; }
        void set_type(const NotifyType& _type) { type = _type; }

        auto get_default_title() -> const char* {
            if (title.empty()) {
                switch (type) {
                    case NotifyType::NotifyType_None:    return "Unknown Type";
                    case NotifyType::NotifyType_Success: return "Success";
                    case NotifyType::NotifyType_Warning: return "Warning";
                    case NotifyType::NotifyType_Error:   return "Error";
                    case NotifyType::NotifyType_Info:    return "Confirmation";
                    case NotifyType::NotifyType_Server:  return "Server";
                }
            }
            return title.c_str();
        }

        auto get_color() -> ImColor {
            switch (type) {
                case NotifyType::NotifyType_None:    return WHITE(255);
                case NotifyType::NotifyType_Success: return LAWNGREEN(255);
                case NotifyType::NotifyType_Warning: return YELLOW(255);
                case NotifyType::NotifyType_Error:   return RED(255);
                case NotifyType::NotifyType_Info:    return ImColor(255, 132, 0, 255);
                case NotifyType::NotifyType_Server:  return ImColor(60, 193, 255, 255);
            }
        }

        auto get_icon() -> const char* {
            switch (this->type) {
                case NotifyType::NotifyType_None:    return nullptr;
                case NotifyType::NotifyType_Success: return ICON_FA_CHECK_CIRCLE;
                case NotifyType::NotifyType_Warning: return ICON_FA_EXCLAMATION_TRIANGLE;
                case NotifyType::NotifyType_Error:   return ICON_FA_TIMES_CIRCLE;
                case NotifyType::NotifyType_Info:    return ICON_FA_INFO_CIRCLE;
                case NotifyType::NotifyType_Server:  return ICON_FA_SERVER;
            }
        }

        int get_elapsed_time() {
            return OWM::GetTickCount() - creation_time;
        }

        auto get_phase() -> NotifyPhase {
            int elapsed = get_elapsed_time();
            if (elapsed > NF_FADE_IN_OUT_TIME + dismiss_time + NF_FADE_IN_OUT_TIME) return NotifyPhase::NotifyPhase_Expired;
            if (elapsed > NF_FADE_IN_OUT_TIME + dismiss_time) return NotifyPhase::NotifyPhase_FadeOut;
            if (elapsed > NF_FADE_IN_OUT_TIME) return NotifyPhase::NotifyPhase_Wait;
            return NotifyPhase::NotifyPhase_FadeIn;
        }

        auto get_fade_percent() -> float {
            auto phase = get_phase();
            int elapsed = get_elapsed_time();
            if (phase == NotifyPhase::NotifyPhase_FadeIn)
                return ((float)elapsed / NF_FADE_IN_OUT_TIME) * NF_OPACITY;
            if (phase == NotifyPhase::NotifyPhase_FadeOut)
                return (1.f - (((float)elapsed - NF_FADE_IN_OUT_TIME - dismiss_time) / NF_FADE_IN_OUT_TIME)) * NF_OPACITY;
            return 1.f * NF_OPACITY;
        }

void RenderNotifications() {
    if (get_phase() == NotifyPhase::NotifyPhase_Expired) return;

    auto vp_size = ImGui::GetMainViewport()->Size;
    auto text_color = get_color();
    text_color.Value.w = get_fade_percent();
    std::string wn = "##TOAST" + title;

    // Căn lề hiển thị
    ImVec2 toast_pos = ImVec2(vp_size.x - NF_PADDING_X, NF_PADDING_Y);
    ImGui::SetNextWindowBgAlpha(text_color.Value.w * 0.85f);
    ImGui::SetNextWindowPos(toast_pos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0.2f));
    ImGui::Begin(wn.c_str(), NULL, NF_FLAGS);
    {
        ImGui::PushTextWrapPos(vp_size.x / 2.5f);

        const char* icon = get_icon();
        ImGui::BeginGroup();

        if (icon) {
            ImGui::TextColored(text_color, "%s  %s", icon, get_default_title());
        } else {
            ImGui::TextColored(text_color, "%s", get_default_title());
        }

        if (!content.empty()) {
            ImGui::Spacing();

            float progress = (float)get_elapsed_time() / (dismiss_time + NF_FADE_IN_OUT_TIME * 2);
            progress = std::clamp(progress, 0.0f, 1.0f);

            // Cầu vồng
            ImVec4 rainbow[] = {
                ImVec4(1, 0, 0, 1), ImVec4(1, 0.5f, 0, 1),
                ImVec4(1, 1, 0, 1), ImVec4(0, 1, 0, 1),
                ImVec4(0, 0, 1, 1), ImVec4(0.29f, 0, 0.51f, 1),
                ImVec4(0.56f, 0, 1, 1)
            };
            int color_index = (int)((1.0f - progress) * 7) % 7;

            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, rainbow[color_index]);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 0.3f));
            ImGui::ProgressBar(progress, ImVec2(-1, 4), "");
            ImGui::PopStyleColor(2);

            ImGui::Spacing();
            ImGui::TextColored(WHITE((int)(text_color.Value.w * 255)), "%s", content.c_str());
        }

        ImGui::EndGroup();
        ImGui::PopTextWrapPos();
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}
    };
}
