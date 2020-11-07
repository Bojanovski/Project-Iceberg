#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "D3DAppValues.h"
#include "XnaCollision.h"
#include <Windows.h>
#include <xnamath.h>

namespace Makina
{
	enum ShadowMappingType { Orthographic, Projective, None };

	class ShadowMap
	{
	public:
		__declspec(dllexport) ShadowMap(D3DAppValues *values, UINT size);
		__declspec(dllexport) ~ShadowMap();

		// Dynamic allocation must be aligned to the 16, therefore custom 'new' and 'delete' operators are needed.
		__declspec(dllexport) void *operator new(size_t size);
		__declspec(dllexport) void operator delete(void *pt);

		__declspec(dllexport) void PrepareForMappingOrthographic(float dimensions, FXMVECTOR pos, FXMVECTOR direction);
		__declspec(dllexport) void PrepareForMappingOrthographic(float viewWidth, float viewHeight, float nearZ, float farZ, FXMVECTOR pos, FXMVECTOR target, FXMVECTOR up);
		__declspec(dllexport) void PrepareForMappingProjective(float fovY, float nearZ, float farZ, FXMVECTOR pos, FXMVECTOR direction);
		__declspec(dllexport) void PrepareForMappingProjective(float fovY, float aspect, float nearZ, float farZ, FXMVECTOR pos, FXMVECTOR target, FXMVECTOR up);
		__declspec(dllexport) void FinishMapping();

		ID3D11ShaderResourceView *GetSRV() { return mDepthMapSRV; }

		UINT GetSize() { return mSize; }
		ShadowMappingType GetType() { return mType; }
		Frustum GetFrustum() { return mCamFrustum; }
		OrientedBox GetOrientedBox() { return mCamVolume; }

		// Get View / Proj matrices.
		XMMATRIX GetView() { return XMLoadFloat4x4(&mView); }
		XMMATRIX GetProj() { return XMLoadFloat4x4(&mProj); }
		XMMATRIX GetViewProj() { return XMLoadFloat4x4(&mView) * XMLoadFloat4x4(&mProj); }
		XMMATRIX GetTransform() { return XMLoadFloat4x4(&mTransform); }
		XMMATRIX GetViewProjTransform() { return XMLoadFloat4x4(&mView) * XMLoadFloat4x4(&mProj) * XMLoadFloat4x4(&mTransform); }

	private:
		D3DAppValues *mValues;
		ShadowMappingType mType;

		// Culling
		Frustum mCamFrustum;
		OrientedBox mCamVolume;

		// Cache View/Proj matrices.
		XMFLOAT4X4 mView;
		XMFLOAT4X4 mProj;
		XMFLOAT4X4 mTransform;

		// Texture properties
		UINT mSize;
		ID3D11ShaderResourceView *mDepthMapSRV;
		ID3D11DepthStencilView *mDepthMapDSV;
		D3D11_VIEWPORT mViewport;
	};
}

#endif
