#include <jni.h>
#include <pthread.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <dlfcn.h>
#include <imgui.h>
#include "imgui_impl_opengl3.h"
#include "Menu.h"
#include "MemoryScanner.h"
#include "TouchReader.h"
#include "And64InlineHook.hpp"
#include "Il2CppHelper.h"
#include <atomic>

// Definicoes basicas para facilitar debug
#define LOG_TAG "GhostSystems"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace GhostSystems;

// Globais
GameState g_State;
Menu* g_Menu = nullptr;
MemoryScanner* g_Scanner = nullptr;
TouchReader* g_TouchReader = nullptr;

bool g_ImGuiInitialized = false;

// Hooks de EGL
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay, EGLSurface);
eglSwapBuffers_t orig_eglSwapBuffers = nullptr;

void* m_get_height = nullptr;
void* m_get_width = nullptr;
bool m_ScreenMethodsCached = false;

// Unity Input cache
void* m_get_touchCount = nullptr;
void* m_get_touch = nullptr;
void* m_get_mousePosition = nullptr;
void* m_get_mouseButton = nullptr;
bool m_InputMethodsCached = false;

void CacheScreenMethods() {
    if (m_ScreenMethodsCached) return;
    
    void* screenClass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Screen");
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
    void* inputClass = Il2Cpp::GetClass("UnityEngine.InputLegacyModule.dll", "UnityEngine", "Input");
    if (!inputClass) {
        inputClass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Input");
    }

    if (inputClass) {
        m_get_touchCount = Il2Cpp::class_get_method_from_name(inputClass, "get_touchCount", 0);
        m_get_touch = Il2Cpp::class_get_method_from_name(inputClass, "GetTouch", 1);
        m_get_mousePosition = Il2Cpp::class_get_method_from_name(inputClass, "get_mousePosition", 0);
        m_get_mouseButton = Il2Cpp::class_get_method_from_name(inputClass, "GetMouseButton", 1);
    }

    if (inputClass) {
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

        g_Menu = new Menu(g_State);

        g_ImGuiInitialized = true;
        LOGI("ImGui Context Initialized");
    }

    if (Il2Cpp::thread_attach && Il2Cpp::domain_get) {
        Il2Cpp::thread_attach(Il2Cpp::domain_get());
    }

    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = 1.0f / 60.0f;

    // Sincroniza com Il2Cpp para atualizar a resolucao da tela
    if (Il2Cpp::domain_get && Il2Cpp::thread_attach) {
        CacheScreenMethods();
        CacheInputMethods();
        
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

        // Tenta pegar o toque nativo pelo Unity Input (Funciona para Android perfeitamente)
        if (m_InputMethodsCached) {
            bool touching = false;
            float touchX = -1.0f;
            float touchY = -1.0f;

            // Em Unity, GetMouseButton(0) simula o primeiro toque do dedo (ID = 0)
            if (m_get_mouseButton && m_get_mousePosition) {
                int btnIndex = 0;
                void* args_btn[1] = { &btnIndex };
                
                void* isTouchingObj = Il2Cpp::runtime_invoke(m_get_mouseButton, nullptr, args_btn, nullptr);
                
                if (isTouchingObj) {
                    touching = *(bool*)((uintptr_t)isTouchingObj + 0x10);
                }

                if (touching) {
                    void* mousePosObj = Il2Cpp::runtime_invoke(m_get_mousePosition, nullptr, nullptr, nullptr);
                    if (mousePosObj) {
                        // Vector3 tem os floats x,y,z a partir do offset 0x10 do objeto
                        touchX = *(float*)((uintptr_t)mousePosObj + 0x10);
                        touchY = *(float*)((uintptr_t)mousePosObj + 0x14);

                        // Unity Inverte o eixo Y na tela (Y=0 é na base inferior). ImGui usa Y=0 no topo
                        touchY = io.DisplaySize.y - touchY;
                    }
                }
            }

            if (touching) {
                io.MousePos = ImVec2(touchX, touchY);
                io.MouseDown[0] = true;
            } else {
                io.MouseDown[0] = false;
            }
        }
    }

    if (g_TouchReader) {
        // g_TouchReader->updateImGui(io); // Desativado para usar a API da Unity
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

    // Inicializa Scanner de memoria numa thread separada (Thread-safety garantida pelo mtx)
    g_Scanner = new MemoryScanner(g_State);
    g_Scanner->start();

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
