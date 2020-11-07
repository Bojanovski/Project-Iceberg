
#ifndef FUNCTIONS_FX
#define FUNCTIONS_FX

#include "TextureGenerator-Utilities.fx"

[numthreads(THREADS_NUMBER, 1, 1)]
void CS3DSphericalGradient(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	float3 p;
	p.x = dispatchThreadID.x / gTexDimensions[0];
	p.y = dispatchThreadID.y / gTexDimensions[1];
	p.z = dispatchThreadID.z / gTexDimensions[2];

	float3 center = float3(0.5f, 0.5f, 0.5f);
	float value = 1.0f - length(center - p) * 2.0f;

	gOutput3D[dispatchThreadID.xyz] = float4(value, 0.0f, 0.0f, 1.0f);
}

#endif
