#ifndef BLUREFFECT_H
#define BLUREFFECT_H

#include "D3DUtilities.h"
#include "d3dx11Effect.h"
#include "D3DAppValues.h"
#include "Effect.h"

namespace Makina
{
	class BlurEffect : public Effect
	{
	public:
		__declspec(dllexport) BlurEffect(D3DAppValues *values, const DXGI_FORMAT &Format);
		__declspec(dllexport) ~BlurEffect();

		__declspec(dllexport) void OnResize();
		__declspec(dllexport) void SetGaussianWeights(float sigma);
		__declspec(dllexport) void Blur(ID3D11ShaderResourceView* inputSRV, ID3D11UnorderedAccessView* inputUAV, int blurCount);

	private:
		ID3DX11EffectTechnique* mHorzBlurTech;
		ID3DX11EffectTechnique* mVertBlurTech;

		ID3DX11EffectScalarVariable* mWeights;
		ID3DX11EffectShaderResourceVariable* mInputMap;
		ID3DX11EffectUnorderedAccessViewVariable* mOutputMap;

		// Textures
		DXGI_FORMAT mFormat;
		ID3D11ShaderResourceView* mBlurredOutputTexSRV;
		ID3D11UnorderedAccessView* mBlurredOutputTexUAV;
	};
}

#endif