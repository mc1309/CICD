/**
 * NativeTemplate.cpp
 *
 * Thin JNI bridge between the Java/Android layer and Renderer, which owns
 * the Fan Model. All GL logic lives in Scene/.
 *
 * Native method signatures match MainActivity.java:
 *   package com.example.helloworldandroid
 *   class   MainActivity
 *
 * Touch events arrive here already marshalled through
 * GLSurfaceView.queueEvent(...) on the Java side, so every native call
 * below executes safely on the GL thread.
 */
#define LOG_TAG "GLPIFrameworkIntroNative"
#include <jni.h>
#include <android/asset_manager_jni.h>
#include "Platform.h"
#include "Renderer.h"   // Renderer.h -> Model.h only; Renderer.cpp knows Fan
// ---------------------------------------------------------------------------
// JNI entry points
// ---------------------------------------------------------------------------
extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeInit(
        JNIEnv* env, jobject /*thiz*/, jobject assetManager)
{
    LOGI("nativeInit called");
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    Renderer::Instance().setAssetManager(mgr);
    return Renderer::Instance().initializeRenderer() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeResize(
        JNIEnv* /*env*/, jobject /*thiz*/, jint width, jint height)
{
Renderer::Instance().resize(width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeRender(
        JNIEnv* /*env*/, jobject /*thiz*/)
{
Renderer::Instance().render();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeTouchDown(
        JNIEnv* /*env*/, jobject /*thiz*/, jfloat x, jfloat y)
{
Renderer::Instance().touchDown(x, y);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeTouchMove(
        JNIEnv* /*env*/, jobject /*thiz*/, jfloat x, jfloat y)
{
Renderer::Instance().touchMove(x, y);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeTouchUp(
        JNIEnv* /*env*/, jobject /*thiz*/)
{
Renderer::Instance().touchUp();
}