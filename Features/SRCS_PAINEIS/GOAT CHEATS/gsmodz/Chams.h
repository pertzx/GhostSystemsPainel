#ifndef HENZRY_CHAMS
#define HENZRY_CHAMS
#include <GLES2/gl2.h>
#include <dlfcn.h>
#include <And64InlineHook.hpp>
#include <chrono> 
#include <math.h>
#include <imgui.h>
#include <imports.h>
static void *handle;
bool shading = false;
bool wireframe = false;
bool enableRGB = false;
static const char* shaderName;
static float colorTime = 0.0f;
ImVec4 coreschams = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

void updateBlendColor() {
auto now = std::chrono::steady_clock::now().time_since_epoch();
colorTime = std::chrono::duration_cast<std::chrono::seconds>(now).count() * 0.5f;
float r = (sinf(colorTime) * 0.5f + 0.5f);
float g = (sinf(colorTime + 2.0f * M_PI / 3.0f) * 0.5f + 0.5f);
float b = (sinf(colorTime + 4.0f * M_PI / 3.0f) * 0.5f + 0.5f);
glBlendColor(r, g, b, 1.0f);
}

void applyColor() {
if (enableRGB) {
updateBlendColor();
} else {
glBlendColor(coreschams.x, coreschams.y, coreschams.z, coreschams.w);
}
}

void setShader(const char* s) {
shaderName = s;
}
const char* getShader() {
return shaderName;
}
bool getShadingEnabled() {
return shading;
}
bool getWireframeEnabled() {
return wireframe;
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
old_glDrawElements(mode, count, type, indices);
if (mode != GL_TRIANGLES || count < 1000) return;
GLint currProgram;
glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
GLint id = old_glGetUniformLocation(currProgram, getShader());
if (id == -1) return;

applyColor();

if (getWireframeEnabled()) {
glDepthRangef(1, 0.5);
glEnable(GL_BLEND);
glBlendFuncSeparate(GL_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE, GL_ZERO);
glLineWidth(2.0f);
old_glDrawElements(GL_LINE_LOOP, count, type, indices);
}
if (getShadingEnabled()) {
glDepthRangef(1, 0.5);
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glBlendEquation(GL_FUNC_ADD);
glDepthFunc(GL_ALWAYS);
old_glDrawElements(GL_TRIANGLES, count, type, indices);
glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
glDepthFunc(GL_LESS);
glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
}
glDepthFunc(GL_ALWAYS);
old_glDrawElements(mode, count, type, indices);
glDepthFunc(GL_LESS);
glDepthRangef(0.5, 1);
glColorMask(1, 1, 1, 1);
glDisable(GL_BLEND);
}

bool mlovinit() {
handle = NULL;
handle = dlopen(OBFUSCATE("libGLESv2.so"), RTLD_LAZY);
return handle != NULL;
}

void LogShaders() {
auto addr_glGetUniformLocation = (uintptr_t)dlsym(handle, OBFUSCATE("glGetUniformLocation"));
const char *dlsym_error = dlerror();
if (!dlsym_error) {
A64HookFunction((void *)addr_glGetUniformLocation, (void *)new_glGetUniformLocation, (void **)&old_glGetUniformLocation);
}
}

void Wallhack() {
auto addr_glDrawElements = (uintptr_t)dlsym(handle, OBFUSCATE("glDrawElements"));
const char *dlsym_error = dlerror();
if (!dlsym_error) {
A64HookFunction((void *)addr_glDrawElements, (void *)new_glDrawElements, (void **)&old_glDrawElements);
}
}

#endif


