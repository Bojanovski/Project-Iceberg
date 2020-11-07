
#ifndef RIDGEDMULTIFRACTAL_FX
#define RIDGEDMULTIFRACTAL_FX

#include "TextureGenerator-Utilities.fx"

[numthreads(THREADS_NUMBER, 1, 1)]
void CSRidged(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	float tex = 0;

	for (uint i = 0; i < gOctaves; i++)
	{
		float sampledColor = SampleGradient(dispatchThreadID.xy * pow(abs(gLacunarity), i)); // abs() is only for warning
		tex += pow(abs(1/gLacunarity), i) * FilterRidged(sampledColor);
	}

	tex = ScaleRidged(tex, 0, 1);


	gOutput[dispatchThreadID.xy] = float4(tex, 0.0f, 0.0f, 1.0f);
}

[numthreads(THREADS_NUMBER, 1, 1)]
void CSSphericalRidged(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	// Spherical to cartesian transformation http://www.math.montana.edu/frankw/ccp/multiworld/multipleIVP/spherical/learn.htm
	float rho = 0.9f; // just not to catch tiled gradients on the edge of the box, better against repetitiveness
	float theta = dispatchThreadID.x/gTexDimensions[0]*PI*2.0;
	float phi = dispatchThreadID.y/gTexDimensions[1]*PI;
	float3 cartesian = float3(rho*sin(theta)*sin(phi), rho*cos(phi), rho*cos(theta)*sin(phi)); // in range [-1,1]
	// set to [0,1]
	cartesian = (cartesian + float3(1.0f, 1.0f, 1.0f))*0.5f;

	// Now to sampling
	float tex = 0;
	for (uint i = 0; i < gOctaves; i++)
	{
		float sampledColor = Sample3DGradient(cartesian * pow(abs(gLacunarity), i));// abs() is only for warning
		tex += pow(abs(1/gLacunarity), i) * FilterRidged(sampledColor);
	}

	tex = ScaleRidged(tex, 0, 1);


	gOutput[dispatchThreadID.xy] = float4(tex, 0.0f, 0.0f, 1.0f);
}

#endif
