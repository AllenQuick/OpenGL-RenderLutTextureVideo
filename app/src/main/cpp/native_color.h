//
// Created by admin on 2025/7/14.
//
#include <jni.h>

#ifndef OPENGLES_REDENERER_NATIVE_COLOR_H
#define OPENGLES_REDENERER_NATIVE_COLOR_H


#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL surfaceCreated(JNIEnv *, jobject, jint);

JNIEXPORT void JNICALL surfaceChanged(JNIEnv *, jobject, jint, jint);

JNIEXPORT void JNICALL onDrawFrame(JNIEnv *, jobject);

#ifdef __cplusplus
}

#endif
#endif //OPENGLES_REDENERER_NATIVE_COLOR_H
