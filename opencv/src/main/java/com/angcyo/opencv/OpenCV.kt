package com.angcyo.opencv

import android.content.Context
import android.graphics.Bitmap
import android.util.Log
import java.io.File
import java.io.FileNotFoundException
import java.io.FileOutputStream

/**
 * @author [angcyo](mailto:angcyo@126.com)
 * @since 2022/04/27
 */
object OpenCV {

    init {
        System.loadLibrary("opencv_bmp")
    }

    /**图片转GCode*/
    fun bitmapToGCode(context: Context, bitmap: Bitmap): String {
        val name = System.currentTimeMillis().toString()
        ///data/user/0/com.angcyo.opencv.demo/cache/1651043155246.png
        ///storage/emulated/0/Android/data/com.angcyo.opencv.demo/cache
        val tempFile = File(context.externalCacheDir, "$name.png")
        ///data/user/0/com.angcyo.opencv.demo/cache/1651043155246.gcode
        val gcodeFile = File(context.externalCacheDir, "$name.gcode")
        val format = Bitmap.CompressFormat.PNG
        try {
            bitmap.compress(format, 100, FileOutputStream(tempFile))
            val result = nativeBitmapToGCode(
                tempFile.absolutePath,
                gcodeFile.absolutePath,
                50.0,
                0.1,
                0,
                45.0
            )
            Log.i("OpenCV", result.toString() + "")
        } catch (e: FileNotFoundException) {
            e.printStackTrace()
        }
        return gcodeFile.readText()
    }

    /**
     * @param inputPath  图片输入的路径
     * @param outputPath GCode输出路径
     * @param scale      50.0;//printing size max 50 mm
     * @param lineSpace  0.125;// line space mm
     * @param angle      雕刻方向 font direction,0:0 1:90 2:180 3:270
     * @param anglec     线的填充角度 0 / 45
     */
    private external fun nativeBitmapToGCode(
        inputPath: String,
        outputPath: String,
        scale: Double /*mm*/,
        lineSpace: Double,
        angle: Int,
        anglec: Double
    ): Int
}