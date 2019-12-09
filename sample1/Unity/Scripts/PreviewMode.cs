using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

[System.Runtime.InteropServices.StructLayoutAttribute(System.Runtime.InteropServices.LayoutKind.Sequential, CharSet = System.Runtime.InteropServices.CharSet.Ansi)]
public struct PreviewMode
{
    public int width;
    public int height;
    public int fps;
}
