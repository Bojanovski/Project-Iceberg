//
//
//[numthreads(THREADS_NUMBER, 1, 1)]
//void CSJustNoise(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
//{
//	// Seed.
//	float seed = wang_hash(gSeed + dispatchThreadID.x + dispatchThreadID.y*THREADS_NUMBER);
//
//	// Generate a few numbers...
//    uint r = rand_xorshift(seed);
//    r = rand_xorshift(r);
//	
//	// Get end result.
//	float rand = float(rand_xorshift(r)) * (1.0f / 4294967296.0f);
//	gOutput[dispatchThreadID.xy] = float4(rand, rand, rand, 1.0f);
//}
//
//// -----2D-----
//
//technique11 Voronoi2D
//{
//	pass P0
//	{
//		SetVertexShader(NULL);
//		SetPixelShader(NULL);
//		SetComputeShader(CompileShader(cs_5_0, CS2DVoronoi()));
//	}
//}
//
//technique11 PerlinTech
//{
//    pass P0
//    {
//		SetVertexShader( NULL );
//        SetPixelShader( NULL );
//		SetComputeShader( CompileShader( cs_5_0, CSPerlin() ) );
//    }
//}
//
//technique11 RidgedTech
//{
//    pass P0
//    {
//		SetVertexShader( NULL );
//        SetPixelShader( NULL );
//		SetComputeShader( CompileShader( cs_5_0, CSRidged() ) );
//    }
//}
//
//technique11 RidgedPerlinMix1Tech
//{
//    pass P0
//    {
//		SetVertexShader( NULL );
//        SetPixelShader( NULL );
//		SetComputeShader( CompileShader( cs_5_0, CSRidgedPerlinMix1() ) );
//    }
//}
//
//// -----3D-----
//
//technique11 SphericalPerlinTech
//{
//    pass P0
//    {
//		SetVertexShader( NULL );
//        SetPixelShader( NULL );
//		SetComputeShader( CompileShader( cs_5_0, CSSphericalPerlin() ) );
//    }
//}
//
//technique11 SphericalRidgedTech
//{
//    pass P0
//    {
//		SetVertexShader( NULL );
//        SetPixelShader( NULL );
//		SetComputeShader( CompileShader( cs_5_0, CSSphericalRidged() ) );
//    }
//}
//
//technique11 SphericalRidgedPerlinMix1Tech
//{
//    pass P0
//    {
//		SetVertexShader( NULL );
//        SetPixelShader( NULL );
//		SetComputeShader( CompileShader( cs_5_0, CSSphericalRidgedPerlinMix1() ) );
//    }
//} 
