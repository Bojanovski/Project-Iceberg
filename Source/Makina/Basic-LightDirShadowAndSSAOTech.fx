#ifndef BASIC_LIGHTDIRSHADOWANDSSAOTECHNIQUE_FX
#define BASIC_LIGHTDIRSHADOWANDSSAOTECHNIQUE_FX

#include "GeneralUtilities.fx"

VertexOutShadowAndSSAO VSLightDirShadowAndSSAOTech(VertexIn vIn)
{
	VertexOutShadowAndSSAO vOut;
	
	// Transform to world space space.
	vOut.PosW    = mul(float4(vIn.PosL, 1.0f), gWorld).xyz;
	vOut.NormalW  = mul(vIn.NormalL, (float3x3)gWorldInvTranspose);
	vOut.TangentW  = mul(vIn.TangentL, (float3x3)gWorldInvTranspose);
		
	// Transform to homogeneous clip space.
	vOut.PosH = mul(float4(vIn.PosL, 1.0f), gWorldViewProj);
	
	// Output vertex attributes for interpolation across triangle.
	vOut.TexCoord = mul(float4(vIn.TexCoord, 0.0f, 1.0f), gTexTransform).xy;

	// Generate projective tex-coords to project shadow map onto scene.
	vOut.ShadowPosH = mul(float4(vIn.PosL, 1.0f), gShadowTransform);

	// Generate projective tex-coords to project SSAO map onto scene.
	vOut.ScreenPosH = mul(float4(vIn.PosL, 1.0f), gWorldViewProjTex);

	return vOut;
}

float4 PSLightDirShadowAndSSAOTech(VertexOutShadowAndSSAO pIn, uniform bool reflection) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pIn.NormalW = normalize(pIn.NormalW); 

	float3 normalMapSample = gNormalMap.Sample(linearSampler, pIn.TexCoord).rgb;
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample, pIn.NormalW, pIn.TangentW);

	float3 toEyeW = normalize(gEyePosW - pIn.PosW);

	// Only the first light casts a shadow.
	float shadow = CalcShadowFactor(samShadow, gShadowMap, pIn.ShadowPosH, gShadowMapSize);

	// Finish texture projection and sample SSAO map.
	pIn.ScreenPosH /= pIn.ScreenPosH.w;
	float ambientAccess = gSsaoMap.Sample(linearSampler, pIn.ScreenPosH.xy, 0.0f).r;

	// Start with a sum of zero. 
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Sum the light contribution from each light source.
	float4 A, D, S;

	ComputeDirectionalLight(gMaterial, gDirLight, bumpedNormalW, toEyeW, A, D, S);
	ambient += A*ambientAccess;  
	diffuse += D*shadow;
	spec    += S*shadow;
	
	float4 texColor = gDiffuseMap.Sample(samAnisotropic, pIn.TexCoord);

	if (reflection)
	{
		float3 reflectionVector = reflect(-toEyeW, bumpedNormalW);
		float4 reflectionColor = gCubeMap.Sample(samAnisotropic, reflectionVector);
		texColor = texColor * (1.0f - gReflection) + gMaterial.Reflect * reflectionColor * gReflection;
	}
	float4 litColor = texColor * (ambient + diffuse) + spec;



	// Common to take alpha from diffuse material and texture.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}

#endif