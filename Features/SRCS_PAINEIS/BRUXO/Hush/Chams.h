#ifndef HENZRY_CHAMS
#define HENZRY_CHAMS

#include <GLES2/gl2.h>
#include <dlfcn.h>

#include <chrono>
#include <sys/time.h>
#include "Tools.hpp"

#define RAINBOW_DELAY 16  // ~60 FPS

// Função para obter tempo atual em milissegundos
long currentTimeMillis() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return (time.tv_sec * 1000) + (time.tv_usec / 1000);
}

// Substitua esta parte problemática:
static long lastRainbowTime = 0;  // Variável para controle de tempo

static void *handle;

bool chams = false;
bool shading = false;
bool wireframe = false;
bool glow = false;
bool outline = false;

bool rainbow = false;

static const char* shaderName;
static bool enableRainbow1;

// Variáveis para os sliders
static float rgb1 = 255.0f;
static float rgb2 = 0.0f;
static float rgb3 = 0.0f;

// Variáveis usadas no shader - inicializadas com os valores dos sliders
static float r = 255.0f;
static float g = 0.0f;
static float b = 0.0f;

static int w = 0;
static int a = 255;

float red = 255.0f;
float green = 0.0f;
float blue = 0.0f;
float mi = 0.0f;

void setShader(const char* s) {
    shaderName = s;
}
const char* getShader() {
    return shaderName;
}

// Função para atualizar as cores quando os sliders mudam
void updateColors() {
    r = rgb1;
    g = rgb2;
    b = rgb3;
}

bool getWallhackEnabled(){
    return Active && chams;
}

bool getShadingEnabled(){
    return Active && shading;
}

bool getWireframeEnabled(){
    return Active && wireframe;
}

bool getGlowEnabled(){
    return glow;
}

bool getOutlineEnabled(){
    return outline;
}

bool getRainbowEnabled(){
    return Active && rainbow;
}

bool getRainbow1Enabled(){
    return enableRainbow1;
}

int (*old_glGetUniformLocation)(GLuint, const GLchar *);
GLint new_glGetUniformLocation(GLuint program, const GLchar *name) {
    return old_glGetUniformLocation(program, name);
}

bool isCurrentShader(const char *shader) {
    GLint currProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
    return old_glGetUniformLocation(currProgram, shader) != -1;
}

void (*old_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void *indices);
void new_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices) {
    // Atualiza as cores antes de desenhar
    updateColors();
    
    old_glDrawElements(mode, count, type, indices);
    if (mode != GL_TRIANGLES || count < 1000) return;
    {
        GLint currProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);

        GLint id = old_glGetUniformLocation(currProgram, getShader());
        if (id == -1) return;

        if (Active && getWireframeEnabled()) {
            if (wireframe) {
                glDepthRangef(1, 0.5);
            }
            else {
                glDepthRangef(0.5, 1);
            }
            glBlendColor(GLfloat(r/255), GLfloat(g/255), GLfloat(b/255), 1);
            glColorMask(1, 1, 1, 1);
            glEnable(GL_BLEND);
            glBlendFuncSeparate(GL_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE, GL_ZERO);
            glLineWidth(w);
            old_glDrawElements(GL_LINE_LOOP, count, type, indices);
        }

        if (Active && getWallhackEnabled()) {
            glBlendColor(GLfloat(r/255), GLfloat(g/255), GLfloat(b/255), GLfloat(a/255));
            glColorMask(r, g, b, 255);
            glEnable(GL_BLEND);
            glBlendFunc(GL_CONSTANT_ALPHA, GL_CONSTANT_COLOR);
        }
        
        if (Active && getShadingEnabled()) {
           glDepthRangef(1, 0.5);
           glEnable(GL_BLEND);
           glBlendFunc(GL_SRC_COLOR, GL_CONSTANT_COLOR);
           glBlendEquation(GL_FUNC_ADD);
           glBlendColor(GLfloat(r/255), GLfloat(g/255), GLfloat(b/255), 1);
           glDepthFunc(GL_ALWAYS);
           old_glDrawElements(GL_TRIANGLES, count, type, indices);
           glColorMask(r, g, b, 255);
           glBlendFunc(GL_DST_COLOR, GL_ONE);
           glDepthFunc(GL_LESS);
           glBlendColor(0.0, 0.0, 0.0, 0.0);
        }
        
        if (getGlowEnabled()) {
            glEnable(GL_BLEND);
            glBlendColor(GLfloat(r/255), GLfloat(g/255), GLfloat(b/255), 1);
            glColorMask(1, 1, 1, 1);
            glEnable(GL_BLEND);
            glBlendFuncSeparate(GL_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE, GL_ZERO);
            glLineWidth(w);
   
            glDepthRangef(0.5, 1);
            old_glDrawElements(GL_LINES, count, type, indices);
            glBlendColor(1, 1, 1, 1);
            glDepthRangef(1, 0.5);
            old_glDrawElements(GL_TRIANGLES, count, type, indices);
        }
		
        if (getOutlineEnabled()) {
            glDepthRangef(1, 0.5);
            glLineWidth(w);
            glEnable(GL_BLEND);
            glColorMask(1, 1, 1, 1);
            glBlendFuncSeparate(GL_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE, GL_ZERO);
            glBlendColor(0, 0, 0, 1);
            old_glDrawElements(GL_TRIANGLES, count, type, indices);
            glBlendColor(GLfloat(r/255), GLfloat(g/255), GLfloat(b/255), 1);
            old_glDrawElements(GL_LINES, count, type, indices);
        }
		
// Substitua todo o bloco do rainbow por:
if (Active && getRainbowEnabled()) {
    if (Active && getRainbow1Enabled()) {  // Note os parênteses adicionais
        long currentTime = currentTimeMillis();
        if (currentTime - lastRainbowTime < RAINBOW_DELAY) {
            return;
        }
        lastRainbowTime = currentTime;
        
        // Lógica do rainbow corrigida
        if (red == 255.0f && blue == 0.0f && green < 255.0f) {
            green++;
        } 
        else if (green == 255.0f && red > 0.0f) {
            red--;
        } 
        else if (green == 255.0f && red == 0.0f && blue < 255.0f) {
            blue++;
        } 
        else if (blue == 255.0f) {
            if (green == 0.0f) {
                mi = 1.0f;
                if (red < 255.0f) red++;
            } else {
                green--;
            }
        } 
        else if (mi == 1.0f && red == 255.0f && blue > 0.0f) {
            blue--;
        }
        
        r = red;
        g = green;
        b = blue;
    }
            glBlendColor(GLfloat(r/255), GLfloat(g/255), GLfloat(b/255), GLfloat(a/255));
            glColorMask(1, 1, 1, 1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE_MINUS_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_ALPHA);
        }
        old_glDrawElements(mode, count, type, indices);

        glDepthRangef(0.5, 1);
        glColorMask(1, 1, 1, 1);
        glDisable(GL_BLEND);
    }
}

bool mlovinit(){
    handle = NULL;
    handle = dlopen("libGLESv2.so", RTLD_LAZY);
    if (!handle) {
        return false;
    }
    return true;
}

void LogShaders(){
    auto p_glGetUniformLocation = (const void*(*)(...))dlsym(handle, "glGetUniformLocation");
    const char *dlsym_error = dlerror();
    if (dlsym_error) {
        return;
    } else {
        #if defined(__aarch64__)
            A64HookFunction(reinterpret_cast<void*>(p_glGetUniformLocation), reinterpret_cast<void*>(new_glGetUniformLocation), reinterpret_cast<void**>(&old_glGetUniformLocation));
        #else
            MSHookFunction(reinterpret_cast<void*>(p_glGetUniformLocation), reinterpret_cast<void*>(new_glGetUniformLocation), reinterpret_cast<void**>(&old_glGetUniformLocation));
        #endif
    }
}

void Wallhack() {
    auto p_glDrawElements = (const void*(*)(...))dlsym(handle, "glDrawElements");
    const char *dlsym_error = dlerror();
    if (dlsym_error) {
        return;
    } else {
        #if defined(__aarch64__)
            A64HookFunction(reinterpret_cast<void*>(p_glDrawElements), reinterpret_cast<void*>(new_glDrawElements), reinterpret_cast<void**>(&old_glDrawElements));
        #else
            MSHookFunction(reinterpret_cast<void*>(p_glDrawElements), reinterpret_cast<void*>(new_glDrawElements), reinterpret_cast<void**>(&old_glDrawElements));
        #endif
    }
}

#endif
