
#ifndef BSPLINESURFACES_FINAL_COMPLETE_FX
#define BSPLINESURFACES_FINAL_COMPLETE_FX

#include "LightsUtilities.fx"
#include "CPSurfaces-Utilities.fx"

V_TexCoord VS(V_TexCoord vin)
{
	V_TexCoord vout;
	vout.TexCoord = vin.TexCoord;
	return vout;
}

struct PatchTess
{
	float EdgeTess[3]   : SV_TessFactor;
	float InsideTess	: SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<V_TexCoord, 3> patch, uint patchID : SV_PrimitiveID)
{
	PatchTess pt;
	
	// Uniformly tessellate the patch.
	float tess = CalcTessFactor(gCenter);
	pt.EdgeTess[0] = tess;
	pt.EdgeTess[1] = tess;
	pt.EdgeTess[2] = tess;
	pt.InsideTess = tess;

	return pt;
}

[domain("tri")]
[partitioning("integer")] //[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
V_TexCoord HS(InputPatch<V_TexCoord, 3> p, uint i : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
	V_TexCoord hout;
	hout.TexCoord = p[i].TexCoord;
	return hout;
}

[domain("tri")]
V_PosW_NormalW_TexCoord DS_FinalComplete(PatchTess patchTess, float3 bary : SV_DomainLocation, const OutputPatch<V_TexCoord, 3> tri)
{
	// Interpolate patch attributes to generated vertices.
	float2 texCoord = bary.x*tri[0].TexCoord + bary.y*tri[1].TexCoord + bary.z*tri[2].TexCoord;
	
	V_PosW_NormalW_TexCoord dout;
	float3 tanW;
	ComputePosNormalTangent(texCoord, dout.PosW, dout.NormalW, tanW);
	//dout.PosH = mul(float4(dout.PosW, 1.0f), gViewProj); // Project to homogeneous clip space.
	dout.TexCoord = texCoord;

	return dout;
}

[maxvertexcount(3)]
void GS_FinalComplete(triangle V_PosW_NormalW_TexCoord gin[3], inout TriangleStream<V_PosH_NormalW_TexCoord_ShadowPosH> triStream)
{
	V_PosH_NormalW_TexCoord_ShadowPosH gout[6];

	[unroll] // just copy pasti'n
	for (int i = 0; i < 3; ++i)
	{
		float3 posW = gin[i].PosW;
		gout[i].PosH = mul(float4(posW, 1.0f), gViewProj);
		gout[i].NormalW = gin[i].NormalW;
		gout[i].TexCoord = gin[i].TexCoord;
		gout[i].ShadowPosH = mul(float4(posW, 1.0f), gShadowTransform);
	}

	triStream.Append(gout[0]);
	triStream.Append(gout[1]);
	triStream.Append(gout[2]);
}

float4 PS_FinalComplete(V_PosH_NormalW_TexCoord_ShadowPosH pin, bool isFrontFace : SV_IsFrontFace) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
	pin.NormalW = normalize(pin.NormalW);

	if (!isFrontFace) pin.NormalW = -pin.NormalW;

	//float3 normalMapSample = gNormalMap.Sample(linearSampler, pin.TexCoord).rgb;
	//float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample, pin.NormalW, pin.TanW);

	float3 toEyeW = float3(0.0f, 0.0f, 0.0f); // = normalize(gEyePosW - pin.PosW);

	// Only the first light casts a shadow.
	float shadow = CalcShadowFactor(samShadow, gShadowMap, pin.ShadowPosH, gShadowMapSize);

	// Start with a sum of zero. 
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// Sum the light contribution from each light source.
	float4 lA, lD, lS;

	ComputeDirectionalLight(gMaterial, gDirLight, pin.NormalW, toEyeW, lA, lD, lS);
	ambient += lA;
	diffuse += lD*shadow;
	//spec += lS;

	float4 texColor = gDiffuseMap.Sample(samAnisotropic, pin.TexCoord);
	clip(texColor.a - 0.15f);
	float4 litColor = texColor * (ambient + diffuse) + spec;

	// Common to take alpha from diffuse material and texture.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

	return litColor;
}

#endif