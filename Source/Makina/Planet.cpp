
#include "Planet.h"
#include "Resource.h"
#include "Exceptions.h"
#include "Camera.h"
#include "SceneMapper.h"
#include "TextureGenerator.h"

using namespace std;
using namespace Makina;

struct BillboardVertex
{
	XMFLOAT3 CenterW;
	float Radius;
};

const D3D11_INPUT_ELEMENT_DESC BillboardVertexDesc[2] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "SIZE", 0, DXGI_FORMAT_R32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

Planet::Planet(D3DAppValues *values, int seed, SceneMapper *ndm, TextureGenerator *texGen)
: Effect(values, L"Makina.dll", ID_NEBULA),
PCGObject(seed),
mNDM(ndm)
{
	// Vertex buffer
	BillboardVertex v;
	v.CenterW = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v.Radius = 5.0f;

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(BillboardVertex);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &v;

	HRESULT hr = values->md3dDevice->CreateBuffer(&vbd, &vinitData, &mBillboardVB);
	if (FAILED(hr))
		throw UnexpectedError(L"Planet::Planet() was unable to create vertex buffer.");

	// Effect
	mNebulaTech = mFX->GetTechniqueByName("NebulaTech");
	mView = mFX->GetVariableByName("gView")->AsMatrix();
	mInvView = mFX->GetVariableByName("gInvView")->AsMatrix();
	mFrustumCorners = mFX->GetVariableByName("gFrustumCorners")->AsVector();
	mNearZ = mFX->GetVariableByName("gZNear")->AsScalar();
	mFarZ = mFX->GetVariableByName("gZFar")->AsScalar();
	mNormalDepthMap = mFX->GetVariableByName("gNormalDepthMap")->AsShaderResource();
	m3DNoiseMap = mFX->GetVariableByName("g3DNoiseMap")->AsShaderResource();
	mPaletteMap = mFX->GetVariableByName("gPaletteMap")->AsShaderResource();

	texGen->Generate3DRidgedPerlinMix1(mSeed, 256, 256, 256, &mNoiseSRV);
	vector<XMCOLOR> colors;
	colors.push_back({ 0.0f, 0.0f, 0.0f, 1.0f });
	colors.push_back({ 0.2f, 0.0f, 0.3f, 1.0f });
	colors.push_back({ 0.4f, 0.3f, 0.5f, 1.0f });
	colors.push_back({ 0.7f, 0.0f, 0.7f, 1.0f });
	colors.push_back({ 0.1f, 0.8f, 0.2f, 1.0f });
	colors.push_back({ 0.8f, 0.0f, 0.9f, 1.0f });

	//colors.push_back({ 0.2f, 0.8f, 0.4f, 1.0f });

	colors.push_back({ 1.0f, 1.0f, 1.0f, 1.0f });
	texGen->GenerateColorPaletteMap(colors, &mPaletteSRV);

	BuildVertexLayout();
	OnResize();
}

Planet::~Planet()
{
	ReleaseCOM(mBillboardVB);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mNoiseSRV);
	ReleaseCOM(mPaletteSRV);
}

void Planet::Update(float dt)
{

}

void Planet::Draw(float dt)
{
	mValues->md3dImmediateContext->IASetInputLayout(mInputLayout);
	mValues->md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(BillboardVertex);
	UINT offset = 0;
	mValues->md3dImmediateContext->IASetVertexBuffers(0, 1, &mBillboardVB, &stride, &offset);

	mFrustumCorners->SetFloatVectorArray(reinterpret_cast<float*>(&mFrustumFarCorner), 0, 4);
	XMVECTOR det;
	XMMATRIX viewInv = XMMatrixInverse(&det, mValues->mCamera->View());
	mView->SetMatrix(reinterpret_cast<float*>(&mValues->mCamera->View()));
	mInvView->SetMatrix(reinterpret_cast<float*>(&viewInv));
	mNearZ->SetFloat(mValues->mCamera->GetNearZ());
	mFarZ->SetFloat(mValues->mCamera->GetFarZ());
	mNormalDepthMap->SetResource(mNDM->GetSRV());
	m3DNoiseMap->SetResource(mNoiseSRV);
	mPaletteMap->SetResource(mPaletteSRV);

	D3DX11_TECHNIQUE_DESC techDesc;
	mNebulaTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mNebulaTech->GetPassByIndex(p)->Apply(0, mValues->md3dImmediateContext);
		mValues->md3dImmediateContext->Draw(1, 0);
	}
}

void Planet::OnResize()
{
	// Build frustum far corners
	float halfHeight = mNDM->GetZFarPlane() * tanf(0.5f*mValues->mCamera->GetFovY());
	float halfWidth = mValues->mCamera->GetAspect() * halfHeight;

	mFrustumFarCorner[0] = XMFLOAT4(+halfWidth, +halfHeight, mNDM->GetZFarPlane(), 0.0f);
	mFrustumFarCorner[1] = XMFLOAT4(-halfWidth, +halfHeight, mNDM->GetZFarPlane(), 0.0f);
	mFrustumFarCorner[2] = XMFLOAT4(+halfWidth, -halfHeight, mNDM->GetZFarPlane(), 0.0f);
	mFrustumFarCorner[3] = XMFLOAT4(-halfWidth, -halfHeight, mNDM->GetZFarPlane(), 0.0f);
}

void Planet::BuildVertexLayout()
{
	//Create input layout
	D3DX11_PASS_DESC passDesc;
	mNebulaTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HRESULT hr = mValues->md3dDevice->CreateInputLayout(BillboardVertexDesc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create input layout in Planet::BuildVertexLayout()!");
}