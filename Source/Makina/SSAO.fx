#include "SSAO-BlurTech.fx"
#include "SSAO-RayCastingTech.fx"


technique11 Ssao16BitTech
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_5_0, VSRayCastingTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSRayCasting16BitTech(14) ) );
    }
}

technique11 Ssao8BitTech
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_5_0, VSRayCastingTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSRayCasting8BitTech(14) ) );
    }
}

technique11 HorzBlur16Bit
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_5_0, VSBlurTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSBlur16BitTech(true) ) );
    }
}

technique11 VertBlur16Bit
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_5_0, VSBlurTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSBlur16BitTech(false) ) );
    }
}

technique11 HorzBlur8Bit
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_5_0, VSBlurTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSBlur8BitTech(true) ) );
    }
}

technique11 VertBlur8Bit
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_5_0, VSBlurTech() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PSBlur8BitTech(false) ) );
    }
}