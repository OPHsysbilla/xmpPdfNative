//
// Created by satur on 10/22/21.
//
#include <jni.h>
#include <string>
#include "fileutils.cpp"

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_xmpnative_XmpNative_getMetaData(JNIEnv *env, jclass clazz, jstring pdfPath) {
    const char *path = env->GetStringUTFChars(pdfPath, 0);
    if (tryUnitTestFile(path) == 0) { // success
        LOGD("successs")
    } else {
        LOGD("failed")
    }
}