LOCAL_PATH := $(call my-dir)
MAIN_LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE := libcurl
LOCAL_SRC_FILES := eLogin/curl/curl-android-$(TARGET_ARCH_ABI)/lib/libcurl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libssl
LOCAL_SRC_FILES := eLogin/curl/openssl-android-$(TARGET_ARCH_ABI)/lib/libssl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcrypto
LOCAL_SRC_FILES := eLogin/curl/openssl-android-$(TARGET_ARCH_ABI)/lib/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE           := libffutil
#LOCAL_MODULE_FILENAME  := $(TARGET_ARCH_ABI)
LOCAL_CFLAGS           := -Wno-error=format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w
LOCAL_CFLAGS           += -fno-rtti -fno-exceptions -fpermissive
LOCAL_CPPFLAGS         := -Wno-error=format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w -Werror -s -std=c++17
LOCAL_CPPFLAGS         += -Wno-error=c++11-narrowing -fms-extensions -fno-rtti -fno-exceptions -fpermissive
LOCAL_LDFLAGS          += -Wl,--gc-sections,--strip-all, -llog
LOCAL_LDLIBS           := -llog -landroid -lEGL -lGLESv3 -lGLESv2 -lGLESv1_CM -lz

LOCAL_C_INCLUDES       += $(LOCAL_PATH)
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/Hush
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/ZygModule
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/Misc
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/Misc/unity
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/Misc/update/xdl/include
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/Misc/update
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/Misc/update/xdl
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/Misc/imgui
LOCAL_C_INCLUDES       += $(LOCAL_PATH)/Misc/imgui/fonts
LOCAL_C_INCLUDES += $(LOCAL_PATH)/eLogin/curl/curl-android-$(TARGET_ARCH_ABI)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/eLogin/curl/openssl-android-$(TARGET_ARCH_ABI)/

FILE_LIST               += $(wildcard $(LOCAL_PATH)/Misc/imgui/*.c*)
FILE_LIST               += $(wildcard $(LOCAL_PATH)/Misc/update/*.c*)
FILE_LIST               += $(wildcard $(LOCAL_PATH)/Misc/update/xdl/*.c*)
FILE_LIST               += $(wildcard $(LOCAL_PATH)/ZygModule/*.c*)
LOCAL_SRC_FILES        := $(FILE_LIST:$(LOCAL_PATH)/%=%)


LOCAL_STATIC_LIBRARIES := libcurl libssl libcrypto
include $(BUILD_SHARED_LIBRARY)

