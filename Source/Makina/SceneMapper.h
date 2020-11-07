
#ifndef SCENEMAPPER_H
#define SCENEMAPPER_H

#include "D3DAppValues.h"
#include "Effect.h"
#include "Camera.h"
#include <Windows.h>
#include <xnamath.h>

namespace Makina
{
	class SceneMapper
	{
	public:
		__declspec(dllexport) SceneMapper(D3DAppValues *values);
		__declspec(dllexport) ~SceneMapper();

		// Dynamic allocation must be aligned to the 16, therefore custom 'new' and 'delete' operators are needed.
		__declspec(dllexport) void *operator new(size_t size);
		__declspec(dllexport) void operator delete(void *pt);

		__declspec(dllexport) void SetProperties(float zFarPlane);
		__declspec(dllexport) void PrepareForMapping();
		__declspec(dllexport) void FinishMapping();

		__declspec(dllexport) void OnResize();

		ID3D11ShaderResourceView *GetSRV() { return mSRV; }
		XMFLOAT4X4 &GetCameraProj() { return mCamProj; }
		float GetZFarPlane() { return mZFarPlane; }

	private:
		__declspec(dllexport) void BuildTextureViews();

		D3DAppValues *mValues;
		float mZFarPlane;
		XMFLOAT4X4 mCamProj;
		Camera mSavedCam;

		UINT mRenderTargetWidth;
		UINT mRenderTargetHeight;

		// Texture views
		ID3D11RenderTargetView *mRTV;
		ID3D11ShaderResourceView *mSRV;
		ID3D11DepthStencilView *mDSV;
	};
}

#endif
