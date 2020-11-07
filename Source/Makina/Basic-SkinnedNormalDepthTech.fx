#ifndef BASIC_SKINNEDNORMALDEPTHTECHNIQUE_FX
#define BASIC_SKINNEDNORMALDEPTHTECHNIQUE_FX

#include "GeneralUtilities.fx"

//-----------------------------------------------------------------------------
//								Depth Only
//-----------------------------------------------------------------------------

VertexOutPT VSSkinnedDepthOnlyAlphaClipTech(SkinnedVertexIn vIn)
{
	VertexOutPT vOut;

	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vIn.Weights.x;
	weights[1] = vIn.Weights.y;
	weights[2] = vIn.Weights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 4; ++i)
	{
		posL += weights[i] * mul(float4(vIn.PosL, 1.0f), gBoneTransforms[vIn.BoneIndices[i]]).xyz;
	}

	// Transform to homogeneous clip space.
	vOut.PosH = mul(float4(posL, 1.0f), gWorldViewProj);

	// Just pass the texture coordinates
	vOut.TexCoord = vIn.TexCoord;

	return vOut;
};

VertexOutP VSSkinnedDepthOnlyTech(SkinnedVertexIn vIn)
{
	VertexOutP vOut;

	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vIn.Weights.x;
	weights[1] = vIn.Weights.y;
	weights[2] = vIn.Weights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 4; ++i)
	{
		posL += weights[i] * mul(float4(vIn.PosL, 1.0f), gBoneTransforms[vIn.BoneIndices[i]]).xyz;
	}

	// Transform to homogeneous clip space.
	vOut.PosH = mul(float4(posL, 1.0f), gWorldViewProj);

	return vOut;
};

//-----------------------------------------------------------------------------
//								Normal and Depth 16-bit FLOAT
//-----------------------------------------------------------------------------

VertexOutNormalAndDepth VSSkinnedNormalAndDepthTech(SkinnedVertexIn vIn)
{
	VertexOutNormalAndDepth vOut;

	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vIn.Weights.x;
	weights[1] = vIn.Weights.y;
	weights[2] = vIn.Weights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 4; ++i)
	{
		// Assume no nonuniform scaling when transforming normals, so 
		// that we do not have to use the inverse-transpose.
		posL += weights[i] * mul(float4(vIn.PosL, 1.0f), gBoneTransforms[vIn.BoneIndices[i]]).xyz;
		normalL += weights[i] * mul(vIn.NormalL, (float3x3)gBoneTransforms[vIn.BoneIndices[i]]);
		tangentL += weights[i] * mul(vIn.TangentL, (float3x3)gBoneTransforms[vIn.BoneIndices[i]]);
	}

	// Transform to homogeneous clip space.
	vOut.PosH = mul(float4(posL, 1.0f), gWorldViewProj);

	// Transform to view space
	vOut.PosV = mul(float4(posL, 1.0f), gWorldView);
	vOut.NormalV = mul(normalL, (float3x3)gWorldInvTransposeView);
	vOut.TangentV = mul(tangentL, (float3x3)gWorldInvTransposeView);

	// Just pass the texture coordinates
	vOut.TexCoord = vIn.TexCoord;

	return vOut;
};

#endif