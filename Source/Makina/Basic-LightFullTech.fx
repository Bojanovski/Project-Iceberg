#ifndef BASIC_LIGHTFULLTECHNIQUE_FX
#define BASIC_LIGHTFULLTECHNIQUE_FX

#include "GeneralUtilities.fx"

VertexOut VSLightFullTech(VertexIn vIn)
{
	VertexOut vOut;
	 
	// Transform to world space.
	vOut.PosW     = mul(float4(vIn.PosL, 1.0f), gWorld).xyz;
	vOut.NormalW  = mul(vIn.NormalL, (float3x3)gWorldInvTranspose);
	vOut.TangentW  = mul(vIn.TangentL, (float3x3)gWorldInvTranspose);

	// Transform to homogeneous clip space.
	vOut.PosH     = mul(float4(vIn.PosL, 1.0f), gWorldViewProj);

	// Experimental (http://outerra.blogspot.com/2009/08/logarithmic-z-buffer.html)
	//float C = 10000;
	//vOut.PosH.z = log(C * vOut.PosH.z + 1) / log(C * 1000.0f + 1) * vOut.PosH.w;
	//vOut.PosH.z = vOut.PosH.z / 1000.0f * vOut.PosH.w;

	// Just pass the texture coordinates
	vOut.TexCoord = mul(float4(vIn.TexCoord, 0.0f, 1.0f), gTexTransform).xy;

	return vOut;
};

float4 PSLightFullTech(VertexOut pIn) : SV_Target
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

	ComputePointLight(gMaterial, gPointLight, pIn.PosW, bumpedNormalW, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	spec    += S;

	ComputeSpotLight(gMaterial, gSpotLight, pIn.PosW, bumpedNormalW, toEyeW, A, D, S);
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