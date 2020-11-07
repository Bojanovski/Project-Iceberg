#ifndef BITMAP_FX
#define BITMAP_FX

#include "2DGraphics-Utilities.fx"

struct VertexIn_Bitmap
{
	float2 TexCoord : TEXCOORD;
};

struct VertexOut_Bitmap
{
	float4 PosH     : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

VertexOut_Bitmap Bitmap_VS(VertexIn_Bitmap vsIn)
{
	VertexOut_Bitmap vsOut;
	
	float2 pos = vsIn.TexCoord * gOffsetAndSize.zw + gOffsetAndSize.xy;
	pos = pos * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);

	vsOut.PosH = float4(pos.x, pos.y, 0.0f, 1.0f);
	vsOut.TexCoord = vsIn.TexCoord;

	return vsOut;
}

float4 BitmapLinear_PS(VertexOut_Bitmap psIn) : SV_TARGET
{
	return gTexture.Sample(linearSampler, psIn.TexCoord) * gColor;
}

float4 BitmapPoint_PS(VertexOut_Bitmap psIn) : SV_TARGET
{
	return gTexture.Sample(pointSampler, psIn.TexCoord) * gColor;
}

#endif