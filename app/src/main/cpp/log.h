//
// Created by admin on 2025/7/31.
//

#ifndef OPENGLES_REDENERER_LOG_H
#define OPENGLES_REDENERER_LOG_H

#include <android/log.h>
#define LOG_TAG "EGL_Render"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG, __VA_ARGS__)

#endif //OPENGLES_REDENERER_LOG_H
