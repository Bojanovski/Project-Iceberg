
#ifndef RIDGEDPERLINMIX1_FX
#define RIDGEDPERLINMIX1_FX

#include "TextureGenerator-Utilities.fx"

[numthreads(THREADS_NUMBER, 1, 1)]
void CSRidgedPerlinMix1(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	// Ridged and perlin
	float texR = 0; // mountain ridges
	float texSP = 0; // hills and meadows

	uint i;
	for (i = 0; i < gOctaves; i++)
	{
		float sampledTexR = SampleGradient(dispatchThreadID.xy * pow(abs(gLacunarity), i)); // abs() is only for warning
		texR += pow(abs(1/gLacunarity), i) * FilterRidged(sampledTexR);
		
		float sampledTexSP = SampleGradient((float2(gTexDimensions[0]*0.3f, gTexDimensions[1]*0.3f) + dispatchThreadID.xy*2.0f) * pow(abs(gLacunarity), i));
		texSP += pow(abs(1/gLacunarity), i) * sampledTexSP;
	}

	texR = ScaleRidged(texR, 0.6f, 1.0f);
	texSP = ScalePerlin(texSP, 0.0f, 0.3f);

	// huge patches (continents)
	float texHP = SampleGradient(float2(gTexDimensions[0]*0.8f, gTexDimensions[1]*0.8f) + dispatchThreadID.xy);
	texHP = ScalePerlin(texHP, 0, 1);

	// Mix
	float tex = lerp(texSP, texR, texHP);

	//if (tex < 0.5f) tex = 0.0f;

	gOutput[dispatchThreadID.xy] = float4(tex, 0.0f, 0.0f, 1.0f);
}

[numthreads(THREADS_NUMBER, 1, 1)]
void CSSphericalRidgedPerlinMix1(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	// Spherical to cartesian transformation http://www.math.montana.edu/frankw/ccp/multiworld/multipleIVP/spherical/learn.htm
	float rho = 0.9f; // just not to catch tiled gradients on the edge of the box, better against repetitiveness
	float theta = dispatchThreadID.x/gTexDimensions[0]*PI*2.0;
	float phi = dispatchThreadID.y/gTexDimensions[1]*PI;
	float3 cartesian = float3(rho*sin(theta)*sin(phi), rho*cos(phi), rho*cos(theta)*sin(phi)); // in range [-1,1]
	// set to [0,1]
	cartesian = (cartesian + float3(1.0f, 1.0f, 1.0f))*0.5f;

	// Now to sampling
	float texR = 0; // mountain ridges (ridged)
	float texSP = 0; // hills and meadows (perlin)

	uint i;
	for (i = 0; i < gOctaves; i++)
	{
	float sampledColor = Sample3DGradient(cartesian * pow(abs(gLacunarity), i));// abs() is only for warning

		float sampledTexR = Sample3DGradient(cartesian * pow(abs(gLacunarity), i)); // abs() is only for warning
		texR += pow(abs(1/gLacunarity), i) * FilterRidged(sampledTexR);

		float sampledTexSP = Sample3DGradient((float3(0.3f, 0.3f, 0.3f) + cartesian*2.0f) * pow(abs(gLacunarity), i));
		texSP += pow(abs(1/gLacunarity), i) * sampledTexSP;
	}

	texR = ScaleRidged(texR, 0.6f, 1.0f);
	texSP = ScalePerlin(texSP, 0.0f, 0.3f);

	// huge patches (continents)
	float texHP = Sample3DGradient(float3(0.8f, 0.8f, 0.8f) + cartesian);
	texHP = ScalePerlin(texHP, 0, 1);

	// Mix
	float tex = lerp(texSP, texR, texHP);

	//if (tex < 0.5f) tex = 0.0f;

	gOutput[dispatchThreadID.xy] = float4(tex, 0.0f, 0.0f, 1.0f);
}

[numthreads(THREADS_NUMBER, 1, 1)]
void CS3DRidgedPerlinMix1(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	float x = dispatchThreadID.x / gTexDimensions[0];
	float y = dispatchThreadID.y / gTexDimensions[1];
	float z = dispatchThreadID.z / gTexDimensions[2];
	float3 cartesian = float3(x, y, z); // in range [0,1]

	float3 center = float3(0.5f, 0.5f, 0.5f);
	float amount = 1.0f - length(center - cartesian) * 2.0f;
	amount = pow(abs(amount), 0.5f); // lessen it towards the edges

	// Now to sampling
	float texR = 0; // mountain ridges (ridged)
	float texSP = 0; // hills and meadows (perlin)

	uint i;
	for (i = 0; i < gOctaves; i++)
	{
		float sampledColor = Sample3DGradient(cartesian * pow(abs(gLacunarity), i));// abs() is only for warning

		float sampledTexR = Sample3DGradient(cartesian * pow(abs(gLacunarity), i)); // abs() is only for warning
		texR += pow(abs(1 / gLacunarity), i) * FilterRidged(sampledTexR);

		float sampledTexSP = Sample3DGradient((float3(0.3f, 0.3f, 0.3f) + cartesian*2.0f) * pow(abs(gLacunarity), i));
		texSP += pow(abs(1 / gLacunarity), i) * sampledTexSP;
	}

	texR = ScaleRidged(texR, 0.6f, 1.0f);
	texSP = ScalePerlin(texSP, 0.0f, 0.3f);

	// huge patches (continents)
	float texHP = Sample3DGradient(float3(0.8f, 0.8f, 0.8f) + cartesian);
	texHP = ScalePerlin(texHP, 0, 1);

	// Mix
	float tex = lerp(texSP, texR, texHP);

	//if (tex < 0.5f) tex = 0.0f;

	gOutput3D[dispatchThreadID.xyz] = float4(tex * amount, 0.0f, 0.0f, 1.0f);
}

#endif
