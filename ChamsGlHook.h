#pragma once
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <android/log.h>
#include <dlfcn.h>

#define LOG_TAG "GhostSystems"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace GhostSystems {
class Menu;
void initChamsGlHook();
void removeChamsGlHook();
}
