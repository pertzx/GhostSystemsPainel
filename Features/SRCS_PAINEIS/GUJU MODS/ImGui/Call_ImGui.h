#pragma once

#include "Font.h"
#include "Themes.h"
#include "imgui.h"
#include "backends/imgui_impl_android.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "ImguiPP.h"
#include "stb_image.h"
#include "Icon_Menu.h"
#include "Notify.h"
using namespace std;
#include "ImGuiExtra.h"
#include "Icon.h"
#include "Iconcpp.h"
#include "font6.h"
#include "fa_solid_900.h"
#include "iconMaterialIcons_Regular.h"
#include "MaterialIcons_Regular.h"
#include "NotoEmoji.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <string>
#include <map>
#include <chrono>
#include <cmath>
#include <curl/curl.h> 
#include <jni.h> 
#include <vector>

extern JavaVM *jvm1;
extern jobject g_Activity;

static auto startTime = std::chrono::steady_clock::now();
std::chrono::system_clock::time_point lastUpdateTime;
int currentSecond = -1;

static int activeTab = 0; 
static bool godMode = false;
static bool oneHit = false;
static bool wallHack = false;
static float speedHack = 10.0f;
static bool espLine = false;
static bool espBox = false;

bool UseCustomKeys = false; // if your don't have website

struct LocalKeyData {
    string key;
    string expireDate;
};

std::vector<LocalKeyData> CustomKeysList = {
    {"Nigga", "2099-12-31 23:59"},
    {"Diddy444444",    "2025-12-31 12:00"},
    {"TEST-KEY-7DAY",   "2024-05-20 09:30"}
};

ImVec4 accentcolor = ImVec4(0.663f, 0.196f, 0.788f, 1.00f);

using namespace ImGui;

namespace DripCol {
    const ImVec4 BgDark      = ImVec4(0.05f, 0.05f, 0.05f, 1.00f); 
    const ImVec4 BgSection   = ImVec4(0.05f, 0.05f, 0.05f, 1.00f); 
    const ImVec4 BgItem      = ImVec4(0.14f, 0.14f, 0.14f, 1.00f); 
    const ImVec4 Border      = ImVec4(0.22f, 0.22f, 0.22f, 1.00f); 
    const ImVec4 Accent      = ImVec4(0.663f, 0.196f, 0.788f, 1.00f); 
    const ImVec4 TextGray    = ImVec4(0.60f, 0.60f, 0.60f, 1.00f); 
    const ImVec4 TextWhite   = ImVec4(1.000f, 1.000f, 1.000f, 1.00f);
    const ImVec4 Button      = ImVec4(0.663f, 0.196f, 0.788f, 1.00f); 
    const ImVec4 ButtonHover = ImVec4(0.600f, 0.150f, 0.720f, 1.00f);
    const ImVec4 Separator   = ImVec4(1.000f, 1.000f, 1.000f, 0.08f);
    const ImVec4 BorderDark  = ImVec4(0.200f, 0.200f, 0.200f, 1.00f); 
}

namespace JNIHelper {

    struct ScopedJNIEnv {
        JNIEnv* env;
        bool attached;
        ScopedJNIEnv(JavaVM* vm) : env(nullptr), attached(false) {
            if (!vm) return;
            int status = vm->GetEnv((void**)&env, JNI_VERSION_1_6);
            if (status == JNI_EDETACHED) {
                if (vm->AttachCurrentThread(&env, nullptr) == 0) {
                    attached = true;
                } else { env = nullptr; }
            }
        }
        ~ScopedJNIEnv() {
            if (attached && jvm1) jvm1->DetachCurrentThread();
        }
    };

    jobject GetCurrentActivity(JNIEnv* env) {
        if (g_Activity) return g_Activity;
        return nullptr;
    }

    std::string GetAndroidID() {
        if (!jvm1) return "no_jvm";
        
        ScopedJNIEnv scopedEnv(jvm1);
        JNIEnv* env = scopedEnv.env;
        if (!env) return "no_env";

        jobject activity = GetCurrentActivity(env);
        if (!activity) return "no_activity";

        jclass contextClass = env->GetObjectClass(activity);
        jmethodID getContentResolver = env->GetMethodID(contextClass, "getContentResolver", "()Landroid/content/ContentResolver;");
        if (!getContentResolver) return "err_get_cr";
        
        jobject contentResolver = env->CallObjectMethod(activity, getContentResolver);
        if (!contentResolver) return "err_content_resolver";

        jclass settingsSecureClass = env->FindClass("android/provider/Settings$Secure");
        if (!settingsSecureClass) return "err_settings_class";

        jmethodID getString = env->GetStaticMethodID(settingsSecureClass, "getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;");
        if (!getString) return "err_get_string";

        jstring androidIdParam = env->NewStringUTF("android_id");

        jstring hwidJString = (jstring)env->CallStaticObjectMethod(settingsSecureClass, getString, contentResolver, androidIdParam);
        
        env->DeleteLocalRef(androidIdParam);

        if (!hwidJString) return "null_hwid";

        const char* nativeString = env->GetStringUTFChars(hwidJString, 0);
        std::string result = nativeString;
        env->ReleaseStringUTFChars(hwidJString, nativeString);

        return result;
    }

    std::string GetAndroidClipboard() {
        if (!jvm1) return ""; 
        ScopedJNIEnv scopedEnv(jvm1);
        JNIEnv* env = scopedEnv.env;
        if (!env) return "";

        jobject activity = GetCurrentActivity(env);
        if (!activity) return "";

        jclass contextClass = env->GetObjectClass(activity);
        jfieldID clipboardServiceField = env->GetStaticFieldID(contextClass, "CLIPBOARD_SERVICE", "Ljava/lang/String;");
        jstring clipboardService = (jstring)env->GetStaticObjectField(contextClass, clipboardServiceField);

        jmethodID getSystemService = env->GetMethodID(contextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
        jobject clipboardManager = env->CallObjectMethod(activity, getSystemService, clipboardService);
        if (!clipboardManager) return "";

        jclass clipboardManagerClass = env->FindClass("android/content/ClipboardManager");
        jmethodID getPrimaryClip = env->GetMethodID(clipboardManagerClass, "getPrimaryClip", "()Landroid/content/ClipData;");
        if (!getPrimaryClip) return ""; 
        jobject clipData = env->CallObjectMethod(clipboardManager, getPrimaryClip);
        if (!clipData) return "";

        jclass clipDataClass = env->FindClass("android/content/ClipData");
        jmethodID getItemAt = env->GetMethodID(clipDataClass, "getItemAt", "(I)Landroid/content/ClipData$Item;");
        jobject item = env->CallObjectMethod(clipData, getItemAt, 0);
        if (!item) return "";

        jclass itemClass = env->FindClass("android/content/ClipData$Item");
        jmethodID getText = env->GetMethodID(itemClass, "getText", "()Ljava/lang/CharSequence;");
        jobject charSequence = env->CallObjectMethod(item, getText);
        if (!charSequence) return "";

        jclass charSequenceClass = env->FindClass("java/lang/CharSequence");
        jmethodID toString = env->GetMethodID(charSequenceClass, "toString", "()Ljava/lang/String;");
        jstring text = (jstring)env->CallObjectMethod(charSequence, toString);
        if (!text) return "";

        const char* nativeString = env->GetStringUTFChars(text, 0);
        std::string result = nativeString;
        env->ReleaseStringUTFChars(text, nativeString);
        return result;
    }
}

namespace Auth {
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    class AuthSystem {
    private:
        const string API_URL = "https://8080-ibsoqeux67uhdylwcd3ca-83ba27f9.sg1.manus.computer/api.php";
        // if your have website but don't have .PHP DM me in Telegram (@Diddy444444) 
        
        bool isLoggedIn = false;
        string currentHWID = "";
        string lastMessage = "";

        bool IsKeyExpired(string expireDateStr) {
            if (expireDateStr.empty()) return true; 

            int y, m, d, H = 23, M = 59;
            
            if (sscanf(expireDateStr.c_str(), "%d-%d-%d %d:%d", &y, &m, &d, &H, &M) != 5) {
                if (sscanf(expireDateStr.c_str(), "%d-%d-%d", &y, &m, &d) != 3) {
                    return true;
                }
                H = 23; M = 59;
            }

            std::time_t t = std::time(nullptr);
            std::tm* now = std::localtime(&t);
            int curY = now->tm_year + 1900;
            int curM = now->tm_mon + 1;
            int curD = now->tm_mday;
            int curH = now->tm_hour;
            int curMin = now->tm_min;

            if (curY > y) return true;
            if (curY < y) return false;

            if (curM > m) return true;
            if (curM < m) return false;

            if (curD > d) return true;
            if (curD < d) return false;

            if (curH > H) return true;
            if (curH < H) return false;

            if (curMin >= M) return true;

            return false;
        }

        string SendRequest(string key, string hwid) {
            CURL* curl;
            CURLcode res;
            string response;

            curl = curl_easy_init();
            if (curl) {
                string full_url = API_URL + "?key=" + key + "&hwid=" + hwid;
                
                curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
                
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); 
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
                
                curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

                res = curl_easy_perform(curl);
                if (res != CURLE_OK) {
                    return "Connection Error";
                }
                curl_easy_cleanup(curl);
            } else {
                return "CURL Init Failed";
            }
            return response;
        }

    public:
        AuthSystem() {
            currentHWID = ""; 
        }

        string Login(string key) {
            if (key.length() < 3) {
                lastMessage = "Key too short";
                return lastMessage;
            }

            if (currentHWID.empty() || currentHWID == "no_jvm") {
                currentHWID = JNIHelper::GetAndroidID();
                
                if (currentHWID == "no_jvm" || currentHWID == "no_activity") {
                    lastMessage = "Initialization Error: " + currentHWID;
                    return lastMessage;
                }
            }

            if (UseCustomKeys) {
                bool keyFound = false;
                for (const auto& k : CustomKeysList) {
                    if (k.key == key) {
                        keyFound = true;
                        if (IsKeyExpired(k.expireDate)) {
                            lastMessage = "Key Expired (" + k.expireDate + ")";
                            return lastMessage;
                        } else {
                            isLoggedIn = true;
                            lastMessage = "Success";
                            return "Success";
                        }
                    }
                }
                if (!keyFound) {
                    lastMessage = "Invalid Key (Custom)";
                    return lastMessage;
                }
            }

            string result = SendRequest(key, currentHWID);
            
            if (result.find("Login Success") != string::npos || 
                result.find("Registered HWID") != string::npos) {
                
                isLoggedIn = true;
                lastMessage = "Success";
                return "Success";
            } 
            else if (result.find("HWID MISMATCH") != string::npos) {
                lastMessage = "HWID Mismatch (Locked)";
                return lastMessage;
            }
            else if (result.find("KEY EXPIRED") != string::npos) {
                lastMessage = "Key Expired";
                return lastMessage;
            }
            else if (result.find("KEY BANNED") != string::npos) {
                lastMessage = "Key Banned";
                return lastMessage;
            }
            else if (result.find("KEY NOT FOUND") != string::npos) {
                lastMessage = "Key Not Found";
                return lastMessage;
            }
            else if (result.find("Connection Error") != string::npos) {
                lastMessage = "Connection Error";
                return lastMessage;
            }
            else {
                lastMessage = "Invalid Response";
                return lastMessage;
            }
        }
        string GetStatusMessage() { return lastMessage; }
    };
}

namespace ImGuiX {
    
    static float GScale = 1.0f; 
    static char keyBuffer[256] = "";
    static Auth::AuthSystem authSystem; 
    static bool isCheckingKey = false;
    static string statusMsg = "";
    
    static std::string UpdateExpireDate = "2026-12-30 12:22"; // change your time 
    
    bool IsDateExpired(string expireDateStr) {
        if (expireDateStr.empty()) return false; 

        int y, m, d, H = 23, M = 59;
        
        if (sscanf(expireDateStr.c_str(), "%d-%d-%d %d:%d", &y, &m, &d, &H, &M) != 5) {
            if (sscanf(expireDateStr.c_str(), "%d-%d-%d", &y, &m, &d) != 3) {
                return false;
            }
            H = 23; M = 59;
        }

        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        int curY = now->tm_year + 1900;
        int curM = now->tm_mon + 1;
        int curD = now->tm_mday;
        int curH = now->tm_hour;
        int curMin = now->tm_min;

        if (curY > y) return true;
        if (curY < y) return false;

        if (curM > m) return true;
        if (curM < m) return false;

        if (curD > d) return true;
        if (curD < d) return false;

        if (curH > H) return true;
        if (curH < H) return false;

        if (curMin >= M) return true;

        return false;
    }

    void SetAccentColor(float r, float g, float b, float a) {
        accentcolor = ImVec4(r, g, b, a);
    }

    void SetAccentColor(const ImColor& newColor) {
        accentcolor = newColor.Value;
    }

    std::string GetRuntime() {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
        int hours = duration / 3600;
        int minutes = (duration % 3600) / 60;
        int seconds = duration % 60;
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, seconds);
        return std::string(buffer);
    }

    void DripSection(const char* text) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return;
        
        float h = 40.0f * GScale; 
        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = ImVec2(GetContentRegionAvail().x, h);
        
        ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(size);
        if (!ItemAdd(bb, 0)) return;
        
        ImDrawList* draw = window->DrawList;
        
        float barW = 10.0f * GScale;
        float barH = 30.0f * GScale;
        
        float topOffset = 5.0f * GScale; 
        float barY = pos.y + topOffset;
        
        draw->AddRectFilled(
            ImVec2(pos.x, barY), 
            ImVec2(pos.x + barW, barY + barH), 
            GetColorU32(DripCol::Accent), 
            2.0f * GScale
        );
        
        float textOffX = 15.0f * GScale;
        ImVec2 textSize = CalcTextSize(text);
        
        ImVec2 textPos = ImVec2(pos.x + barW + textOffX, barY + (barH - textSize.y)/2);
        
        draw->AddText(textPos, GetColorU32(ImVec4(0.8f, 0.8f, 0.8f, 1.0f)), text);
    }

    bool TabButton(const char* label, bool selected, const ImVec2& size) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return false;
        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        const ImVec2 labelSize = CalcTextSize(label, NULL, true);
        ImVec2 pos = window->DC.CursorPos;
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(size, style.FramePadding.y);
        if (!ItemAdd(bb, id)) return false;
        bool hovered, held;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held);
        if (pressed) MarkItemEdited(id);
        auto draw = GetWindowDrawList();
        const ImU32 bgCol = GetColorU32(selected ? ImGuiCol_Button : ImGuiCol_ButtonActive);
        draw->AddRectFilled(bb.Min, bb.Max, bgCol);
        float t = selected ? 1.0f : 0.0f;
        float animSpeed = 0.50f; 
        if (g.LastActiveId == g.CurrentWindow->GetID(label)) {
            float tAnim = ImSaturate(g.LastActiveIdTimer / animSpeed);
            t = selected ? (tAnim) : (0);
        }
        auto textColor = ImGui::GetColorU32(ImLerp(ImVec4(180 / 255.f, 180 / 255.f, 180 / 255.f, 180 / 255.f), accentcolor, t));
        draw->AddText(ImVec2(pos.x + size.x / 2 - labelSize.x / 2, pos.y + size.y / 2 - labelSize.y / 2), textColor, label);
        const float lineHeight = 3.0f;  
        const float lineWidth = size.x;
        const ImU32 lineColor = ImGui::GetColorU32(ImLerp(ImVec4(180 / 255.f, 180 / 255.f, 180 / 255.f, 180 / 255.f), accentcolor, t));
        const float lineY = pos.y + size.y - lineHeight; 
        draw->AddLine(ImVec2(pos.x, lineY), ImVec2(pos.x + lineWidth, lineY), lineColor, lineHeight); 
        return pressed;
    }

    void CenteredText(const char* text) {
        ImVec2 window_size = ImGui::GetWindowSize();
        ImVec2 text_size = ImGui::CalcTextSize(text);
        int x = (window_size.x - text_size.x) / 2;
        int y = static_cast<int>(ImGui::GetCursorPosY());
        ImGui::SetCursorPosX(x);
        ImGui::Text(text);
        ImGui::SetCursorPosY(static_cast<float>(y + text_size.y));
    }
    
    void DrawUpdateScreen() {
        float s = GScale;
        ImGuiIO& io = ImGui::GetIO();
        
        ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 150));

        float winWidth = 575.0f * s;
        ImGui::SetNextWindowSize(ImVec2(winWidth, 0)); 
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.4f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));

        if (ImGui::Begin("MaintenanceCard", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
            
            ImDrawList* draw = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetWindowPos();
            ImVec2 sz = ImGui::GetWindowSize();
            
            //ImU32 colBg = ImGui::GetColorU32(DripCol::BgDark); 
            ImU32 colBg = IM_COL32(18, 18, 22, 255);
            ImU32 colAccent = ImGui::GetColorU32(DripCol::Accent); 
            
            float rounding = 8.0f * s;
            
            draw->AddRectFilled(p, ImVec2(p.x + sz.x, p.y + sz.y), colBg, rounding, ImDrawFlags_RoundCornersTop);
            
            float cursorY = 0.0f;
            
            cursorY += 25.0f * s; 
            ImGui::SetCursorPosY(cursorY);
            
            float oldScale = ImGui::GetFont()->Scale;
            ImGui::GetFont()->Scale *= 0.85f; 
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
            
            CenteredText("DRIP CLIENT | MOBILE");
            
            ImGui::PopFont();
            ImGui::GetFont()->Scale = oldScale; 
            
            cursorY = ImGui::GetCursorPosY() + (25.0f * s);
            
            float lineThickness = 8.0f * s;
            ImVec2 lineStart = ImVec2(p.x, p.y + cursorY);
            ImVec2 lineEnd = ImVec2(p.x + sz.x, p.y + cursorY + lineThickness);
            draw->AddRectFilled(lineStart, lineEnd, colAccent);
            
            cursorY += lineThickness;
            
            cursorY += 25.0f * s; 
            ImGui::SetCursorPosY(cursorY);
            
            ImGui::PushStyleColor(ImGuiCol_Text, DripCol::Accent);
            CenteredText("Application In Maintenance !!!");
            ImGui::PopStyleColor();
            
            cursorY += 50.0f * s; 
            ImGui::SetCursorPosY(cursorY);
        }
        ImGui::End();
        
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }

    void DrawKeySystem(bool* pShowKeySystem) {
        if (!pShowKeySystem || !(*pShowKeySystem)) return;

        float s = GScale;
        ImGuiIO& io = ImGui::GetIO();

        ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), io.DisplaySize, IM_COL32(0, 0, 0, 200));

        float winWidth = 600.0f * s;
        ImGui::SetNextWindowSize(ImVec2(winWidth, 0));
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));

        if (ImGui::Begin("KeySystemCard", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
            
            ImDrawList* draw = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetWindowPos();
            ImVec2 sz = ImGui::GetWindowSize();
            
            //ImU32 colBg = ImGui::GetColorU32(DripCol::BgDark); 
            ImU32 colBg = IM_COL32(18, 18, 22, 255);
            ImU32 colAccent = ImGui::GetColorU32(DripCol::Accent);
            
            float rounding = 8.0f * s; 

            draw->AddRectFilled(p, ImVec2(p.x + sz.x, p.y + sz.y), colBg, rounding);
            
            float cursorY = 0.0f;

            cursorY += 25.0f * s;
            ImGui::SetCursorPosY(cursorY);

            float oldScale = ImGui::GetFont()->Scale;
            ImGui::GetFont()->Scale *= 0.85f; 
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
            
            CenteredText("DRIP CLIENT | MOBILE");
            
            ImGui::PopFont();
            ImGui::GetFont()->Scale = oldScale;

            cursorY = ImGui::GetCursorPosY() + (25.0f * s);

            float lineH = 8.0f * s;
            draw->AddRectFilled(ImVec2(p.x, p.y + cursorY), ImVec2(p.x + sz.x, p.y + cursorY + lineH), colAccent);
            
            cursorY += lineH;

            float paddingX = 25.0f * s;
            float paddingY = 40.0f * s; 
            
            cursorY += paddingY;
            ImGui::SetCursorPosY(cursorY);
            
            float itemWidth = sz.x - (paddingX * 2);
            ImGui::SetCursorPosX(paddingX);

            ImVec4 btnColor = ImVec4(0.102f, 0.110f, 0.118f, 1.0f);
            ImVec4 btnActive = ImVec4(0.149f, 0.161f, 0.173f, 1.0f);

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 20.0f * s);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f); 
            
            ImGui::PushStyleColor(ImGuiCol_FrameBg, DripCol::BgItem);
            ImGui::PushStyleColor(ImGuiCol_Text, DripCol::TextWhite);
            
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f * s, 7.0f * s)); 
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f); 
            ImGui::PushStyleColor(ImGuiCol_Border, DripCol::BorderDark);
            
            ImGui::PushItemWidth(itemWidth);
            ImGui::InputTextWithHint("##KeyInput", "Insert key...", keyBuffer, sizeof(keyBuffer));
            ImGui::PopItemWidth();
            
            ImGui::PopStyleColor(1);
            ImGui::PopStyleVar(2);

            ImGui::Dummy(ImVec2(0, 25.0f * s));
            ImGui::SetCursorPosX(paddingX);

            ImGui::PushStyleColor(ImGuiCol_Button, btnColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, btnActive);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, btnActive);

            float btnHeight = 50.0f * s;
            if (ImGui::Button("Paste", ImVec2(itemWidth, btnHeight))) {
                string clipText = JNIHelper::GetAndroidClipboard();
                if (!clipText.empty()) {
                    snprintf(keyBuffer, sizeof(keyBuffer), "%s", clipText.c_str());
                }
            }

            ImGui::Dummy(ImVec2(0, 20.0f * s));
            ImGui::SetCursorPosX(paddingX);

            if (ImGui::Button(isCheckingKey ? "Checking..." : "Access", ImVec2(itemWidth, btnHeight))) {
                if (!isCheckingKey) {
                    isCheckingKey = true;
                    string res = authSystem.Login(keyBuffer);
                    
                    if (res == "Success") {
                        *pShowKeySystem = false; 
                    } else {
                        statusMsg = res; 
                    }
                    isCheckingKey = false;
                }
            }
            
            if (!statusMsg.empty()) {
                float msgW = CalcTextSize(statusMsg.c_str()).x;
                ImGui::SetCursorPosX((sz.x - msgW) / 2);
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", statusMsg.c_str());
            }

            ImGui::PopStyleColor(5); 
            ImGui::PopStyleVar(2);   

            ImGui::Dummy(ImVec2(0, 50.0f * s));
            
            cursorY = ImGui::GetCursorPosY();
        }
        ImGui::End();

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }

    void NeonLineAnimation(){
      ImVec2 P1, P2;
      ImDrawList* pDrawList;
      const auto& CurrentWindowPos = ImGui::GetWindowPos();
      const auto& pWindowDrawList = ImGui::GetWindowDrawList();
      P1 = ImVec2(1.000f, 1.000f);
      P1.x += CurrentWindowPos.x;
      P1.y += CurrentWindowPos.y;
      P2 = ImVec2(1000.000f, 1.000f);
      P2.x += CurrentWindowPos.x;
      P2.y += CurrentWindowPos.y;
      pDrawList = pWindowDrawList;
      float neonColorR = sin(ImGui::GetTime() * 2.0f) * 0.5f + 0.5f;
      float neonColorG = cos(ImGui::GetTime() * 2.0f) * 0.5f + 0.5f;
      float neonColorB = sin(ImGui::GetTime() * 2.0f) * cos(ImGui::GetTime() * 2.0f) * 0.5f + 0.5f;
      pDrawList->AddLine(P1, P2, ImColor(neonColorR, neonColorG, neonColorB, 1.000f), 3.000f);
    }

    void Line(ImColor color){
      ImVec2 P1, P2;
      ImDrawList* pDrawList;
      const auto& CurrentWindowPos = ImGui::GetWindowPos();
      const auto& pWindowDrawList = ImGui::GetWindowDrawList();
      P1 = ImVec2(1.000f, 1.000f);
      P1.x += CurrentWindowPos.x;
      P1.y += CurrentWindowPos.y;
      P2 = ImVec2(1000.000f, 1.000f);
      P2.x += CurrentWindowPos.x;
      P2.y += CurrentWindowPos.y;
      pDrawList = pWindowDrawList;
      pDrawList->AddLine(P1, P2, color, 3.000f);
    }

    void LineRed(){
      ImVec2 P1, P2;
      ImDrawList* pDrawList;
      const auto& CurrentWindowPos = ImGui::GetWindowPos();
      const auto& pWindowDrawList = ImGui::GetWindowDrawList();
      P1 = ImVec2(1.000f, 1.000f);
      P1.x += CurrentWindowPos.x;
      P1.y += CurrentWindowPos.y;
      P2 = ImVec2(1000.000f, 1.000f);
      P2.x += CurrentWindowPos.x;
      P2.y += CurrentWindowPos.y;
      pDrawList = pWindowDrawList;
      pDrawList->AddLine(P1, P2, ImColor(1.000f, 0.000f, 0.000f, 1.000f), 3.000f);
    }

    void RainbowAnimationLine(float thickness) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 menu_pos = ImGui::GetWindowPos();
        ImVec2 menu_size = ImGui::GetWindowSize();
        static float color_offset = 0.0f; 
        color_offset += 0.004f; 
        ImVec2 end_pos = ImVec2(menu_pos.x + menu_size.x, menu_pos.y + thickness);
        draw_list->AddRectFilledMultiColor(
            menu_pos,
            end_pos,
            ImColor::HSV(ImFmod(color_offset, 1.0f), 1.0f, 1.0f),
            ImColor::HSV(ImFmod(color_offset + 0.25f, 1.0f), 1.0f, 1.0f),
            ImColor::HSV(ImFmod(color_offset + 0.5f, 1.0f), 1.0f, 1.0f),
            ImColor::HSV(ImFmod(color_offset + 0.75f, 1.0f), 1.0f, 1.0f));
    }

    bool VerticalSliderFloat(const char* label, const ImVec2& size, float* v, float v_min, float v_max)
    {
        ImGui::PushID(label);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton(label, size);
        if (ImGui::IsItemActive()) {
            ImGui::SetTooltip("%.3f", *v);
            float delta = ImGui::GetIO().MouseDelta.y;
            float range = v_max - v_min;
            *v -= (delta / size.y) * range;
            *v = ImClamp(*v, v_min, v_max);
        }
        ImGui::GetWindowDrawList()->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), ImGui::GetColorU32(ImGuiCol_FrameBg));
        float fraction = 1.0f - (*v - v_min) / (v_max - v_min);
        float knobPosY = fraction * (size.y - size.x) + size.x / 2; 
        ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x, pos.y + knobPosY - size.x/2), 
                                                  ImVec2(pos.x + size.x, pos.y + knobPosY + size.x/2), 
                                                  ImGui::GetColorU32(ImGuiCol_SliderGrabActive));
        ImGui::SameLine();
        ImGui::Text(label);
        ImGui::PopID();
        return ImGui::IsItemDeactivatedAfterEdit();
    }

    static void HelpMarker(const char* label, const char* desc, float fontSize)
    {
    	ImGui::Text(label);
    	ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * fontSize);
            ImGui::TextUnformatted(desc);
            ImGui::EndTooltip();
        }
    }
    	
    void Switch(const char* label, bool* v)
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImGuiStyle& style = ImGui::GetStyle();
        float height = ImGui::GetFrameHeight();
        float width = height * 1.55f;
        float radius = height * 0.50f;
        ImGui::InvisibleButton(label, ImVec2(width, height));
        if (ImGui::IsItemClicked()) *v = !*v;
        float t = *v ? 1.0f : 0.0f;
        ImGuiContext& g = *GImGui;
        float ANIM_SPEED = 0.08f;
        if (g.LastActiveId == g.CurrentWindow->GetID(label)) {
            float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
            t = *v ? (t_anim) : (1.0f - t_anim);
        }
        ImU32 col_bg;
        if (ImGui::IsItemHovered())
            col_bg = ImGui::GetColorU32(ImLerp(style.Colors[ImGuiCol_FrameBgHovered], style.Colors[ImGuiCol_FrameBg], t));
        else
            col_bg = ImGui::GetColorU32(ImLerp(style.Colors[ImGuiCol_FrameBg], style.Colors[ImGuiCol_FrameBgActive], t));
        draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
        draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
        ImGui::SameLine();
        ImGui::Text(label);
    }
    	
    void Date() {
        std::time_t currentTime = std::time(nullptr);
        std::tm* localTime = std::localtime(&currentTime);
        char dateString[12]; 
        std::strftime(dateString, sizeof(dateString), "%Y-%m-%d", localTime);
        ImGui::Text("Current Date: %s", dateString);
    }

    void XClock() {
        std::time_t currentTime = std::time(nullptr);
        std::tm* localTime = std::localtime(&currentTime);
        int currentSec = localTime->tm_sec;
        if (currentSec != currentSecond) {
            lastUpdateTime = std::chrono::system_clock::now();
            currentSecond = currentSec;
        }
        auto now = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsedSeconds = now - lastUpdateTime;
        char timeString[9]; 
        std::strftime(timeString, sizeof(timeString), "%H:%M:%S", localTime);
        ImGui::Text("Current Time: %s", timeString);
        ImGui::Text("Elapsed Time: %.2f seconds", elapsedSeconds.count());
    }

    void RainbowText(const char* text) {
        ImVec4 rainbowColors[] = {
            ImVec4(1.0f, 0.0f, 0.0f, 1.0f),   
            ImVec4(1.0f, 0.5f, 0.0f, 1.0f),   
            ImVec4(1.0f, 1.0f, 0.0f, 1.0f), 
            ImVec4(0.0f, 1.0f, 0.0f, 1.0f), 
            ImVec4(0.0f, 0.0f, 1.0f, 1.0f),  
            ImVec4(0.5f, 0.0f, 0.5f, 1.0f)  
        };
        static float time = 0.0f;
        time += 0.01f;
        int colorIndex = int(time) % 6;
        float lerpFactor = time - int(time);
        ImVec4 color1 = rainbowColors[colorIndex];
        ImVec4 color2 = rainbowColors[(colorIndex + 1) % 6];
        ImVec4 lerpedColor;
        lerpedColor.x = color1.x + lerpFactor * (color2.x - color1.x);
        lerpedColor.y = color1.y + lerpFactor * (color2.y - color1.y);
        lerpedColor.z = color1.z + lerpFactor * (color2.z - color1.z);
        lerpedColor.w = color1.w + lerpFactor * (color2.w - color1.w);
        ImGui::TextColored(lerpedColor, text);
    }
    	
    void RainbowBegin(const char* title, bool* p_opn, ImGuiWindowFlags flags) {
        ImVec4 rainbowColors[] = {
            ImVec4(1.0f, 0.0f, 0.0f, 1.0f),   
            ImVec4(1.0f, 0.5f, 0.0f, 1.0f),   
            ImVec4(1.0f, 1.0f, 0.0f, 1.0f),   
            ImVec4(0.0f, 1.0f, 0.0f, 1.0f),   
            ImVec4(0.0f, 0.0f, 1.0f, 1.0f),   
            ImVec4(0.5f, 0.0f, 0.5f, 1.0f)    
        };
        static float time = 0.0f;
        time += 0.01f;
        int colorIndex = int(time) % 6;
        float lerpFactor = time - int(time);
        ImVec4 color1 = rainbowColors[colorIndex];
        ImVec4 color2 = rainbowColors[(colorIndex + 1) % 6];
        ImVec4 lerpedColor;
        lerpedColor.x = color1.x + lerpFactor * (color2.x - color1.x);
        lerpedColor.y = color1.y + lerpFactor * (color2.y - color1.y);
        lerpedColor.z = color1.z + lerpFactor * (color2.z - color1.z);
        lerpedColor.w = color1.w + lerpFactor * (color2.w - color1.w);
        ImGui::PushStyleColor(ImGuiCol_Text, lerpedColor);
        ImGui::Begin(title, p_opn, flags);
        ImGui::PopStyleColor();
    }

    void SetupDripStyle(float scaleFactor = 3.5f) { 
        ImGuiStyle& style = ImGui::GetStyle();
        style = ImGuiStyle();
        GScale = scaleFactor / 3.5f; 
        style.ScaleAllSizes(scaleFactor); 
        style.WindowRounding    = 12.0f * GScale; 
        style.ChildRounding     = 8.0f * GScale;
        style.FrameRounding     = 8.0f * GScale;
        style.GrabRounding      = 4.0f * GScale;
        style.PopupRounding     = 8.0f * GScale;
        style.ScrollbarRounding = 10.0f * GScale;
        style.WindowPadding     = ImVec2(10 * GScale, 10 * GScale);
        style.FramePadding      = ImVec2(8 * GScale, 6 * GScale);
        style.ItemSpacing       = ImVec2(8 * GScale, 8 * GScale); 
        style.ScrollbarSize     = 4.0f * GScale;
        style.WindowBorderSize  = 1.0f;
        style.ChildBorderSize   = 1.0f;
        style.PopupBorderSize   = 1.0f;
        style.FrameBorderSize   = 0.0f; 
        style.Colors[ImGuiCol_WindowBg]       = DripCol::BgDark;
        style.Colors[ImGuiCol_ChildBg]        = DripCol::BgSection;
        style.Colors[ImGuiCol_PopupBg]        = DripCol::BgSection;
        style.Colors[ImGuiCol_Text]           = DripCol::TextWhite;
        style.Colors[ImGuiCol_Border]         = DripCol::Border;
        style.Colors[ImGuiCol_FrameBg]        = DripCol::BgSection;
        style.Colors[ImGuiCol_Button]         = DripCol::Accent;
        style.Colors[ImGuiCol_ButtonHovered]  = DripCol::ButtonHover;
        style.Colors[ImGuiCol_ButtonActive]   = DripCol::ButtonHover;
        style.Colors[ImGuiCol_ScrollbarBg]    = ImVec4(0,0,0,0);
        style.Colors[ImGuiCol_ScrollbarGrab]  = DripCol::Border;
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = DripCol::TextGray;
        style.Colors[ImGuiCol_ScrollbarGrabActive]  = DripCol::Accent;
        style.Colors[ImGuiCol_CheckMark]      = DripCol::Accent;
    }

    void DripHeader(const char* text, bool* isCollapsed) {}

    bool DripTab(const char* label, bool active, float width_pct)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;
        const ImGuiID id = window->GetID(label);
        float tabWidth    = width_pct;
        float tabHeight   = 65.0f * GScale; 
        ImVec2 pos = window->DC.CursorPos;
        ImRect bb(ImVec2(pos.x, pos.y), ImVec2(pos.x + tabWidth, pos.y + tabHeight));
        ImGui::ItemSize(ImVec2(tabWidth, tabHeight));
        if (!ImGui::ItemAdd(bb, id)) return false;
        bool hovered, held;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held);
        ImU32 bgCol = GetColorU32(DripCol::Accent);
        if (hovered) bgCol = GetColorU32(DripCol::ButtonHover);
        float round = 8.0f * GScale;
        window->DrawList->AddRectFilled(bb.Min, bb.Max, bgCol, round);
        float oldScale = ImGui::GetFont()->Scale;
        ImGui::GetFont()->Scale *= 0.75f; 
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImVec2 textSize = ImGui::CalcTextSize(label);
        window->DrawList->AddText(
            ImVec2(bb.Min.x + (bb.GetWidth() - textSize.x) * 0.5f, bb.Min.y + (bb.GetHeight() - textSize.y) * 0.5f),
            GetColorU32(DripCol::TextWhite),
            label
        );
        ImGui::PopFont();
        ImGui::GetFont()->Scale = oldScale; 
        return pressed;
    }

    bool DripCheckbox(const char* label, bool* v) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return false;
        const ImGuiID id = window->GetID(label);
        float h = 40.0f * GScale; 
        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = ImVec2(GetContentRegionAvail().x, h);
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(size);
        if (!ItemAdd(bb, id)) return false;
        float radius = 17.0f * GScale; 
        float circleX = bb.Min.x + (20.0f * GScale); 
        float circleY = bb.Min.y + (size.y / 2);
        float touchPad = 10.0f * GScale; 
        ImRect interact_bb(
            ImVec2(circleX - radius - touchPad, circleY - radius - touchPad),
            ImVec2(circleX + radius + touchPad, circleY + radius + touchPad)
        );
        bool hovered, held;
        bool pressed = ButtonBehavior(interact_bb, id, &hovered, &held);
        if (pressed) *v = !(*v);
        ImDrawList* draw = window->DrawList;
        ImVec2 rectMin = ImVec2(circleX - radius, circleY - radius);
        ImVec2 rectMax = ImVec2(circleX + radius, circleY + radius);
        draw->AddRectFilled(rectMin, rectMax, GetColorU32(DripCol::BgItem), 6.0f * GScale); 
        if (*v) {
            ImVec2 center(circleX, circleY);
            float s = GScale;
            float checkW = 10.0f * s; 
            float checkH = 23.0f * s;
            float thickness = 3.5f * s; 
            float cos45 = 0.707f;
            float sin45 = 0.707f;
            ImVec2 p1_local = ImVec2(checkW, 0); 
            ImVec2 p2_local = ImVec2(checkW, checkH); 
            ImVec2 p3_local = ImVec2(0, checkH); 
            float offsetX = center.x + (5.3f * s); 
            float offsetY = center.y - (15.0f * s); 
            auto rotate = [&](ImVec2 p) -> ImVec2 {
                return ImVec2(
                    p.x * cos45 - p.y * sin45 + offsetX, 
                    p.x * sin45 + p.y * cos45 + offsetY
                );
            };
            ImVec2 p1 = rotate(p1_local);
            ImVec2 p2 = rotate(p2_local);
            ImVec2 p3 = rotate(p3_local);
            ImVec2 points[3] = { p1, p2, p3 };
            ImU32 CheckColor = GetColorU32(DripCol::Accent);
            draw->AddPolyline(points, 3, CheckColor, 0, thickness);
        }
        float textX = circleX + radius + (10.0f * GScale); 
        float oldScale = ImGui::GetFont()->Scale;
        ImGui::GetFont()->Scale *= 0.70f; 
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImVec2 textSize = CalcTextSize(label);
        draw->AddText(ImVec2(textX, circleY - textSize.y/2), GetColorU32(DripCol::TextWhite), label);
        ImGui::PopFont();
        ImGui::GetFont()->Scale = oldScale; 
        return pressed;
    }

    void DripSlider(const char* label, float* v, float v_min, float v_max, const char* fmt = "%.0f")
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return;
        const ImGuiID id = window->GetID(label);
        float h = 42.0f * GScale; 
        ImVec2 pos  = window->DC.CursorPos;
        ImVec2 size = ImVec2(GetContentRegionAvail().x, h);
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(size);
        if (!ItemAdd(bb, id)) return;
        ImDrawList* draw = window->DrawList;
        float sliderOffsetY = 4.0f * GScale;
        float centerY = bb.Min.y + (size.y / 2.0f) + sliderOffsetY;
        float labelOffsetX = 0.0f * GScale; 
        ImVec2 textSize = CalcTextSize(label);
        draw->AddText(
            ImVec2(bb.Min.x + labelOffsetX, centerY - textSize.y / 2.0f),
            GetColorU32(DripCol::TextGray),
            label
        );
        float labelW  = textSize.x + (10.0f * GScale);
        float sliderW = size.x - labelW - (10.0f * GScale);
        float visualH = 32.0f * GScale; 
        ImVec2 sMin(bb.Min.x + labelW, centerY - visualH / 2.0f);
        ImVec2 sMax(sMin.x + sliderW, sMin.y + visualH);
        ImRect interactBB(sMin, sMax);
        bool hovered, held;
        ButtonBehavior(interactBB, id, &hovered, &held);
        if (held) {
            float mouseX = GetIO().MousePos.x;
            float vNorm = ImClamp((mouseX - sMin.x) / sliderW, 0.0f, 1.0f);
            *v = v_min + vNorm * (v_max - v_min);
        }
        draw->AddRectFilled(sMin, sMax, GetColorU32(DripCol::BgItem), visualH / 2.0f);
        float range = v_max - v_min;
        float thumbPx;
        if (range <= 100.0f) thumbPx = 25.0f;
        else if (range >= 1000.0f) thumbPx = 5.0f;
        else {
            float t = (range - 100.0f) / 900.0f;
            thumbPx = 20.0f - t * (20.0f - 5.0f);
        }
        float thumbW = thumbPx * GScale;
        float vNorm = (*v - v_min) / (v_max - v_min);
        float trackLen = sMax.x - sMin.x;
        float thumbLeft = sMin.x + (vNorm * (trackLen - thumbW));
        ImVec2 tMin(thumbLeft, sMin.y);
        ImVec2 tMax(thumbLeft + thumbW, sMax.y);
        ImU32 ThumbPurple = GetColorU32(DripCol::Accent);
        draw->AddRectFilled(tMin, tMax, ThumbPurple, visualH / 2.0f);
        char valBuf[32];
        sprintf(valBuf, fmt, *v);
        ImFont* font = ImGui::GetIO().Fonts->Fonts[0];
        float fontSize = font->FontSize * 0.8f; 
        ImVec2 valSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, valBuf);
        ImVec2 valPos(
            sMin.x + (sliderW / 2.0f) - (valSize.x / 2.0f),
            centerY - (valSize.y / 2.0f)
        );
        draw->AddText(font, fontSize, ImVec2(valPos.x, valPos.y + 1), IM_COL32(0, 0, 0, 160), valBuf);
        draw->AddText(font, fontSize, valPos, GetColorU32(DripCol::TextWhite), valBuf);
    }

    void DripSliderInt(const char* label, int* v, int v_min, int v_max, const char* fmt = "%d")
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return;
        const ImGuiID id = window->GetID(label);
        float h = 42.0f * GScale; 
        ImVec2 pos  = window->DC.CursorPos;
        ImVec2 size = ImVec2(GetContentRegionAvail().x, h);
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(size);
        if (!ItemAdd(bb, id)) return;
        ImDrawList* draw = window->DrawList;
        float sliderOffsetY = 4.0f * GScale;
        float centerY = bb.Min.y + (size.y / 2.0f) + sliderOffsetY;
        float labelOffsetX = 0.0f * GScale; 
        ImVec2 textSize = CalcTextSize(label);
        draw->AddText(
            ImVec2(bb.Min.x + labelOffsetX, centerY - textSize.y / 2.0f),
            GetColorU32(DripCol::TextGray),
            label
        );
        float labelW  = textSize.x + (10.0f * GScale);
        float sliderW = size.x - labelW - (10.0f * GScale);
        float visualH = 32.0f * GScale;
        ImVec2 sMin(bb.Min.x + labelW, centerY - visualH / 2.0f);
        ImVec2 sMax(sMin.x + sliderW, sMin.y + visualH);
        ImRect interactBB(sMin, sMax);
        bool hovered, held;
        ButtonBehavior(interactBB, id, &hovered, &held);
        if (held) {
            float mouseX = GetIO().MousePos.x;
            float vNorm = ImClamp((mouseX - sMin.x) / sliderW, 0.0f, 1.0f);
            *v = v_min + (int)(vNorm * (v_max - v_min));
        }
        draw->AddRectFilled(sMin, sMax, GetColorU32(DripCol::BgItem), visualH / 2.0f);
        float range = (float)(v_max - v_min);
        float thumbPx;
        if (*v < 3) {
            thumbPx = 100.0f;
        }
        else if (range <= 100.0f) {
            thumbPx = 45.0f;
        }
        else if (range >= 1000.0f) {
            thumbPx = 5.0f;
        }
        else {
            float t = (range - 100.0f) / 900.0f;
            thumbPx = 40.0f - t * (40.0f - 5.0f);
        }
        float thumbW = thumbPx * GScale;
        float vNorm = (float)(*v - v_min) / range;
        float trackLen = sMax.x - sMin.x;
        float thumbLeft = sMin.x + (vNorm * (trackLen - thumbW));
        ImVec2 tMin(thumbLeft, sMin.y);
        ImVec2 tMax(thumbLeft + thumbW, sMax.y);
        ImU32 ThumbPurple = GetColorU32(DripCol::Accent);
        draw->AddRectFilled(tMin, tMax, ThumbPurple, visualH / 2.0f);
        char valBuf[32];
        sprintf(valBuf, fmt, *v);
        ImFont* font = ImGui::GetIO().Fonts->Fonts[0];
        float fontSize = font->FontSize * 0.8f; 
        ImVec2 valSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, valBuf);
        ImVec2 valPos(
            sMin.x + (sliderW / 2.0f) - (valSize.x / 2.0f),
            centerY - (valSize.y / 2.0f)
        );
        draw->AddText(font, fontSize, ImVec2(valPos.x, valPos.y + 1), IM_COL32(0, 0, 0, 160), valBuf);
        draw->AddText(font, fontSize, valPos, GetColorU32(DripCol::TextWhite), valBuf);
    }
    
    void DripCombo(const char* label, int* current_item, const char* const items[], int items_count) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return;
        const ImGuiID id = window->GetID(label);
        float h = 42.0f * GScale; 
        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = ImVec2(GetContentRegionAvail().x, h);
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(size);
        if (!ItemAdd(bb, id)) return;
        ImDrawList* draw = window->DrawList;
        float centerY = bb.Min.y + (size.y / 2);
        ImVec2 textSize = CalcTextSize(label);
        float labelOffsetX = 0.0f * GScale; 
        draw->AddText(ImVec2(bb.Min.x + labelOffsetX, centerY - textSize.y/2), GetColorU32(DripCol::TextGray), label);
        float labelW = textSize.x + (10.0f * GScale);
        float comboW = size.x - labelW - (10.0f * GScale);
        float comboH = 32.0f * GScale; 
        ImVec2 cMin(bb.Min.x + labelW, centerY - comboH/2);
        ImVec2 cMax(cMin.x + comboW, cMin.y + comboH);
        ImRect comboBB(cMin, cMax);
        bool hovered, held;
        bool pressed = ButtonBehavior(comboBB, id, &hovered, &held);
        draw->AddRectFilled(cMin, cMax, GetColorU32(DripCol::BgItem), 4.0f * GScale);
        float arrowSize = 24.0f * GScale; 
        ImRect arrowBB(ImVec2(cMax.x - arrowSize, cMin.y), cMax);
        draw->AddRectFilled(arrowBB.Min, arrowBB.Max, GetColorU32(DripCol::Accent), 4.0f * GScale, ImDrawFlags_RoundCornersRight);
        float triSz = 7.5f * GScale; 
        ImVec2 triC = arrowBB.GetCenter();
        draw->AddTriangleFilled(
             ImVec2(triC.x - triSz, triC.y - triSz*0.6f),
             ImVec2(triC.x + triSz, triC.y - triSz*0.6f),
             ImVec2(triC.x, triC.y + triSz*0.6f),
             IM_COL32(255,255,255,255));
        const char* preview = (items_count > 0 && *current_item >= 0 && *current_item < items_count) ? items[*current_item] : "";
        ImVec2 pSize = CalcTextSize(preview);
        ImGui::PushClipRect(cMin, ImVec2(arrowBB.Min.x, cMax.y), true);
        draw->AddText(ImVec2(cMin.x + (10.0f * GScale), centerY - pSize.y/2), GetColorU32(DripCol::TextWhite), preview);
        ImGui::PopClipRect();
        if (pressed) {
            ImGui::OpenPopup(label);
        }
        float popupHeight = items_count * (35.0f * GScale) + (20.0f * GScale); 
        float screenH = ImGui::GetIO().DisplaySize.y;
        bool showAbove = (cMax.y + popupHeight) > screenH;
        ImVec2 popupPos;
        if (showAbove) {
            popupPos = ImVec2(cMin.x, cMin.y - popupHeight); 
        } else {
            popupPos = ImVec2(cMin.x, cMax.y);
        }
        ImGui::SetNextWindowPos(popupPos);
        ImGui::SetNextWindowSizeConstraints(ImVec2(comboW, 0), ImVec2(comboW, 400 * GScale));
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 8.0f * GScale);
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.26f, 0.26f, 0.26f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.35f, 0.35f, 0.35f, 1.0f)); 
        if (BeginPopup(label)) {
            ImGui::Dummy(ImVec2(0, 5.0f * GScale)); 
            for (int i = 0; i < items_count; i++) {
                bool isSelected = (*current_item == i);
                if (Selectable(items[i], isSelected)) {
                    *current_item = i;
                }
                if (isSelected) SetItemDefaultFocus();
            }
            ImGui::Dummy(ImVec2(0, 5.0f * GScale)); 
            EndPopup();
        }
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();
    }
}

void ImGui_RunningText(const char* text, float speed, ImU32 textColor = 0)
{
    static float offset = 0.0f;
    static float textWidth = 0.0f;
    static bool initialized = false;
    if (!initialized) {
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        textWidth = ImGui::CalcTextSize(text).x;
        ImGui::PopFont();
        initialized = true;
    }
    offset -= speed;
    if (offset < -textWidth) offset = ImGui::GetWindowWidth();
    ImGui::SetCursorPosX(offset);
    ImVec4 color = (textColor == 0) ? ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Text)) : ImGui::ColorConvertU32ToFloat4(textColor);
    ImGui::TextColored(color, "%s", text);
}

string GetCurrentTime() {
    time_t currentTime = time(nullptr);
    tm* timeInfo = localtime(&currentTime);
    ostringstream oss;
    oss << put_time(timeInfo, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

int ConvertDateTimeToInt(const char* datetimeString) {
    tm timeInfo = {};
    istringstream ss(datetimeString);
    ss >> timeInfo.tm_year; ss.ignore();
    ss >> timeInfo.tm_mon; ss.ignore();
    ss >> timeInfo.tm_mday; ss.ignore();
    ss >> timeInfo.tm_hour; ss.ignore();
    ss >> timeInfo.tm_min; ss.ignore();
    ss >> timeInfo.tm_sec;
    timeInfo.tm_year -= 1900;
    timeInfo.tm_mon -= 1;
    return static_cast<int>(mktime(&timeInfo));
}

void MauYuim() { ImGuiX::SetupDripStyle(); }
void DrawImGuiStyle(){ ImGuiX::SetupDripStyle(); }

void KlelYuimimguivip1() {
    ImGuiIO& io = ImGui::GetIO();
    static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };
    ImFontConfig icons_config;    
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF("static/Oswald-Regular.ttf", 30.0f); 
    io.Fonts->AddFontFromFileTTF("fonts/fa-solid-900.ttf", 30.0f, &icons_config, icons_ranges);
    io.Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(Custom), sizeof(Custom), 35.f, NULL, io.Fonts->GetGlyphRangesVietnamese());
    io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 30.0f, &icons_config, icons_ranges);
    io.KeyMap[ImGuiKey_UpArrow] = 19;
    io.KeyMap[ImGuiKey_DownArrow] = 20;
    io.KeyMap[ImGuiKey_LeftArrow] = 21;
    io.KeyMap[ImGuiKey_RightArrow] = 22;
    io.KeyMap[ImGuiKey_Enter] = 66;
    io.KeyMap[ImGuiKey_Backspace] = 67;
    io.KeyMap[ImGuiKey_Escape] = 111;
    io.KeyMap[ImGuiKey_Delete] = 112;
    io.KeyMap[ImGuiKey_Home] = 122;
    io.KeyMap[ImGuiKey_End] = 123;
    ImGui_ImplOpenGL3_Init("#version 300 es");
    ImGui::GetStyle().ScaleAllSizes(3.5f);
    ImGuiX::SetupDripStyle(3.5f);
};