
#ifndef PLANET_H
#define PLANET_H

#include "D3DAppValues.h"
#include "Effect.h"
#include "PCGObject.h"

namespace Makina
{
	class SceneMapper;
	class TextureGenerator;

	class Planet : public Effect, public PCGObject
	{
	public:
		__declspec(dllexport) Planet(D3DAppValues *values, int seed, SceneMapper *ndm, TextureGenerator *texGen);
		__declspec(dllexport) ~Planet();

		__declspec(dllexport) void Update(float dt);
		__declspec(dllexport) void Draw(float dt);
		__declspec(dllexport) void OnResize();

	private:
		void BuildVertexLayout();

		SceneMapper *mNDM;
		XMFLOAT4 mFrustumFarCorner[4];

		ID3D11Buffer *mBillboardVB;
		ID3DX11EffectTechnique *mNebulaTech;
		ID3D11InputLayout *mInputLayout;
		ID3DX11EffectMatrixVariable *mView;
		ID3DX11EffectMatrixVariable *mInvView;
		ID3DX11EffectVectorVariable *mFrustumCorners;
		ID3DX11EffectScalarVariable *mNearZ;
		ID3DX11EffectScalarVariable *mFarZ;
		ID3DX11EffectShaderResourceVariable *mNormalDepthMap;
		ID3DX11EffectShaderResourceVariable *m3DNoiseMap;
		ID3DX11EffectShaderResourceVariable *mPaletteMap;
		ID3D11ShaderResourceView *mNoiseSRV;
		ID3D11ShaderResourceView *mPaletteSRV;
	};
}

#endif
