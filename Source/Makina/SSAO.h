#ifndef SSAO_H
#define SSAO_H

#include "D3DAppValues.h"
#include "Effect.h"
#include "Camera.h"
#include "SceneMapper.h"
#include <Windows.h>
#include <xnamath.h>

namespace Makina
{
	class SSAO : public Effect
	{
	public:
		__declspec(dllexport) SSAO(D3DAppValues *values, SceneMapper *ndm);
		__declspec(dllexport) ~SSAO();

		// Dynamic allocation must be aligned to the 16, therefore custom 'new' and 'delete' operators are needed.
		__declspec(dllexport) void *operator new(size_t size);
		__declspec(dllexport) void operator delete(void *pt);

		__declspec(dllexport) void SetProperties(float occlusionRadius, float occlusionFadeStart, float occlusionFadeEnd, float surfaceEpsilon);
		__declspec(dllexport) void ComputeSsao();
		__declspec(dllexport) void BlurAmbientMap(int blurCount);

		__declspec(dllexport) void OnResize();

		ID3D11ShaderResourceView *GetSRV() { return mAmbientSRV0; }

	private:	
		__declspec(dllexport) void BlurAmbientMap(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV, bool horzBlur);
		__declspec(dllexport) void BuildFullScreenQuad();
		__declspec(dllexport) void CreateInputLayout();
		__declspec(dllexport) void BuildRandomVectorTexture();
		__declspec(dllexport) void BuildTextureViews();
		__declspec(dllexport) void BuildOffsetVectors();
		__declspec(dllexport) void BuildFrustumFarCorners();

		ID3D11Buffer *mScreenQuadVB;
		ID3D11Buffer *mScreenQuadIB;
		ID3D11InputLayout *mInputLayout;

		SceneMapper *mNDM;

		UINT mRenderTargetWidth;
		UINT mRenderTargetHeight;
		D3D11_VIEWPORT mAmbientMapViewport;

		XMFLOAT4 mFrustumFarCorner[4];
		XMFLOAT4 mOffsets[14];

		// Texture views
		ID3D11ShaderResourceView *mRandomVectorSRV;

		// Need two for ping-ponging during blur.
		ID3D11RenderTargetView *mAmbientRTV0;
		ID3D11ShaderResourceView *mAmbientSRV0;

		ID3D11RenderTargetView *mAmbientRTV1;
		ID3D11ShaderResourceView *mAmbientSRV1;

		// Effect variables
		ID3DX11EffectTechnique *mSSAO16BitTech;
		ID3DX11EffectTechnique *mSSAO8BitTech;
		ID3DX11EffectTechnique *mSSAOHorzBlur16BitTech;
		ID3DX11EffectTechnique *mSSAOVertBlur16BitTech;
		ID3DX11EffectTechnique *mSSAOHorzBlur8BitTech;
		ID3DX11EffectTechnique *mSSAOVertBlur8BitTech;

		ID3DX11EffectMatrixVariable *mViewToTexSpace;
		ID3DX11EffectVectorVariable *mOffsetVectors;
		ID3DX11EffectVectorVariable *mFrustumCorners;
		ID3DX11EffectScalarVariable *mOcclusionRadius;
		ID3DX11EffectScalarVariable *mOcclusionFadeStart;
		ID3DX11EffectScalarVariable *mOcclusionFadeEnd;
		ID3DX11EffectScalarVariable *mSurfaceEpsilon;

		ID3DX11EffectScalarVariable *mTextelWidth;
		ID3DX11EffectScalarVariable *mTextelHeight;		
		
		ID3DX11EffectScalarVariable *mZNear;
		ID3DX11EffectScalarVariable *mZFar;

		ID3DX11EffectShaderResourceVariable *mNormalDepthMap;
		ID3DX11EffectShaderResourceVariable *mRandomVecMap;
		ID3DX11EffectShaderResourceVariable *mInputImage;
	};
}

#endif
