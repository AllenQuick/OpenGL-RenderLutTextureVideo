package com.zt.opengles_redenerer

import android.icu.number.IntegerWidth
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import java.lang.ref.WeakReference

/**
 * @ClassName CubeSurface
 * @Description TODO
 * @Author MYM
 * @Date 2025/8/5 19:26
 */
class CubeSurface:SurfaceHolder.Callback {
    private val mWeakReference: WeakReference<MainActivity>
    lateinit var currentSurface: Surface
    private var height: Int = 0
    private var width: Int = 0
    constructor(mainActivity: MainActivity) {
        mWeakReference = WeakReference(mainActivity)
    }

    override fun surfaceCreated(p0: SurfaceHolder) {
        mWeakReference.get()?.let { mainActivity->
            with(mainActivity) {
                /*if (isCube) {
                    val isSuccess = initEGL(p0.surface)
                    Log.e("surfaceCreate", "EGL环境是否创建成功::$isSuccess")
                }*/
                currentSurface = p0.surface
                initRenderVideo(p0.surface)
            }
        }

    }

    override fun surfaceChanged(
        p0: SurfaceHolder,
        p1: Int,
        p2: Int,
        p3: Int
    ) {
        width = p2
        height = p3
//        render()
    }

    override fun surfaceDestroyed(p0: SurfaceHolder) {
        mWeakReference.get()?.let {mainActivity->
            with(mainActivity) {
                /*if (isCube) {
                    cleanupEGL()
                    Log.e("surfaceDestroy", "销毁")
                }*/
            }
        }
    }

    fun render(){
        mWeakReference.get()?.let { mainActivity ->
            with(mainActivity){
                    renderWithOutGLSurface(width, height)
            }
        }
    }

}