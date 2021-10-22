package com.example.xmpnative;

/**
 * Created by lei.jialin on 10/22/21
 */
public class XmpNative {
    static {
        System.loadLibrary("xmpmark");
    }

    public static native String getMetaData(String pdfPath);
}
