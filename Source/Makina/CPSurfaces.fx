
#ifndef BSPLINESURFACES_FX
#define BSPLINESURFACES_FX

#include "LightsUtilities.fx"
#include "CPSurfaces-Utilities.fx"
#include "CPSurfaces-FinalComplete.fx"
#include "CPSurfaces-NormalDepthTech.fx"

technique11 BSplineDraw_FinalComplete
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS_FinalComplete()));
		SetGeometryShader(CompileShader(gs_5_0, GS_FinalComplete()));
		SetPixelShader(CompileShader(ps_5_0, PS_FinalComplete()));

		SetRasterizerState(NoCulling);
	}
}

technique11 BSplineDraw_DepthOnly
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS_DepthOnly()));
		SetGeometryShader(CompileShader(gs_5_0, GS_DepthOnly()));
		SetPixelShader(NULL);

		SetRasterizerState(Depth);
	}
}

technique11 BSplineDraw_DepthOnlyAlphaClip
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS_DepthOnlyAlphaClip()));
		SetGeometryShader(CompileShader(gs_5_0, GS_DepthOnlyAlphaClip()));
		SetPixelShader(CompileShader(ps_5_0, PS_DepthOnlyAlphaClip()));

		SetRasterizerState(Depth);
	}
}

#endif