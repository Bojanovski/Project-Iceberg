#ifndef TEXT_FX
#define TEXT_FX

#include "2DGraphics-Utilities.fx"

struct VertexIn_Text
{
	float4 PosRec      : POSITION;
	float4 TexRec      : TEXCOORD;
};

struct VertexOut_Text
{
	float4 PosRecH     : POSITION;
	float4 TexRec      : TEXCOORD;
};

struct GeoOut_Text
{
	float4 PosH       : SV_POSITION;
	float2 TexCoord   : TEXCOORD;
};

VertexOut_Text Text_VS(VertexIn_Text vsIn)
{
	VertexOut_Text vsOut;
	vsOut.PosRecH  = vsIn.PosRec * float4(gOffsetAndSize.z, -gOffsetAndSize.w, gOffsetAndSize.z, -gOffsetAndSize.w) + float4(-1.0f, 1.0f, -1.0f, 1.0f) + float4(gOffsetAndSize.xy, gOffsetAndSize.xy) * float4(2.0f, -2.0f, 2.0f, -2.0f);
	vsOut.TexRec   = vsIn.TexRec;

	return vsOut;
}

[maxvertexcount(4)]
void Text_GS(point VertexOut_Text gsIn[1], inout TriangleStream<GeoOut_Text> triStream)
{
	GeoOut_Text gOut;

	gOut.PosH     = float4(gsIn[0].PosRecH.x, gsIn[0].PosRecH.w, 0.0f, 1.0f);
	gOut.TexCoord = float2(gsIn[0].TexRec.x, gsIn[0].TexRec.w);
	triStream.Append(gOut);

	gOut.PosH     = float4(gsIn[0].PosRecH.x, gsIn[0].PosRecH.y, 0.0f, 1.0f);
	gOut.TexCoord = float2(gsIn[0].TexRec.x, gsIn[0].TexRec.y);
	triStream.Append(gOut);

	gOut.PosH     = float4(gsIn[0].PosRecH.z, gsIn[0].PosRecH.w, 0.0f, 1.0f);
	gOut.TexCoord = float2(gsIn[0].TexRec.z, gsIn[0].TexRec.w);
	triStream.Append(gOut);

	gOut.PosH     = float4(gsIn[0].PosRecH.z, gsIn[0].PosRecH.y, 0.0f, 1.0f);
	gOut.TexCoord = float2(gsIn[0].TexRec.z, gsIn[0].TexRec.y);
	triStream.Append(gOut);
}

float4 Text_PS(GeoOut_Text psIn) : SV_TARGET
{
	float4 texColor = gTexture.Sample(linearSampler, psIn.TexCoord);
	return float4(gColor.rgb, texColor.r);
}

#endif