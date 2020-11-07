#include "ShadowMap.h"
#include "Exceptions.h"

using namespace Makina;

ShadowMap::ShadowMap(D3DAppValues *values, UINT size)
	: mValues(values),
	mType(None),
	mSize(size),
	mDepthMapDSV(0),
	mDepthMapSRV(0)
{
	mViewport.TopLeftX = 0.0f;
	mViewport.TopLeftY = 0.0f;
	mViewport.Width    = static_cast<float>(mSize);
	mViewport.Height   = static_cast<float>(mSize);
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;

	// Use typeless format because the DSV is going to interpret
	// the bits as DXGI_FORMAT_D24_UNORM_S8_UINT, whereas the SRV is going to interpret
	// the bits as DXGI_FORMAT_R24_UNORM_X8_TYPELESS.
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width     = mSize;
	texDesc.Height    = mSize;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format    = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count   = 1;  
	texDesc.SampleDesc.Quality = 0;  
	texDesc.Usage          = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0; 
	texDesc.MiscFlags      = 0;

	ID3D11Texture2D* depthMap = 0;
	HRESULT hr = mValues->md3dDevice->CreateTexture2D(&texDesc, 0, &depthMap);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create texture for shadow mapping!");

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = mValues->md3dDevice->CreateDepthStencilView(depthMap, &dsvDesc, &mDepthMapDSV);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create DSV for shadow mapping!");

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	hr = mValues->md3dDevice->CreateShaderResourceView(depthMap, &srvDesc, &mDepthMapSRV);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create SRV for shadow mapping!");

	// View saves a reference to the texture so we can release our reference.
	depthMap->Release();
}

ShadowMap::~ShadowMap()
{
	ReleaseCOM(mDepthMapDSV);
	ReleaseCOM(mDepthMapSRV);
}

void *ShadowMap::operator new(size_t size)
{
	void *storage = _aligned_malloc(size, 16);
	if (NULL == storage)
	{
		throw AllocationError(L"No free memory. (ShadowMap.cpp)");
	}
	return storage;
}

void ShadowMap::operator delete(void *pt)
{
	_aligned_free(pt);
}

void ShadowMap::PrepareForMappingOrthographic(float dimensions, FXMVECTOR pos, FXMVECTOR direction)
{
	float width = dimensions, height = dimensions, nearZ = 1.0f, farZ = 1.0f + dimensions;

	XMVector3Normalize(direction);
	XMVECTOR posT = pos - direction * (dimensions/2.0f + 1.0f);
	XMVECTOR target = pos + direction;

	PrepareForMappingOrthographic(width, height, nearZ, farZ, posT, target, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
}

void ShadowMap::PrepareForMappingOrthographic(float viewWidth, float viewHeight, float nearZ, float farZ, FXMVECTOR pos, FXMVECTOR target, FXMVECTOR up)
{
	mType = Orthographic;

	// First camera matrices
	XMMATRIX P = XMMatrixOrthographicLH(viewWidth, viewHeight, nearZ, farZ);
	XMStoreFloat4x4(&mProj, P);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMStoreFloat4x4(&mTransform, T);

	// Unbind shadow map as a shader input because we are going to render to it now.
	// The shadow might might be at any slot, so clear all slots.
	ID3D11ShaderResourceView* nullSRV[16] = { 0 };
	mValues->md3dImmediateContext->PSSetShaderResources(0, 16, nullSRV);

	// Now immediate context
	mValues->md3dImmediateContext->RSSetViewports(1, &mViewport);

	// Set null render target because we are only going to draw to depth buffer.
	// Setting a null render target will disable color writes.
	ID3D11RenderTargetView* renderTargets[1] = {0};
	mValues->md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthMapDSV);

	mValues->md3dImmediateContext->ClearDepthStencilView(mDepthMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// OBB culling
	OrientedBox temp;
	temp.Center = XMFLOAT3(0.0f, 0.0f, nearZ + (farZ - nearZ)/2);
	temp.Extents = XMFLOAT3(viewWidth/2.0f, viewHeight/2.0f, (farZ - nearZ)/2);
	temp.Orientation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	XMVECTOR detView;
	XMMATRIX invView = XMMatrixInverse(&detView, V);
	XMVECTOR scale, rotQuat, translation;
	XMMatrixDecompose(&scale, &rotQuat, &translation, invView);

	TransformOrientedBox(&mCamVolume, &temp, XMVectorGetX(scale), rotQuat, translation);
}

void ShadowMap::PrepareForMappingProjective(float fovY, float nearZ, float farZ, FXMVECTOR pos, FXMVECTOR direction)
{
	XMVECTOR target = pos + direction;

	PrepareForMappingProjective(fovY, 1.0f, nearZ, farZ, pos, target, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
}

void ShadowMap::PrepareForMappingProjective(float fovY, float aspect, float nearZ, float farZ, FXMVECTOR pos, FXMVECTOR target, FXMVECTOR up)
{
	mType = Projective;

	// First camera matrices
	XMMATRIX P = XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);
	XMStoreFloat4x4(&mProj, P);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMStoreFloat4x4(&mTransform, T);

	// Unbind shadow map as a shader input because we are going to render to it now.
	// The shadow might might be at any slot, so clear all slots.
	ID3D11ShaderResourceView* nullSRV[16] = { 0 };
	mValues->md3dImmediateContext->PSSetShaderResources(0, 16, nullSRV);

	// Now immediate context
	mValues->md3dImmediateContext->RSSetViewports(1, &mViewport);

	// Set null render target because we are only going to draw to depth buffer.
	// Setting a null render target will disable color writes.
	ID3D11RenderTargetView* renderTargets[1] = {0};
	mValues->md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthMapDSV);

	mValues->md3dImmediateContext->ClearDepthStencilView(mDepthMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Frustum culling
	Frustum temp;
	ComputeFrustumFromProjection(&temp, &P);

	XMVECTOR detView;
	XMMATRIX invView = XMMatrixInverse(&detView, V);
	XMVECTOR scale, rotQuat, translation;
	XMMatrixDecompose(&scale, &rotQuat, &translation, invView);
	TransformFrustum(&mCamFrustum, &temp, XMVectorGetX(scale), rotQuat, translation);
}

void ShadowMap::FinishMapping()
{
	mType = None;
}