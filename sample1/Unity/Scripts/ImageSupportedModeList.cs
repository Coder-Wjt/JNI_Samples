using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

public struct ImageSupportedModeList
{
    public const int IMAGEMODECOUNT = 255;
    public int ImageModeCount; 
    [System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.ByValArray, SizeConst = IMAGEMODECOUNT, ArraySubType = System.Runtime.InteropServices.UnmanagedType.Struct)]
    public PreviewMode[] ImageModeList;
}
