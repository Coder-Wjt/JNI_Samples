using UnityEngine;
using System.Collections;
using System;

/// <summary>
/// RGB类型
/// </summary>
[System.Runtime.InteropServices.StructLayoutAttribute(System.Runtime.InteropServices.LayoutKind.Sequential, CharSet = System.Runtime.InteropServices.CharSet.Ansi)]
public struct RGBImage
{

    /// <summary>
    /// 初始化
    /// </summary>
    /// <param name="imageData">数据</param>
    public RGBImage(RGBImage imageData)
        : this(imageData, false)
    {

    }

    /// <summary>
    /// 初始化
    /// </summary>
    /// <param name="imageData">数据</param>
    /// <param name="IsCompress">是否压缩,压缩了的数据直接传进c++有可能会报错</param>
    public RGBImage(RGBImage imageData, bool IsCompress)
    {
        if (!IsCompress) ImageData = new byte[RGBDATALENGTH];
        else ImageData = new byte[RGBDATALENGTH];
        Array.Copy(imageData.ImageData, ImageData, imageData.ImageData.Length);
        Timestamp = imageData.Timestamp;
        width = imageData.width;
        height = imageData.height;
    }

    /// <summary>
    /// 拷贝
    /// </summary>
    /// <param name="imageData"></param>
    public void Copy(RGBImage imageData)
    {
        Array.Copy(imageData.ImageData, ImageData, imageData.ImageData.Length);
        Timestamp = imageData.Timestamp;
        width = imageData.width;
        height = imageData.height;
    }


    /// <summary>
    /// 当前x轴长度
    /// </summary>
    public int width;

    /// <summary>
    /// 当前y轴长度
    /// </summary>
    public int height;
    /// <summary>
    /// 时间戳
    /// </summary>
    public ulong Timestamp;

    /// <summary>
    /// 支持最大分辩率
    /// </summary>
    public const int RGBDATALENGTH = 1920 * 1080 * 4;
    /// <summary>
    /// RGB数据
    /// </summary>
    [System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.ByValArray, SizeConst = RGBDATALENGTH, ArraySubType = System.Runtime.InteropServices.UnmanagedType.Struct)]
    //public short[] ImageData;
    public byte[] ImageData;

}
