
#ifndef DYNAMICCUBEMAP_H
#define DYNAMICCUBEMAP_H

#include "D3DUtilities.h"
#include "Camera.h"
#include "BasicEffect.h"
#include "d3dx11Effect.h"

namespace Makina
{
	class DynamicCubeMap
	{
	public:
		__declspec(dllexport) DynamicCubeMap(D3DAppValues *values, bool generateMipMaps);
		__declspec(dllexport) ~DynamicCubeMap();

		// Dynamic allocation must be aligned to the 16, therefore custom 'new' and 'delete' operators are needed.
		__declspec(dllexport) void *operator new(size_t size);
		__declspec(dllexport) void operator delete(void *pt);

		__declspec(dllexport) void PrepareForMapping(XMFLOAT3 &pos);
		__declspec(dllexport) void MapFaceAt(int index);
		__declspec(dllexport) void FinishMapping();

		__declspec(dllexport) ID3D11ShaderResourceView *GetSRV() { return mCubeMapSRV; };

		static const int CubeMapSize;

	private:
		__declspec(dllexport) void BuildDynamicCubeMapViews();
		__declspec(dllexport) void BuildCubeFaceCamera();

		bool mGenerateMipMaps;

		D3DAppValues *mValues;
		Camera *mCam;
		Camera mSavedCam;
		XMFLOAT3 mCenter;
		XMFLOAT3 mTargets[6];
		XMFLOAT3 mUps[6];

		D3D11_VIEWPORT mCubeMapViewport;

		ID3D11ShaderResourceView *mCubeMapSRV;
		ID3D11RenderTargetView *mCubeMapRTV[6];
		ID3D11DepthStencilView *mCubeMapDSV;
	};
}

#endif
