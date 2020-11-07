
#ifndef VORONOI2D_FX
#define VORONOI2D_FX

#include "TextureGenerator-Utilities.fx"

struct Voronoi2DPoint
{
	float3 gColor;
	float2 gPos;
};

cbuffer cbVoronoi
{
	int gVoronoiPointsNum;
};

StructuredBuffer<Voronoi2DPoint> gVoronoi2DPoints;

[numthreads(THREADS_NUMBER, 1, 1)]
void CS2DVoronoi(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	float2 offset = dispatchThreadID.xy - gVoronoi2DPoints[0].gPos;
	float lSq = dot(offset, offset);
	float closestSoFarSq = lSq;
	float4 tex = float4(gVoronoi2DPoints[0].gColor, 1.0f);

	for (int i = 1; i < gVoronoiPointsNum; i++)
	{
		float2 offset = dispatchThreadID.xy - gVoronoi2DPoints[i].gPos;
		float lSq = dot(offset, offset);
		if (lSq < closestSoFarSq)
		{
			closestSoFarSq = lSq;
			tex = float4(gVoronoi2DPoints[i].gColor, 1.0f);
		}
	}
	gOutput[dispatchThreadID.xy] = tex;
}

#endif
