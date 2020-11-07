
#ifndef OBJECT2D_H
#define OBJECT2D_H

#include <Windows.h>
#include <xnamath.h>
#include "D3DAppValues.h"

namespace Makina
{
	class Object2D
	{
	public:
		__declspec(dllexport) Object2D(D3DAppValues *values, float posX, float posY, float sizeX, float sizeY, FXMVECTOR color);
		__declspec(dllexport) virtual ~Object2D();

		virtual void Draw() = 0;
		virtual void OnResize() {}
		void SetSRV(ID3D11ShaderResourceView *DiffuseMapSRV) { this->mTextureSRV = DiffuseMapSRV; }
		ID3D11ShaderResourceView *GetSRV() { return this->mTextureSRV; }

	protected:
		D3DAppValues *mD3DAppValues;

		ID3D11Buffer *mVertexBuffer;
		ID3D11Buffer *mIndexBuffer;
		UINT mIndexCount;
		UINT mVertexCount;
		ID3D11InputLayout *mInputLayout;

		// Effect related
		ID3DX11EffectTechnique *mFXTechnique;
		ID3DX11EffectShaderResourceVariable *mTextureVar;
		ID3DX11EffectVectorVariable *mOffsetAndSizeVar;
		ID3DX11EffectVectorVariable *mColorVar;

		// Texture view
		ID3D11ShaderResourceView *mTextureSRV;

		// The rest
		ID3D11DepthStencilState *mDepthDisabled;
		ID3D11BlendState *mBlendState;

		float mPosX, mPosY, mSizeX, mSizeY;
		XMFLOAT4 mColor;
	};
}

#endif
