
#ifndef SKYBOX_H
#define SKYBOX_H

#include "D3DAppValues.h"
#include "Effect.h"

namespace Makina
{
	class SkyBox
	{
	public:
		__declspec(dllexport) SkyBox(D3DAppValues *values);
		__declspec(dllexport) ~SkyBox();
		__declspec(dllexport) void Draw();
		void SetCubeMapSRV(ID3D11ShaderResourceView *cubeMapSRV) { mCubeMap->SetResource(cubeMapSRV); }

	private:
		__declspec(dllexport) void BuildGeometryBuffers();
		__declspec(dllexport) void BuildVertexLayout();

		ID3D11Buffer *mVertexBuffer;
		ID3D11Buffer *mIndexBuffer;
		UINT mIndexCount;
		UINT mVertexCount;

		Effect mFX;
		ID3DX11EffectTechnique *mFXTech;
		ID3DX11EffectMatrixVariable *mWorldViewProj;
		ID3DX11EffectShaderResourceVariable *mCubeMap;
		ID3D11InputLayout *mInputLayout;
		
		D3DAppValues *mValues;
	};
}

#endif
