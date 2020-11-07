#ifndef GRAPHICS2D_FX
#define GRAPHICS2D_FX

#include "2DGraphics-Bitmap.fx"
#include "2DGraphics-Text.fx"

technique11 BitmapPoint
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, Bitmap_VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, BitmapPoint_PS() ) );
    }
}

technique11 BitmapLinear
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, Bitmap_VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, BitmapLinear_PS() ) );
    }
}

technique11 Text
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, Text_VS() ) );
		SetGeometryShader( CompileShader(gs_5_0, Text_GS() ) );
        SetPixelShader( CompileShader( ps_5_0, Text_PS() ) );
    }
}

#endif