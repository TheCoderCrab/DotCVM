#include <jni.h>
#include <j_app_main.h>
#include <app_main.h>
#include <iostream>
#include <main.h>
#include <log.h>

static jmethodID s_SetPixelMethod;
static jmethodID s_GetPixelMethod;
static jmethodID s_RefreshScrMethod;
static jmethodID s_RequestCloseMethod;
static JNIEnv* s_JEnv;
static jclass s_AppMainclass;

JNIEXPORT jint JNICALL Java_AppMain__1init(JNIEnv * env, jclass clazz) // Native code pre-entry point
{
    s_SetPixelMethod = env->GetStaticMethodID(clazz, "setPixel", "(III)V");
    s_RefreshScrMethod = env->GetStaticMethodID(clazz, "refreshScr", "()V");
    s_RequestCloseMethod = env->GetStaticMethodID(clazz, "requestClose", "()V");
    s_GetPixelMethod = env->GetStaticMethodID(clazz, "getPixel", "(II)I");

    if(s_SetPixelMethod == nullptr) // setPixel(III)V not found
    {
        std::cout << "setPixel(III)V not found" << std::endl;
        return 1;
    }
    if(s_RefreshScrMethod == nullptr) // refreshScr()V not found
    {
        std::cout << "refreshScr()V not found" << std::endl;
        return 2;
    }
    if(s_RequestCloseMethod == nullptr) // requestClose()V not found
    {
        std::cout << "requestClose()V not found" << std::endl;
        return 3;
    }
    if(s_GetPixelMethod == nullptr) // getPixel(II)I not found
    {
        std::cout << "getPixel(II)I not found" << std::endl;
        return 4;
    }

    s_JEnv = env;
    s_AppMainclass = clazz;
    return 0;
}

JNIEXPORT void JNICALL Java_AppMain__1run(JNIEnv*, jclass, jobjectArray) // Native code run
{
    cmain(); // Calling the real main
}

JNIEXPORT void JNICALL Java_AppMain__1update(JNIEnv *, jclass)
{
    update(); // Calling the real update
}

JNIEXPORT void JNICALL Java_AppMain__1exit(JNIEnv*, jclass)
{
    exit(); // Calling the real exit
}
  

void setPixel(int32_t x, int32_t y, int32_t color)
{
    s_JEnv->CallStaticVoidMethod(s_AppMainclass, s_SetPixelMethod, x, y, color);
}

int32_t getPixel(int32_t x, int32_t y)
{
    return s_JEnv->CallStaticIntMethod(s_AppMainclass, s_GetPixelMethod, x, y);
}

void refreshScr()
{
    s_JEnv->CallStaticVoidMethod(s_AppMainclass, s_RefreshScrMethod);
}

void requestClose()
{
    s_JEnv->CallStaticVoidMethod(s_AppMainclass, s_RequestCloseMethod);
}
