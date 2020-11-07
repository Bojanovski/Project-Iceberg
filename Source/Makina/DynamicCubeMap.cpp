#include "DynamicCubeMap.h"

using namespace Makina;

const int DynamicCubeMap::CubeMapSize = 256;

DynamicCubeMap::DynamicCubeMap(D3DAppValues *values, bool generateMipMaps)
	: mValues(values),
	mCam((Camera *)values->mCamera),
	mSavedCam(values),
	mGenerateMipMaps(generateMipMaps),
	mCubeMapSRV(0),
	mCubeMapDSV(0)
{
	BuildDynamicCubeMapViews();
}

DynamicCubeMap::~DynamicCubeMap()
{
	if (mCubeMapSRV) mCubeMapSRV->Release();
	if (mCubeMapDSV) mCubeMapDSV->Release();
	for (int i = 0; i < 6; i++)
		mCubeMapRTV[i]->Release();
}

void *DynamicCubeMap::operator new(size_t size)
{
	void *storage = _aligned_malloc(size, 16);
	if (NULL == storage)
	{
		throw AllocationError(L"No free memory. (DynamicCubeMap.cpp)");
	}
	return storage;
}

void DynamicCubeMap::operator delete(void *pt)
{
	_aligned_free(pt);
}

void DynamicCubeMap::PrepareForMapping(XMFLOAT3 &pos)
{
	// Copy camera values.
	mSavedCam = *mCam;

	// Now build new camera's vector properties.
	mCam->SetPosition(pos);
	BuildCubeFaceCamera();
}

void DynamicCubeMap::MapFaceAt(int index)
{
	// Set camera
	mCam->SetLens(0.5f*XM_PI, 1.0f, 0.1f, 1000.0f);
	mCam->LookAt(mCenter, mTargets[index], mUps[index]);

	// Set render target and viewport
	ID3D11RenderTargetView *renderTargets[1];
	renderTargets[0] = mCubeMapRTV[index];
	mValues->md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mCubeMapDSV);
	mValues->md3dImmediateContext->RSSetViewports(1, &mCubeMapViewport);

	mValues->md3dImmediateContext->ClearRenderTargetView(mCubeMapRTV[index], reinterpret_cast<const float *>(&Colors::Black));
	mValues->md3dImmediateContext->ClearDepthStencilView(mCubeMapDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	mValues->mBasicEffect->UpdateVariables();
}

void DynamicCubeMap::FinishMapping()
{
	// Copy camera values back to our camera.
	*mCam = mSavedCam;

	// Have hardware generate mipmap levels of cube map.
	if (mGenerateMipMaps)
		mValues->md3dImmediateContext->GenerateMips(mCubeMapSRV);

	mValues->mBasicEffect->UpdateVariables();
}

void DynamicCubeMap::BuildDynamicCubeMapViews()
{
	//
	// Cubemap is a special texture array with 6 elements.
	//

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = CubeMapSize;
	texDesc.Height = CubeMapSize;
	texDesc.MipLevels = 0;
	texDesc.ArraySize = 6;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	if (mGenerateMipMaps)
		texDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

	ID3D11Texture2D* cubeTex = 0;
	HRESULT hr = mValues->md3dDevice->CreateTexture2D(&texDesc, 0, &cubeTex);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create texture array for dynamic cube mapping!");

	//
	// Create a render target view to each cube map face 
	// (i.e., each element in the texture array).
	// 

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.MipSlice = 0;

	for(int i = 0; i < 6; ++i)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		hr = mValues->md3dDevice->CreateRenderTargetView(cubeTex, &rtvDesc, &mCubeMapRTV[i]);
		if (FAILED(hr))
			throw UnexpectedError(L"Failed to create texture RTV array for dynamic cube mapping!");
	}

	//
	// Create a shader resource view to the cube map.
	//

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = (mGenerateMipMaps) ? -1 : 1;

	hr = mValues->md3dDevice->CreateShaderResourceView(cubeTex, &srvDesc, &mCubeMapSRV);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create texture SRV for dynamic cube mapping!");

	cubeTex->Release();

	//
	// We need a depth texture for rendering the scene into the cubemap
	// that has the same resolution as the cubemap faces.  
	//

	D3D11_TEXTURE2D_DESC depthTexDesc;
	depthTexDesc.Width = CubeMapSize;
	depthTexDesc.Height = CubeMapSize;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;

	ID3D11Texture2D* depthTex = 0;
	hr = mValues->md3dDevice->CreateTexture2D(&depthTexDesc, 0, &depthTex);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create depth-stencil texture for dynamic cube mapping!");

	// Create the depth stencil view for the entire cube
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = depthTexDesc.Format;
	dsvDesc.Flags  = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = mValues->md3dDevice->CreateDepthStencilView(depthTex, &dsvDesc, &mCubeMapDSV);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create DSV for dynamic cube mapping!");

	depthTex->Release();

	//
	// Viewport for drawing into cubemap.
	// 

	mCubeMapViewport.TopLeftX = 0.0f;
	mCubeMapViewport.TopLeftY = 0.0f;
	mCubeMapViewport.Width    = (float)CubeMapSize;
	mCubeMapViewport.Height   = (float)CubeMapSize;
	mCubeMapViewport.MinDepth = 0.0f;
	mCubeMapViewport.MaxDepth = 1.0f;
}

void DynamicCubeMap::BuildCubeFaceCamera()
{
	// Generate the cube map about the given position.
	mCenter = XMFLOAT3(mCam->GetPosition());

	// Look along each coordinate axis.
	mTargets[0] = XMFLOAT3(mCenter.x+1.0f, mCenter.y, mCenter.z); // +X
	mTargets[1] = XMFLOAT3(mCenter.x-1.0f, mCenter.y, mCenter.z); // -X
	mTargets[2] = XMFLOAT3(mCenter.x, mCenter.y+1.0f, mCenter.z); // +Y
	mTargets[3] = XMFLOAT3(mCenter.x, mCenter.y-1.0f, mCenter.z); // -Y
	mTargets[4] = XMFLOAT3(mCenter.x, mCenter.y, mCenter.z+1.0f); // +Z
	mTargets[5] = XMFLOAT3(mCenter.x, mCenter.y, mCenter.z-1.0f); // -Z

	// Use world up vector (0,1,0) for all directions except +Y/-Y.  In these cases, we
	// are looking down +Y or -Y, so we need a different "up" vector.
	mUps[0] = XMFLOAT3(0.0f, 1.0f, 0.0f);  // +X
	mUps[1] = XMFLOAT3(0.0f, 1.0f, 0.0f);  // -X
	mUps[2] = XMFLOAT3(0.0f, 0.0f, -1.0f); // +Y
	mUps[3] = XMFLOAT3(0.0f, 0.0f, +1.0f); // -Y
	mUps[4] = XMFLOAT3(0.0f, 1.0f, 0.0f);  // +Z
	mUps[5] = XMFLOAT3(0.0f, 1.0f, 0.0f);  // -Z
}