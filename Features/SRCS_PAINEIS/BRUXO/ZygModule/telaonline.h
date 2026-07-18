#include "aaui.h"

#define O(str) OBFUSCATE(str)
#define SO(str) std::string(OBFUSCATE(str))

JavaVM *jvm;
JNIEnv *genv;

bool isExpired() {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    std::tm expirationDate = {};
    expirationDate.tm_year = 2025 - 1900;
    expirationDate.tm_mon = 12;
    expirationDate.tm_mday = 10;
    return std::difftime(std::mktime(now), std::mktime(&expirationDate)) > 0;
}

std::string getClipboard() {
    std::string result;
    jvm->AttachCurrentThread(&genv, NULL);
    auto looperClass = genv->FindClass(OBFUSCATE("android/os/Looper"));
    auto prepareMethod = genv->GetStaticMethodID(looperClass, OBFUSCATE("prepare"), OBFUSCATE("()V"));
    genv->CallStaticVoidMethod(looperClass, prepareMethod);
    jclass activityThreadClass = genv->FindClass(OBFUSCATE("android/app/ActivityThread"));
    jfieldID sCurrentActivityThreadField = genv->GetStaticFieldID(activityThreadClass, OBFUSCATE("sCurrentActivityThread"), OBFUSCATE("Landroid/app/ActivityThread;"));
    jobject sCurrentActivityThread = genv->GetStaticObjectField(activityThreadClass, sCurrentActivityThreadField);
    jfieldID mInitialApplicationField = genv->GetFieldID(activityThreadClass, OBFUSCATE("mInitialApplication"), OBFUSCATE("Landroid/app/Application;"));
    jobject mInitialApplication = genv->GetObjectField(sCurrentActivityThread, mInitialApplicationField);
    auto contextClass = genv->FindClass(OBFUSCATE("android/content/Context"));
    auto getSystemServiceMethod = genv->GetMethodID(contextClass, OBFUSCATE("getSystemService"), OBFUSCATE("(Ljava/lang/String;)Ljava/lang/Object;"));
    auto str = genv->NewStringUTF(OBFUSCATE("clipboard"));
    auto clipboardManager = genv->CallObjectMethod(mInitialApplication, getSystemServiceMethod, str);
    genv->DeleteLocalRef(str);
    jclass ClipboardManagerClass = genv->FindClass(OBFUSCATE("android/content/ClipboardManager"));
    auto getText = genv->GetMethodID(ClipboardManagerClass, OBFUSCATE("getText"), OBFUSCATE("()Ljava/lang/CharSequence;"));
    jclass CharSequenceClass = genv->FindClass(OBFUSCATE("java/lang/CharSequence"));
    auto toStringMethod = genv->GetMethodID(CharSequenceClass, OBFUSCATE("toString"), OBFUSCATE("()Ljava/lang/String;"));
    auto text = genv->CallObjectMethod(clipboardManager, getText);
    if (text) {
        str = (jstring) genv->CallObjectMethod(text, toStringMethod);
        result = genv->GetStringUTFChars(str, 0);
        genv->DeleteLocalRef(str);
        genv->DeleteLocalRef(text);
    }
    genv->DeleteLocalRef(CharSequenceClass);
    genv->DeleteLocalRef(ClipboardManagerClass);
    genv->DeleteLocalRef(clipboardManager);
    genv->DeleteLocalRef(contextClass);
    genv->DeleteLocalRef(mInitialApplication);
    genv->DeleteLocalRef(activityThreadClass);   
    jvm->DetachCurrentThread();
    return result.c_str();
}

jclass UnityPlayer_cls;
jfieldID UnityPlayer_CurrentActivity_fid;

jobject getGlobalContext() {
    jclass activityThread = genv->FindClass(OBFUSCATE("android/app/ActivityThread"));
    jmethodID currentActivityThread = genv->GetStaticMethodID(activityThread, OBFUSCATE("currentActivityThread"), OBFUSCATE("()Landroid/app/ActivityThread;"));
    jobject at = genv->CallStaticObjectMethod(activityThread, currentActivityThread);
    jmethodID getApplication = genv->GetMethodID(activityThread, OBFUSCATE("getApplication"), OBFUSCATE("()Landroid/app/Application;"));
    jobject context = genv->CallObjectMethod(at, getApplication);
    return context;
}

void displayKeyboard(bool pShow) {
    jclass ctx = genv->FindClass(OBFUSCATE("android/content/Context"));
    jfieldID fid = genv->GetStaticFieldID(ctx, OBFUSCATE("INPUT_METHOD_SERVICE"), OBFUSCATE("Ljava/lang/String;"));
    jmethodID mid = genv->GetMethodID(ctx, OBFUSCATE("getSystemService"), OBFUSCATE("(Ljava/lang/String;)Ljava/lang/Object;"));
    jobject context = genv->GetStaticObjectField(UnityPlayer_cls, UnityPlayer_CurrentActivity_fid);
    jobject InputManObj = genv->CallObjectMethod(context, mid, (jstring) genv->GetStaticObjectField(ctx, fid));
    jclass ClassInputMethodManager = genv->FindClass(OBFUSCATE("android/view/inputmethod/InputMethodManager"));
    jmethodID toggleSoftInputId = genv->GetMethodID(ClassInputMethodManager, OBFUSCATE("toggleSoftInput"), OBFUSCATE("(II)V"));
    if (pShow) {
        genv->CallVoidMethod(InputManObj, toggleSoftInputId, 2, 0);
    } else {
        genv->CallVoidMethod(InputManObj, toggleSoftInputId, 0, 0);
    }
}

struct {
    struct {
        struct {
            jobject Builder(JNIEnv *env) {
                jclass BuilderClass = env->FindClass("android/os/StrictMode$ThreadPolicy$Builder");
                jmethodID BuilderID = env->GetMethodID(BuilderClass,"<init>", "()V");
                return env->NewGlobalRef(env->NewObject(BuilderClass, BuilderID));
            }
            jobject permitAll(JNIEnv *env, jobject builder) {
                jclass BuilderClass = env->FindClass("android/os/StrictMode$ThreadPolicy$Builder");
                jmethodID permitAllID = env->GetMethodID(BuilderClass,"permitAll","()Landroid/os/StrictMode$ThreadPolicy$Builder;");
                return env->NewGlobalRef(env->CallObjectMethod(builder, permitAllID));
            }
            jobject build(JNIEnv *env, jobject permitAll) {
                jclass BuilderClass = env->FindClass("android/os/StrictMode$ThreadPolicy$Builder");
                jmethodID buildID = env->GetMethodID(BuilderClass,"build","()Landroid/os/StrictMode$ThreadPolicy;");
                return env->NewGlobalRef(env->CallObjectMethod(permitAll, buildID));
            }
        }Builder;
    }ThreadPolicy;

    void setThreadPolicy(JNIEnv *env, jobject policy) {
        jclass StrictModeclass = env->FindClass("android/os/StrictMode");
        jmethodID setThreadPolicy = env->GetStaticMethodID(StrictModeclass,"setThreadPolicy","(Landroid/os/StrictMode$ThreadPolicy;)V");
        env->CallStaticVoidMethod(StrictModeclass,setThreadPolicy, policy);
        env->DeleteLocalRef(StrictModeclass);
    }
}StrictMode;

struct {
    jobject HttpURLConnection(JNIEnv *env, const char *url) {
        jclass urlclass = env->FindClass("java/net/URL");
        jmethodID urlcontruc = env->GetMethodID(urlclass, "<init>", "(Ljava/lang/String;)V");
        jobject mainurl = env->NewObject(urlclass, urlcontruc, env->NewStringUTF(url));
        return env->NewGlobalRef(env->NewObject(urlclass, urlcontruc, env->NewStringUTF(url)));
    }

    jobject openConnection(JNIEnv *env, jobject http) {
        jclass urlclass = env->FindClass("java/net/URL");
        jmethodID openConnectionID = env->GetMethodID(urlclass,"openConnection","()Ljava/net/URLConnection;");
        return env->NewGlobalRef(env->CallObjectMethod(http, openConnectionID));
    }
}URL;

struct {
    jobject BufferedInputStream(JNIEnv *env, jobject in) {
        jclass BufferedInputStreamClass = env->FindClass("java/io/BufferedInputStream");
        jmethodID BufferedInputStreamID = env->GetMethodID(BufferedInputStreamClass, "<init>", "(Ljava/io/InputStream;)V");
        return env->NewGlobalRef(env->NewObject(BufferedInputStreamClass, BufferedInputStreamID,in));
    }
}BufferedInputStream;

struct {
    jobject getInputStream(JNIEnv *env, jobject urlConnection) {
        jclass httpcon = env->FindClass("java/net/HttpURLConnection");
        jmethodID impu = env->GetMethodID(httpcon, "getInputStream", "()Ljava/io/InputStream;");
        return env->NewGlobalRef(env->CallObjectMethod(urlConnection, impu));
    }
    void setRequestMethod(JNIEnv *env, jobject urlConnection, const char *method) {
        jclass httpcon = env->FindClass("java/net/HttpURLConnection");
        jmethodID getmthodurl = env->GetMethodID(httpcon, "setRequestMethod", "(Ljava/lang/String;)V");
        env->CallVoidMethod(urlConnection, getmthodurl, env->NewStringUTF(method));
        env->DeleteLocalRef(httpcon);
    }
    void setDoOutput(JNIEnv *env, jobject urlConnection, jboolean dooutput){
        jclass httpcon = env->FindClass("java/net/HttpURLConnection");
        jmethodID setDoOutputid = env->GetMethodID(httpcon, "setDoOutput", "(Z)V");
        env->CallVoidMethod(urlConnection, setDoOutputid, dooutput);
        env->DeleteLocalRef(httpcon);
    }
    void setRequestProperty(JNIEnv *env, jobject urlConnection, const char *key, const char *value) {
        jclass httpcon = env->FindClass("java/net/HttpURLConnection");
        jmethodID setrequespr = env->GetMethodID(httpcon, "setRequestProperty", "(Ljava/lang/String;Ljava/lang/String;)V");
        env->CallVoidMethod(urlConnection, setrequespr, env->NewStringUTF(key), env->NewStringUTF(value));
        env->DeleteLocalRef(httpcon);
    }
    void setFixedLengthStreamingMode(JNIEnv *env, jobject urlConnection, int contentLength){
        jclass httpcon = env->FindClass("java/net/HttpURLConnection");
        jmethodID setFixed = env->GetMethodID(httpcon, "setFixedLengthStreamingMode", "(I)V");
        env->CallVoidMethod(urlConnection, setFixed, contentLength);
        env->DeleteLocalRef(httpcon);
    }
    jobject getOutputStream(JNIEnv *env, jobject urlConnection) {
        jclass httpcon = env->FindClass("java/net/HttpURLConnection");
        jmethodID outhphtppp = env->GetMethodID(httpcon, "getOutputStream", "()Ljava/io/OutputStream;");
        return env->NewGlobalRef(env->CallObjectMethod(urlConnection, outhphtppp));
    }
}HttpURLConnection;

struct {
    jobject InputStreamReader(JNIEnv *env, jobject in, const char* utf) {
        jclass InputStreamReaderClass = env->FindClass("java/io/InputStreamReader");
        jmethodID InputStreamReaderID = env->GetMethodID(InputStreamReaderClass, "<init>", "(Ljava/io/InputStream;Ljava/lang/String;)V");
        return env->NewGlobalRef(env->NewObject(InputStreamReaderClass, InputStreamReaderID,in,env->NewStringUTF(utf)));
    }
}InputStreamReader;

struct {
    jobject BufferedReader(JNIEnv *env, jobject in, int sz) {
        jclass BufferedReaderClass = env->FindClass("java/io/BufferedReader");
        jmethodID BufferedReaderID = env->GetMethodID(BufferedReaderClass, "<init>", "(Ljava/io/Reader;I)V");
        return env->NewGlobalRef(env->NewObject(BufferedReaderClass, BufferedReaderID,in,sz));
    }
    jstring readLine(JNIEnv *env, jobject in) {
        jclass BufferedReaderClass = env->FindClass("java/io/BufferedReader");
        jmethodID readLineID = env->GetMethodID(BufferedReaderClass, "readLine", "()Ljava/lang/String;");
        return (jstring)env->NewGlobalRef(env->CallObjectMethod(in, readLineID));
    }
}BufferedReader;

struct {
    jobject StringBuilder(JNIEnv *env) {
        jclass StringBuilderClass = env->FindClass("java/lang/StringBuilder");
        jmethodID StringBuilderID = env->GetMethodID(StringBuilderClass, "<init>", "()V");
        return env->NewGlobalRef(env->NewObject(StringBuilderClass, StringBuilderID));
    }
    jobject append(JNIEnv *env, jobject obj, jstring str) {
        jclass StringBuilderClass = env->FindClass("java/lang/StringBuilder");
        jmethodID appendID = env->GetMethodID(StringBuilderClass,"append", "(Ljava/lang/String;)Ljava/lang/StringBuilder;");
        return env->NewGlobalRef(env->CallObjectMethod(obj, appendID,str));
    }
    jstring toString(JNIEnv *env, jobject obj) {
        jclass Object = env->FindClass("java/lang/StringBuilder");
        jmethodID toString = env->GetMethodID(Object,"toString", "()Ljava/lang/String;");
        return (jstring) env->NewGlobalRef(env->CallObjectMethod(obj, toString));
    }
}StringBuilder;

const char *getRequestURL(JNIEnv *env, const char *url) {
    jobject policy = StrictMode.ThreadPolicy.Builder.build(env,StrictMode.ThreadPolicy.Builder.permitAll(env,StrictMode.ThreadPolicy.Builder.Builder(env)));
    StrictMode.setThreadPolicy(env,policy);
    jobject urlConnection = URL.openConnection(env,URL.HttpURLConnection(env,url));
    jobject in = BufferedInputStream.BufferedInputStream(env,HttpURLConnection.getInputStream(env,urlConnection));
    jobject reader = BufferedReader.BufferedReader(env,InputStreamReader.InputStreamReader(env,in,"UTF-8"),8);
    jobject sb = StringBuilder.StringBuilder(env);
    jstring line;
    while((line = BufferedReader.readLine(env,reader)) != nullptr) {
        StringBuilder.append(env,sb,line);
    }
    jstring str = StringBuilder.toString(env,sb);
    return env->GetStringUTFChars(str,0);
}

jobject getJNIContext(JNIEnv *env) {
    jclass activityThreadCls = env->FindClass("android/app/ActivityThread");
    jmethodID currentActivityThread = env->GetStaticMethodID(activityThreadCls, "currentActivityThread", "()Landroid/app/ActivityThread;");
    jobject activityThreadObj = env->CallStaticObjectMethod(activityThreadCls, currentActivityThread);
    jmethodID getApplication = env->GetMethodID(activityThreadCls, "getApplication", "()Landroid/app/Application;");
    jobject context = env->CallObjectMethod(activityThreadObj, getApplication);
    return context;
}

const char *GetAndroidID(JNIEnv *env, jobject context) {
    jclass contextClass = env->FindClass("android/content/Context");
    jmethodID getContentResolverMethod = env->GetMethodID(contextClass,"getContentResolver","()Landroid/content/ContentResolver;");
    jclass settingSecureClass = env->FindClass("android/provider/Settings$Secure");
    jmethodID getStringMethod = env->GetStaticMethodID(settingSecureClass,"getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;");
    auto obj = env->CallObjectMethod(context, getContentResolverMethod);
    auto str = (jstring) env->CallStaticObjectMethod(settingSecureClass, getStringMethod, obj,env->NewStringUTF("android_id"));
    return env->GetStringUTFChars(str, 0);
}

const char *GetDeviceModel(JNIEnv *env) {
    jclass buildClass = env->FindClass("android/os/Build");
    jfieldID modelId = env->GetStaticFieldID(buildClass, "MODEL","Ljava/lang/String;");
    auto str = (jstring) env->GetStaticObjectField(buildClass, modelId);
    return env->GetStringUTFChars(str, 0);
}

const char *GetDeviceBrand(JNIEnv *env) {
    jclass buildClass = env->FindClass("android/os/Build");
    jfieldID modelId = env->GetStaticFieldID(buildClass, "BRAND","Ljava/lang/String;");
    auto str = (jstring) env->GetStaticObjectField(buildClass, modelId);
    return env->GetStringUTFChars(str, 0);
}

const char *GetDeviceUniqueIdentifier(JNIEnv *env, const char *uuid) {
    jclass uuidClass = env->FindClass("java/util/UUID");
    auto len = strlen(uuid);
    jbyteArray myJByteArray = env->NewByteArray(len);
    env->SetByteArrayRegion(myJByteArray, 0, len, (jbyte *) uuid);
    jmethodID nameUUIDFromBytesMethod = env->GetStaticMethodID(uuidClass,"nameUUIDFromBytes","([B)Ljava/util/UUID;");
    jmethodID toStringMethod = env->GetMethodID(uuidClass, "toString","()Ljava/lang/String;");
    auto obj = env->CallStaticObjectMethod(uuidClass, nameUUIDFromBytesMethod, myJByteArray);
    auto str = (jstring) env->CallObjectMethod(obj, toStringMethod);
    return env->GetStringUTFChars(str, 0);
}

std::string msg;
bool isLoginSuccess = false;
bool isChecking = false;

std::string ValidateKey(JavaVM *vm, const char *key) {
    if (!vm) return SO("FATAL ERROR");
    JNIEnv *env = nullptr;
    vm->AttachCurrentThread(&env, 0);
    
    char params[1024];
    const char *isURL = O("https://4m.xiters.site/mod/CheckLogin.php?user=%s&pass=%s&uid=%s");
    auto object = getJNIContext(env);
    
    std::string hwid = key;
    hwid += GetAndroidID(env, object);
    hwid += GetDeviceModel(env);
    hwid += GetDeviceBrand(env);
    std::string UUID = GetDeviceUniqueIdentifier(env, hwid.c_str());

    snprintf(params, sizeof(params), isURL, key, key, UUID.c_str());
    const char *Request = getRequestURL(env, params);

    vm->DetachCurrentThread();

    if(strcmp(isURL, OBFUSCATE("https://4m.xiters.site/mod/CheckLogin.php?user=%s&pass=%s&uid=%s")) != 0) {
        return O("Server error");
    }

    std::string FindKey = (key);
    if (FindKey.find(SO("*")) != std::string::npos || FindKey.find(SO("'")) != std::string::npos) {
        return O("Invalid key format");
    }

    std::string FindURL = (isURL);
    std::string FindParams = (params);
    
    if (FindURL.find(SO("https://4m.xiters.site")) != std::string::npos && 
        FindParams.find(SO("https://4m.xiters.site")) != std::string::npos) {
        
        if(Request != nullptr) {
            std::string request = (Request);
            if (request.find(UUID) != std::string::npos) {
                if(strcmp(Request, UUID.c_str()) != 0) {
                    return O("Server error");
                }
                return O("SUCCESS");
            } else if(request.find("ﾠ") != std::string::npos) {
                return O("");
            } else {
                return request.c_str();
            }
        } else {
            return O("Server error");
        }
    } else {
        return O("Server error");
    }
}

void AutoCheckClipboard() {
    isChecking = true;
    
    // Obtém o conteúdo da área de transferência
    std::string clipboardKey = getClipboard();
    
    if (clipboardKey.empty()) {
        msg = SO("Clipboard is empty");
        isChecking = false;
        return;
    }
    
    // Valida a key com o servidor
    msg = ValidateKey(jvm, clipboardKey.c_str());
    
    // Verifica se foi bem-sucedido
    if (msg == SO("SUCCESS")) {
        isLoginSuccess = true;
    } else {
        isLoginSuccess = false;
    }
    
    isChecking = false;
}

// Função principal de renderização
// ... código anterior

// Função principal de renderização
void MainRender() {
    // 1. Se ainda não verificou, faz a verificação automática (roda apenas uma vez)
    static bool checkedOnce = false;
    if (!checkedOnce && !isChecking) {
        checkedOnce = true;
        std::thread check_thread(AutoCheckClipboard);
        check_thread.detach();
    }
    
    // 2. Se está verificando, mostra mensagem de carregamento
    if (isChecking) {
        ImVec2 screen_size = ImGui::GetIO().DisplaySize;
        ImGui::SetNextWindowPos(ImVec2(screen_size.x * 0.5f, screen_size.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.082f, 0.082f, 0.082f, 0.95f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        
        if (ImGui::Begin("##Checking", nullptr, 
                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | 
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove)) {
         //   ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), O("Validating key..."));
            ImGui::End();
        }
        
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
        return; // Retorna para não renderizar o menu ou o erro enquanto verifica
    }
    
    // 3. Verifica o resultado da validação (só entra aqui se !isChecking)
    
    // Se login foi bem-sucedido, renderiza o menu principal
    if (isLoginSuccess) {
        MenuRender();
        GetPointers();

        if (inMatch) {
            DrawESP(egl.width, egl.height);
            AimbotRageVoid();
            AimbotLegitVoid();
	        void UpPlayerVoid();
        }
        
    } else {
        // Mostra mensagem de erro se a validação falhou e não está mais verificando
        ImVec2 screen_size = ImGui::GetIO().DisplaySize;
        ImGui::SetNextWindowPos(ImVec2(screen_size.x * 0.5f, screen_size.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.082f, 0.082f, 0.082f, 0.95f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        
        if (ImGui::Begin("##Error", nullptr, 
                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | 
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove)) {
            
            // Aqui você pode exibir a mensagem de erro armazenada em 'msg'
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), O("Key Expirada!")); // Título do erro
            ImGui::Separator();
            ImGui::TextWrapped("%s", msg.c_str()); // Mensagem detalhada da falha (ex: Invalid key, Server error, etc.)

            ImGui::End();
        }
        
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }
}

