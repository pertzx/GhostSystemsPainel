#include "aaui.h"


#define O(str) OBFUSCATE(str)
#define SO(str) std::string(OBFUSCATE(str))

JavaVM *jvm;
JNIEnv *genv;
/*
bool isExpired() {
std::time_t t = std::time(nullptr);
std::tm* now = std::localtime(&t);
std::tm expirationDate = {};
expirationDate.tm_year = 2025 - 1900;
expirationDate.tm_mon = 12;
expirationDate.tm_mday = 10;
return std::difftime(std::mktime(now), std::mktime(&expirationDate)) > 0;
}
*/
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



jobject getGlobalContext()
{
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

int ShowSoftKeyboardInput() {
jint result;
jint flags = 0;

JNIEnv *env;
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
return env->NewGlobalRef(env->CallObjectMethod(builder, permitAllID));;
}
jobject build(JNIEnv *env, jobject permitAll) {
jclass BuilderClass = env->FindClass("android/os/StrictMode$ThreadPolicy$Builder");
jmethodID buildID = env->GetMethodID(BuilderClass,"build","()Landroid/os/StrictMode$ThreadPolicy;");
return env->NewGlobalRef(env->CallObjectMethod(permitAll, buildID));;
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
return env->NewGlobalRef(env->CallObjectMethod(http, openConnectionID));;
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
return env->NewGlobalRef(env->CallObjectMethod(urlConnection, impu));;
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
return (jstring)env->NewGlobalRef(env->CallObjectMethod(in, readLineID));;
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
return env->NewGlobalRef(env->CallObjectMethod(obj, appendID,str));;
}
jstring toString(JNIEnv *env, jobject obj) {
jclass Object = env->FindClass("java/lang/StringBuilder");
jmethodID toString = env->GetMethodID(Object,"toString", "()Ljava/lang/String;");
return (jstring) env->NewGlobalRef(env->CallObjectMethod(obj, toString));;
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
return env->GetStringUTFChars(str,0);;
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
inline std::string getClipboard(JavaVM* jvm) {
if (jvm == nullptr) return {};
std::string result;
JNIEnv* env;
if (jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return {};

jclass looperClass = env->FindClass("android/os/Looper");
jmethodID prepareMethod = env->GetStaticMethodID(looperClass, "prepare", "()V");
env->CallStaticVoidMethod(looperClass, prepareMethod);
jclass activityThreadClass = env->FindClass("android/app/ActivityThread");
jfieldID sCurrentActivityThreadField = env->GetStaticFieldID(activityThreadClass, "sCurrentActivityThread", "Landroid/app/ActivityThread;");
jobject sCurrentActivityThread = env->GetStaticObjectField(activityThreadClass, sCurrentActivityThreadField);
jfieldID mInitialApplicationField = env->GetFieldID(activityThreadClass, "mInitialApplication", "Landroid/app/Application;");
jobject mInitialApplication = env->GetObjectField(sCurrentActivityThread, mInitialApplicationField);
jclass contextClass = env->FindClass("android/content/Context");
jmethodID getSystemServiceMethod = env->GetMethodID(contextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
jstring clipboardStr = env->NewStringUTF("clipboard");
jobject clipboardManager = env->CallObjectMethod(mInitialApplication, getSystemServiceMethod, clipboardStr);
env->DeleteLocalRef(clipboardStr);
jclass clipboardManagerClass = env->FindClass("android/content/ClipboardManager");
jmethodID getTextMethod = env->GetMethodID(clipboardManagerClass, "getText", "()Ljava/lang/CharSequence;");
jclass charSequenceClass = env->FindClass("java/lang/CharSequence");
jmethodID toStringMethod = env->GetMethodID(charSequenceClass, "toString", "()Ljava/lang/String;");
jobject text = env->CallObjectMethod(clipboardManager, getTextMethod);
if (text) {
jstring jstr = (jstring)env->CallObjectMethod(text, toStringMethod);
const char* cstr = env->GetStringUTFChars(jstr, nullptr);
result = cstr;
env->ReleaseStringUTFChars(jstr, cstr);
env->DeleteLocalRef(jstr);
env->DeleteLocalRef(text);
}

env->DeleteLocalRef(charSequenceClass);
env->DeleteLocalRef(clipboardManagerClass);
env->DeleteLocalRef(clipboardManager);
env->DeleteLocalRef(contextClass);
env->DeleteLocalRef(mInitialApplication);
env->DeleteLocalRef(sCurrentActivityThread);
env->DeleteLocalRef(activityThreadClass);

jvm->DetachCurrentThread();

return result;
}

std::string msg;
static char Username[128];
static char Password[128];
bool isLogin = false;
bool loginn = false;
bool showLoginScreen = true;
bool isLoginSuccess = false;
static bool remember;
char username[128] = "";
char password[128] = "";

std::string Teste(JavaVM *vm, const char *user, const char *pass) {
if (!vm) return SO("FATAL ERROR");
JNIEnv *env = nullptr;
vm->AttachCurrentThread(&env, 0);
char params[1024];
const char *isURL = O("https://leomodzvipadmin.x10.mx/mod/CheckLogin.php?user=%s&pass=%s&uid=%s");
auto object = getJNIContext(env);
std::string hwid = user;
hwid += GetAndroidID(env, object);
hwid += GetDeviceModel(env);
hwid += GetDeviceBrand(env);
std::string UUID = GetDeviceUniqueIdentifier(env, hwid.c_str());

snprintf(params, sizeof(params), isURL, user, pass,UUID.c_str());
const char *Request = getRequestURL(env,params);

vm->DetachCurrentThread();

if(strcmp(isURL, OBFUSCATE("https://leomodzvipadmin.x10.mx/mod/CheckLogin.php?user=%s&pass=%s&uid=%s")) != 0) {
return O("Unable to get server information...");
}

std::string FindUser = (user);
if (FindUser.find(SO("*")) != std::string::npos)
{
return O("Unable to get server information...");
}
if (FindUser.find(SO("'")) != std::string::npos)
{
return O("Unable to get server information...");
}

std::string FindURL = (isURL);
std::string FindParams = (params);
if (FindURL.find(SO("https://leomodzvipadmin.x10.mx")) != std::string::npos && FindParams.find(SO("https://leomodzvipadmin.x10.mx/mod/")) != std::string::npos)
{
if(Request != nullptr)
{
std::string request = (Request);
if (request.find(UUID) != std::string::npos)
{
if(strcmp(Request,UUID.c_str()) != 0)
{
return O("Unable to get server information...");
}
return O("Init");
}
else if(request.find("ﾠ") != std::string::npos)
{
return O("Incorrect username or password...");
}
else
{
return request.c_str();
}
} else {
return O("Unable to get server information...");
}
}
else
{
return O("Unable to get server information...");
}
}

void LoginThread2(const std::string &username, const std::string &password) {
isLogin = true;
msg = Teste(jvm,username.c_str(),password.c_str());
isLogin = false;
}

void ShowLogin2() {

if (isLogin) {
ImVec2 screen_size = ImGui::GetIO().DisplaySize;
ImVec2 top_left_pos = ImVec2((screen_size.x - gsButtonWidth) * 0.5f, (screen_size.y - gsButtonHeight) * 0.5f);
ImGui::SetNextWindowSize(ImVec2(gsButtonWidth, gsButtonHeight), ImGuiCond_Always);
ImGui::SetNextWindowPos(top_left_pos, ImGuiCond_Always);
ImGui::Begin(OBFUSCATE("Bottaaonesp"), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);

ImVec4 invisibleColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
ImDrawList* draw_list = ImGui::GetWindowDrawList();
ImVec2 rect_min = ImGui::GetWindowPos();
ImVec2 rect_max = ImVec2(rect_min.x + gsButtonWidth, rect_min.y + gsButtonHeight);
draw_list->AddRectFilled(rect_min, rect_max, ImGui::GetColorU32(invisibleColor));
draw_list->AddRect(rect_min, rect_max, ImGui::GetColorU32(invisibleColor), 10.0f, ImDrawFlags_RoundCornersAll, borderWidth);

ImGui::SetWindowFontScale(textScale);
ImVec2 text_size = ImGui::CalcTextSize(OBFUSCATE("GS MODZ ON"));
ImVec2 text_pos = ImVec2((gsButtonWidth - text_size.x) * 0.5f, (gsButtonHeight - text_size.y) * 0.5f);
ImGui::SetCursorPos(text_pos);
ImGui::TextColored(invisibleColor, OBFUSCATE("GS MODZ ON"));
ImGui::SetWindowFontScale(1.4f);

if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) {
clickStartTime3 = ImGui::GetTime();
} else if (ImGui::IsMouseReleased(0)) {
if (clickStartTime3 != -1.0f && (ImGui::GetTime() - clickStartTime3 >= 0.1f)) {
isLogin = false;
showLoginScreen = true;
}
clickStartTime3 = -1.0f;
}

if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) {
isDragging3 = true;
}

if (isDragging3 && ImGui::IsMouseDown(0)) {
ImVec2 mouse_pos = ImGui::GetMousePos();
gsButtonPos3 = ImVec2(mouse_pos.x - gsButtonWidth * 0.5f, mouse_pos.y - gsButtonHeight * 0.5f);
}

if (ImGui::IsMouseReleased(0)) {
isDragging3 = false;
}

ImGui::End();
}

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    const ImVec4 purple_custom = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    const ImVec4 black_solid(0.00f, 0.00f, 0.00f, 1.00f);

    colors[ImGuiCol_WindowBg]        = black_solid;
    colors[ImGuiCol_ChildBg]         = black_solid;
    colors[ImGuiCol_TitleBg]         = purple_custom;
    colors[ImGuiCol_TitleBgActive]   = purple_custom;
    colors[ImGuiCol_TitleBgCollapsed]= purple_custom;

    colors[ImGuiCol_Button]          = purple_custom;
    colors[ImGuiCol_ButtonHovered]   = purple_custom;
    colors[ImGuiCol_ButtonActive]    = purple_custom;

    colors[ImGuiCol_Header]          = purple_custom;
    colors[ImGuiCol_HeaderHovered]   = purple_custom;
    colors[ImGuiCol_HeaderActive]    = purple_custom;

    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]  = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_FrameBgActive]   = purple_custom;

    colors[ImGuiCol_SliderGrab]      = purple_custom;
    colors[ImGuiCol_SliderGrabActive]= purple_custom;

    colors[ImGuiCol_CheckMark]       = purple_custom;

    colors[ImGuiCol_Border]       = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize  = 0.0f;
    style.TabBorderSize    = 0.0f;

    style.FrameRounding = 8.0f;
    style.GrabRounding = 12.0f;
    style.WindowRounding = 10.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.TabRounding = 6.0f;
    style.GrabMinSize = 15.0f;

    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.DisplaySafeAreaPadding = ImVec2(4, 4);
    style.FramePadding = ImVec2(10, 6);
    style.ItemSpacing = ImVec2(8, 4);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 20.0f;
    
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    
if (showLoginScreen) {
            if (ImGui::Begin(OBFUSCATE("SH4DE CLIENT LOGIN"), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
            
            if (loginn) {
            ImGui::CenteredText(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), O("Logging in..."));
            } else {
            
            ImGui::InputTextWithHint("##user", "Username", Username, 128, ImGuiInputTextFlags_CharsNoBlank);
            if (ImGui::Button(O("Paste Username"), {ImGui::GetContentRegionAvail().x , 0})) {
            strcpy(Username, getClipboard().c_str());
            }
            
            ImGui::Separator();
            
            ImGui::InputTextWithHint("##pw", "Password", Password, 128, ImGuiInputTextFlags_CharsNoBlank);
            if (ImGui::Button(O("Paste Password"), {ImGui::GetContentRegionAvail().x , 0})) {
            strcpy(Password, getClipboard().c_str());
            }
            
            ImGui::Separator();
            ImGui::Separator();
            ImGui::Separator();
            
            if (ImGui::Button(O("Login"), {ImGui::GetContentRegionAvail().x, 0})) {
            std::thread login_thread(LoginThread2, std::string(Username), std::string(Password));
            login_thread.detach();
            }
            
            ImGui::Separator();
            
            if (!(strcmp(msg.c_str(), OBFUSCATE("Init")))) {
                showLoginScreen = false;
                loginn = false;
                isLoginSuccess = true;
            } else {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", msg.c_str());
            }
        }
                    
        ImGui::End();
    }
 ImGui::PopStyleColor(2);
}

if (isLoginSuccess) {

}
}

