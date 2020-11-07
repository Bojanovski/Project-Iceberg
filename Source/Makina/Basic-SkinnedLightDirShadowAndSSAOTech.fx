
#ifndef BASIC_SKINNEDLIGHTDIRSHADOWANDSSAOTECHNIQUE_FX
#define BASIC_SKINNEDLIGHTDIRSHADOWANDSSAOTECHNIQUE_FX

#include "GeneralUtilities.fx"

VertexOutShadowAndSSAO VSSkinnedLightDirShadowAndSSAOTech(SkinnedVertexIn vIn)
{
	VertexOutShadowAndSSAO vOut;
	
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

	// Transform to world space space.
	vOut.PosW = mul(float4(posL, 1.0f), gWorld).xyz;
	vOut.NormalW = mul(normalL, (float3x3)gWorldInvTranspose);
	vOut.TangentW = mul(tangentL, (float3x3)gWorldInvTranspose);
	
	// Transform to homogeneous clip space.
	vOut.PosH = mul(float4(posL, 1.0f), gWorldViewProj);
	
	// Output vertex attributes for interpolation across triangle.
	vOut.TexCoord = mul(float4(vIn.TexCoord, 0.0f, 1.0f), gTexTransform).xy;

	// Generate projective tex-coords to project shadow map onto scene.
	vOut.ShadowPosH = mul(float4(posL, 1.0f), gShadowTransform);

	// Generate projective tex-coords to project SSAO map onto scene.
	vOut.ScreenPosH = mul(float4(posL, 1.0f), gWorldViewProjTex);

	return vOut;
}

#endif
