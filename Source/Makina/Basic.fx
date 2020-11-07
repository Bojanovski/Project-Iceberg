#include "LightsUtilities.fx"
#include "Basic-NormalDepthTech.fx"
#include "Basic-GouraudTech.fx"
#include "Basic-LightDirTech.fx"
#include "Basic-LightDirShadowTech.fx"
#include "Basic-LightDirShadowAndSSAOTech.fx"
#include "Basic-LightDirTessTech.fx"
#include "Basic-LightFullTech.fx"
#include "Basic-LightAndNTB.fx"
#include "Basic-JustTexture.fx"
#include "Basic-LightReflectionTech.fx"
#include "Basic-SkinnedLightDirShadowAndSSAOTech.fx"
#include "Basic-SkinnedNormalDepthTech.fx"

technique11 DepthOnlyTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VSDepthOnlyTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( NULL );

		SetRasterizerState(Depth);
    }
}

technique11 DepthOnlyAlphaClipTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VSDepthOnlyAlphaClipTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSDepthOnlyAlphaClipTech() ) );
    }
}

technique11 NormalAndDepth16BitTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VSNormalAndDepthTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSNormalAndDepth16BitTech() ) );

		SetRasterizerState(BackfaceCull);
    }
}

technique11 NormalAndDepthAlphaClip16BitTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VSNormalAndDepthTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSNormalAndDepthAlphaClip16BitTech() ) );

		SetRasterizerState(BackfaceCull);
    }
}

technique11 NormalAndDepth8BitTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VSNormalAndDepthTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSNormalAndDepth8BitTech() ) );

		SetRasterizerState(BackfaceCull);
    }
}

technique11 NormalAndDepthAlphaClip8BitTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VSNormalAndDepthTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSNormalAndDepthAlphaClip8BitTech() ) );

		SetRasterizerState(BackfaceCull);
    }
}

technique11 GouraudTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VSGouraudTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSGouraudTech() ) );
    }
}

technique11 GouraudFastTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VSGouraudTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSGouraudFastTech() ) );
    }
}

technique11 LightFullTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VSLightFullTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSLightFullTech() ) );
    }
}

technique11 LightDirTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VSLightFullTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSLightDirTech() ) );
    }
}

technique11 LightDirShadowTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VSLightDirShadowTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSLightDirShadowTech() ) );
    }
}

technique11 LightDirShadowAndSSAOTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VSLightDirShadowAndSSAOTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSLightDirShadowAndSSAOTech(false) ) );
    }
}

technique11 LightDirShadowReflectionAndSSAOTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VSLightDirShadowAndSSAOTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSLightDirShadowAndSSAOTech(true) ) );
    }
}

technique11 LightDirTessTech
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_5_0, VSLightDirTessTech() ) );
        SetHullShader( CompileShader( hs_5_0, HSLightDirTessTech() ) );
        SetDomainShader( CompileShader( ds_5_0, DSLightDirTessTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSLightDirTessTech() ) );
    }
}

technique11 LightReflectionTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VSLightFullTech() ) ); // vs stays the same
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSLightReflectionTech() ) );
    }
}

technique11 LightAndNTB
{
	// First draw everything...
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VSLightFullTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSLightDirTech() ) );
    }
	
	// ... and now normal, tangent and bitangent.
    pass P1
    {
        SetVertexShader( CompileShader( vs_5_0, NTB_VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, NTB_GS() ) );
        SetPixelShader( CompileShader( ps_5_0, NTB_PS() ) );
    }
}

technique11 JustTexture
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VSJustTexture() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSJustTexture() ) );
    }
}

// Skinned mesh techniques

technique11 SkinnedDepthOnlyTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VSSkinnedDepthOnlyTech()));
		SetGeometryShader(NULL);
		SetPixelShader(NULL);

		SetRasterizerState(Depth);
	}
}

technique11 SkinnedDepthOnlyAlphaClipTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VSSkinnedDepthOnlyAlphaClipTech()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSDepthOnlyAlphaClipTech()));
	}
}

technique11 SkinnedNormalAndDepth16BitTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VSSkinnedNormalAndDepthTech()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSNormalAndDepth16BitTech()));

		SetRasterizerState(BackfaceCull);
	}
}

technique11 SkinnedNormalAndDepthAlphaClip16BitTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VSSkinnedNormalAndDepthTech()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSNormalAndDepthAlphaClip16BitTech()));

		SetRasterizerState(BackfaceCull);
	}
}

technique11 SkinnedNormalAndDepth8BitTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VSSkinnedNormalAndDepthTech()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSNormalAndDepth8BitTech()));

		SetRasterizerState(BackfaceCull);
	}
}

technique11 SkinnedNormalAndDepthAlphaClip8BitTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VSSkinnedNormalAndDepthTech()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSNormalAndDepthAlphaClip8BitTech()));

		SetRasterizerState(BackfaceCull);
	}
}

technique11 SkinnedLightDirShadowAndSSAOTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VSSkinnedLightDirShadowAndSSAOTech()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSLightDirShadowAndSSAOTech(false)));
	}
}
