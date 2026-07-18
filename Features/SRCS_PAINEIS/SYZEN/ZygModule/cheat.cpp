#include <esp.h>
#include <layout.h>
#include <imports.h>
#include <jni.h>
#include <telaonline.h>
#include <EGL/egl.h>
#include <dlfcn.h>
#include <unistd.h>
#include <pthread.h>

class _BYTE; class _QWORD;
class _DWORD; class _WORD;
#define _QWORD long 
#define _DWORD long
#define _BYTE  long 
#define _WORD  long


bool initialized = false;
EGLBoolean(*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
static bool showMenu = false;
static bool threeFingerTouchActive = false;

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
	eglQuerySurface(dpy, surface, EGL_WIDTH, &egl.width);
	eglQuerySurface(dpy, surface, EGL_HEIGHT, &egl.height);
	if (!egl.setup) {
		ImGui::CreateContext();
		SetupImgui();
		InitTexture();
		egl.setup = true;
	}

	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplAndroid_NewFrame(egl.width, egl.height);
	ImGui::NewFrame();

	// Quantidade de toques
	int touchCount = (((int (*)())(Il2CppGetMethodOffset(
		OBFUSCATE("UnityEngine.dll"), 
		OBFUSCATE("UnityEngine"), 
		OBFUSCATE("Input"), 
		OBFUSCATE("get_touchCount"))))());

	// Verifica se existem exatamente 3 dedos tocando
	if (touchCount == 4) {
		bool allThreeBegan = true;
		for (int i = 0; i < 4; i++) {
			UnityEngine_Touch_Fields touch = ((UnityEngine_Touch_Fields(*)(int))(Il2CppGetMethodOffset(
				OBFUSCATE("UnityEngine.dll"), 
				OBFUSCATE("UnityEngine"), 
				OBFUSCATE("Input"), 
				OBFUSCATE("GetTouch"), 1)))(i);

			if (touch.m_Phase != TouchPhase::Began) {
				allThreeBegan = false;
				break;
			}
		}

		// Só ativa quando os 3 dedos tocarem ao mesmo tempo (fase Began)
		if (allThreeBegan && !threeFingerTouchActive) {
			showMenu = !showMenu;
			threeFingerTouchActive = true;
		}
	} else {
		threeFingerTouchActive = false;
	}

	// Tratar o primeiro dedo como mouse (opcional)
	if (touchCount > 0) {
		UnityEngine_Touch_Fields touch = ((UnityEngine_Touch_Fields(*)(int))(Il2CppGetMethodOffset(
			OBFUSCATE("UnityEngine.dll"), 
			OBFUSCATE("UnityEngine"), 
			OBFUSCATE("Input"), 
			OBFUSCATE("GetTouch"), 1)))(0);

		float reverseY = io.DisplaySize.y - touch.m_Position.fields.y;
		switch (touch.m_Phase) {
		case TouchPhase::Began:
		case TouchPhase::Stationary:
			io.MousePos = ImVec2(touch.m_Position.fields.x, reverseY);
			io.MouseDown[0] = true;
			break;
		case TouchPhase::Ended:
		case TouchPhase::Canceled:
			io.MouseDown[0] = false;
			clearMousePos = true;
			break;
		case TouchPhase::Moved:
			io.MousePos = ImVec2(touch.m_Position.fields.x, reverseY);
			break;
		default:
			break;
		}
	}

	if (showMenu) {
		menu();
		RenderBotaoFlutuante();
		RenderBotaoSelint();
        GetPointers();
        if (inMatch) {
        DrawESP(egl.width, egl.height);
        AimbotRageVoid();
		AimbotLegitVoid();
		UpdateSpeedhack();
     }
	 
	 if (!initialized) {
         std::thread(AimSilentVoid).detach();
         initialized = true;
    }
	 
	}

	ImGui::EndFrame();
	ImGui::Render();

    glViewport(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return old_eglSwapBuffers(dpy, surface);
}


void* loadCheat(void* org) {
    while (!getLibraryLoaded(OBFUSCATE("libil2cpp.so")))
    
     sleep(3);

    Il2CppAttach();

    void* libEGL = dlopen(OBFUSCATE("libEGL.so"), RTLD_NOW | RTLD_GLOBAL);
    if (!libEGL) return nullptr;

    A64HookFunction(
        (void*)dlsym(libEGL, OBFUSCATE("eglSwapBuffers")),
        (void*)hook_eglSwapBuffers,
        (void**)&old_eglSwapBuffers
    );

	if (mlovinit()) {
		setShader(OBFUSCATE("_AlphaMask"));
		LogShaders(); Wallhack();
	}
	
    return nullptr;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
	jvm = vm;
	pthread_t ptid;
    pthread_create(&ptid, nullptr, loadCheat, nullptr);
	return JNI_VERSION_1_6;
}

