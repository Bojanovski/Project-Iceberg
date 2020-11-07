#ifndef BASIC_LIGHTDIRTECHNIQUE_FX
#define BASIC_LIGHTDIRTECHNIQUE_FX

#include "GeneralUtilities.fx"

float4 PSLightDirTech(VertexOut pIn) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pIn.NormalW = normalize(pIn.NormalW); 

	float3 normalMapSample = gNormalMap.Sample(linearSampler, pIn.TexCoord).rgb;
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample, pIn.NormalW, pIn.TangentW);

	float3 toEyeW = normalize(gEyePosW - pIn.PosW);

	// Start with a sum of zero. 
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Sum the light contribution from each light source.
	float4 A, D, S;

	ComputeDirectionalLight(gMaterial, gDirLight, bumpedNormalW, toEyeW, A, D, S);
	ambient += A;  
	diffuse += D;
	spec    += S;
	
	float4 texColor = gDiffuseMap.Sample(samAnisotropic, pIn.TexCoord);
	  
	float4 litColor = texColor * (ambient + diffuse) + spec;

	// Common to take alpha from diffuse material and texture.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}

#endif