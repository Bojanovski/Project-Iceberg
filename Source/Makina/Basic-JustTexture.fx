#ifndef BASIC_JUSTTEXTURE_FX
#define BASIC_JUSTTEXTURE_FX

#include "GeneralUtilities.fx"

VertexOutPT VSJustTexture(VertexInPT vIn)
{
	VertexOutPT vOut;

	// Transform to homogeneous clip space.
	vOut.PosH     = mul(float4(vIn.PosL, 1.0f), gWorldViewProj);

	// Just pass the texture coordinates
	vOut.TexCoord = mul(float4(vIn.TexCoord, 0.0f, 1.0f), gTexTransform).xy;

	return vOut;
};

float4 PSJustTexture(VertexOutPT pIn) : SV_Target
{
    return gDiffuseMap.Sample(samAnisotropic, pIn.TexCoord);
}

#endif