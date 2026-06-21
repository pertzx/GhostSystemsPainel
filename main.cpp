#include <jni.h>
#include <pthread.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <dlfcn.h>
#include <imgui.h>
#include "imgui_impl_opengl3.h"
#include "MemoryScanner.h"
#include "TouchReader.h"
#include "And64InlineHook.hpp"
#include "Il2CppHelper.h"
#include <atomic>

// Forward declaration de g_Scanner (usado em Menu.h)
extern GhostSystems::MemoryScanner* g_Scanner;

#include "Menu.h"

#define LOG_TAG "GhostSystems"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace GhostSystems;

// Globais
GameState g_State;
FeatureConfig g_FeatureConfig;
namespace GhostSystems { Menu* g_Menu = nullptr; }
using GhostSystems::g_Menu;
GhostSystems::MemoryScanner* g_Scanner = nullptr;
TouchReader* g_TouchReader = nullptr;

bool g_ImGuiInitialized = false;

// Hooks de EGL
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay, EGLSurface);
eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

// Hook para Unity Main Thread
typedef float (*get_deltaTime_t)(void* methodInfo);
get_deltaTime_t orig_get_deltaTime = nullptr;

// Hook para Silent Aim (GetAimRotation) - Removido spoofing agressivo
typedef GhostSystems::QuaternionArgs (*get_AimRotation_t)(void* player, void* methodInfo);
get_AimRotation_t orig_get_AimRotation = nullptr;

GhostSystems::QuaternionArgs hook_get_AimRotation(void* player, void* methodInfo) {
    if (orig_get_AimRotation) {
        return orig_get_AimRotation(player, methodInfo);
    }
    
    GhostSystems::QuaternionArgs zeroRot = {0, 0, 0, 1};
    return zeroRot;
}

// Hook para Magic Bullet (GetLookDirection da Câmera) - O método definitivo para Raycast em Unity
typedef Vector3 (*GetLookDirection_t)(void* cameraController, void* methodInfo);
GetLookDirection_t orig_GetLookDirection = nullptr;

Vector3 hook_GetLookDirection(void* cameraController, void* methodInfo) {
    if (GhostSystems::g_Menu && GhostSystems::g_Menu->silentAim && GhostSystems::g_Menu->aimbotHasTarget) {
        if (GhostSystems::g_Menu->aimbotShouldAim) {
            Vector3 targetPos = GhostSystems::g_Menu->aimbotTargetPos;
            Vector3 myPos = g_State.localPlayerPos;
            myPos.y += 1.4f; // Ajuste para a altura da arma/ombro
            
            float dx = targetPos.x - myPos.x;
            float dy = targetPos.y - myPos.y;
            float dz = targetPos.z - myPos.z;
            
            float length = std::sqrt(dx*dx + dy*dy + dz*dz);
            if (length > 0.001f) {
                return { dx / length, dy / length, dz / length };
            }
        }
    }
    if (orig_GetLookDirection) {
        return orig_GetLookDirection(cameraController, methodInfo);
    }
    return {0, 0, 0};
}

// Hook para Silent Aim (SetAimRotation) - NUNCA PUXAR A TELA
typedef void (*set_AimRotation_t)(void* player, GhostSystems::QuaternionArgs rot, bool flag, void* methodInfo);
set_AimRotation_t orig_set_AimRotation = nullptr;

void hook_set_AimRotation(void* player, GhostSystems::QuaternionArgs rot, bool flag, void* methodInfo) {
    // Para Silent Aim 100% invisivel, NUNCA altere a rotacao aqui, pois isso gira a camera do jogador local!
    // Apenas passamos a rotacao original para evitar "Aim Pulling" (puxar a mira).
    if (orig_set_AimRotation) {
        orig_set_AimRotation(player, rot, flag, methodInfo);
    }
}

// Hook para Silent Aim (StartFiring) - Modificando a direção
typedef void (*StartFiring_t)(void* player, void* weaponParam, void* methodInfo);
StartFiring_t orig_StartFiring = nullptr;

void hook_StartFiring(void* player, void* weaponParam, void* methodInfo) {
    if (GhostSystems::g_Menu && GhostSystems::g_Menu->silentAim && GhostSystems::g_Menu->aimbotHasTarget) {
        if (weaponParam) {
            // Usa heuristica para encontrar e substituir a direcao (Vector3 normalizado)
            void* klass = Il2Cpp::object_get_class(weaponParam);
            if (klass) {
                void* iter = nullptr;
                void* field = nullptr;
                while ((field = Il2Cpp::class_get_fields(klass, &iter)) != nullptr) {
                    void* fieldType = Il2Cpp::field_get_type(field);
                    const char* typeName = fieldType ? Il2Cpp::type_get_name(fieldType) : "";
                    
                    if (typeName && strstr(typeName, "Vector3")) {
                        size_t offset = Il2Cpp::field_get_offset(field);
                        if (offset > 0) {
                            Vector3* dirField = (Vector3*)((uintptr_t)weaponParam + offset);
                            
                            // Heuristica: Um vetor de direcao geralmente e normalizado (length = 1)
                            float currentLen = std::sqrt(dirField->x * dirField->x + dirField->y * dirField->y + dirField->z * dirField->z);
                            
                            // Se o tamanho for proximo de 1.0, entao e a direcao!
                            if (currentLen > 0.8f && currentLen < 1.2f) {
                                Vector3 targetPos = GhostSystems::g_Menu->aimbotTargetPos;
                                Vector3 myPos = g_State.localPlayerPos;
                                myPos.y += 1.4f; // Ajuste para a altura do ombro
                                
                                float dx = targetPos.x - myPos.x;
                                float dy = targetPos.y - myPos.y;
                                float dz = targetPos.z - myPos.z;
                                
                                float length = std::sqrt(dx*dx + dy*dy + dz*dz);
                                if (length > 0.001f) {
                                    dirField->x = dx / length;
                                    dirField->y = dy / length;
                                    dirField->z = dz / length;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (orig_StartFiring) {
        orig_StartFiring(player, weaponParam, methodInfo);
    }
}

// Hook para Magic Bullet (GetFireDirection) - Modifica apenas a direcao da bala
typedef Vector3 (*GetFireDirection_t)(void* instance, bool* isSkill, void* methodInfo);
GetFireDirection_t orig_GetFireDirection = nullptr;

Vector3 hook_GetFireDirection(void* instance, bool* isSkill, void* methodInfo) {
    if (GhostSystems::g_Menu && GhostSystems::g_Menu->silentAim && GhostSystems::g_Menu->aimbotHasTarget && GhostSystems::g_Menu->aimbotShouldAim) {
        Vector3 targetPos = GhostSystems::g_Menu->aimbotTargetPos;
        Vector3 myPos = g_State.localPlayerPos;
        
        // Ajuste de altura simples, pois localPlayerPos normalmente é nos pés/quadril
        // TargetPos já é a posição da cabeça (HeadTF) ou bone selecionado
        myPos.y += 1.5f; 

        float dx = targetPos.x - myPos.x;
        float dy = targetPos.y - myPos.y;
        float dz = targetPos.z - myPos.z;
        
        float length = std::sqrt(dx*dx + dy*dy + dz*dz);
        if (length > 0.001f) {
            Vector3 aimDir;
            aimDir.x = dx / length;
            aimDir.y = dy / length;
            aimDir.z = dz / length;
            return aimDir;
        }
    }
    
    if (orig_GetFireDirection) {
        return orig_GetFireDirection(instance, isSkill, methodInfo);
    }
    
    return {0, 0, 0};
}


float hook_get_deltaTime(void* methodInfo) {
    if (GhostSystems::g_Menu) {
        GhostSystems::g_Menu->OnMainThreadTick();
    }
    if (orig_get_deltaTime) {
        return orig_get_deltaTime(methodInfo);
    }
    return 0.016f;
}

void* m_get_height = nullptr;
void* m_get_width = nullptr;
bool m_ScreenMethodsCached = false;

// Unity Input cache
void* m_get_touchCount = nullptr;
void* m_get_touch = nullptr;
void* m_get_mousePosition = nullptr;
void* m_get_mouseButton = nullptr;
bool m_InputMethodsCached = false;
bool m_TimeHooked = false;
bool m_GameHooksApplied = false;

void CacheMainThreadHooks() {
    if (!m_TimeHooked) {
        void* timeClass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Time");
        if (!timeClass) {
            timeClass = Il2Cpp::GetClass("UnityEngine.CoreModule", "UnityEngine", "Time");
        }
        
        if (timeClass) {
            void* dtMethod = Il2Cpp::class_get_method_from_name(timeClass, "get_deltaTime", 0);
            if (dtMethod) {
                A64HookFunction((void*)*(void**)dtMethod, (void*)hook_get_deltaTime, (void**)&orig_get_deltaTime);
                m_TimeHooked = true;
            }
        }
    }
    
    if (!m_GameHooksApplied) {
        void* playerClass = Il2Cpp::GetClass("Assembly-CSharp.dll", "COW.GamePlay", "Player");
        if (!playerClass) playerClass = Il2Cpp::GetClass("Assembly-CSharp", "COW.GamePlay", "Player");
        
        if (playerClass) {
            void* setAimMethod = Il2Cpp::class_get_method_from_name(playerClass, "SetAimRotation", 2);
            if (setAimMethod) A64HookFunction((void*)*(void**)setAimMethod, (void*)hook_set_AimRotation, (void**)&orig_set_AimRotation);

            void* getAimMethod = Il2Cpp::class_get_method_from_name(playerClass, "GetAimRotation", 0);
            if (getAimMethod) A64HookFunction((void*)*(void**)getAimMethod, (void*)hook_get_AimRotation, (void**)&orig_get_AimRotation);

            void* startFiringMethod = Il2Cpp::class_get_method_from_name(playerClass, "StartFiring", 1);
            if (startFiringMethod) A64HookFunction((void*)*(void**)startFiringMethod, (void*)hook_StartFiring, (void**)&orig_StartFiring);

            void* getFireDirectionMethod = Il2Cpp::class_get_method_from_name(playerClass, "GetFireDirection", 1);
            if (getFireDirectionMethod) {
                A64HookFunction((void*)*(void**)getFireDirectionMethod, (void*)hook_GetFireDirection, (void**)&orig_GetFireDirection);
            } else {
                void* weaponClass = Il2Cpp::GetClass("Assembly-CSharp", "COW.GamePlay", "WeaponBase");
                if (!weaponClass) weaponClass = Il2Cpp::GetClass("Assembly-CSharp.dll", "COW.GamePlay", "WeaponBase");
                if (weaponClass) {
                    getFireDirectionMethod = Il2Cpp::class_get_method_from_name(weaponClass, "GetFireDirection", 1);
                    if (getFireDirectionMethod) A64HookFunction((void*)*(void**)getFireDirectionMethod, (void*)hook_GetFireDirection, (void**)&orig_GetFireDirection);
                }
            }
            
            // Hook GetLookDirection da CameraController
            void* cameraControllerClass = Il2Cpp::GetClass("Assembly-CSharp.dll", "COW.GamePlay", "CameraController");
            if (!cameraControllerClass) cameraControllerClass = Il2Cpp::GetClass("Assembly-CSharp", "COW.GamePlay", "CameraController");
            if (cameraControllerClass) {
                void* getLookDirMethod = Il2Cpp::class_get_method_from_name(cameraControllerClass, "GetLookDirection", 0);
                if (getLookDirMethod) A64HookFunction((void*)*(void**)getLookDirMethod, (void*)hook_GetLookDirection, (void**)&orig_GetLookDirection);
            }
            
            m_GameHooksApplied = true;
        }
    }
}

void CacheScreenMethods() {
    if (m_ScreenMethodsCached) return;
    
    void* screenClass = Il2Cpp::GetClass("UnityEngine.CoreModule", "UnityEngine", "Screen");
    if (!screenClass) {
        screenClass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Screen");
    }
    if (screenClass) {
        m_get_height = Il2Cpp::class_get_method_from_name(screenClass, "get_height", 0);
        m_get_width = Il2Cpp::class_get_method_from_name(screenClass, "get_width", 0);
    }
    
    if (m_get_height && m_get_width) {
        m_ScreenMethodsCached = true;
        LOGI("Screen methods cached successfully.");
    }
}

void CacheInputMethods() {
    if (m_InputMethodsCached) return;

    // Tentativa na CoreModule (versoes antigas do Unity) e na InputLegacyModule (versoes novas)
    void* inputClass = Il2Cpp::GetClass("UnityEngine.InputLegacyModule", "UnityEngine", "Input");
    if (!inputClass) {
        inputClass = Il2Cpp::GetClass("UnityEngine.InputLegacyModule.dll", "UnityEngine", "Input");
    }
    if (!inputClass) {
        inputClass = Il2Cpp::GetClass("UnityEngine.CoreModule", "UnityEngine", "Input");
    }
    if (!inputClass) {
        inputClass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Input");
    }

    if (inputClass) {
        m_get_touchCount = Il2Cpp::class_get_method_from_name(inputClass, "get_touchCount", 0);
        m_get_touch = Il2Cpp::class_get_method_from_name(inputClass, "GetTouch", 1);
        m_get_mousePosition = Il2Cpp::class_get_method_from_name(inputClass, "get_mousePosition", 0);
        m_get_mouseButton = Il2Cpp::class_get_method_from_name(inputClass, "GetMouseButton", 1);
    }

    if (m_get_touchCount || m_get_touch || m_get_mousePosition || m_get_mouseButton) {
        m_InputMethodsCached = true;
        LOGI("Unity Input methods cached successfully.");
    }
}

// O Hook principal de Renderizacao EGL para Android (OpenGL ES 2.0+)
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_ImGuiInitialized) {
        // Inicializacao do contexto ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
        io.DisplaySize = ImVec2(1920, 1080); // Atualizado dinamicamente abaixo
        
        // Setup ImGui bindings para OpenGL 3
        ImGui_ImplOpenGL3_Init("#version 300 es");

        g_Menu = new Menu(g_State, g_FeatureConfig);

        g_ImGuiInitialized = true;
        LOGI("ImGui Context Initialized");
    }

    {
        static int s_il2cppInitFrame = 0;
        s_il2cppInitFrame++;
        if (!Il2Cpp::domain_get && (s_il2cppInitFrame % 60) == 0) {
            Il2Cpp::Initialize();
        }
    }

    if (Il2Cpp::thread_attach && Il2Cpp::domain_get) {
        Il2Cpp::thread_attach(Il2Cpp::domain_get());
    }

    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = 1.0f / 60.0f;

    // Atualiza a resolucao usando o EGL surface (independente do Il2Cpp)
    {
        EGLint surfaceW = 0;
        EGLint surfaceH = 0;
        if (eglQuerySurface(dpy, surface, EGL_WIDTH, &surfaceW) == EGL_TRUE &&
            eglQuerySurface(dpy, surface, EGL_HEIGHT, &surfaceH) == EGL_TRUE) {
            if (surfaceW > 0 && surfaceH > 0) {
                io.DisplaySize = ImVec2((float)surfaceW, (float)surfaceH);
                if (g_TouchReader) {
                    g_TouchReader->setScreenSize(surfaceW, surfaceH);
                }
            }
        }
    }

    // Sincroniza com Il2Cpp para atualizar a resolucao da tela (quando disponivel)
    if (Il2Cpp::domain_get && Il2Cpp::thread_attach) {
        CacheScreenMethods();
        CacheInputMethods();
        CacheMainThreadHooks();
        
        if (m_ScreenMethodsCached) {
            // Atualiza resolucao
            void* widthObj = Il2Cpp::runtime_invoke(m_get_width, nullptr, nullptr, nullptr);
            void* heightObj = Il2Cpp::runtime_invoke(m_get_height, nullptr, nullptr, nullptr);
            
            int screenW = 1920, screenH = 1080;
            if (widthObj && heightObj) {
                screenW = *(int*)((uintptr_t)widthObj + 0x10);
                screenH = *(int*)((uintptr_t)heightObj + 0x10);
                io.DisplaySize = ImVec2((float)screenW, (float)screenH);
                
                if (g_TouchReader) {
                    g_TouchReader->setScreenSize(screenW, screenH);
                }
            }
        }
    }

    // Input: tenta Unity Input; se falhar ou Il2Cpp nao estiver pronto, usa TouchReader
    {
        bool handled = false;

        if (Il2Cpp::domain_get && Il2Cpp::thread_attach && m_InputMethodsCached && m_get_mouseButton && m_get_mousePosition) {
            bool touching = false;
            float touchX = -1.0f;
            float touchY = -1.0f;

            int btnIndex = 0;
            void* args_btn[1] = { &btnIndex };
            void* isTouchingObj = Il2Cpp::runtime_invoke(m_get_mouseButton, nullptr, args_btn, nullptr);
            if (isTouchingObj) {
                touching = *(bool*)((uintptr_t)isTouchingObj + 0x10);
                if (touching) {
                    void* mousePosObj = Il2Cpp::runtime_invoke(m_get_mousePosition, nullptr, nullptr, nullptr);
                    if (mousePosObj) {
                        touchX = *(float*)((uintptr_t)mousePosObj + 0x10);
                        touchY = *(float*)((uintptr_t)mousePosObj + 0x14);
                        touchY = io.DisplaySize.y - touchY;

                        io.AddMousePosEvent(touchX, touchY);
                        io.AddMouseButtonEvent(0, true);
                        handled = true;
                    }
                } else {
                    io.AddMouseButtonEvent(0, false);
                    handled = true;
                }
            }
        }

        if (!handled && Il2Cpp::domain_get && Il2Cpp::thread_attach && m_InputMethodsCached && m_get_touchCount && m_get_touch) {
            void* touchCountObj = Il2Cpp::runtime_invoke(m_get_touchCount, nullptr, nullptr, nullptr);
            int touchCount = 0;
            if (touchCountObj) {
                touchCount = *(int*)((uintptr_t)touchCountObj + 0x10);
            }

            if (touchCount > 0) {
                int touchIndex = 0;
                void* args_touch[1] = { &touchIndex };
                void* touchObj = Il2Cpp::runtime_invoke(m_get_touch, nullptr, args_touch, nullptr);
                if (touchObj) {
                    float x1 = *(float*)((uintptr_t)touchObj + 0x14);
                    float y1 = *(float*)((uintptr_t)touchObj + 0x18);

                    float x2 = *(float*)((uintptr_t)touchObj + 0x18);
                    float y2 = *(float*)((uintptr_t)touchObj + 0x1C);

                    auto isValid = [&](float x, float y) {
                        return x >= 0.0f && y >= 0.0f && x <= io.DisplaySize.x && y <= io.DisplaySize.y;
                    };

                    float useX = x1;
                    float useY = y1;
                    if (!isValid(useX, useY) && isValid(x2, y2)) {
                        useX = x2;
                        useY = y2;
                    }

                    if (isValid(useX, useY)) {
                        useY = io.DisplaySize.y - useY;
                        io.AddMousePosEvent(useX, useY);
                        io.AddMouseButtonEvent(0, true);
                        handled = true;
                    }
                }
            } else {
                io.AddMouseButtonEvent(0, false);
                handled = true;
            }
        }

        if (!handled && g_TouchReader) {
            g_TouchReader->updateImGui(io);
            handled = true;
        }

        if (!handled) {
            io.AddMouseButtonEvent(0, false);
        }
    }

    // Inicio do Frame ImGui (Double Buffering)
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    // Renderiza o painel principal
    if (g_Menu) {
        g_Menu->render();
    }

    ImGui::Render();
    
    // Renderiza os draw data
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return orig_eglSwapBuffers(dpy, surface);
}



// Thread principal inicializada apos injecao
void* MainThread(void* arg) {
    LOGI("Library injected. Initializing GhostSystems Scanner.");
    
    // Inicializa o leitor de touch independente do Unity
    g_TouchReader = new TouchReader();
    g_TouchReader->start();

    // Inicializa Scanner de memoria (será ativado quando o painel for aberto)
    g_Scanner = new GhostSystems::MemoryScanner(g_State, g_FeatureConfig);

    // Obtem endereco do EGL e faz o hook em runtime
    void* libEgl = dlopen("libEGL.so", RTLD_NOW);
    if (libEgl) {
        void* swapBuffers = dlsym(libEgl, "eglSwapBuffers");
        if (swapBuffers) {
            A64HookFunction(swapBuffers, (void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
            LOGI("Hook eglSwapBuffers applied successfully. Setup complete.");
        } else {
            LOGE("Failed to find eglSwapBuffers in libEGL.so");
        }
    } else {
        LOGE("Failed to load libEGL.so");
    }

    return nullptr;
}

// Ponto de entrada __attribute__((constructor)) executado no momento da injecao (dlopen)
__attribute__((constructor))
void InitLibrary() {
    pthread_t thread;
    pthread_create(&thread, nullptr, MainThread, nullptr);
}
