//
// Created by admin on 2025/7/15.
//
#include <jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "native_color_lib.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "coroutine.h"
#include "log.h"
#include "RenderVideo.h"

/**
 * 动态注册
 */
JNINativeMethod methods[] = {
        {"initEGL","(Landroid/view/Surface;)Z",(void *)initEGL},
        {"renderWithOutGLSurface","(II)V",(void *)renderWithOutGLSurface},
        {"removeSurface","()V",(void *)removeSurface},
        {"cleanupEGL","()V", (void *)cleanupEGL},
        {"surfaceCreated", "(I)V",  (void *) surfaceCreated},
        {"surfaceChanged", "(II)V", (void *) surfaceChanged},
        {"onDrawFrame",    "()V",   (void *) onDrawFrame},
        {"initFileData","()V",(void *)initRenderVideo},
        {"renderSurface","(Landroid/view/Surface;)V",(void *)renderSurface}
};



GLint compileShader(GLenum i, const char *shaders);

GLint linkProgram(GLint shader, GLint shader1, GLuint* program);

/**
 * 动态注册
 * @param env
 * @return
 */
jint registerNativeMethod(JNIEnv *env) {
    jclass cl = env->FindClass("com/zt/opengles_redenerer/MainActivity");
    if ((env->RegisterNatives(cl, methods, sizeof(methods) / sizeof(methods[0]))) < 0) {
        return -1;
    }
    return 0;
}

/**
 * 加载默认回调
 * @param vm
 * @param reserved
 * @return
 */
jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    //注册方法
    if (registerNativeMethod(env) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_6;
}

void OrthoProjectionMatrix(float* matrix, float left, float right, float bottom, float top, float near, float far) {
    matrix[0] = 2.0f / (right - left);
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = -(right + left) / (right - left);

    matrix[4] = 0.0f;
    matrix[5] = 2.0f / (top - bottom);
    matrix[6] = 0.0f;
    matrix[7] = -(top + bottom) / (top - bottom);

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = -2.0f / (far - near);
    matrix[11] = -(far + near) / (far - near);

    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
}


EGLDisplay eglDisplay;
EGLContext eglContext;
EGLSurface eglSurface;
EGLConfig eglConfig;
// 获取 Surface 对象
ANativeWindow *nativeWindow;

jboolean initEGL(JNIEnv *env, jobject obj, jobject surface){
    // 获取 EGLDisplay
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisplay == EGL_NO_DISPLAY) {
        LOGI("Failed to get EGL display");
        return false;
    }
//    test();

    int major, minor;
    if (!eglInitialize(eglDisplay, &major, &minor)) {
        LOGI("Failed to initialize EGL");
        return false;
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
        return false;
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
        return false;
    }


    nativeWindow = ANativeWindow_fromSurface(env, surface);

    // 创建 EGLSurface
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, nativeWindow, nullptr);
    if (eglSurface == EGL_NO_SURFACE) {
        LOGI("Failed to create EGL surface");
        return false;
    }

    // 设置当前上下文
    if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
        LOGI("Failed to make EGL context current");
        return false;
    }

    // 渲染测试
    LOGI("渲染成功");
    return true;
}

void renderWithOutGLSurface(JNIEnv *env, jobject obj, jint width, jint height) {
    // 清除屏幕
    LOGI("开始绘制");

    // 在这里执行渲染操作，如绘制三角形等
    surfaceCreated(env,obj,-65536);//red
    surfaceChanged(env,obj,width,height);
    LOGI("render宽：%d 高：%d",width,height);
    onDrawFrame(env,obj);
    LOGI("交换后");
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        LOGE("GL ERROR: %x", err);
    }
    // 交换缓冲区，将图像渲染到 SurfaceView 上
//    eglSwapBuffers(eglDisplay, eglSurface);
}


void removeSurface(JNIEnv *, jobject) {
    if (eglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (eglSurface != EGL_NO_SURFACE) {
            eglDestroySurface(eglDisplay, eglSurface);
        }
        eglDisplay = EGL_NO_DISPLAY;
        eglSurface = EGL_NO_SURFACE;
    }
}


void cleanupEGL(JNIEnv *env, jobject obj) {
    if (eglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (eglSurface != EGL_NO_SURFACE) {
            eglDestroySurface(eglDisplay, eglSurface);
        }
    }
    if (eglContext != EGL_NO_CONTEXT) {
        eglDestroyContext(eglDisplay, eglContext);
    }
    eglTerminate(eglDisplay);

}

void surfaceCreated(JNIEnv *env, jobject obj, jint color) {
    // 设置清除颜色
    GLfloat redF = ((color >> 16) & 0xFF) / 255.0f;
    GLfloat greenF = ((color >> 8) & 0xFF) / 255.0f;
    GLfloat blueF = (color & 0xFF) / 255.0f;
    GLfloat alphaF = ((color >> 24) & 0xFF) / 255.0f;
    glClearColor(redF, greenF, blueF, alphaF);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

//    glClear(GL_COLOR_BUFFER_BIT);
    GLint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaders);
    GLint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaders);
    linkProgram(vertexShader,fragmentShader,&triangleProgramId);


    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);


    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, vertex_array.size() * sizeof(GLfloat), vertex_array.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);  // layout(location = 0)
    glEnableVertexAttribArray(0);

// --- 顶点颜色 VBO ---
    glGenBuffers(1, &vboColor);
    glBindBuffer(GL_ARRAY_BUFFER, vboColor);
    glBufferData(GL_ARRAY_BUFFER, color_array.size() * sizeof(GLfloat), color_array.data(), GL_STATIC_DRAW);


    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);  // layout(location = 1)
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    /*glGenVertexArrays(1,&lineVao);
    glBindVertexArray(lineVao);

    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, line_array.size() * sizeof(GLfloat), line_array.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);  // layout(location = 0)
    glEnableVertexAttribArray(0);

// --- 顶点颜色 VBO ---
    glGenBuffers(1, &vboColor);
    glBindBuffer(GL_ARRAY_BUFFER, vboColor);
    glBufferData(GL_ARRAY_BUFFER, color_line.size() * sizeof(GLfloat), color_line.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);  // layout(location = 1)
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);*/



}

void surfaceChanged(JNIEnv *env, jobject obj, jint width, jint height) {
    glViewport(0, 0, width, height);
    LOGI("宽:%d 高:%d",width,height);
    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float) width/(float)height, 0.1f, 100.0f);

// Or, for an ortho camera :
//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

// Camera matrix
    glm::mat4 View = glm::lookAt(
            glm::vec3(4,3,3), // Camera is at (4,3,3), in World Space
            glm::vec3(0,0,0), // and looks at the origin
            glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

// Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 0.7f, 0.7f));
// Our ModelViewProjection : multiplication of our 3 matrices
    mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around
    // Get a handle for our "MVP" uniform
// Only during the initialisation
}

void onDrawFrame(JNIEnv *env, jobject obj) {
    glUseProgram(triangleProgramId);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLuint MatrixID = glGetUniformLocation(triangleProgramId, "MVP");

// Send our transformation to the currently bound shader, in the "MVP" uniform
// This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);


/*
    glBindVertexArray(lineVao);

    glDrawArrays(GL_LINES,0,4);
*/

    glBindVertexArray(vao);


    glDrawArrays(GL_TRIANGLES, 0, 12*3);

    //禁止顶点数组的句柄
    glBindVertexArray(0);
    LOGI("交换前");
    eglSwapBuffers(eglDisplay,eglSurface);
}


void renderSurface(JNIEnv *env, jobject obj, jobject surface) {
    renderVideo.render(env,surface);
}

void initRenderVideo(JNIEnv *env, jobject obj) {
    renderVideo.openDataFile();
}


GLint compileShader(GLenum type,const char* shaderCode){
    //创建一个着色器
    GLint shaderId = glCreateShader(type);
    if (shaderId != 0) {
        //加载到着色器
        glShaderSource(shaderId, 1,&shaderCode, nullptr);
        //编译着色器
        glCompileShader(shaderId);
        //检测状态
        GLint compileStatus[1] = {0};
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, compileStatus);
        if (compileStatus[0] == 0) {
            std::vector<GLchar> infoLog(compileStatus[0]);
            glGetShaderInfoLog(shaderId,compileStatus[0], nullptr,infoLog.data());
            std::cerr << "[Shader Error Log]" << infoLog.data() << std::endl;
            //创建失败
            glDeleteShader(shaderId);
            return 0;
        }
        return shaderId;
    } else {
        //创建失败
        return 0;
    }
}


GLint linkProgram(GLint vertexShaderId, GLint fragmentShaderId, GLuint* program){
    GLint programId = glCreateProgram();
    if (programId != 0) {
        //将顶点着色器加入到程序
        glAttachShader(programId, vertexShaderId);
        //将片元着色器加入到程序中
        glAttachShader(programId, fragmentShaderId);
        //链接着色器程序
        glLinkProgram(programId);
        GLint linkStatus;
        glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
        if (!linkStatus) {
            LOGI("[Program Error Log] 着色器程序链接到程序失败");
            glDeleteProgram(programId);
            return 0;
        }
        *program = programId;
        return programId;
    } else {
        //创建失败
        return 0;
    }
}