package com.zt.opengles_redenerer

import android.content.Context
import android.media.MediaCodec
import android.media.MediaExtractor
import android.media.MediaFormat
import android.view.Surface
import java.nio.ByteBuffer


/**
 * @ClassName VideoHelper
 * @Description TODO
 * @Author MYM
 * @Date 2025/8/5 16:46
 */
class VideoHelper {

    private lateinit var extractor: MediaExtractor
    private lateinit var decoder: MediaCodec
    fun extractorFileToMediaCodec(context: Context,surface: Surface){
        val assetManager = context.assets
        val assetFileDescriptor = assetManager.openFd("test.mp4")
        extractor = MediaExtractor()
        decoder = MediaCodec.createDecoderByType("video/avc")
        extractor.setDataSource(
            assetFileDescriptor.fileDescriptor,
            assetFileDescriptor.startOffset,
            assetFileDescriptor.length
        )
        LogUtil.d(extractor.trackCount)
        selectVideoTrack(surface)?.let {
            LogUtil.d("视频轨道下标：$it")
            extractor.selectTrack(it)
            while (true) {
                val inIndex: Int = decoder.dequeueInputBuffer(10000)
                if (inIndex >= 0) {
                    val inputBuffer: ByteBuffer = decoder.getInputBuffer(inIndex)!!
                    val sampleSize = extractor.readSampleData(inputBuffer, 0)
                    if (sampleSize < 0) break

                    decoder.queueInputBuffer(inIndex, 0, sampleSize, extractor.sampleTime, 0)
                    extractor.advance()
                }

                val info = MediaCodec.BufferInfo()
                val outIndex: Int = decoder.dequeueOutputBuffer(info, 10000)
                if (outIndex >= 0) {
                    decoder.releaseOutputBuffer(outIndex, true) // 渲染到 Surface
                }
            }

        }
        decoder.stop()
        decoder.release()
    }

    private fun selectVideoTrack(surface: Surface): Int?{
        var videoTrackIndex: Int? = null
        for(index in 0 until extractor.trackCount){
            val format = extractor.getTrackFormat(index)
            LogUtil.d(format.getString(MediaFormat.KEY_MIME))
            val mime = format.getString(MediaFormat.KEY_MIME)
            if(mime?.startsWith("video/") == true){
                videoTrackIndex = index
                decoder.configure(format,surface,null,0)
                decoder.start()
                break
            }
        }
        return videoTrackIndex
    }

    fun paintCube(){

    }
}