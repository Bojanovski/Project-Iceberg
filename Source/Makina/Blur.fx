cbuffer cbSettings
{
	float gWeights[11] = 
	{
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f,
	};
}

#define BLURRADIUS 5
#define N 256
#define CACHESIZE (N + 2 * BLURRADIUS)

groupshared float4 gCache[CACHESIZE];

Texture2D gInput;
RWTexture2D<float4> gOutput;

[numthreads(N, 1, 1)]
void HorzBlurCS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	//
	// Fill local thread storage to reduce bandwidth.  To blur 
	// N pixels, we will need to load N + 2*BlurRadius pixels
	// due to the blur radius.
	//
	
	// This thread group runs N threads.  To get the extra 2*BlurRadius pixels, 
	// have 2*BlurRadius threads sample an extra pixel.
	if(groupThreadID.x < BLURRADIUS)
	{
		// Clamp out of bound samples that occur at image borders.
		int x = max(dispatchThreadID.x - BLURRADIUS, 0);
		gCache[groupThreadID.x] = gInput[int2(x, dispatchThreadID.y)];
	}
	if(groupThreadID.x >= N - BLURRADIUS)
	{
		// Clamp out of bound samples that occur at image borders.
		int x = min(dispatchThreadID.x + BLURRADIUS, gInput.Length.x-1);
		gCache[groupThreadID.x+2 * BLURRADIUS] = gInput[int2(x, dispatchThreadID.y)];
	}

	// Clamp out of bound samples that occur at image borders.
	gCache[groupThreadID.x + BLURRADIUS] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];

	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();
	
	//
	// Now blur each pixel.
	//

	float4 blurColor = float4(0, 0, 0, 0);
	
	[unroll]
	for(int i = -BLURRADIUS; i <= BLURRADIUS; ++i)
	{
		int k = groupThreadID.x + BLURRADIUS + i;
		
		blurColor += gWeights[i + BLURRADIUS] * gCache[k];
	}
	
	gOutput[dispatchThreadID.xy] = blurColor;
}

[numthreads(1, N, 1)]
void VertBlurCS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	//
	// Fill local thread storage to reduce bandwidth.  To blur 
	// N pixels, we will need to load N + 2*BlurRadius pixels
	// due to the blur radius.
	//
	
	// This thread group runs N threads.  To get the extra 2*BlurRadius pixels, 
	// have 2*BlurRadius threads sample an extra pixel.
	if(groupThreadID.y < BLURRADIUS)
	{
		// Clamp out of bound samples that occur at image borders.
		int y = max(dispatchThreadID.y - BLURRADIUS, 0);
		gCache[groupThreadID.y] = gInput[int2(dispatchThreadID.x, y)];
	}
	if(groupThreadID.y >= N - BLURRADIUS)
	{
		// Clamp out of bound samples that occur at image borders.
		int y = min(dispatchThreadID.y + BLURRADIUS, gInput.Length.y-1);
		gCache[groupThreadID.y+2 * BLURRADIUS] = gInput[int2(dispatchThreadID.x, y)];
	}
	
	// Clamp out of bound samples that occur at image borders.
	gCache[groupThreadID.y + BLURRADIUS] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];


	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();
	
	//
	// Now blur each pixel.
	//

	float4 blurColor = float4(0, 0, 0, 0);
	
	[unroll]
	for(int i = -BLURRADIUS; i <= BLURRADIUS; ++i)
	{
		int k = groupThreadID.y + BLURRADIUS + i;
		
		blurColor += gWeights[i + BLURRADIUS] * gCache[k];
	}
	
	gOutput[dispatchThreadID.xy] = blurColor;
}

technique11 HorzBlur
{
    pass P0
    {
		SetVertexShader( NULL );
        SetPixelShader( NULL );
		SetComputeShader( CompileShader( cs_5_0, HorzBlurCS() ) );
    }
}

technique11 VertBlur
{
    pass P0
    {
		SetVertexShader( NULL );
        SetPixelShader( NULL );
		SetComputeShader( CompileShader( cs_5_0, VertBlurCS() ) );
    }
}
