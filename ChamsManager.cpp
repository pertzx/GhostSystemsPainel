#include <chrono>
#include <vector>
#include <unordered_map>
#include <android/log.h>
#include "Menu.h"
#include "Entity.h"
#include "Il2CppHelper.h"

#define LOG_TAG "ChamsManager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace GhostSystems {

static uint32_t g_lastUpdateMs = 0;
static constexpr uint32_t UPDATE_INTERVAL_MS = 100;
static std::unordered_map<void*, void*> g_originalMaterials;
static std::unordered_map<void*, void*> g_chamsMaterials;

void ChamsManager::Init() {
    LOGI("ChamsManager initialized");
    g_originalMaterials.clear();
    g_chamsMaterials.clear();
}

void ChamsManager::Shutdown() {
    LOGI("Shutdown - restoring %zu materials", g_originalMaterials.size());
    static void* setMatMethod = nullptr;
    if (!setMatMethod) {
        void* rendererKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Renderer");
        if (!rendererKlass) rendererKlass = Il2Cpp::GetClass("UnityEngine.CoreModule", "UnityEngine", "Renderer");
        if (rendererKlass) {
            setMatMethod = Il2Cpp::GetMethodRecursively(rendererKlass, "set_material", 1);
        }
    }
    for (auto& pair : g_originalMaterials) {
        void* renderer = pair.first;
        void* origMat = pair.second;
        if (renderer && origMat && setMatMethod) {
            void* exc = nullptr;
            void* args[1] = { origMat };
            Il2Cpp::runtime_invoke(setMatMethod, renderer, args, &exc);
        }
    }
    g_originalMaterials.clear();
    g_chamsMaterials.clear();
    LOGI("Shutdown complete");
}

void ChamsManager::Update(bool chamsEnabled, bool /*chamsDrawThroughWalls*/, 
                          const float* enemyHiddenColor, const float* enemyVisibleColor, 
                          const float* allyColor, GameState& sharedState) {
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    uint32_t nowMs32 = (uint32_t)(nowMs & 0xFFFFFFFF);
    if (nowMs32 - g_lastUpdateMs < UPDATE_INTERVAL_MS) return;
    g_lastUpdateMs = nowMs32;
    
    if (!chamsEnabled) {
        Shutdown();
        return;
    }
    
    if (!Il2Cpp::domain_get) return;
    
    std::vector<PlayerEntity> localEntities;
    {
        std::lock_guard<std::mutex> lock(sharedState.mtx);
        localEntities = sharedState.entities;
    }
    
    if (localEntities.empty()) return;
    
    // Cache de classes
    static void* materialKlass = nullptr;
    static void* shaderKlass = nullptr;
    static void* rendererKlass = nullptr;
    static void* componentKlass = nullptr;
    static void* chamsShader = nullptr;
    static bool cached = false;
    
    if (!cached) {
        materialKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Material");
        if (!materialKlass) materialKlass = Il2Cpp::GetClass("UnityEngine.CoreModule", "UnityEngine", "Material");
        
        shaderKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Shader");
        if (!shaderKlass) shaderKlass = Il2Cpp::GetClass("UnityEngine.CoreModule", "UnityEngine", "Shader");
        
        rendererKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "SkinnedMeshRenderer");
        if (!rendererKlass) rendererKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "MeshRenderer");
        if (!rendererKlass) rendererKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Renderer");
        
        componentKlass = Il2Cpp::GetClass("UnityEngine.CoreModule.dll", "UnityEngine", "Component");
        if (!componentKlass) componentKlass = Il2Cpp::GetClass("UnityEngine.CoreModule", "UnityEngine", "Component");
        
        if (shaderKlass) {
            void* findMethod = Il2Cpp::GetMethodRecursively(shaderKlass, "Find", 1);
            if (findMethod) {
                const char* shaderNames[] = {"Universal Render Pipeline/Unlit", "Unlit/Color", "Hidden/Internal-Colored", "Sprites/Default"};
                for (const char* name : shaderNames) {
                    void* strObj = Il2Cpp::string_new(name);
                    if (!strObj) continue;
                    void* args[1] = { strObj };
                    void* exc = nullptr;
                    chamsShader = Il2Cpp::runtime_invoke(findMethod, nullptr, args, &exc);
                    if (chamsShader && !exc) {
                        LOGI("Shader found: %s", name);
                        break;
                    }
                }
            }
        }
        cached = true;
    }
    
    if (!chamsShader) {
        LOGE("No shader found!");
        return;
    }
    
    // Cache de metodos
    static void* setShaderMethod = nullptr;
    static void* setColorMethod = nullptr;
    static void* setIntMethod = nullptr;
    static void* setFloatMethod = nullptr;
    static void* cloneMethod = nullptr;
    static void* setRenderQueueMethod = nullptr;
    static void* getMaterialMethod = nullptr;
    static void* setMaterialMethod = nullptr;
    static void* getComponentMethod = nullptr;
    static void* rendererTypeObj = nullptr;
    static bool methodsCached = false;
    
    if (!methodsCached && materialKlass && rendererKlass && componentKlass) {
        setShaderMethod = Il2Cpp::GetMethodRecursively(materialKlass, "set_shader", 1);
        setColorMethod = Il2Cpp::GetMethodRecursively(materialKlass, "SetColor", 2);
        setIntMethod = Il2Cpp::GetMethodRecursively(materialKlass, "SetInt", 2);
        setFloatMethod = Il2Cpp::GetMethodRecursively(materialKlass, "SetFloat", 2);
        cloneMethod = Il2Cpp::GetMethodRecursively(materialKlass, "Clone", 0);
        setRenderQueueMethod = Il2Cpp::GetMethodRecursively(materialKlass, "set_renderQueue", 1);
        getMaterialMethod = Il2Cpp::GetMethodRecursively(rendererKlass, "get_material", 0);
        setMaterialMethod = Il2Cpp::GetMethodRecursively(rendererKlass, "set_material", 1);
        getComponentMethod = Il2Cpp::GetMethodRecursively(componentKlass, "GetComponentInChildren", 1);
        if (!getComponentMethod) getComponentMethod = Il2Cpp::GetMethodRecursively(componentKlass, "GetComponentInChildren", 2);
        if (rendererKlass) rendererTypeObj = Il2Cpp::type_get_object(Il2Cpp::class_get_type(rendererKlass));
        methodsCached = true;
        LOGI("Methods cached: getMat=%p setMat=%p clone=%p getComp=%p", getMaterialMethod, setMaterialMethod, cloneMethod, getComponentMethod);
    }
    
    if (!setMaterialMethod || !getMaterialMethod || !cloneMethod) {
        LOGE("Missing methods!");
        return;
    }
    
    int entCount = 0, renCount = 0, matCount = 0;
    
    for (auto& entity : localEntities) {
        if (!entity.obj || !entity.isAlive()) continue;
        entCount++;
        
        const float* targetColor = enemyHiddenColor;
        if (entity.alignment == Alignment::ALLY) targetColor = allyColor;
        else if (entity.alignment == Alignment::ENEMY && entity.isVisible) targetColor = enemyVisibleColor;
        
        void* exc = nullptr;
        void* renderer = nullptr;
        if (getComponentMethod && rendererTypeObj) {
            bool includeInactive = true;
            void* args[2] = { rendererTypeObj, &includeInactive };
            uint32_t pCount = Il2Cpp::method_get_param_count(getComponentMethod);
            renderer = Il2Cpp::runtime_invoke(getComponentMethod, entity.obj, pCount == 2 ? args : &args[0], &exc);
        }
        if (!renderer) continue;
        renCount++;
        
        // Salva material original
        if (g_originalMaterials.find(renderer) == g_originalMaterials.end()) {
            void* origMat = Il2Cpp::runtime_invoke(getMaterialMethod, renderer, nullptr, &exc);
            if (origMat && !exc) {
                g_originalMaterials[renderer] = origMat;
                LOGI("Original material saved for renderer %p", renderer);
            } else {
                LOGW("getMaterial failed for renderer %p", renderer);
                continue;
            }
        }
        
        // Clona material
        void* chamsMat = nullptr;
        if (g_chamsMaterials.find(renderer) != g_chamsMaterials.end()) {
            chamsMat = g_chamsMaterials[renderer];
        } else {
            void* origMat = g_originalMaterials[renderer];
            if (origMat && cloneMethod) {
                void* exc2 = nullptr;
                chamsMat = Il2Cpp::runtime_invoke(cloneMethod, origMat, nullptr, &exc2);
                if (chamsMat && !exc2) {
                    g_chamsMaterials[renderer] = chamsMat;
                    LOGI("Material cloned: %p", chamsMat);
                } else {
                    LOGW("Clone failed");
                    continue;
                }
            }
        }
        if (!chamsMat) continue;
        
        // Aplica shader
        if (setShaderMethod && chamsShader) {
            void* args[1] = { chamsShader };
            Il2Cpp::runtime_invoke(setShaderMethod, chamsMat, args, &exc);
        }
        
        // Aplica cor
        if (setColorMethod) {
            struct ColorVal { float r, g, b, a; } col = {targetColor[0], targetColor[1], targetColor[2], 1.0f};
            void* colorStr = Il2Cpp::string_new("_Color");
            void* colorArgs[2] = { colorStr, &col };
            Il2Cpp::runtime_invoke(setColorMethod, chamsMat, colorArgs, &exc);
            void* baseColorStr = Il2Cpp::string_new("_BaseColor");
            void* baseColorArgs[2] = { baseColorStr, &col };
            Il2Cpp::runtime_invoke(setColorMethod, chamsMat, baseColorArgs, &exc);
        }
        
        // ZTest Always
        if (setIntMethod) {
            void* zTestStr = Il2Cpp::string_new("_ZTest");
            int always = 8;
            void* args[2] = { zTestStr, &always };
            Il2Cpp::runtime_invoke(setIntMethod, chamsMat, args, &exc);
            
            void* zWriteStr = Il2Cpp::string_new("_ZWrite");
            int off = 0;
            void* args2[2] = { zWriteStr, &off };
            Il2Cpp::runtime_invoke(setIntMethod, chamsMat, args2, &exc);
        }
        
        // Surface transparent
        if (setFloatMethod) {
            void* surfaceStr = Il2Cpp::string_new("_Surface");
            float t = 1.0f;
            void* args[2] = { surfaceStr, &t };
            Il2Cpp::runtime_invoke(setFloatMethod, chamsMat, args, &exc);
            void* modeStr = Il2Cpp::string_new("_Mode");
            float m = 3.0f;
            void* args2[2] = { modeStr, &m };
            Il2Cpp::runtime_invoke(setFloatMethod, chamsMat, args2, &exc);
        }
        
        // RenderQueue
        if (setRenderQueueMethod) {
            int queue = 4000;
            void* args[1] = { &queue };
            Il2Cpp::runtime_invoke(setRenderQueueMethod, chamsMat, args, &exc);
        }
        
        // Aplica material
        void* setMatArgs[1] = { chamsMat };
        Il2Cpp::runtime_invoke(setMaterialMethod, renderer, setMatArgs, &exc);
        if (!exc) matCount++;
    }
    
    LOGI("Update: entities=%d renderers=%d materials=%d", entCount, renCount, matCount);
}

} // namespace GhostSystems