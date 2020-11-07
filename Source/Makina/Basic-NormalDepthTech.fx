#ifndef BASIC_NORMALDEPTHTECHNIQUE_FX
#define BASIC_NORMALDEPTHTECHNIQUE_FX

#include "GeneralUtilities.fx"

//-----------------------------------------------------------------------------
//								Depth Only
//-----------------------------------------------------------------------------

VertexOutPT VSDepthOnlyAlphaClipTech(VertexInPT vIn)
{
	VertexOutPT vOut;

	// Transform to homogeneous clip space.
	vOut.PosH     = mul(float4(vIn.PosL, 1.0f), gWorldViewProj);

	// Just pass the texture coordinates
	vOut.TexCoord = vIn.TexCoord;

	return vOut;
};

VertexOutP VSDepthOnlyTech(VertexIn vIn)
{
	VertexOutP vOut;

	// Transform to homogeneous clip space.
	vOut.PosH     = mul(float4(vIn.PosL, 1.0f), gWorldViewProj);

	return vOut;
};

void PSDepthOnlyAlphaClipTech(VertexOutPT pIn)
{
	float diffuse = gDiffuseMap.Sample(samAnisotropic, pIn.TexCoord).r;

	// Dont write transparent pixels to shadow map.
	clip(diffuse - 0.15f);
}

//-----------------------------------------------------------------------------
//								Normal and Depth 16-bit FLOAT
//-----------------------------------------------------------------------------

VertexOutNormalAndDepth VSNormalAndDepthTech(VertexIn vIn)
{
	VertexOutNormalAndDepth vOut;

	// Transform to homogeneous clip space.
	vOut.PosH     = mul(float4(vIn.PosL, 1.0f), gWorldViewProj);

	// Transform to view space
	vOut.PosV	= mul(float4(vIn.PosL, 1.0f), gWorldView);
	vOut.NormalV = mul(vIn.NormalL, (float3x3)gWorldInvTransposeView);
	vOut.TangentV = mul(vIn.TangentL, (float3x3)gWorldInvTransposeView);

	// Just pass the texture coordinates
	vOut.TexCoord = vIn.TexCoord;

	return vOut;
};

float4 PSNormalAndDepthAlphaClip16BitTech(VertexOutNormalAndDepth pIn) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pIn.NormalV = normalize(pIn.NormalV);

	float4 texColor = gDiffuseMap.Sample( samAnisotropic, pIn.TexCoord );
	clip(texColor.a - 0.1f);
	
	return float4(pIn.NormalV, pIn.PosV.z);
};

float4 PSNormalAndDepth16BitTech(VertexOutNormalAndDepth pIn) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pIn.NormalV = normalize(pIn.NormalV);
	
	return float4(pIn.NormalV, pIn.PosV.z);
};

//-----------------------------------------------------------------------------
//								Normal and Depth 8-bit UNORM
//-----------------------------------------------------------------------------

float4 VectorsTo8Bit(float3 normal, float depth)
{
	// scale from [-1, 1] to [0, 1]
	float2 normalScaled = (normal.xy + float2(1.0f, 1.0f)) * 0.5f;

	// scale from [zNear, zFar] to [0, 1]
	float depthScaled = (depth - gZNear) / (gZFar - gZNear);

	// distribute depth over two components
	float depthScaledW = frac(depthScaled*255.0f);
	float depthScaledZ = depthScaled - depthScaledW/255.0f; // get rid of the part we already have, it can cause problems due to conversion averaging

	return float4(normalScaled, depthScaledZ, depthScaledW);
}

float4 PSNormalAndDepthAlphaClip8BitTech(VertexOutNormalAndDepth pIn) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pIn.NormalV = normalize(pIn.NormalV);

	float4 texColor = gDiffuseMap.Sample( samAnisotropic, pIn.TexCoord );
	clip(texColor.a - 0.1f);
	
	float3 normalMapSample = gNormalMap.Sample(linearSampler, pIn.TexCoord).rgb;
	//NormalSampleToWorldSpace will work for view space too.
	float3 bumpedNormalV = NormalSampleToWorldSpace(normalMapSample, pIn.NormalV, pIn.TangentV);

	return VectorsTo8Bit(bumpedNormalV, pIn.PosV.z);
};

float4 PSNormalAndDepth8BitTech(VertexOutNormalAndDepth pIn) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pIn.NormalV = normalize(pIn.NormalV);
	
	float3 normalMapSample = gNormalMap.Sample(linearSampler, pIn.TexCoord).rgb;
	//NormalSampleToWorldSpace will work for view space too.
	float3 bumpedNormalV = NormalSampleToWorldSpace(normalMapSample, pIn.NormalV, pIn.TangentV);

	return VectorsTo8Bit(bumpedNormalV, pIn.PosV.z);
};


#endif