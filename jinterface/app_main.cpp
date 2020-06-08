#include <jni.h>
#include <j_app_main.h>
#include <app_main.h>
#include <iostream>
#include <main.h>

static jmethodID s_SetPixelMethod;
static jmethodID s_RefreshScrMethod;
static jmethodID s_RequestCloseMethod;
static JNIEnv* s_JEnv;
static jclass s_AppMainclass;

JNIEXPORT jint JNICALL Java_AppMain__1init(JNIEnv * env, jclass clazz) // Native code pre-entry point
{
    s_SetPixelMethod = env->GetStaticMethodID(clazz, "setPixel", "(III)V");
    s_RefreshScrMethod = env->GetStaticMethodID(clazz, "refreshScr", "()V");
    s_RequestCloseMethod = env->GetStaticMethodID(clazz, "requestClose", "()V");

    if(s_SetPixelMethod == nullptr) // setPixel(III)V not found
    {
        std::cout << "setPixel(III)V not found" << std::endl;
        return 1;
    }
    if(s_RefreshScrMethod == nullptr)
    {
        std::cout << "refreshScr()V not found" << std::endl;
        return 2;
    }
    if(s_RequestCloseMethod == nullptr)
    {
        std::cout << "requestClose()V not found" << std::endl;
        return 3;
    }

    s_JEnv = env;
    s_AppMainclass = clazz;
    return 0;
}

JNIEXPORT void JNICALL Java_AppMain__1run(JNIEnv* env, jclass clazz, jobjectArray args) // Native code entry point
{
    cmain(); // Calling the real entry point
}

JNIEXPORT void JNICALL Java_AppMain__1exit(JNIEnv * env, jclass lazz)
{
    exit(); // Calling the real exit function
}
  

void setPixel(int x, int y, int color)
{
    s_JEnv->CallStaticVoidMethod(s_AppMainclass, s_SetPixelMethod, x, y, color);
}

void refreshScr()
{
    s_JEnv->CallStaticVoidMethod(s_AppMainclass, s_RefreshScrMethod);
}

void requestClose()
{
    s_JEnv->CallStaticVoidMethod(s_AppMainclass, s_RequestCloseMethod);
}
