package com.zt.opengles_redenerer

import android.icu.number.IntegerWidth
import android.opengl.GLES30
import android.opengl.GLSurfaceView
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.FloatBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10


/**
 * @ClassName SimpleRenderer
 * @Description TODO
 * @Author MYM
 * @Date 2025/7/14 18:59
 */
class SimpleRenderer(private val color : Int) : GLSurfaceView.Renderer {

    companion object{
        init {
            System.loadLibrary("native-color-lib")
        }
    }

    private var vertexBuffer: FloatBuffer
    private val vertexPoints = floatArrayOf(
        0.5f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f,
        0.0f, -0.5f, 0.0f,
        -1.0f, 0.0f, 0.0f
    )
    private var colorBuffer: FloatBuffer
    private val colorArray = floatArrayOf(
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f
    )

    external fun surfaceCreated(color: Int)

    external fun surfaceChanged(width: Int, height: Int)

    external fun onDrawFrame()


    private val vertexShaders = "#version 300 es\n" +
            "layout (location = 0) in vec4 vPosition;\n" +
            "layout (location = 1) in vec4 aColor;\n" +
            "out vec4 vColor;\n" +
            "void main() {\n" +
            "     gl_Position  = vPosition;\n" +
            "     gl_PointSize = 10.0;\n" +
            "     vColor = aColor;\n" +
            "}"
    private val fragmentShaders = "#version 300 es\n" +
            "precision mediump float;\n" +
            "in vec4 vColor;\n" +
            "out vec4 fragColor;\n" +
            "void main() {\n" +
            "     fragColor = vColor;\n" +
            "}\n"

    init {
        vertexBuffer =
            ByteBuffer.allocateDirect(vertexPoints.size * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        vertexBuffer.put(vertexPoints)
        vertexBuffer.position(0)
        colorBuffer = ByteBuffer.allocateDirect(colorArray.size * 4)
            .order(ByteOrder.nativeOrder())
            .asFloatBuffer()
        //传入指定的数据
        colorBuffer.put(colorArray)
        colorBuffer.position(0)
    }



    /**
     * 编译
     *
     * @param type  顶点着色器:GLES30.GL_VERTEX_SHADER
     *              片段着色器:GLES30.GL_FRAGMENT_SHADER
     * @param shaderCode
     * @return
     */
    private fun compileShader(type: Int, shaderCode: String?): Int {
        //创建一个着色器
        val shaderId = GLES30.glCreateShader(type)
        if (shaderId != 0) {
            //加载到着色器
            GLES30.glShaderSource(shaderId, shaderCode)
            //编译着色器
            GLES30.glCompileShader(shaderId)
            //检测状态
            val compileStatus = IntArray(1)
            GLES30.glGetShaderiv(shaderId, GLES30.GL_COMPILE_STATUS, compileStatus, 0)
            if (compileStatus[0] == 0) {
                val logInfo = GLES30.glGetShaderInfoLog(shaderId)
                System.err.println(logInfo)
                //创建失败
                GLES30.glDeleteShader(shaderId)
                return 0
            }
            return shaderId
        } else {
            //创建失败
            return 0
        }
    }

    /**
     * 链接小程序
     *
     * @param vertexShaderId 顶点着色器
     * @param fragmentShaderId 片段着色器
     * @return
     */
    fun linkProgram(vertexShaderId: Int, fragmentShaderId: Int): Int {
        val programId = GLES30.glCreateProgram()
        if (programId != 0) {
            //将顶点着色器加入到程序
            GLES30.glAttachShader(programId, vertexShaderId)
            //将片元着色器加入到程序中
            GLES30.glAttachShader(programId, fragmentShaderId)
            //链接着色器程序
            GLES30.glLinkProgram(programId)
            val linkStatus = IntArray(1)
            GLES30.glGetProgramiv(programId, GLES30.GL_LINK_STATUS, linkStatus, 0)
            if (linkStatus[0] == 0) {
                val logInfo = GLES30.glGetProgramInfoLog(programId)
                System.err.println(logInfo)
                GLES30.glDeleteProgram(programId)
                return 0
            }
            return programId
        } else {
            //创建失败
            return 0
        }
    }


    override fun onDrawFrame(p0: GL10?) {
        onDrawFrame()
        /*GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT)

        //准备坐标数据
        GLES30.glVertexAttribPointer(0, 3, GLES30.GL_FLOAT, false, 0, vertexBuffer)
        //启用顶点的句柄
        GLES30.glEnableVertexAttribArray(0)


        GLES30.glVertexAttribPointer(1, 4, GLES30.GL_FLOAT, false, 0, colorBuffer)
        //绘制三角形颜色
        GLES30.glEnableVertexAttribArray(1)

        GLES30.glDrawArrays(GLES30.GL_TRIANGLE_STRIP, 0, 4)

        //禁止顶点数组的句柄
        GLES30.glDisableVertexAttribArray(0);*/

    }

    override fun onSurfaceChanged(
        p0: GL10?,
        width: Int,
        height: Int
    ) {
        surfaceChanged(width,height)
//        GLES30.glViewport(0,0,width,height)
    }

    override fun onSurfaceCreated(
        p0: GL10?,
        p1: EGLConfig?
    ) {
        surfaceCreated(color)

        /*GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT)
        GLES30.glEnable(GLES30.GL_DEPTH_TEST)
        val vertexShader = compileShader(GLES30.GL_VERTEX_SHADER, vertexShaders)
        val fragmentShader = compileShader(GLES30.GL_FRAGMENT_SHADER, fragmentShaders)
        GLES30.glUseProgram(linkProgram(vertexShader,fragmentShader))*/

    }
}