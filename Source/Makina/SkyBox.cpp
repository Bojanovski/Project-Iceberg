#include "SkyBox.h"
#include "D3DUtilities.h"
#include "Exceptions.h"
#include "Resource.h"
#include "Geometry.h"
#include "Camera.h"

using namespace Makina;
using namespace std;

SkyBox::SkyBox(D3DAppValues *values)
	: mValues(values),
	mFX(Effect(values, L"Makina.dll", ID_SKY))
{
	mFXTech				= mFX.GetFX()->GetTechniqueByName("SkyTech");
	mWorldViewProj		= mFX.GetFX()->GetVariableByName("gWorldViewProj")->AsMatrix();
	mCubeMap			= mFX.GetFX()->GetVariableByName("gCubeMap")->AsShaderResource();

	BuildGeometryBuffers();
	BuildVertexLayout();
}

SkyBox::~SkyBox()
{
	mVertexBuffer->Release();
	mIndexBuffer->Release();
	mInputLayout->Release();
}

void SkyBox::Draw()
{
	mValues->md3dImmediateContext->IASetInputLayout(mInputLayout);
	mValues->md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride =sizeof(XMFLOAT3);
	UINT offset = 0;
	mValues->md3dImmediateContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	mValues->md3dImmediateContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	mValues->md3dImmediateContext->OMSetDepthStencilState(0, 0);
	mValues->md3dImmediateContext->RSSetState(0);
	mValues->md3dImmediateContext->OMSetBlendState(0, 0, 0xffffffff);

	//set constants
	XMFLOAT3 eyePos = ((Camera *)mValues->mCamera)->GetPosition();
	XMMATRIX world = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
	XMMATRIX worldViewProj = world * ((Camera *)mValues->mCamera)->View() * ((Camera *)mValues->mCamera)->Proj();
	worldViewProj = XMMatrixMultiply(world, ((Camera *)mValues->mCamera)->ViewProj());
	mWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

	D3DX11_TECHNIQUE_DESC techDesc;
	mFXTech->GetDesc(&techDesc);

	for (UINT i = 0; i < techDesc.Passes; i++)
	{
		mFXTech->GetPassByIndex(i)->Apply(0, mValues->md3dImmediateContext);

		//36 indices for the box
		mValues->md3dImmediateContext->DrawIndexed(mIndexCount, 0, 0);
	}
}

void SkyBox::BuildGeometryBuffers()
{
	BasicMeshData mesh; // radius doesn't matter
	GeometryGenerator::CreateSphere(3.0f, 3, 3, mesh);

	vector<XMFLOAT3> points;
	for(UINT i = 0; i < mesh.Vertices.size(); i++)
		points.push_back(mesh.Vertices[i].Position);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(XMFLOAT3) * points.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &points[0];
	mValues->md3dDevice->CreateBuffer(&vbd, &vinitData, &mVertexBuffer);
	mVertexCount = points.size();

	// Create the index buffer
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mesh.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &mesh.Indices[0];
	mValues->md3dDevice->CreateBuffer(&ibd, &iinitData, &mIndexBuffer);
	mIndexCount = mesh.Indices.size();
}

void SkyBox::BuildVertexLayout()
{
	//Create input layout
	D3DX11_PASS_DESC passDesc;
	mFXTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HRESULT hr = mValues->md3dDevice->CreateInputLayout(VertexPDesc, 1, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout);
	if(FAILED(hr))
		throw UnexpectedError(L"Failed to create input layout in SkyObj!");
}