LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libdobby
LOCAL_SRC_FILES := Nigga/Tools/Dobby/libraries/$(TARGET_ARCH_ABI)/libdobby.a
LOCAL_EXPORT_C_INCLUDE := $(LOCAL_PATH)/Nigga/Tools/Dobby/
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE := libcurl
LOCAL_SRC_FILES := curl/curl-android-$(TARGET_ARCH_ABI)/lib/libcurl.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE := libssl
LOCAL_SRC_FILES := curl/openssl-android-$(TARGET_ARCH_ABI)/lib/libssl.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)

# --------------------- Prebuilt: Munodz (kFox) --------------
# Thư viện chứa implement cho kFox::SetSearchRange/MemorySearch/...
include $(CLEAR_VARS)
LOCAL_MODULE := libmunodz
LOCAL_SRC_FILES := Munodz/$(TARGET_ARCH_ABI)/libMunodz.a
# Xuất đường dẫn header để module khác include được Munodz.hpp
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/Munodz
include $(PREBUILT_STATIC_LIBRARY)


LOCAL_MODULE := libcrypto
LOCAL_SRC_FILES := curl/openssl-android-$(TARGET_ARCH_ABI)/lib/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY)


MAIN_LOCAL_PATH := $(call my-dir)
LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)

include $(CLEAR_VARS)

LOCAL_MODULE    := anogs

# Code optimization
LOCAL_CFLAGS := -Wno-error=format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w
LOCAL_CFLAGS += -fno-rtti -fno-exceptions -fpermissive
LOCAL_CPPFLAGS := -Wno-error=format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w -Werror -s -std=c++17
LOCAL_CPPFLAGS += -Wno-error=c++11-narrowing -fms-extensions -fno-rtti -fno-exceptions -fpermissive
LOCAL_LDFLAGS += -Wl,--gc-sections,--strip-all, -llog
LOCAL_ARM_MODE := arm
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv3 -lGLESv2 -lGLESv1_CM -lz

LOCAL_C_INCLUDE := $(LOCAL_PATH)
LOCAL_C_INCLUDE := $(LOCAL_PATH)/xdl
LOCAL_C_INCLUDE := $(LOCAL_PATH)/ImGui
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Munodz
LOCAL_C_INCLUDE := $(LOCAL_PATH)/ImGui/backends
LOCAL_C_INCLUDE := $(LOCAL_PATH)/Nigga
LOCAL_C_INCLUDES := $(LOCAL_PATH)/curl/curl-android-$(TARGET_ARCH_ABI)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/curl/openssl-android-$(TARGET_ARCH_ABI)/include

LOCAL_SRC_FILES := Main.cpp \
xdl/xdl.c \
xdl/xdl_iterate.c \
xdl/xdl_linker.c \
xdl/xdl_lzma.c \
xdl/xdl_util.c \
ImGui/imgui.cpp \
ImGui/imgui_draw.cpp \
ImGui/imgui_tables.cpp \
ImGui/stb_image.cpp \
ImGui/imgui_widgets.cpp \
ImGui/imgui_stdlib.cpp \
ImGui/ImguiPP.cpp \
ImGui/backends/imgui_impl_opengl3.cpp \
ImGui/backends/imgui_impl_android.cpp \
Includes/SharedPrefs/SharedPrefs.cpp \
Includes/base64/base64.cpp \
KittyMemory/KittyMemory.cpp \
KittyMemory/MemoryPatch.cpp \
KittyMemory/MemoryBackup.cpp \
KittyMemory/KittyUtils.cpp \
KittyMemory/KittyScanner.cpp \
KittyMemory/KittyArm64.cpp \
Nigga/fake_dlfcn.cpp \
Nigga/IL2CppSDK/Il2Cpp.cpp \
Nigga/Tools/MonoString.cpp \
Nigga/Tools/Tools.cpp \
oxorany/oxorany.cpp \


LOCAL_STATIC_LIBRARIES := libdobby libcurl libssl libcrypto libmunodz
LOCAL_CPP_FEATURES     := exceptions
include $(BUILD_SHARED_LIBRARY)

