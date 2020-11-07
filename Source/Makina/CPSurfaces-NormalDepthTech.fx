
#ifndef BSPLINESURFACES_NORMALDEPTHTECHNIQUE_FX
#define BSPLINESURFACES_NORMALDEPTHTECHNIQUE_FX

#include "LightsUtilities.fx"
#include "CPSurfaces-Utilities.fx"

//-----------------------------------------------------------------------------
//								Depth Only
//-----------------------------------------------------------------------------

[domain("tri")]
V_PosW DS_DepthOnly(PatchTess patchTess, float3 bary : SV_DomainLocation, const OutputPatch<V_TexCoord, 3> tri)
{
	V_PosW dout;

	// Interpolate patch attributes to generated vertices.
	float2 texCoord = bary.x*tri[0].TexCoord + bary.y*tri[1].TexCoord + bary.z*tri[2].TexCoord;

	float3 posW;
	ComputePos(texCoord, dout.PosW);

	return dout;
}

[domain("tri")]
V_PosW_TexCoord DS_DepthOnlyAlphaClip(PatchTess patchTess, float3 bary : SV_DomainLocation, const OutputPatch<V_TexCoord, 3> tri)
{
	V_PosW_TexCoord dout;

	// Interpolate patch attributes to generated vertices.
	float2 texCoord = bary.x*tri[0].TexCoord + bary.y*tri[1].TexCoord + bary.z*tri[2].TexCoord;

	float3 posW;
	ComputePos(texCoord, dout.PosW);
	dout.TexCoord = texCoord;

	return dout;
}

// Just a pass trough geometry shader. (to deal with error X4580)
[maxvertexcount(3)]
void GS_DepthOnly(triangle V_PosW gin[3], inout TriangleStream<V_PosH> triStream)
{
	V_PosH gout[3];

	[unroll]
	for (int i = 0; i < 3; ++i)
	{
		gout[i].PosH = mul(float4(gin[i].PosW, 1.0f), gViewProj);
	}

	triStream.Append(gout[0]);
	triStream.Append(gout[1]);
	triStream.Append(gout[2]);
}

// Just a pass trough geometry shader. (to deal with error X4580)
[maxvertexcount(6)]
void GS_DepthOnlyAlphaClip(triangle V_PosW_TexCoord gin[3], inout TriangleStream<V_PosH_TexCoord> triStream)
{
	V_PosH_TexCoord gout[6];

	[unroll]
	for (int i = 0; i < 3; ++i)
	{
		gout[i].PosH = mul(float4(gin[i].PosW, 1.0f), gViewProj);
		gout[i].TexCoord = gin[i].TexCoord;
	}

	triStream.Append(gout[0]);
	triStream.Append(gout[1]);
	triStream.Append(gout[2]);
}

void PS_DepthOnlyAlphaClip(V_PosH_TexCoord pIn)
{
	float diffuse = gDiffuseMap.Sample(linearSampler, pIn.TexCoord).a;
	clip(diffuse - 0.15f);
}

#endif