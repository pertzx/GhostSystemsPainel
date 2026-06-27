#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <android/log.h>
#include <dlfcn.h>
#include "Menu.h"
#include "And64InlineHook.hpp"
#include "Entity.h"

#define LOG_TAG "GhostSystems"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Variaveis globais para os hooks
static void (*glDrawElements_Original)(GLenum mode, GLsizei count, GLenum type, const void* indices) = nullptr;
static void (*glDrawArrays_Original)(GLenum mode, GLint first, GLsizei count) = nullptr;
static void (*glUseProgram_Original)(GLuint program) = nullptr;
static GLint (*glGetUniformLocation_Original)(GLuint program, const GLchar* name) = nullptr;
static void (*glUniform4f_Original)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) = nullptr;
static void (*glUniform3f_Original)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) = nullptr;
static void (*glEnable_Original)(GLenum cap) = nullptr;
static void (*glDisable_Original)(GLenum cap) = nullptr;

static GLuint currentProgram = 0;
static bool chamsActive = false;

// Ponte para acessar o Menu
namespace GhostSystems { extern Menu* g_Menu; }
using GhostSystems::g_Menu;

// Verifica se e shader de personagem/inimigo
static bool isCharacterShader(GLuint program) {
    // Filtro baseado no programa atual
    // Em Free Fire, shaders de personagens geralmente tem uniforms como:
    // _MainTex, _Color, u_diffuseColor, etc.
    // Aqui usamos uma abordagem simples: todos os shaders sao afetados
    return true;
}

// Hook de glUseProgram - captura shader ativo
static void glUseProgram_Hook(GLuint program) {
    currentProgram = program;
    
    if (g_Menu && g_Menu->chamsEnabled && g_Menu->chamsUseGlHook) {
        chamsActive = isCharacterShader(program);
    }
    
    glUseProgram_Original(program);
}

// Hook de glGetUniformLocation - captura localizacoes de uniforms
static GLint glGetUniformLocation_Hook(GLuint program, const GLchar* name) {
    GLint location = glGetUniformLocation_Original(program, name);
    return location;
}

// Hook de glUniform4f - intercepta cor do shader
static void glUniform4f_Hook(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    if (g_Menu && g_Menu->chamsEnabled && g_Menu->chamsUseGlHook && chamsActive) {
        // Substitui cor pela cor do chams
        GLfloat chamsColor[4];
        if (g_Menu->chamsColorMode == 0) {
            // Usa cor de inimigo oculto (atraves da parede)
            for (int i = 0; i < 4; i++) chamsColor[i] = g_Menu->chamsEnemyHiddenColor[i];
        } else {
            for (int i = 0; i < 4; i++) chamsColor[i] = g_Menu->chamsEnemyHiddenColor[i];
        }
        
        glUniform4f_Original(location, chamsColor[0], chamsColor[1], chamsColor[2], chamsColor[3]);
        return;
    }
    
    glUniform4f_Original(location, v0, v1, v2, v3);
}

// Hook de glDrawElements - desativa depth test para chams
static void glDrawElements_Hook(GLenum mode, GLsizei count, GLenum type, const void* indices) {
    if (g_Menu && g_Menu->chamsEnabled && g_Menu->chamsUseGlHook && chamsActive) {
        // Salva estado original
        GLboolean depthMaskEnabled;
        GLint depthFunc;
        GLboolean depthTestEnabled;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMaskEnabled);
        glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
        glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
        
        // Desativa depth testing e writing para desenhar atraves das paredes
        if (g_Menu->chamsDrawThroughWalls) {
            glDepthMask(GL_FALSE);
            glDepthFunc(GL_ALWAYS);
            // Nao desativa GL_DEPTH_TEST completamente para nao quebrar o render
        }
        
        // Chama original
        glDrawElements_Original(mode, count, type, indices);
        
        // Restaura estado
        glDepthMask(depthMaskEnabled);
        glDepthFunc(depthFunc);
    } else {
        glDrawElements_Original(mode, count, type, indices);
    }
}

// Hook de glDrawArrays
static void glDrawArrays_Hook(GLenum mode, GLint first, GLsizei count) {
    if (g_Menu && g_Menu->chamsEnabled && g_Menu->chamsUseGlHook && chamsActive) {
        GLboolean depthMaskEnabled;
        GLint depthFunc;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMaskEnabled);
        glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
        
        if (g_Menu->chamsDrawThroughWalls) {
            glDepthMask(GL_FALSE);
            glDepthFunc(GL_ALWAYS);
        }
        
        glDrawArrays_Original(mode, first, count);
        
        glDepthMask(depthMaskEnabled);
        glDepthFunc(depthFunc);
    } else {
        glDrawArrays_Original(mode, first, count);
    }
}

void GhostSystems::Menu::initChamsGlHook() {
    if (chamsGlHookInitialized) return;
    
    // Obtem endereco das funcoes OpenGL
    void* handle = dlopen("libGLESv3.so", RTLD_NOW);
    if (!handle) {
        handle = dlopen("libGLESv2.so", RTLD_NOW);
    }
    
    if (!handle) {
        LOGI("[ChamsGL] Falha ao carregar biblioteca OpenGL");
        return;
    }
    
    void* glDrawElementsAddr = dlsym(handle, "glDrawElements");
    void* glDrawArraysAddr = dlsym(handle, "glDrawArrays");
    void* glUseProgramAddr = dlsym(handle, "glUseProgram");
    void* glGetUniformLocationAddr = dlsym(handle, "glGetUniformLocation");
    void* glUniform4fAddr = dlsym(handle, "glUniform4f");
    
    if (!glDrawElementsAddr || !glDrawArraysAddr || !glUseProgramAddr || !glUniform4fAddr) {
        LOGI("[ChamsGL] Falha ao encontrar funcoes OpenGL");
        LOGI("[ChamsGL] glDrawElements: %p, glDrawArrays: %p", glDrawElementsAddr, glDrawArraysAddr);
        LOGI("[ChamsGL] glUseProgram: %p, glUniform4f: %p", glUseProgramAddr, glUniform4fAddr);
        dlclose(handle);
        return;
    }
    
    LOGI("[ChamsGL] Instalando hooks...");
    LOGI("[ChamsGL] glDrawElements: %p", glDrawElementsAddr);
    LOGI("[ChamsGL] glDrawArrays: %p", glDrawArraysAddr);
    LOGI("[ChamsGL] glUseProgram: %p", glUseProgramAddr);
    LOGI("[ChamsGL] glUniform4f: %p", glUniform4fAddr);
    
    // Aplica hooks usando And64InlineHook
    A64HookFunction(glDrawElementsAddr, (void*)glDrawElements_Hook, (void**)&glDrawElements_Original);
    A64HookFunction(glDrawArraysAddr, (void*)glDrawArrays_Hook, (void**)&glDrawArrays_Original);
    A64HookFunction(glUseProgramAddr, (void*)glUseProgram_Hook, (void**)&glUseProgram_Original);
    A64HookFunction(glGetUniformLocationAddr, (void*)glGetUniformLocation_Hook, (void**)&glGetUniformLocation_Original);
    A64HookFunction(glUniform4fAddr, (void*)glUniform4f_Hook, (void**)&glUniform4f_Original);
    
    if (glDrawElements_Original && glDrawArrays_Original && glUseProgram_Original && glUniform4f_Original) {
        LOGI("[ChamsGL] Hooks instalados com sucesso!");
        LOGI("[ChamsGL] Original glDrawElements: %p", glDrawElements_Original);
        LOGI("[ChamsGL] Original glUseProgram: %p", glUseProgram_Original);
        LOGI("[ChamsGL] Original glUniform4f: %p", glUniform4f_Original);
    } else {
        LOGI("[ChamsGL] ERRO: Alguns hooks falharam!");
        LOGI("[ChamsGL] glDrawElements_Original: %p", glDrawElements_Original);
        LOGI("[ChamsGL] glDrawArrays_Original: %p", glDrawArrays_Original);
        LOGI("[ChamsGL] glUseProgram_Original: %p", glUseProgram_Original);
        LOGI("[ChamsGL] glUniform4f_Original: %p", glUniform4f_Original);
    }
    
    chamsGlHookInitialized = true;
    dlclose(handle);
}

void GhostSystems::Menu::removeChamsGlHook() {
    if (!chamsGlHookInitialized) return;
    
    LOGI("[ChamsGL] Removendo hooks (nao suportado por And64InlineHook - hooks permanecem ativos)");
    
    chamsGlHookInitialized = false;
    chamsActive = false;
}