#ifndef BASIC_LIGHTANDNTB_FX
#define BASIC_LIGHTANDNTB_FX

#include "GeneralUtilities.fx"

#define AXIS_LENGTH 0.2f

VertexOutNTB NTB_VS(VertexIn vIn)
{
	VertexOutNTB vOut;

	// Transform to world space.
	float3 posW        = mul(float4(vIn.PosL, 1.0f), gWorld).xyz;
	float3 normalW     = mul(vIn.NormalL, (float3x3)gWorldInvTranspose);
	float3 tangentW    = mul(vIn.TangentL, (float3x3)gWorldInvTranspose);
	float3 biTangentW  = cross(tangentW, normalW);

	vOut.NormalPosH = mul(float4(posW + normalW * AXIS_LENGTH, 1.0f), gViewProj);
	vOut.TangentPosH = mul(float4(posW + tangentW * AXIS_LENGTH, 1.0f), gViewProj);
	vOut.BitangentPosH = mul(float4(posW + biTangentW * AXIS_LENGTH, 1.0f), gViewProj);

	// Transform to homogeneous clip space.
	vOut.PosH     = mul(float4(vIn.PosL, 1.0f), gWorldViewProj);

	return vOut;
};

[maxvertexcount(18)]
void NTB_GS (triangle VertexOutNTB gIn[3], inout LineStream<GeoOut> lineStream)
{
	GeoOut gout;

	// 1. Normal
	gout.PosH = gIn[0].PosH;
	gout.Color = float4(0.0f, 1.0f, 0.0f, 1.0f);
	lineStream.Append(gout);
	gout.PosH = gIn[0].NormalPosH;
	gout.Color = float4(0.0f, 1.0f, 0.0f, 1.0f);
	lineStream.Append(gout);
	lineStream.RestartStrip();
	// Tangent
	gout.PosH = gIn[0].PosH;
	gout.Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
	lineStream.Append(gout);
	gout.PosH = gIn[0].TangentPosH;
	gout.Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
	lineStream.Append(gout);
	lineStream.RestartStrip();
	// Bitangent
	gout.PosH = gIn[0].PosH;
	gout.Color = float4(0.0f, 0.0f, 1.0f, 1.0f);
	lineStream.Append(gout);
	gout.PosH = gIn[0].BitangentPosH;
	gout.Color = float4(0.0f, 0.0f, 1.0f, 1.0f);
	lineStream.Append(gout);
	lineStream.RestartStrip();

	// 2. Normal
	gout.PosH = gIn[1].PosH;
	gout.Color = float4(0.0f, 1.0f, 0.0f, 1.0f);
	lineStream.Append(gout);
	gout.PosH = gIn[1].NormalPosH;
	gout.Color = float4(0.0f, 1.0f, 0.0f, 1.0f);
	lineStream.Append(gout);
	lineStream.RestartStrip();
	// Tangent
	gout.PosH = gIn[1].PosH;
	gout.Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
	lineStream.Append(gout);
	gout.PosH = gIn[1].TangentPosH;
	gout.Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
	lineStream.Append(gout);
	lineStream.RestartStrip();
	// Bitangent
	gout.PosH = gIn[1].PosH;
	gout.Color = float4(0.0f, 0.0f, 1.0f, 1.0f);
	lineStream.Append(gout);
	gout.PosH = gIn[1].BitangentPosH;
	gout.Color = float4(0.0f, 0.0f, 1.0f, 1.0f);
	lineStream.Append(gout);
	lineStream.RestartStrip();

	// 3. Normal
	gout.PosH = gIn[2].PosH;
	gout.Color = float4(0.0f, 1.0f, 0.0f, 1.0f);
	lineStream.Append(gout);
	gout.PosH = gIn[2].NormalPosH;
	gout.Color = float4(0.0f, 1.0f, 0.0f, 1.0f);
	lineStream.Append(gout);
	lineStream.RestartStrip();
	// Tangent
	gout.PosH = gIn[2].PosH;
	gout.Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
	lineStream.Append(gout);
	gout.PosH = gIn[2].TangentPosH;
	gout.Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
	lineStream.Append(gout);
	lineStream.RestartStrip();
	// Bitangent
	gout.PosH = gIn[2].PosH;
	gout.Color = float4(0.0f, 0.0f, 1.0f, 1.0f);
	lineStream.Append(gout);
	gout.PosH = gIn[2].BitangentPosH;
	gout.Color = float4(0.0f, 0.0f, 1.0f, 1.0f);
	lineStream.Append(gout);
	lineStream.RestartStrip();
}

float4 NTB_PS(GeoOut pIn) : SV_Target
{
	return pIn.Color;
}

#endif