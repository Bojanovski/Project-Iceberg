#include "SceneMapper.h"
#include "Resource.h"
#include "MathHelper.h"
#include "Exceptions.h"
#include "Camera.h"
#include "D3DUtilities.h"

using namespace Makina;

SceneMapper::SceneMapper(D3DAppValues *values)
: mValues(values),
mSavedCam(values),
mZFarPlane(values->mCamera->GetFarZ()),
mRTV(0),
mSRV(0),
mDSV(0)
{
	OnResize();
}

SceneMapper::~SceneMapper()
{
	mRTV->Release();
	mSRV->Release();
	mDSV->Release();
}

void *SceneMapper::operator new(size_t size)
{
	void *storage = _aligned_malloc(size, 16);
	if (NULL == storage)
	{
		throw AllocationError(L"No free memory. (SceneMapper.cpp)");
	}
	return storage;
}

void SceneMapper::operator delete(void *pt)
{
	_aligned_free(pt);
}

void SceneMapper::SetProperties(float zFarPlane)
{
	// this is needed for camera adjustment
	mZFarPlane = zFarPlane;
}

void SceneMapper::PrepareForMapping()
{
	ID3D11RenderTargetView *renderTargets[1] = { mRTV };
	mValues->md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDSV);
	mValues->md3dImmediateContext->ClearDepthStencilView(mDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	mValues->md3dImmediateContext->RSSetViewports(1, &mValues->mScreenViewport);

	// The this map might be at any slot, so clear all slots.
	ID3D11ShaderResourceView* nullSRV[16] = { 0 };
	mValues->md3dImmediateContext->PSSetShaderResources(0, 16, nullSRV);

	// Clear view space normal to (0,0) and clear depth to be far away.  
	float clearColor[] = { 0.0f, 0.0f, 1e5f, 1e5f };
	mValues->md3dImmediateContext->ClearRenderTargetView(mRTV, clearColor);

	// adjust camera
	mSavedCam = *mValues->mCamera;
	mValues->mCamera->SetLens(mValues->mCamera->GetFovY(), mValues->mCamera->GetAspect(), mValues->mCamera->GetNearZ(), mZFarPlane);
	XMStoreFloat4x4(&mCamProj, mValues->mCamera->Proj());
}

void SceneMapper::FinishMapping()
{
	// restore camera
	*mValues->mCamera = mSavedCam;
}

void SceneMapper::OnResize()
{
	mRenderTargetWidth = mValues->mClientWidth;
	mRenderTargetHeight = mValues->mClientHeight;

	BuildTextureViews();
}


void SceneMapper::BuildTextureViews()
{
	if (mRTV) mRTV->Release();
	if (mSRV) mSRV->Release();
	if (mDSV) mDSV->Release();

	//--------------------------------------------------
	//				Normal Depth Views
	//--------------------------------------------------

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = mRenderTargetWidth;
	texDesc.Height = mRenderTargetHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;


	ID3D11Texture2D* tex = 0;
	HRESULT hr1 = mValues->md3dDevice->CreateTexture2D(&texDesc, 0, &tex);
	HRESULT hr2 = mValues->md3dDevice->CreateShaderResourceView(tex, 0, &mSRV);
	HRESULT hr3 = mValues->md3dDevice->CreateRenderTargetView(tex, 0, &mRTV);
	if (FAILED(hr1) || FAILED(hr2) || FAILED(hr3))
		throw UnexpectedError(L"Failed to create normal depth texture views (SceneMapper).");

	// view saves a reference.
	tex->Release();

	//--------------------------------------------------
	//				Depth Stencil View
	//--------------------------------------------------

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = mRenderTargetWidth;
	depthStencilDesc.Height = mRenderTargetHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	ID3D11Texture2D *dsTex = 0;
	hr1 = mValues->md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &dsTex);
	hr2 = mValues->md3dDevice->CreateDepthStencilView(dsTex, 0, &mDSV);
	if (FAILED(hr1) || FAILED(hr2))
		throw UnexpectedError(L"Failed to create depth stencil view (SceneMapper).");

	// View saves the reference
	dsTex->Release();
}