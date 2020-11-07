#ifndef PARTICLES_FX
#define PARTICLES_FX

#include "Particles-Utilities.fx"
#include "Particles-Sparks.fx"
#include "Particles-Fire.fx"

GeometryShader gsSparksStreamOut = ConstructGSWithSO( 
	CompileShader( gs_5_0, StreamOutSparksGS() ), 
	"POSITION.xyz; VELOCITY.xyz; SIZE.xy; AGE.x; TYPE.x" );
	
technique11 SparksUpdate
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, StreamOutSparksVS() ) );
		SetGeometryShader(gsSparksStreamOut);
        
        // disable pixel shader for stream-out only
        SetPixelShader(NULL);
        
        // we must also disable the depth buffer for stream-out only
        SetDepthStencilState( DisableDepth, 0 );
    }
}

technique11 SparksDraw
{
    pass P0
    {
        SetVertexShader(   CompileShader( vs_5_0, DrawSparksVS() ) );
        SetGeometryShader( CompileShader( gs_5_0, DrawSparksGS() ) );
        SetPixelShader(    CompileShader( ps_5_0, DrawSparksPS() ) );
        
        SetDepthStencilState( NoDepthWrites, 0 );
    }
}

GeometryShader gsFireStreamOut = ConstructGSWithSO(
	CompileShader(gs_5_0, StreamOutFireGS()),
	"POSITION.xyz; VELOCITY.xyz; SIZE.xy; AGE.x; TYPE.x");

technique11 FireUpdate
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, StreamOutFireVS()));
		SetGeometryShader(gsFireStreamOut);

		// disable pixel shader for stream-out only
		SetPixelShader(NULL);

		// we must also disable the depth buffer for stream-out only
		SetDepthStencilState(DisableDepth, 0);
	}
}

technique11 FireDraw
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, DrawFireVS()));
		SetGeometryShader(CompileShader(gs_5_0, DrawFireGS()));
		SetPixelShader(CompileShader(ps_5_0, DrawFirePS()));

		SetBlendState(AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);
		SetDepthStencilState(NoDepthWrites, 0);
	}
}

#endif