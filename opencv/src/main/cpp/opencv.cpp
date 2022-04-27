
/**
 * 2022-4-27
 * */

#include <jni.h>
#include <string>
#include "bmpTogcode.h"

extern "C"
JNIEXPORT jint JNICALL
Java_com_angcyo_opencv_OpenCV_nativeBitmapToGCode(JNIEnv *env, jobject thiz, jstring input_path,
                                                  jstring output_path, jdouble scale,
                                                  jdouble line_space, jint angle, jdouble anglec) {

    const char *inputPath = env->GetStringUTFChars(input_path, nullptr);
    const char *outputPath = env->GetStringUTFChars(output_path, nullptr);

    const int result = bmpTogcodeFromPath(inputPath, outputPath, scale, line_space, angle, anglec);

    env->ReleaseStringUTFChars(input_path, inputPath);
    env->ReleaseStringUTFChars(output_path, outputPath);

    return result;
}