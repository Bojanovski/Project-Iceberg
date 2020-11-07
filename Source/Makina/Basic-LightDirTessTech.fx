#ifndef BASIC_LIGHTDIRTESSTECHNIQUE_FX
#define BASIC_LIGHTDIRTESSTECHNIQUE_FX

#include "GeneralUtilities.fx"

VertexOutTess VSLightDirTessTech(VertexIn vIn)
{
	VertexOutTess vOut;
	 
	// Transform to world space.
	vOut.PosW     = mul(float4(vIn.PosL, 1.0f), gWorld).xyz;
	vOut.NormalW  = mul(vIn.NormalL, (float3x3)gWorldInvTranspose);
	vOut.TangentW  = mul(vIn.TangentL, (float3x3)gWorldInvTranspose);

	// Just pass the texture coordinates
	vOut.TexCoord = vIn.TexCoord;

	float d = distance(vOut.PosW, gEyePosW);

	// Normalized tessellation factor. 
	// The tessellation is 
	//   0 if d >= gMinTessDistance and
	//   1 if d <= gMaxTessDistance.  
	float tess = saturate( (gMinTessDistance - d) / (gMinTessDistance - gMaxTessDistance) );
	
	// Rescale [0,1] --> [gMinTessFactor, gMaxTessFactor].
	vOut.TessFactor = gMinTessFactor + tess*(gMaxTessFactor-gMinTessFactor);

	return vOut;
};

struct PatchTess
{
	float EdgeTess[3] : SV_TessFactor;
	float InsideTess  : SV_InsideTessFactor;
};

PatchTess PatchHS(InputPatch<VertexOutTess,3> patch, uint patchID : SV_PrimitiveID)
{
	PatchTess pt;
	
	// Average tess factors along edges, and pick an edge tess factor for 
	// the interior tessellation.  It is important to do the tess factor
	// calculation based on the edge properties so that edges shared by 
	// more than one triangle will have the same tessellation factor.  
	// Otherwise, gaps can appear.
	pt.EdgeTess[0] = 0.5f*(patch[1].TessFactor + patch[2].TessFactor);
	pt.EdgeTess[1] = 0.5f*(patch[2].TessFactor + patch[0].TessFactor);
	pt.EdgeTess[2] = 0.5f*(patch[0].TessFactor + patch[1].TessFactor);
	pt.InsideTess  = pt.EdgeTess[0];
	
	return pt;
}

struct HullOut
{
	float3 PosW			: POSTION;
	float3 NormalW		: NORMAL;
	float3 TangentW		: TANGENT;
	float2 TexCoord		: TEXCOORD;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HullOut HSLightDirTessTech(InputPatch<VertexOutTess,3> p, uint i : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
	HullOut hout;
	
	// Pass through shader.
	hout.PosW     = p[i].PosW;
	hout.NormalW  = p[i].NormalW;
	hout.TangentW = p[i].TangentW;
	hout.TexCoord = p[i].TexCoord;
	
	return hout;
}

struct DomainOut
{
	float4 PosH     : SV_POSITION;
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
	float3 TangentW : TANGENT;
	float2 TexCoord : TEXCOORD;
};

// The domain shader is called for every vertex created by the tessellator.  
// It is like the vertex shader after tessellation.
[domain("tri")]
DomainOut DSLightDirTessTech(PatchTess patchTess, float3 bary : SV_DomainLocation, const OutputPatch<HullOut,3> tri)
{
	DomainOut dout;
	
	// Interpolate patch attributes to generated vertices.
	dout.PosW     = bary.x*tri[0].PosW     + bary.y*tri[1].PosW     + bary.z*tri[2].PosW;
	dout.NormalW  = bary.x*tri[0].NormalW  + bary.y*tri[1].NormalW  + bary.z*tri[2].NormalW;
	dout.TangentW = bary.x*tri[0].TangentW + bary.y*tri[1].TangentW + bary.z*tri[2].TangentW;
	dout.TexCoord = bary.x*tri[0].TexCoord + bary.y*tri[1].TexCoord + bary.z*tri[2].TexCoord;
	
	// Interpolating normal can unnormalize it, so normalize it.
	dout.NormalW = normalize(dout.NormalW);
	
	//
	// Displacement mapping.
	//
	
	// Choose the mipmap level based on distance to the eye; specifically, choose
	// the next miplevel every MipInterval units, and clamp the miplevel in [0,6].
	const float MipInterval = 20.0f;
	float mipLevel = clamp( (distance(dout.PosW, gEyePosW) - MipInterval) / MipInterval, 0.0f, 6.0f);
	
	// Sample height map (stored in alpha channel).
	float h = gNormalMap.SampleLevel(linearSampler, dout.TexCoord, mipLevel).a;
	
	// Offset vertex along normal.
	dout.PosW += (gHeightScale*(h-1.0f))*dout.NormalW;
	
	// Project to homogeneous clip space.
	dout.PosH = mul(float4(dout.PosW, 1.0f), gViewProj);
	
	return dout;
}

float4 PSLightDirTessTech(DomainOut pIn) : SV_Target
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