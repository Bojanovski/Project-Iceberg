
#include "LightsUtilities.fx"
#include "SSAO-Utilities.fx"

#define SAMPLES_NUM 60

cbuffer cbPerFrame
{
	float4x4 gView;
	float4x4 gInvView;
};

cbuffer cbFixed
{
	float2 gTexC[4] = 
	{
		float2(1.0f, 1.0f),
		float2(0.0f, 1.0f),
		float2(1.0f, 0.0f),
		float2(0.0f, 0.0f)
	};
};

Texture3D g3DNoiseMap;
Texture2D gPaletteMap;

SamplerState samLinearWrap
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
	AddressW = WRAP;
};

SamplerState samLinearClamp
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

struct VertexIn
{
	float3	CenterW	: POSITION;		// Sphere center in world space
	float	Radius	: SIZE;			// Sphere radius.
};

struct VertexOut
{
	float3	CenterW	: POSITION;
	float	Radius	: SIZE;
};

struct GeoOut
{
	float4	PosH	: SV_POSITION;
    float2	Tex		: TEXCOORD;
	float3	SCentV	: POSITION;
	float	SRad	: SIZE;
    uint	PrimID	: SV_PrimitiveID;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// Just pass data over to geometry shader.
	vout.CenterW = vin.CenterW;
	vout.Radius = vin.Radius;

	return vout;
}
 
 // We expand each point into a quad (4 vertices), so the maximum number of vertices
 // we output per geometry shader invocation is 4.
[maxvertexcount(4)]
void GS(point VertexOut gin[1], 
        uint primID : SV_PrimitiveID, 
        inout TriangleStream<GeoOut> triStream)
{
	// Output a triangle strip.
	GeoOut gout;
	[unroll]
	for(int i = 0; i < 4; ++i)
	{
		gout.PosH		= float4(gTexC[i] * 2.0f - float2(1.0f, 1.0f), 0.0f, 1.0f);
		gout.Tex		= float2(gTexC[i].x, -gTexC[i].y) + float2(0.0f, 1.0f);
		gout.SCentV		= mul(float4(gin[0].CenterW, 1.0f), gView).xyz;
		gout.SRad		= gin[0].Radius;
		gout.PrimID		= primID;
		triStream.Append(gout);
	}
}

//float4 GetFractalColorAtPoint(float3 cartesian)
//{
//	float texR = 0; // mountain ridges (ridged)
//	float texSP = 0; // hills and meadows (perlin)
//
//	uint i;
//	for (i = 0; i < 3; i++)
//	{
//		float sampledTexR = Sample3DGradient(cartesian * pow(abs(gLacunarity), i)); // abs() is only for warning
//		texR += pow(abs(1 / gLacunarity), i) * FilterRidged(sampledTexR);
//
//		float sampledTexSP = Sample3DGradient((float3(0.3f, 0.3f, 0.3f) + cartesian*2.0f) * pow(abs(gLacunarity), i));
//		texSP += pow(abs(1 / gLacunarity), i) * sampledTexSP;
//	}
//
//	texR = ScaleRidged(texR, 0.6f, 1.0f);
//	texSP = ScalePerlin(texSP, 0.0f, 0.3f);
//
//	// huge patches (continents)
//	float texHP = Sample3DGradient(float3(0.8f, 0.8f, 0.8f) + cartesian);
//	texHP = ScalePerlin(texHP, 0, 1);
//
//	// Mix
//	float tex = lerp(texSP, texR, texHP);
//
//	//if (tex < 0.5f) tex = 0.0f;
//
//	return float4(tex, 0.0f, 0.0f, 1.0f);
//}

float4 PS(GeoOut pin) : SV_Target
{
	float3 posV = lerp(
	lerp(gFrustumCorners[1].xyz, gFrustumCorners[0].xyz, pin.Tex.x),
	lerp(gFrustumCorners[3].xyz, gFrustumCorners[2].xyz, pin.Tex.x),
	pin.Tex.y);

	float3 m = -pin.SCentV;
	float sRadSq = pin.SRad*pin.SRad;
	float c = dot(m, m) - sRadSq;
	float3 d = normalize(posV);
	float b = dot(m, d);
	float discr = b*b - c;
	if (discr <= 0.0f) return float4(0.0f, 0.0f, 0.0f, 1.0f); // Missing the sphere or hitting it tangentially will cause early exit.
	float t1 = -b - sqrt(discr);
	float t2 = -b + sqrt(discr);

	float3 q1 = float3(0.0f, 0.0f, 0.0f);
	if (t1 > 0.0f) q1 += t1 * d;
	float3 q2 = float3(0.0f, 0.0f, 0.0f);
	if (t2 > 0.0f) q2 += t2 * d;

	float pz;
	DepthFrom8Bit(gNormalDepthMap.SampleLevel(samNormalDepth8Bit, pin.Tex, 0.0f), pz);
	float3 p = posV * (pz / gZFar);

	if (p.z < q1.z) q1 = p;
	if (p.z < q2.z) q2 = p;

	// Transform to world space
	q1 = mul(float4(q1, 1.0f), gInvView).xyz;
	q2 = mul(float4(q2, 1.0f), gInvView).xyz;
	float3 sphereCenterW = mul(float4(pin.SCentV, 1.0f), gInvView).xyz;
	
	// Sample values on ray
	float3 r = q2 - q1;
	float rLen = length(r);
	float dr = rLen / SAMPLES_NUM;
	float3 sum = float3(0.0f, 0.0f, 0.0f);
	float3 rt = q1;
	float3 nr = normalize(r);
	float sphereDiameter = pin.SRad * 2.0f;
	float noiseScale = 1.0f / sphereDiameter;
	[unroll]
	for (int i = 0; i < SAMPLES_NUM; ++i)
	{
		float3 textureSpace = noiseScale*(rt - sphereCenterW) - float3(0.5f, 0.5f, 0.5f);
		float color = g3DNoiseMap.Sample(samLinearWrap, textureSpace).r;
		float3 fromPalette = gPaletteMap.Sample(samLinearClamp, float2(color, 0.0f)).xyz;
		sum += dr*pow(fromPalette, 5.0f);
		rt += dr*nr;
	}

	float4 texColor = float4(sum, 1.0f);
	return texColor;
}

technique11 NebulaTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );

		// set blending
		SetBlendState(AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);
		// disable the depth buffer
		SetDepthStencilState(DisableDepth, 0);
		// set rasterizer
		SetRasterizerState(NoCulling);
    }
}
