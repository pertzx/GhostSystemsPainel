#pragma once

//
// Created by GHr_Ryuuka on 05/12/2021.
//

#include <curl/curl.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

JavaVM *jvm;
JNIEnv *env;
using json = nlohmann::json;
std::string g_Token, g_Auth;
bool bValid = false;
std::string EXP = "";
static bool isLogin = false;
static std::string err;
std::string UUID = "";
static string response;
struct MemoryStruct {
	char *memory;
	size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *) userp;
	
	mem->memory = (char *) realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL) {
		return 0;
	}
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	return realsize;
}

jobject getGlobalContext(JNIEnv *env) {
    jclass activityThread = env->FindClass("android/app/ActivityThread");
    jmethodID currentActivityThread = env->GetStaticMethodID(activityThread,"currentActivityThread","()Landroid/app/ActivityThread;");
    jobject at = env->CallStaticObjectMethod(activityThread, currentActivityThread);

    jmethodID getApplication = env->GetMethodID(activityThread, "getApplication","()Landroid/app/Application;");
    jobject context = env->CallObjectMethod(at, getApplication);
    return context;
}

void Toast(const char *text, int length) {
    jvm->AttachCurrentThread(&env, NULL);   
    jstring jstr = env->NewStringUTF(text);
    jclass toast = env->FindClass(OBFUSCATE("android/widget/Toast"));
    jmethodID methodMakeText =env->GetStaticMethodID(toast,OBFUSCATE("makeText"),OBFUSCATE("(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;"));
    jobject toastobj = env->CallStaticObjectMethod(toast, methodMakeText,getGlobalContext(env), jstr, length);
    jmethodID methodShow = env->GetMethodID(toast, OBFUSCATE("show"), OBFUSCATE("()V"));
    env->CallVoidMethod(toastobj, methodShow);
}

void OpenLink(const char *url) {
    jvm->AttachCurrentThread(&env, NULL);
	JNIEnv *env;
	jobject context;
	
    jclass intentClass = env->FindClass(OBFUSCATE("android/content/Intent"));
    jmethodID intentConstructor = env->GetMethodID(intentClass, OBFUSCATE("<init>"), OBFUSCATE("(Ljava/lang/String;)V"));
    jobject intentObject = env->NewObject(intentClass, intentConstructor, env->NewStringUTF(OBFUSCATE("android.intent.action.VIEW")));

    jclass uriClass = env->FindClass(OBFUSCATE("android/net/Uri"));

    jmethodID parseMethod = env->GetStaticMethodID(uriClass, OBFUSCATE("parse"), OBFUSCATE("(Ljava/lang/String;)Landroid/net/Uri;"));
    jobject uriObject = env->CallStaticObjectMethod(uriClass, parseMethod, env->NewStringUTF(url));

    jmethodID setDataMethod = env->GetMethodID(intentClass, OBFUSCATE("setData"), OBFUSCATE("(Landroid/net/Uri;)Landroid/content/Intent;"));
    env->CallObjectMethod(intentObject, setDataMethod, uriObject);

    jmethodID startActivityMethod = env->GetMethodID(env->GetObjectClass(context), OBFUSCATE("startActivity"), OBFUSCATE("(Landroid/content/Intent;)V"));
    env->CallVoidMethod(context, startActivityMethod, intentObject);

    env->DeleteLocalRef(intentClass);
    env->DeleteLocalRef(intentObject);
    env->DeleteLocalRef(uriClass);
    env->DeleteLocalRef(uriObject);
    jvm->DetachCurrentThread();
}

int ShowSoftKeyboardInput() {
	jint result;
	jint flags = 0;
	jvm->AttachCurrentThread(&env, NULL);
	
	jclass looperClass = env->FindClass(OBFUSCATE("android/os/Looper"));
	auto prepareMethod = env->GetStaticMethodID(looperClass, OBFUSCATE("prepare"), OBFUSCATE("()V"));
	env->CallStaticVoidMethod(looperClass, prepareMethod);
	
	jclass activityThreadClass = env->FindClass(OBFUSCATE("android/app/ActivityThread"));
	jfieldID sCurrentActivityThreadField = env->GetStaticFieldID(activityThreadClass, OBFUSCATE("sCurrentActivityThread"), OBFUSCATE("Landroid/app/ActivityThread;"));
	jobject sCurrentActivityThread = env->GetStaticObjectField(activityThreadClass, sCurrentActivityThreadField);
	
	jfieldID mInitialApplicationField = env->GetFieldID(activityThreadClass, OBFUSCATE("mInitialApplication"), OBFUSCATE("Landroid/app/Application;"));
	jobject mInitialApplication = env->GetObjectField(sCurrentActivityThread, mInitialApplicationField);
	
	jclass contextClass = env->FindClass(OBFUSCATE("android/content/Context"));
	jfieldID fieldINPUT_METHOD_SERVICE = env->GetStaticFieldID(contextClass, OBFUSCATE("INPUT_METHOD_SERVICE"), OBFUSCATE("Ljava/lang/String;"));
	jobject INPUT_METHOD_SERVICE = env->GetStaticObjectField(contextClass, fieldINPUT_METHOD_SERVICE);
	jmethodID getSystemServiceMethod = env->GetMethodID(contextClass, OBFUSCATE("getSystemService"), OBFUSCATE("(Ljava/lang/String;)Ljava/lang/Object;"));
	jobject callObjectMethod = env->CallObjectMethod(mInitialApplication, getSystemServiceMethod, INPUT_METHOD_SERVICE);
	
	jclass classInputMethodManager = env->FindClass(OBFUSCATE("android/view/inputmethod/InputMethodManager"));
    jmethodID toggleSoftInputId = env->GetMethodID(classInputMethodManager, OBFUSCATE("toggleSoftInput"), OBFUSCATE("(II)V"));
	
	if (result) {
		env->CallVoidMethod(callObjectMethod, toggleSoftInputId, 2, flags);
	} else {
		env->CallVoidMethod(callObjectMethod, toggleSoftInputId, flags, flags);
	}
	
	env->DeleteLocalRef(classInputMethodManager);
	env->DeleteLocalRef(callObjectMethod);
	env->DeleteLocalRef(contextClass);
    env->DeleteLocalRef(mInitialApplication);
    env->DeleteLocalRef(activityThreadClass);
	jvm->DetachCurrentThread();
	
	return result;
}

int PollUnicodeChars() {
	
	jvm->AttachCurrentThread(&env, NULL);
	
	jclass looperClass = env->FindClass(OBFUSCATE("android/os/Looper"));
	auto prepareMethod = env->GetStaticMethodID(looperClass, OBFUSCATE("prepare"), OBFUSCATE("()V"));
	env->CallStaticVoidMethod(looperClass, prepareMethod);
	
	jclass activityThreadClass = env->FindClass(OBFUSCATE("android/app/ActivityThread"));
	jfieldID sCurrentActivityThreadField = env->GetStaticFieldID(activityThreadClass, OBFUSCATE("sCurrentActivityThread"), OBFUSCATE("Landroid/app/ActivityThread;"));
	jobject sCurrentActivityThread = env->GetStaticObjectField(activityThreadClass, sCurrentActivityThreadField);
	
	jfieldID mInitialApplicationField = env->GetFieldID(activityThreadClass, OBFUSCATE("mInitialApplication"), OBFUSCATE("Landroid/app/Application;"));
	jobject mInitialApplication = env->GetObjectField(sCurrentActivityThread, mInitialApplicationField);
	
	jclass keyEventClass = env->FindClass(OBFUSCATE("android/view/KeyEvent"));
	jmethodID getUnicodeCharMethod = env->GetMethodID(keyEventClass, OBFUSCATE("getUnicodeChar"), OBFUSCATE("(I)I"));
	
	ImGuiIO& io = ImGui::GetIO();
    
	int return_key = env->CallIntMethod(keyEventClass, getUnicodeCharMethod);
	
	env->DeleteLocalRef(keyEventClass);
	env->DeleteLocalRef(mInitialApplication);
    env->DeleteLocalRef(activityThreadClass);
	jvm->DetachCurrentThread();
	
	return return_key;
}

std::string getClipboard() {
	std::string result;
	jvm->AttachCurrentThread(&env, NULL);
	
	auto looperClass = env->FindClass(OBFUSCATE("android/os/Looper"));
	auto prepareMethod = env->GetStaticMethodID(looperClass, OBFUSCATE("prepare"), OBFUSCATE("()V"));
	env->CallStaticVoidMethod(looperClass, prepareMethod);
	
	jclass activityThreadClass = env->FindClass(OBFUSCATE("android/app/ActivityThread"));
	jfieldID sCurrentActivityThreadField = env->GetStaticFieldID(activityThreadClass, OBFUSCATE("sCurrentActivityThread"), OBFUSCATE("Landroid/app/ActivityThread;"));
	jobject sCurrentActivityThread = env->GetStaticObjectField(activityThreadClass, sCurrentActivityThreadField);
	
	jfieldID mInitialApplicationField = env->GetFieldID(activityThreadClass, OBFUSCATE("mInitialApplication"), OBFUSCATE("Landroid/app/Application;"));
	jobject mInitialApplication = env->GetObjectField(sCurrentActivityThread, mInitialApplicationField);
	
	auto contextClass = env->FindClass(OBFUSCATE("android/content/Context"));
	auto getSystemServiceMethod = env->GetMethodID(contextClass, OBFUSCATE("getSystemService"), OBFUSCATE("(Ljava/lang/String;)Ljava/lang/Object;"));
	
	auto str = env->NewStringUTF(OBFUSCATE("clipboard"));
	auto clipboardManager = env->CallObjectMethod(mInitialApplication, getSystemServiceMethod, str);
	env->DeleteLocalRef(str);
	
	jclass ClipboardManagerClass = env->FindClass(OBFUSCATE("android/content/ClipboardManager"));
    auto getText = env->GetMethodID(ClipboardManagerClass, OBFUSCATE("getText"), OBFUSCATE("()Ljava/lang/CharSequence;"));

    jclass CharSequenceClass = env->FindClass(OBFUSCATE("java/lang/CharSequence"));
    auto toStringMethod = env->GetMethodID(CharSequenceClass, OBFUSCATE("toString"), OBFUSCATE("()Ljava/lang/String;"));

    auto text = env->CallObjectMethod(clipboardManager, getText);
    if (text) {
        str = (jstring) env->CallObjectMethod(text, toStringMethod);
        result = env->GetStringUTFChars(str, 0);
        env->DeleteLocalRef(str);
        env->DeleteLocalRef(text);
    }
    env->DeleteLocalRef(CharSequenceClass);
    env->DeleteLocalRef(ClipboardManagerClass);
    env->DeleteLocalRef(clipboardManager);
    env->DeleteLocalRef(contextClass);
    env->DeleteLocalRef(mInitialApplication);
    env->DeleteLocalRef(activityThreadClass); 	
    jvm->DetachCurrentThread();
    return result.c_str();
}

bool secureCompare(const std::string &a, const std::string &b) {
    if (a.size() != b.size()) return false;
    volatile uint8_t result = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

std::string Login(const char *user_key) {
	JNIEnv *env;
    jvm->AttachCurrentThread(&env, 0);
	
	auto looperClass = env->FindClass(OBFUSCATE("android/os/Looper"));
	auto prepareMethod = env->GetStaticMethodID(looperClass, OBFUSCATE("prepare"), OBFUSCATE("()V"));
	env->CallStaticVoidMethod(looperClass, prepareMethod);
	
	jclass activityThreadClass = env->FindClass(OBFUSCATE("android/app/ActivityThread"));
	jfieldID sCurrentActivityThreadField = env->GetStaticFieldID(activityThreadClass, OBFUSCATE("sCurrentActivityThread"), OBFUSCATE("Landroid/app/ActivityThread;"));
	jobject sCurrentActivityThread = env->GetStaticObjectField(activityThreadClass, sCurrentActivityThreadField);
	
	jfieldID mInitialApplicationField = env->GetFieldID(activityThreadClass, OBFUSCATE("mInitialApplication"), OBFUSCATE("Landroid/app/Application;"));
	jobject mInitialApplication = env->GetObjectField(sCurrentActivityThread, mInitialApplicationField);
	
	std::string hwid = user_key;
	hwid += Tools::GetAndroidID(env, mInitialApplication);
	hwid += Tools::GetDeviceModel(env);
	hwid += Tools::GetDeviceBrand(env);
	std::string UUID = Tools::GetDeviceUniqueIdentifier(env, hwid.c_str());
	jvm->DetachCurrentThread();
	std::string errMsg;
	
	struct MemoryStruct chunk{};
	chunk.memory = (char *) malloc(1);
	chunk.size = 0;
	
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	
	if (curl) {
        std::string check_key = OBFUSCATE("http://yuimsever.x10.bz/connect");
        std::string CrackConCac = OBFUSCATE("https");
        
        if (check_key != check_key) {
			ImGui_ImplAndroid_Shutdown();
            ImGui_ImplOpenGL3_Shutdown();
            ImGui::DestroyContext();        
		}
		curl_easy_setopt(curl, CURLOPT_URL, (check_key.c_str()));
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, (CrackConCac.c_str()));
		
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, OBFUSCATE("Content-Type: application/x-www-form-urlencoded"));
		
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		char data[4096];
		sprintf(data, OBFUSCATE("game=FF&user_key=%s&serial=%s"), user_key, UUID.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &chunk);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		
		res = curl_easy_perform(curl);
		if (res == CURLE_OK) {
			try {
				json result = json::parse(chunk.memory);
				if (result[std::string(OBFUSCATE("status"))] == true) {
					std::string token = result[std::string(OBFUSCATE("data"))][std::string(OBFUSCATE("token"))].get<std::string>();
					time_t rng = result[std::string(OBFUSCATE("data"))][std::string(OBFUSCATE("rng"))].get<time_t>();
					EXP = result[std::string(OBFUSCATE("data"))][std::string(OBFUSCATE("EXP"))].get<std::string>();
					
					
					if (rng + 30 > time(0)) {
						std::string auth = OBFUSCATE("FF");
						auth += std::string(OBFUSCATE("-"));
						auth += user_key;
						auth += std::string(OBFUSCATE("-"));
						auth += UUID;
						auth += std::string(OBFUSCATE("-"));
						auth += std::string(OBFUSCATE("Vm8Lk7Uj2JmsjCPVPVjrLa7zgfx3uz9E"));
						
						std::string outputAuth = Tools::CalcMD5(auth);
						g_Token = token;
						g_Auth = outputAuth;
						bValid = g_Token == g_Auth;
					}
				} else {
					errMsg = result[std::string(OBFUSCATE("reason"))].get<std::string>();
				}
			} catch (json::exception &e) {
				errMsg = "{";
				errMsg += e.what();
				errMsg += "}\n{";
				errMsg += chunk.memory;
				errMsg += "}";
			}
		} else {
			errMsg = curl_easy_strerror(res);
		}
	}
	curl_easy_cleanup(curl);
	jvm->DetachCurrentThread();
	return bValid ? "OK" : errMsg;
}
