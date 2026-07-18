#include <jni.h>
#include <string>

extern JavaVM* jvm; // usa a instância global já definida em outro arquivo

int GetBatteryPercent() {
    if (!jvm) return -1;

    JNIEnv* env;
    bool didAttach = false;

    // Garante que estamos em uma thread com acesso ao JNIEnv
    if (jvm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        if (jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return -1;
        didAttach = true;
    }

    // Pega a classe ActivityThread e a activity atual
    jclass activityThread = env->FindClass("android/app/ActivityThread");
    jmethodID currentActivityThread = env->GetStaticMethodID(activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
    jobject activityThreadObj = env->CallStaticObjectMethod(activityThread, currentActivityThread);
    jmethodID getApplication = env->GetMethodID(activityThread, "getApplication", "()Landroid/app/Application;");
    jobject context = env->CallObjectMethod(activityThreadObj, getApplication);

    // Cria o IntentFilter para pegar a bateria
    jclass intentFilterClass = env->FindClass("android/content/IntentFilter");
    jmethodID intentFilterConstructor = env->GetMethodID(intentFilterClass, "<init>", "(Ljava/lang/String;)V");
    jstring actionBatteryChanged = env->NewStringUTF("android.intent.action.BATTERY_CHANGED");
    jobject intentFilter = env->NewObject(intentFilterClass, intentFilterConstructor, actionBatteryChanged);

    // Chama registerReceiver(null, intentFilter)
    jclass contextClass = env->GetObjectClass(context);
    jmethodID registerReceiver = env->GetMethodID(contextClass, "registerReceiver", "(Landroid/content/BroadcastReceiver;Landroid/content/IntentFilter;)Landroid/content/Intent;");
    jobject batteryIntent = env->CallObjectMethod(context, registerReceiver, nullptr, intentFilter);

    // Extrai os dados de nível de bateria e escala
    jclass intentClass = env->GetObjectClass(batteryIntent);
    jmethodID getIntExtra = env->GetMethodID(intentClass, "getIntExtra", "(Ljava/lang/String;I)I");

    jstring levelStr = env->NewStringUTF("level");
    jstring scaleStr = env->NewStringUTF("scale");

    int level = env->CallIntMethod(batteryIntent, getIntExtra, levelStr, -1);
    int scale = env->CallIntMethod(batteryIntent, getIntExtra, scaleStr, -1);

    if (didAttach) jvm->DetachCurrentThread();

    if (level == -1 || scale == -1) return -1;

    return (int)(((float)level / (float)scale) * 100.0f);
}
