#include "SSAO.h"
#include "Resource.h"
#include "MathHelper.h"
#include "Exceptions.h"
#include "Camera.h"
#include "D3DUtilities.h"

using namespace Makina;

SSAO::SSAO(D3DAppValues *values, SceneMapper *ndm)
	: Effect(values, L"Makina.dll", ID_SSAO),
	mNDM(ndm),
	mAmbientRTV0(0),
	mAmbientSRV0(0),
	mAmbientRTV1(0),
	mAmbientSRV1(0)
{
	mSSAO16BitTech			= mFX->GetTechniqueByName("Ssao16BitTech");
	mSSAO8BitTech			= mFX->GetTechniqueByName("Ssao8BitTech");
	mSSAOHorzBlur16BitTech  = mFX->GetTechniqueByName("HorzBlur16Bit");
	mSSAOVertBlur16BitTech  = mFX->GetTechniqueByName("VertBlur16Bit");
	mSSAOHorzBlur8BitTech   = mFX->GetTechniqueByName("HorzBlur8Bit");
	mSSAOVertBlur8BitTech   = mFX->GetTechniqueByName("VertBlur8Bit");

	mViewToTexSpace			= mFX->GetVariableByName("gViewToTexSpace")->AsMatrix();
	mOffsetVectors			= mFX->GetVariableByName("gOffsetVectors")->AsVector();
	mFrustumCorners			= mFX->GetVariableByName("gFrustumCorners")->AsVector();
	mOcclusionRadius		= mFX->GetVariableByName("gOcclusionRadius")->AsScalar();
	mOcclusionFadeStart		= mFX->GetVariableByName("gOcclusionFadeStart")->AsScalar();
	mOcclusionFadeEnd		= mFX->GetVariableByName("gOcclusionFadeEnd")->AsScalar();
	mSurfaceEpsilon			= mFX->GetVariableByName("gSurfaceEpsilon")->AsScalar();

	mTextelWidth			= mFX->GetVariableByName("gTexelWidth")->AsScalar();
	mTextelHeight			= mFX->GetVariableByName("gTexelHeight")->AsScalar();	
	
	mZNear					= mFX->GetVariableByName("gZNear")->AsScalar();
	mZFar					= mFX->GetVariableByName("gZFar")->AsScalar();

	mNormalDepthMap			= mFX->GetVariableByName("gNormalDepthMap")->AsShaderResource();
	mRandomVecMap			= mFX->GetVariableByName("gRandomVecMap")->AsShaderResource();
	mInputImage				= mFX->GetVariableByName("gInputImage")->AsShaderResource();

	BuildFullScreenQuad();
	CreateInputLayout();

	BuildRandomVectorTexture();
	BuildOffsetVectors();

	OnResize();
}

SSAO::~SSAO()
{
	mScreenQuadVB->Release();
	mScreenQuadIB->Release();
	mInputLayout->Release();

	mRandomVectorSRV->Release();

	mAmbientRTV0->Release();
	mAmbientSRV0->Release();

	mAmbientRTV1->Release();
	mAmbientSRV1->Release();
}

void *SSAO::operator new(size_t size)
{
	void *storage = _aligned_malloc(size, 16);
	if (NULL == storage)
	{
		throw AllocationError(L"No free memory. (SSAO.cpp)");
	}
	return storage;
}

void SSAO::operator delete(void *pt)
{
	_aligned_free(pt);
}

void SSAO::SetProperties(float occlusionRadius, float occlusionFadeStart, float occlusionFadeEnd, float surfaceEpsilon)
{
	// this goes directly to effect
	mOcclusionRadius->SetFloat(occlusionRadius);
	mOcclusionFadeStart->SetFloat(occlusionFadeStart);
	mOcclusionFadeEnd->SetFloat(occlusionFadeEnd);
	mSurfaceEpsilon->SetFloat(surfaceEpsilon);
}

void SSAO::ComputeSsao()
{
	// Bind the ambient map as the render target.  Observe that this pass does not bind 
	// a depth/stencil buffer--it does not need it, and without one, no depth test is
	// performed, which is what we want.
	ID3D11RenderTargetView* renderTargets[1] = {mAmbientRTV0};
	mValues->md3dImmediateContext->OMSetRenderTargets(1, renderTargets, 0);
	mValues->md3dImmediateContext->ClearRenderTargetView(mAmbientRTV0, reinterpret_cast<const float*>(&Colors::Black));
	mValues->md3dImmediateContext->RSSetViewports(1, &mAmbientMapViewport);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	static const XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX P = XMLoadFloat4x4(&mNDM->GetCameraProj());
	XMMATRIX PT = XMMatrixMultiply(P, T);

	mViewToTexSpace->SetMatrix(reinterpret_cast<float*>(&PT));
	mOffsetVectors->SetFloatVectorArray(reinterpret_cast<float*>(&mOffsets), 0, 14);
	mFrustumCorners->SetFloatVectorArray(reinterpret_cast<float*>(&mFrustumFarCorner), 0, 4);
	mRandomVecMap->SetResource(mRandomVectorSRV);
	mNormalDepthMap->SetResource(mNDM->GetSRV());

	mZNear->SetFloat(mValues->mCamera->GetNearZ());
	mZFar->SetFloat(mNDM->GetZFarPlane());

	UINT stride = sizeof(VertexPNT);
    UINT offset = 0;

	mValues->md3dImmediateContext->IASetInputLayout(mInputLayout);
    mValues->md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mValues->md3dImmediateContext->IASetVertexBuffers(0, 1, &mScreenQuadVB, &stride, &offset);
	mValues->md3dImmediateContext->IASetIndexBuffer(mScreenQuadIB, DXGI_FORMAT_R16_UINT, 0);
	
	ID3DX11EffectTechnique* tech = mSSAO8BitTech;
	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		tech->GetPassByIndex(p)->Apply(0, mValues->md3dImmediateContext);
		mValues->md3dImmediateContext->DrawIndexed(6, 0, 0);
    }
}

void SSAO::BlurAmbientMap(int blurCount)
{
	for(int i = 0; i < blurCount; ++i)
	{
		// Ping-pong the two ambient map textures as we apply
		// horizontal and vertical blur passes.
		BlurAmbientMap(mAmbientSRV0, mAmbientRTV1, true);
		BlurAmbientMap(mAmbientSRV1, mAmbientRTV0, false);
	}
}

void SSAO::BlurAmbientMap(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV, bool horzBlur)
{
	ID3D11RenderTargetView* renderTargets[1] = {outputRTV};
	mValues->md3dImmediateContext->OMSetRenderTargets(1, renderTargets, 0);
	mValues->md3dImmediateContext->ClearRenderTargetView(outputRTV, reinterpret_cast<const float*>(&Colors::Black));
	mValues->md3dImmediateContext->RSSetViewports(1, &mAmbientMapViewport);

	mTextelWidth->SetFloat(1.0f / mAmbientMapViewport.Width );
	mTextelHeight->SetFloat(1.0f / mAmbientMapViewport.Height );
	mInputImage->SetResource(inputSRV);
	mNormalDepthMap->SetResource(mNDM->GetSRV());

	mZNear->SetFloat(mValues->mCamera->GetNearZ());
	mZFar->SetFloat(mNDM->GetZFarPlane());

	ID3DX11EffectTechnique* tech;
	if(horzBlur)
	{
		tech = mSSAOHorzBlur8BitTech;
	}
	else
	{
		tech = mSSAOVertBlur8BitTech;
	}

	UINT stride = sizeof(VertexPNT);
    UINT offset = 0;

	mValues->md3dImmediateContext->IASetInputLayout(mInputLayout);
    mValues->md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mValues->md3dImmediateContext->IASetVertexBuffers(0, 1, &mScreenQuadVB, &stride, &offset);
	mValues->md3dImmediateContext->IASetIndexBuffer(mScreenQuadIB, DXGI_FORMAT_R16_UINT, 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		tech->GetPassByIndex(p)->Apply(0, mValues->md3dImmediateContext);
		mValues->md3dImmediateContext->DrawIndexed(6, 0, 0);

		// Unbind the input SRV as it is going to be an output in the next blur.
		mInputImage->SetResource(0);
		tech->GetPassByIndex(p)->Apply(0, mValues->md3dImmediateContext);
    }
}

void SSAO::OnResize()
{
	mRenderTargetWidth = mValues->mClientWidth;
	mRenderTargetHeight = mValues->mClientHeight;

	BuildFrustumFarCorners();
	BuildTextureViews();

	// We render to ambient map at half the resolution.
	mAmbientMapViewport.TopLeftX = 0.0f;
	mAmbientMapViewport.TopLeftY = 0.0f;
	mAmbientMapViewport.Width = mRenderTargetWidth / 2.0f;
	mAmbientMapViewport.Height = mRenderTargetHeight / 2.0f;
	mAmbientMapViewport.MinDepth = 0.0f;
	mAmbientMapViewport.MaxDepth = 1.0f;
}

void SSAO::BuildFullScreenQuad()
{
	VertexPNT v[4];

	v[0].Pos = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	v[1].Pos = XMFLOAT3(-1.0f, +1.0f, 0.0f);
	v[2].Pos = XMFLOAT3(+1.0f, +1.0f, 0.0f);
	v[3].Pos = XMFLOAT3(+1.0f, -1.0f, 0.0f);

	// Store far plane frustum corner indices in Normal.x slot.
	v[0].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v[1].Normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
	v[2].Normal = XMFLOAT3(2.0f, 0.0f, 0.0f);
	v[3].Normal = XMFLOAT3(3.0f, 0.0f, 0.0f);

	v[0].Tex = XMFLOAT2(0.0f, 1.0f);
	v[1].Tex = XMFLOAT2(0.0f, 0.0f);
	v[2].Tex = XMFLOAT2(1.0f, 0.0f);
	v[3].Tex = XMFLOAT2(1.0f, 1.0f);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(VertexPNT) * 4;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;

	HRESULT hr = mValues->md3dDevice->CreateBuffer(&vbd, &vinitData, &mScreenQuadVB);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create screen quad VB (SSAO).");

	USHORT indices[6] = 
	{
		0, 1, 2,
		0, 2, 3
	};

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * 6;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;

	hr = mValues->md3dDevice->CreateBuffer(&ibd, &iinitData, &mScreenQuadIB);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create screen quad IB (SSAO).");
}

void SSAO::CreateInputLayout()
{
	//Create input layout
	D3DX11_PASS_DESC passDesc;
	mSSAO8BitTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HRESULT hr = mValues->md3dDevice->CreateInputLayout(VertexPNTDesc, 3, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout);
	if(FAILED(hr))
		throw UnexpectedError(L"Failed to create input layout! (SSAO.cpp)");
}

void SSAO::BuildRandomVectorTexture()
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 256;
	texDesc.Height = 256;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {0};
	initData.SysMemPitch = 256*sizeof(XMCOLOR);

	XMCOLOR color[256*256];
	for(int i = 0; i < 256; ++i)
	{
		for(int j = 0; j < 256; ++j)
		{
			XMFLOAT3 v(MathHelper::RandF(), MathHelper::RandF(), MathHelper::RandF());

			color[i*256+j] = XMCOLOR(v.x, v.y, v.z, 0.0f);
		}
	}

	initData.pSysMem = color;

	ID3D11Texture2D* tex = 0;
	HRESULT hr = mValues->md3dDevice->CreateTexture2D(&texDesc, &initData, &tex);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create random texture (SSAO).");

	hr = mValues->md3dDevice->CreateShaderResourceView(tex, 0, &mRandomVectorSRV);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create random texture SRV (SSAO).");

	// view saves a reference.
	tex->Release();
}

void SSAO::BuildTextureViews()
{
	if (mAmbientRTV0) mAmbientRTV0->Release();
	if (mAmbientSRV0) mAmbientSRV0->Release();

	if (mAmbientRTV1) mAmbientRTV1->Release();
	if (mAmbientSRV1) mAmbientSRV1->Release();

	// Render ambient map at half resolution.
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	texDesc.Width = mRenderTargetWidth / 2;
	texDesc.Height = mRenderTargetHeight / 2;
	texDesc.Format = DXGI_FORMAT_R16_FLOAT;

	ID3D11Texture2D* ambientTex0 = 0;
	HRESULT hr1 = mValues->md3dDevice->CreateTexture2D(&texDesc, 0, &ambientTex0);
	HRESULT hr2 = mValues->md3dDevice->CreateShaderResourceView(ambientTex0, 0, &mAmbientSRV0);
	HRESULT hr3 = mValues->md3dDevice->CreateRenderTargetView(ambientTex0, 0, &mAmbientRTV0);
	if (FAILED(hr1) || FAILED(hr2) || FAILED(hr3))
		throw UnexpectedError(L"Failed to create ambient map texture views (SSAO).");


	ID3D11Texture2D* ambientTex1 = 0;
	hr1 = mValues->md3dDevice->CreateTexture2D(&texDesc, 0, &ambientTex1);
	hr2 = mValues->md3dDevice->CreateShaderResourceView(ambientTex1, 0, &mAmbientSRV1);
	hr3 = mValues->md3dDevice->CreateRenderTargetView(ambientTex1, 0, &mAmbientRTV1);
	if (FAILED(hr1) || FAILED(hr2) || FAILED(hr3))
		throw UnexpectedError(L"Failed to create ambient map texture views (SSAO).");

	// view saves a reference.
	ambientTex0->Release();
	ambientTex1->Release();
}

void SSAO::BuildFrustumFarCorners()
{
	float aspect = (float)mRenderTargetWidth / (float)mRenderTargetHeight;

	float halfHeight = mNDM->GetZFarPlane() * tanf(0.5f*mValues->mCamera->GetFovY());
	float halfWidth  = aspect * halfHeight;

	mFrustumFarCorner[0] = XMFLOAT4(-halfWidth, -halfHeight, mNDM->GetZFarPlane(), 0.0f);
	mFrustumFarCorner[1] = XMFLOAT4(-halfWidth, +halfHeight, mNDM->GetZFarPlane(), 0.0f);
	mFrustumFarCorner[2] = XMFLOAT4(+halfWidth, +halfHeight, mNDM->GetZFarPlane(), 0.0f);
	mFrustumFarCorner[3] = XMFLOAT4(+halfWidth, -halfHeight, mNDM->GetZFarPlane(), 0.0f);
}

void SSAO::BuildOffsetVectors()
{
	// Start with 14 uniformly distributed vectors.  We choose the 8 corners of the cube
	// and the 6 center points along each cube face.  We always alternate the points on 
	// opposites sides of the cubes.  This way we still get the vectors spread out even
	// if we choose to use less than 14 samples.

	// 8 cube corners
	mOffsets[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	mOffsets[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

	mOffsets[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	mOffsets[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

	mOffsets[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	mOffsets[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

	mOffsets[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	mOffsets[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

	// 6 centers of cube faces
	mOffsets[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	mOffsets[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

	mOffsets[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	mOffsets[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

	mOffsets[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	mOffsets[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

	for(int i = 0; i < 14; ++i)
	{
		// Create random lengths in [0.25, 1.0].
		float s = MathHelper::RandF(0.25f, 1.0f);

		XMVECTOR v = s * XMVector4Normalize(XMLoadFloat4(&mOffsets[i]));

		XMStoreFloat4(&mOffsets[i], v);
	}
}