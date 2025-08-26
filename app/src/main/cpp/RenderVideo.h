//
// Created by admin on 2025/8/6.
//

#ifndef OPENGLES_REDENERER_RENDERVIDEO_H
#define OPENGLES_REDENERER_RENDERVIDEO_H

#include <media/NdkMediaCodec.h>
#include <media/NdkMediaExtractor.h>
#include "log.h"
#include <iostream>
#include <vector>
#include <EGL/egl.h>

class RenderVideo {
public:
    AMediaFormat* format;
    const char* mime;
    const char* vertexShaders = R"(
            #version 300 es
            layout(location = 0) in vec4 aPosition;
            layout(location = 1) in vec2 aTexCoord;
            out vec2 vTexCoord;
            void main() {
                gl_Position = aPosition;
                vTexCoord = aTexCoord;
            }

)";
    const char* fragmentShaders = R"(
            #version 300 es
            precision mediump float;
            precision mediump sampler3D;

            uniform sampler2D texY;
            uniform sampler2D texU;
            uniform sampler2D texV;
            uniform sampler3D lutTex;
            uniform float lutSize;

            in vec2 vTexCoord;
            out vec4 fragColor;

            void main() {
                float y = texture(texY, vTexCoord).r;
                float u = texture(texU, vTexCoord).r - 0.5;
                float v = texture(texV, vTexCoord).r - 0.5;

                // YUV420 -> RGB (BT.709)
                vec3 rgb;
                rgb.r = y + 1.5748 * v;
                rgb.g = y - 0.183 * u - 0.614 * v;
                rgb.b = y + 1.8556 * u;

                rgb = clamp(rgb,0.01,0.99);

                // LUT 采样
                vec3 uvw = rgb * (lutSize - 1.0) / lutSize + 0.5 / lutSize;
                vec3 mapped = texture(lutTex, uvw).rgb;
                if(vTexCoord.x < 0.5)
                    fragColor = vec4(rgb,1.0);
                else
                    fragColor = vec4(mapped, 1.0);
            }
)";
    // x, y, u, v
    GLfloat vertexData[16] = {
            // 左下角
            -1.0f, -1.0f,  0.0f, 1.0f,
            // 右下角
            1.0f, -1.0f,  1.0f, 1.0f,
            // 左上角
            -1.0f,  1.0f,  0.0f, 0.0f,
            // 右上角
            1.0f,  1.0f,  1.0f, 0.0f
    };
    GLuint vao,vbo,texY,texU,texV,renderProgram,lutTex;
    int lutSize;
    RenderVideo();
    void render();
    GLuint loadCubeLUT(const char* cubePath,int& lutSizeOut);

    void bindSurface(JNIEnv *env, jobject surface);

private:
    AMediaExtractor* mMediaExtractor;
    AMediaCodec* mMediaCodec;
    ANativeWindow* mNativeWindow;
    EGLDisplay eglDisplay;
    EGLContext eglContext;
    EGLSurface eglSurface;
    EGLConfig eglConfig;
    bool isRunning;
    void renderFrameToTexture(uint8_t* yuvData);
    int chooseVideoTrack(){
        int numTracks = AMediaExtractor_getTrackCount(mMediaExtractor);
        for (int i = 0; i < numTracks; ++i) {
            format = AMediaExtractor_getTrackFormat(mMediaExtractor, i);
            if (AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mime) &&
                strncmp(mime, "video/", 6) == 0) {
                AMediaExtractor_selectTrack(mMediaExtractor, i);
                mMediaCodec = AMediaCodec_createDecoderByType(mime);
                return i;
            }
        }
        return 0;
    };
    void initProgram();
    void initTexture();
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
                GLint logLen = 0;
                glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLen);
                if (logLen > 1) {
                    std::vector<char> log(logLen);
                    glGetShaderInfoLog(shaderId, logLen, nullptr, log.data());
                    printf("Shader compile error:\n%s\n", log.data());
                }
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
        LOGE("create program %d", programId);
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

    void initEGL(JNIEnv *env, jobject surface);

};


#endif //OPENGLES_REDENERER_RENDERVIDEO_H
