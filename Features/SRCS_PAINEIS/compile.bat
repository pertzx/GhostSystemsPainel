@echo off
setlocal enabledelayedexpansion

:: Configuracoes
set "BASEDIR=%~dp0"
set "OUTPUT_DIR=%BASEDIR%output"
set "JNI_DIR=%OUTPUT_DIR%\jni"

set "NDK_PATH=C:\Android\ndk\30.0.14904198"
set "TARGET_ARCH=arm64-v8a"

:: Verificar NDK
if not exist "%NDK_PATH%\ndk-build.cmd" (
    if exist "C:\Users\M�rcio Ribeiro\AppData\Local\Android\Sdk\ndk\30.0.14904198\ndk-build.cmd" (
        set "NDK_PATH=C:\Users\M�rcio Ribeiro\AppData\Local\Android\Sdk\ndk\30.0.14904198"
    ) else if exist "C:\Program Files\android-ndk-r27d\ndk-build.cmd" (
        set "NDK_PATH=C:\Program Files\android-ndk-r27d"
    ) else if exist "C:\Users\M�rcio Ribeiro\AppData\Local\Android\Sdk\ndk\27.1.12297006\ndk-build.cmd" (
        set "NDK_PATH=C:\Users\M�rcio Ribeiro\AppData\Local\Android\Sdk\ndk\27.1.12297006"
    )
)

for %%I in ("%NDK_PATH%") do set "NDK_PATH_SHORT=%%~sI"

:: Criar diretorios
if not exist "%JNI_DIR%" mkdir "%JNI_DIR%"

:: Gerar Android.mk com todos .cpp recursivamente
echo LOCAL_PATH := $(call my-dir) > "%JNI_DIR%\Android.mk"
echo include $(CLEAR_VARS) >> "%JNI_DIR%\Android.mk"
echo LOCAL_MODULE := painel >> "%JNI_DIR%\Android.mk"

:: Coletar todos .cpp
set SRC_LIST=
for /r "%BASEDIR%" %%f in (*.cpp) do (
    set "rel=%%~pnxf"
    rem Caminho relativo ao SRCS_PAINEIS: remover prefixo do BASEDIR
    set "rel=!rel:%BASEDIR%=!"
    echo ../../GhostSystemsPainel/Features/SRCS_PAINEIS/!rel! >> "%JNI_DIR%\sources.list"
)

type "%JNI_DIR%\sources.list" | findstr /v "^$" > "%JNI_DIR%\srcs.tmp"
set /p SRC_LIST=<"%JNI_DIR%\srcs.tmp"
echo LOCAL_SRC_FILES := !SRC_LIST! >> "%JNI_DIR%\Android.mk"
del "%JNI_DIR%\sources.list" "%JNI_DIR%\srcs.tmp" 2>nul

:: Includes (mesmos do projeto principal)
echo LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../../../GhostSystemsPainel ^
$(LOCAL_PATH)/../../../../../painel_cheat/imgui ^
$(LOCAL_PATH)/../../../../../painel_cheat/imgui/backends >> "%JNI_DIR%\Android.mk"

echo LOCAL_LDFLAGS += -llog -landroid -lEGL -lGLESv2 -lGLESv3 >> "%JNI_DIR%\Android.mk"
echo LOCAL_CPPFLAGS += -std=c++17 -fexceptions -frtti >> "%JNI_DIR%\Android.mk"
echo include $(BUILD_SHARED_LIBRARY) >> "%JNI_DIR%\Android.mk"

:: Application.mk
echo APP_ABI := %TARGET_ARCH% > "%JNI_DIR%\Application.mk"
echo APP_PLATFORM := android-24 >> "%JNI_DIR%\Application.mk"
echo APP_STL := c++_static >> "%JNI_DIR%\Application.mk"

:: Compilar
echo [INFO] Compilando fontes dos paineis...
call "%NDK_PATH_SHORT%\ndk-build.cmd" NDK_PROJECT_PATH="%OUTPUT_DIR%" APP_BUILD_SCRIPT="%JNI_DIR%\Android.mk" NDK_APPLICATION_MK="%JNI_DIR%\Application.mk" APP_ABI="%TARGET_ARCH%" -j4

if errorlevel 1 (
    echo [ERRO] Falha na compilacao dos paineis.
    exit /b 1
)

:: Mover .so
if exist "%OUTPUT_DIR%\libs\%TARGET_ARCH%\libpainel.so" (
    move /y "%OUTPUT_DIR%\libs\%TARGET_ARCH%\libpainel.so" "%OUTPUT_DIR%\libpainel.so" >nul
    echo [OK] Biblioteca painel compilada: %OUTPUT_DIR%\libpainel.so
)

:: Limpar
rmdir /s /q "%OUTPUT_DIR%\obj" 2>nul
rmdir /s /q "%OUTPUT_DIR%\libs" 2>nul
rmdir /s /q "%JNI_DIR%" 2>nul

echo [INFO] Compilacao dos paineis concluida.
exit /b 0
