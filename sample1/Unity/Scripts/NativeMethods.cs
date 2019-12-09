using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System;

public class NativeMethods {
    [DllImport("astra_android_bridge", EntryPoint = "openCamera")]
    public static extern bool openCamera();
    [DllImport("astra_android_bridge", EntryPoint = "closeCamera")]
    public static extern bool closeCamera();
    [DllImport("astra_android_bridge", EntryPoint = "startCamera")]
    public static extern bool startCamera();
    [DllImport("astra_android_bridge", EntryPoint = "stopCamera")]
    public static extern bool stopCamera();
    [DllImport("astra_android_bridge", EntryPoint = "getAvailableCameraModes")]
    public static extern void getAvailableCameraModes(ref ImageSupportedModeList pSupportedModeList);
    [DllImport("astra_android_bridge", EntryPoint = "getCameraMode")]
    public static extern void getCameraMode(ref PreviewMode Mode);
    [DllImport("astra_android_bridge", EntryPoint = "setCameraMode")]
    public static extern void setCameraMode(PreviewMode Mode);
    [DllImport("astra_android_bridge", EntryPoint = "GetImageData")]
    public static extern void GetImageData(IntPtr ImageData);
    [DllImport("astra_android_bridge", EntryPoint = "GetYUV420Data")]
    public static extern IntPtr GetYUV420Data();
    [DllImport("astra_android_bridge", EntryPoint = "dispose")]
    public static extern void dispose();

}
