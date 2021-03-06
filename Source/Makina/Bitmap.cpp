#include "Bitmap.h"
#include "Geometry.h"
#include "Exceptions.h"

using namespace Makina;
using namespace std;

Bitmap::Bitmap(D3DAppValues *values, ID3D11ShaderResourceView *bitmapTexSRV, float posX, float posY, float sizeX, float sizeY, FXMVECTOR color)
: Object2D(values, posX, posY, sizeX, sizeY, color)
{
	mTextureSRV = bitmapTexSRV;

	// Get technique
	mFXTechnique = values->m2DGraphicsFX->GetTechniqueByName("BitmapLinear");

	// Create geometry
	BuildGeometryBuffers();
	CreateInputLayout();
}

Bitmap::~Bitmap()
{

}

void Bitmap::Draw()
{
	mD3DAppValues->md3dImmediateContext->IASetInputLayout(mInputLayout);
	mD3DAppValues->md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(VertexT2D);
	UINT offset = 0;
	mD3DAppValues->md3dImmediateContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	mD3DAppValues->md3dImmediateContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	mD3DAppValues->md3dImmediateContext->OMSetDepthStencilState(mDepthDisabled, 0);
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	mD3DAppValues->md3dImmediateContext->OMSetBlendState(mBlendState, blendFactor, 0xffffffff);
	mD3DAppValues->md3dImmediateContext->RSSetState(NULL);

	// Set variables
	mTextureVar->SetResource(mTextureSRV);
	XMVECTOR offsetAndSize = XMVectorSet(mPosX, mPosY, mSizeX, mSizeY);
	mOffsetAndSizeVar->SetFloatVector((float *)&offsetAndSize);
	mColorVar->SetFloatVector((float *)&mColor);

	// Draw
	D3DX11_TECHNIQUE_DESC techDesc;
	mFXTechnique->GetDesc(&techDesc);

	for (UINT i = 0; i < techDesc.Passes; i++)
	{
		mFXTechnique->GetPassByIndex(i)->Apply(0, mD3DAppValues->md3dImmediateContext);

		//6 indices for the quad
		mD3DAppValues->md3dImmediateContext->DrawIndexed(mIndexCount, 0, 0);
	}

	//// Set shader resource to null because we will use this texture later.
	//ID3D11ShaderResourceView *pSRV[1] = {NULL};
	//mD3DAppValues->md3dImmediateContext->PSSetShaderResources(0, 1, pSRV);
}

void Bitmap::ChangeProp(float posX, float posY, float sizeX, float sizeY, FXMVECTOR color)
{
	mPosX = posX;
	mPosY = posY;
	mSizeX = sizeX;
	mSizeY = sizeY;
	XMStoreFloat4(&mColor, color);
}

void Bitmap::BuildGeometryBuffers()
{
	//Release if necessary
	if (mVertexBuffer)
	{
		mVertexBuffer->Release();
		mVertexBuffer = NULL;
	}
	if (mIndexBuffer)
	{
		mIndexBuffer->Release();
		mIndexBuffer = NULL;
	}

	BasicMeshData mesh;
	GeometryGenerator::CreateFullscreenQuad(mesh);
	vector<VertexT2D> vertices(mesh.Vertices.size());
	for (UINT i = 0; i < mesh.Vertices.size(); i++)
	{
		vertices[i].Tex = mesh.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(VertexT2D)* mesh.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HRESULT hr = mD3DAppValues->md3dDevice->CreateBuffer(&vbd, &vinitData, &mVertexBuffer);
	if (FAILED(hr)) throw UnexpectedError(wstring(L"Failed to create vertex buffer! (Bitmap.cpp)"));
	mVertexCount = mesh.Vertices.size();

	// Create the index buffer
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT)* mesh.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &mesh.Indices[0];
	hr = mD3DAppValues->md3dDevice->CreateBuffer(&ibd, &iinitData, &mIndexBuffer);
	if (FAILED(hr)) throw UnexpectedError(wstring(L"Failed to create index buffer! (Bitmap.cpp)"));
	mIndexCount = mesh.Indices.size();
}

void Bitmap::CreateInputLayout()
{
	D3DX11_PASS_DESC passDesc;
	mFXTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	HRESULT hr = mD3DAppValues->md3dDevice->CreateInputLayout(VertexT2DDesc, 1, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout);
	if (FAILED(hr))throw UnexpectedError(L"Failed to create input layout! (Bitmap.cpp)");
}