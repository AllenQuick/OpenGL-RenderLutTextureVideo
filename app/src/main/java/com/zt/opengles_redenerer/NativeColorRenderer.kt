package com.zt.opengles_redenerer

import android.opengl.GLSurfaceView
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

/**
 * @ClassName NativeColorRenderer
 * @Description TODO
 * @Author MYM
 * @Date 2025/7/14 15:50
 */
class NativeColorRenderer(private val color : Int) : GLSurfaceView.Renderer {
    companion object{
        init {
            System.loadLibrary("native-color")
        }
    }

    external fun surfaceCreated(color: Int)

    external fun surfaceChanged(width: Int, height: Int)

    external fun onDrawFrame()

    override fun onDrawFrame(p0: GL10?) {
        onDrawFrame()
    }

    override fun onSurfaceChanged(
        p0: GL10?,
        width: Int,
        height: Int
    ) {
        surfaceChanged(width, height)
    }

    override fun onSurfaceCreated(
        p0: GL10?,
        p1: EGLConfig?
    ) {
        surfaceCreated(color)
    }
}