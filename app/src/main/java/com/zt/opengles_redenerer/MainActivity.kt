package com.zt.opengles_redenerer

import android.content.pm.ActivityInfo
import android.graphics.Color
import android.media.MediaPlayer
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.os.Handler
import android.os.HandlerThread
import android.os.Looper
import android.os.Message
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity


class MainActivity : AppCompatActivity(), Handler.Callback {
    private lateinit var mGLSurfaceView: GLSurfaceView
    private lateinit var videoHelper: VideoHelper
    private lateinit var mHandlerThread: HandlerThread
    private lateinit var mHandler: Handler
    lateinit var cubeSurface: CubeSurface
    private var isFirstInit = true

    companion object{
        init {
            System.loadLibrary("native-color-lib")
            Color.RED
        }
    }

    external fun initEGL(surface: Surface): Boolean

    external fun renderWithOutGLSurface(width: Int, height: Int)

    external fun removeSurface()

    external fun cleanupEGL()

    external fun surfaceCreated(color: Int)

    external fun surfaceChanged(width: Int, height: Int)

    external fun onDrawFrame()

    external fun initRenderVideo(surface: Surface): Boolean

    external fun renderSurface()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        if(isFirstInit) {
            cubeSurface = CubeSurface(this)
            mHandlerThread = HandlerThread("IO")
            mHandlerThread.start()
            mHandler = Handler(mHandlerThread.looper,this)
            isFirstInit = false
            videoHelper = VideoHelper()
        }
        initListener()
    }

    private fun initListener(){
        findViewById<SurfaceView>(R.id.sv_video)!!.holder.addCallback(cubeSurface)
        findViewById<TextView>(R.id.tv_cube).setOnClickListener {
            removeSurface()
            initEGL(cubeSurface.currentSurface)
            cubeSurface.render()
        }
        findViewById<TextView>(R.id.tv_revert).setOnClickListener {
            requestedOrientation = if(requestedOrientation == ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE)
                ActivityInfo.SCREEN_ORIENTATION_PORTRAIT
            else
                ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
        }
        findViewById<TextView>(R.id.tv_video).setOnClickListener {
            removeSurface()
            val message = mHandler.obtainMessage()
            message.what = 1
            mHandler.sendMessage(message)

        }
    }

    private fun setUpViews(surface: Surface){
        mGLSurfaceView = GLSurfaceView(this)
        mGLSurfaceView.setEGLContextClientVersion(3)
        mGLSurfaceView.setRenderer(SimpleRenderer(Color.BLACK))
    }




    override fun handleMessage(message: Message): Boolean {
        when(message.what){
            1->{
//                videoHelper.extractorFileToMediaCodec(this@MainActivity,cubeSurface.currentSurface)
                renderSurface()
            }
            2->{
                videoHelper.paintCube()
            }
            else -> {}
        }
        return true
    }


}