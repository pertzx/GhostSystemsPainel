@echo off
setlocal enabledelayedexpansion

set "BASEDIR=%~dp0"
set "BASEDIR_SHORT=%~dps0"
set "OUTPUT_DIR=%BASEDIR_SHORT%output"
set "JNI_DIR=%OUTPUT_DIR%\jni"

set "NDK_PATH=C:\Android\ndk\30.0.14904198"
set "TARGET_ARCH=arm64-v8a"

if not exist "%NDK_PATH%\ndk-build.cmd" (
  if exist "C:\Users\Márcio Ribeiro\AppData\Local\Android\Sdk\ndk\30.0.14904198\ndk-build.cmd" (
    set "NDK_PATH=C:\Users\Márcio Ribeiro\AppData\Local\Android\Sdk\ndk\30.0.14904198"
  ) else if exist "C:\Program Files\android-ndk-r27d\ndk-build.cmd" (
    set "NDK_PATH=C:\Program Files\android-ndk-r27d"
  ) else if exist "C:\Users\Márcio Ribeiro\AppData\Local\Android\Sdk\ndk\27.1.12297006\ndk-build.cmd" (
    set "NDK_PATH=C:\Users\Márcio Ribeiro\AppData\Local\Android\Sdk\ndk\27.1.12297006"
  )
)

for %%I in ("%NDK_PATH%") do set "NDK_PATH_SHORT=%%~sI"
for %%I in ("%OUTPUT_DIR%") do set "OUTPUT_DIR_SHORT=%%~sI"
for %%I in ("%JNI_DIR%") do set "JNI_DIR_SHORT=%%~sI"

if not exist "%JNI_DIR%" mkdir "%JNI_DIR%"

echo === Compilando painel GOAT CHEATS ===

set "SRC_LINE="
for /r "%BASEDIR%" %%f in (*.cpp) do (
  for %%I in ("%%f") do set "rel=%%~sI"
  set "SRC_LINE=!SRC_LINE! !rel!"
)

echo LOCAL_PATH := $(call my-dir) > "%JNI_DIR%\Android.mk"
echo include $(CLEAR_VARS) >> "%JNI_DIR%\Android.mk"
echo LOCAL_MODULE := goat >> "%JNI_DIR%\Android.mk"
echo LOCAL_SRC_FILES := !SRC_LINE! >> "%JNI_DIR%\Android.mk"

echo LOCAL_C_INCLUDES := ^
$(LOCAL_PATH)/../../../../../GhostSystemsPainel ^
$(LOCAL_PATH)/../../../../../painel_cheat/imgui ^
$(LOCAL_PATH)/../../../../../painel_cheat/imgui/backends ^
$(LOCAL_PATH)/../../include ^
$(LOCAL_PATH)/../../Unity ^
$(LOCAL_PATH)/../../KittyMemory ^
$(LOCAL_PATH)/../../zygisk >> "%JNI_DIR%\Android.mk"

echo LOCAL_LDFLAGS += -llog -landroid -lEGL -lGLESv2 -lGLESv3 >> "%JNI_DIR%\Android.mk"
echo LOCAL_CPPFLAGS += -std=c++17 -fexceptions -frtti >> "%JNI_DIR%\Android.mk"
echo include $(BUILD_SHARED_LIBRARY) >> "%JNI_DIR%\Android.mk"

echo APP_ABI := %TARGET_ARCH% > "%JNI_DIR%\Application.mk"
echo APP_PLATFORM := android-24 >> "%JNI_DIR%\Application.mk"
echo APP_STL := c++_static >> "%JNI_DIR%\Application.mk"

call "%NDK_PATH_SHORT%\ndk-build.cmd" NDK_PROJECT_PATH="%OUTPUT_DIR_SHORT%" APP_BUILD_SCRIPT="%JNI_DIR_SHORT%\Android.mk" NDK_APPLICATION_MK="%JNI_DIR_SHORT%\Application.mk" APP_ABI="%TARGET_ARCH%" -j4

if errorlevel 1 (
  echo [ERRO] Falha na compilacao do painel GOAT CHEATS.
  rmdir /s /q "%OUTPUT_DIR%" 2>nul
  pause
  exit /b 1
)

if exist "%OUTPUT_DIR%\libs\%TARGET_ARCH%\libgoat.so" (
  move /y "%OUTPUT_DIR%\libs\%TARGET_ARCH%\libgoat.so" "%OUTPUT_DIR%\goat.so" >nul
  echo [OK] Biblioteca compilada: %OUTPUT_DIR%\goat.so
) else (
  echo [ERRO] libgoat.so nao encontrada
)

rmdir /s /q "%OUTPUT_DIR%\obj"
rmdir /s /q "%OUTPUT_DIR%\libs"
rmdir /s /q "%JNI_DIR%"
echo [INFO] Compilacao GOAT CHEATS concluida.
pause
exit /b 0
