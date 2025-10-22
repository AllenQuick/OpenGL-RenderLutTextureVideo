//
// Created by admin on 2025/8/6.
//

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <android/native_window_jni.h>
#include <GLES3/gl3.h>
#include "RenderVideo.h"
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>


RenderVideo::RenderVideo() {
    mMediaExtractor = AMediaExtractor_new();
}


int32_t videoWidth,videoHeight,surfaceWidth,surfaceHeight,videoStride;
void RenderVideo::initTexture(){
    glGenTextures(1, &texY);
    glBindTexture(GL_TEXTURE_2D, texY);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, videoWidth, videoHeight, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &texU);
    glBindTexture(GL_TEXTURE_2D, texU);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, videoWidth/2, videoHeight/2, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &texV);
    glBindTexture(GL_TEXTURE_2D, texV);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, videoWidth/2, videoHeight/2, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    glGenFramebuffers(1, &videoFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, videoFbo);

    GLuint fboTexture;
    // 创建纹理作为 FBO 渲染目标
    glGenTextures(1, &fboTexture);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, videoWidth, videoHeight, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGE("FBO not complete");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    lutTex = loadCubeLUT("/data/data/com.zt.opengles_redenerer/cache/sony_1_cube.cube", lutSize);


    LOGI("纹理创建成功%d  %d",videoWidth,videoHeight);
}

void RenderVideo::openDataFile() {
    int fd = open("/data/data/com.zt.opengles_redenerer/cache/test.mp4",O_RDONLY);
    if (fd < 0) {
        LOGE("打开失败: %s", strerror(errno));
        return;
    }
    off64_t start = 0;
    off64_t videoLength = 0;
    videoLength = lseek(fd, 0, SEEK_END);
    if (videoLength <= 0) {
        LOGE("文件大小异常: %lld", videoLength);
        close(fd);
        return;
    }
    LOGE("文件长度%lld",videoLength);
    lseek(fd, 0, SEEK_SET);
    AMediaExtractor_setDataSourceFd(mMediaExtractor,fd,start,videoLength);

    chooseVideoTrack();
//    mNativeWindow = ANativeWindow_fromSurface(env,surface);
    media_status_t status = AMediaCodec_configure(mMediaCodec, format, nullptr,
                                                  nullptr,0);

    initEGL();

}

void RenderVideo::checkAndActivateSurface(JNIEnv *env, jobject surface) {
    // 获取 ANativeWindow
    ANativeWindow* newNativeWindow = ANativeWindow_fromSurface(env, surface);

    // 检查 ANativeWindow 是否有效
    if (!newNativeWindow) {
        LOGE("Failed to get a valid native window from surface");
        return;
    }


    // 检查 Surface 大小
    surfaceWidth = ANativeWindow_getWidth(newNativeWindow);
    surfaceHeight = ANativeWindow_getHeight(newNativeWindow);

    if (surfaceWidth <= 0 || surfaceHeight <= 0) {
        LOGE("Surface size is invalid, skipping rendering");
        return;
    }

    // 如果 Surface 或 EGLSurface 无效，重新初始化
    // 释放旧的 NativeWindow
    if (mNativeWindow != nullptr) {
        LOGE("释放nativeWindow");
        ANativeWindow_release(mNativeWindow);
    }

    if (eglSurface != EGL_NO_SURFACE) {
        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);  // Unbind EGLSurface
        eglDestroySurface(eglDisplay, eglSurface);
        eglSurface = EGL_NO_SURFACE;
        LOGE("销毁surface");
    }

    // 更新 NativeWindow
    mNativeWindow = newNativeWindow;


    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, mNativeWindow, nullptr);
    if (eglSurface == EGL_NO_SURFACE) {
        LOGE("Failed to create EGL surface");
        EGLint eglError = eglGetError();
        LOGE("Failed to make EGL context current, error code: 0x%x", eglError);
        return;
    }

    // 重新绑定 EGLSurface 和 EGLContext
    if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
        LOGE("Failed to make EGL context current");
        return;
    }




    LOGI("Surface reactivated successfully");

}


void RenderVideo::render(JNIEnv *env, jobject surface) {
    if(isRunning)
        return;
    isRunning = true;
    checkAndActivateSurface(env,surface);
    AMediaCodec_start(mMediaCodec);
    LOGE("render");
    if (ANativeWindow_getWidth(mNativeWindow) <= 0 || ANativeWindow_getHeight(mNativeWindow) <= 0) {
        LOGE("Surface is invalid, skip rendering");
        return;
    }
    while (isRunning) {
        ssize_t inputIndex = AMediaCodec_dequeueInputBuffer(mMediaCodec, 10000);
        if (inputIndex >= 0) {
            size_t bufSize;
            uint8_t* buf = AMediaCodec_getInputBuffer(mMediaCodec, inputIndex, &bufSize);

            ssize_t sampleSize = AMediaExtractor_readSampleData(mMediaExtractor, buf, bufSize);
            if (sampleSize > 0) {
                AMediaCodec_queueInputBuffer(mMediaCodec, inputIndex, 0, sampleSize,
                                             AMediaExtractor_getSampleTime(mMediaExtractor), 0);
                AMediaExtractor_advance(mMediaExtractor);
            } else {
                AMediaCodec_queueInputBuffer(mMediaCodec, inputIndex, 0, 0, 0,
                                             AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
                isRunning = false;
            }
        }

        AMediaCodecBufferInfo info;
        ssize_t outputIndex = AMediaCodec_dequeueOutputBuffer(mMediaCodec, &info, 10000);
        if (outputIndex >= 0) {
            //使用AMediaCodec渲染到surface时使用一下注释
//            AMediaCodec_releaseOutputBuffer(mMediaCodec, outputIndex, true); // true -> 渲染到 Surface
            size_t bufSize;
            uint8_t* outputBuf = AMediaCodec_getOutputBuffer(mMediaCodec, outputIndex, &bufSize);
            // ✅ 拿到帧数据，上传到 OpenGL
            if(!isRunning)
                totalTimeStripe = info.presentationTimeUs;
            renderFrameToTexture(outputBuf,&info.presentationTimeUs);
            AMediaCodec_releaseOutputBuffer(mMediaCodec, outputIndex, false);
        } else if (outputIndex == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
            // 可读取新格式（宽高）
            AMediaFormat* newFormat = AMediaCodec_getOutputFormat(mMediaCodec);
            AMediaFormat_getInt32(newFormat, AMEDIAFORMAT_KEY_WIDTH, &videoWidth);
            AMediaFormat_getInt32(newFormat, AMEDIAFORMAT_KEY_HEIGHT, &videoHeight);
            AMediaFormat_getInt32(format, "stride", &videoStride);
            int32_t colorFormat = -1;
            AMediaFormat_getInt32(newFormat, AMEDIAFORMAT_KEY_COLOR_FORMAT, &colorFormat);
            LOGI("format:%d   videoStride:%d",colorFormat,videoStride);
            initProgram();

        }
//        sleep(1);
    }
    AMediaCodec_flush(mMediaCodec);
    AMediaExtractor_seekTo(mMediaExtractor, 0, AMEDIAEXTRACTOR_SEEK_CLOSEST_SYNC);

    LOGI("视频播放结束");
}


void renderFrame(){

}

void RenderVideo::renderFrameToTexture(uint8_t *yuvData,const int64_t *currentTimeStripe) {
    /*glBindFramebuffer(GL_FRAMEBUFFER,videoFbo);
    glViewport(0,0,surfaceWidth,surfaceHeight);
    renderYUVAndLutFrame(yuvData,currentTimeStripe);

*/
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glViewport(0, 0, surfaceWidth, surfaceHeight);

    renderYUVAndLutFrame(yuvData,currentTimeStripe);

    eglSwapBuffers(eglDisplay,eglSurface);

}

void RenderVideo::renderYUVAndLutFrame(uint8_t *yuvData, const int64_t *currentTimeStripe) {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(renderProgram);
    LOGE("rendFrameToTexture videoWidth:%d videoHeight:%d",videoWidth,videoHeight);
    // 计算分量大小
    int ySize = videoWidth * videoHeight;
    int uvWidth = videoWidth / 2;
    int uvHeight = videoHeight / 2;
    int uSize = uvWidth * uvHeight;

    uint8_t* yPlane = yuvData;
    uint8_t* uPlane = yuvData + ySize;
    uint8_t* vPlane = yuvData + ySize + uSize;

    LOGE("Y: %d %d %d %d", yPlane[0], yPlane[1], yPlane[2], yPlane[3]);
    LOGE("U: %d %d %d %d", uPlane[0], uPlane[1], uPlane[2], uPlane[3]);
    LOGE("V: %d %d %d %d", vPlane[0], vPlane[1], vPlane[2], vPlane[3]);
    // 上传纹理数据
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texY);
    glUniform1i(glGetUniformLocation(renderProgram, "texY"), 0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, videoWidth, videoHeight,
                    GL_RED, GL_UNSIGNED_BYTE, yPlane);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texU);
    glUniform1i(glGetUniformLocation(renderProgram, "texU"), 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, uvWidth, uvHeight,
                    GL_RED, GL_UNSIGNED_BYTE, uPlane);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texV);
    glUniform1i(glGetUniformLocation(renderProgram, "texV"), 2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, uvWidth, uvHeight,
                    GL_RED, GL_UNSIGNED_BYTE, vPlane);

    LOGI("总长：%lld 目前：%lld",totalTimeStripe,*currentTimeStripe);
    // LUT
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_3D, lutTex);
    glUniform1i(glGetUniformLocation(renderProgram, "lutTex"), 3);

    glUniform1f(glGetUniformLocation(renderProgram, "lutSize"), (float)lutSize);
    glUniform1f(glGetUniformLocation(renderProgram,"split"),(float)*currentTimeStripe/(float)totalTimeStripe);

    // 清除并绘制

    glBindVertexArray(vao);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        LOGE("GL ERROR: %x ,lutsize:%d", err,lutSize);
    }
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


void RenderVideo::initProgram() {
    // 设置当前线程egl上下文
    if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
        LOGI("Failed to make EGL context current");
        return;
    }
    LOGE("着色器代码加载");
    //绑定着色器到renderProgram renderProgram可以作为着色器程序 下面三行代码
    GLint vertexS = compileShader(GL_VERTEX_SHADER, vertexShaders);
    GLint fragmentS = compileShader(GL_FRAGMENT_SHADER, fragmentShaders);
    linkProgram(vertexS,fragmentS,&renderProgram);
    LOGE("program %d", renderProgram);
    //vao:vertex array object 顶点数组区对象
    glGenVertexArrays(1, &vao);
    //vbo:vertex buffer object 顶点缓冲区对象
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0); // aPosition

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1); // aTexCoord

    glBindVertexArray(0);

    initTexture();
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        LOGE("GL ERROR: %x", err);
    }
}

void RenderVideo::initEGL() {
    // 获取 EGLDisplay
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisplay == EGL_NO_DISPLAY) {
        LOGI("Failed to get EGL display");
        return;
    }
//    test();

    int major, minor;
    if (!eglInitialize(eglDisplay, &major, &minor)) {
        LOGI("Failed to initialize EGL");
        return;
    }

    // 配置 EGL
    const EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
    };
    EGLConfig configs[1];
    EGLint numConfigs;
    if (!eglChooseConfig(eglDisplay, attribs, configs, 1, &numConfigs)) {
        LOGI("Failed to choose EGL config");
        return;
    }
    eglConfig = configs[0];

    // 创建 EGLContext（OpenGL ES 3.0）
    const EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 3,
            EGL_NONE
    };
    eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs);
    if (eglContext == EGL_NO_CONTEXT) {
        LOGI("Failed to create EGL context");
        return;
    }

//    glClearColor(0.0, 0.0, 0.0, 1.0);


    LOGI("EGL环境创建成功");
}

GLuint RenderVideo::loadCubeLUT(const char *cubePath, int& lutSizeOut) {
    FILE* fp = fopen(cubePath, "r");
    if (!fp) {
        __android_log_print(ANDROID_LOG_ERROR, "LUT", "Failed to open .cube file: %s", cubePath);
        return 0;
    }

    char line[256];
    int lutSize = 0;
    std::vector<float> data;

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n')
            continue;

        std::string str(line);
        if (str.find("LUT_3D_SIZE") != std::string::npos) {
            sscanf(line, "LUT_3D_SIZE %d", &lutSize);
            continue;
        }

        float r, g, b;
        if (sscanf(line, "%f %f %f", &r, &g, &b) == 3) {
            data.push_back(r);
            data.push_back(g);
            data.push_back(b);
        }
    }

    fclose(fp);

    if (lutSize == 0) {
        __android_log_print(ANDROID_LOG_ERROR, "LUT", "Invalid .cube file: missing LUT_3D_SIZE");
        return 0;
    }

    if ((int)data.size() != lutSize * lutSize * lutSize * 3) {
        __android_log_print(ANDROID_LOG_ERROR, "LUT", "LUT data size mismatch, got %d floats", (int)data.size());
        return 0;
    }

    GLuint tex3D;
    glGenTextures(1, &tex3D);
    glBindTexture(GL_TEXTURE_3D, tex3D);

    glTexImage3D(GL_TEXTURE_3D,
                 0,
                 GL_RGB16F,
                 lutSize, lutSize, lutSize,
                 0,
                 GL_RGB,
                 GL_FLOAT,
                 data.data());

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    lutSizeOut = lutSize;
    return tex3D;
}








