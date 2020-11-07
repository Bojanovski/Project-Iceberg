#ifndef BASIC_GOURAUDTECH_FX
#define BASIC_GOURAUDTECH_FX

#include "GeneralUtilities.fx"

VertexOutGouraud VSGouraudTech(VertexIn vIn)
{
	VertexOutGouraud vOut;
	 
	// Transform to world space.
	float3 posW     = mul(float4(vIn.PosL, 1.0f), gWorld).xyz;
	float3 normalW  = normalize(mul(vIn.NormalL, (float3x3)gWorldInvTranspose));

	// Transform to homogeneous clip space.
	vOut.PosH     = mul(float4(vIn.PosL, 1.0f), gWorldViewProj);

	// Just pass the texture coordinates
	vOut.TexCoord = vIn.TexCoord;

	float4 diffuse = saturate(dot(normalW, -gDirLight.Direction)) * gDirLight.Diffuse * gMaterial.Diffuse;
	float4 ambient = gDirLight.Ambient * gMaterial.Ambient;

	vOut.Color = diffuse + ambient;

	return vOut;
};

float4 PSGouraudTech(VertexOutGouraud pIn) : SV_Target
{
	float4 texColor = gDiffuseMap.Sample(samAnisotropic, pIn.TexCoord);
	  
	float4 litColor = texColor * pIn.Color;

	// Common to take alpha from diffuse material and texture.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}

float4 PSGouraudFastTech(VertexOutGouraud pIn) : SV_Target
{
	float4 texColor = gDiffuseMap.Sample(fastSampler, pIn.TexCoord);
	  
	float4 litColor = texColor * pIn.Color;

	// Common to take alpha from diffuse material and texture.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}

#endif