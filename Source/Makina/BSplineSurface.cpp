
#include "BSplineSurface.h"
#include "Exceptions.h"
#include "RenderStatesManager.h"

using namespace Makina;
using namespace std;

struct VertexBSpline
{
	XMFLOAT2 Tex;
};

static D3D11_INPUT_ELEMENT_DESC VertexBSplineDesc[] =
{
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

BSplineSurface::BSplineSurface(D3DAppValues *values, UINT countU, UINT countV, float uSize, float vSize)
: mValues(values),
mUCP_Count(countU),
mVCP_Count(countV),
mMaxTessDistance(5.0f),
mMinTessDistance(70.0f),
mMinTessFactor(1.0f),
mMaxTessFactor(4.0f)
{
	// Technique
	mBSplineDraw_FinalComplete = mValues->mCPSurfacesFX->GetTechniqueByName("BSplineDraw_FinalComplete");
	mBSplineDraw_DepthOnly = mValues->mCPSurfacesFX->GetTechniqueByName("BSplineDraw_DepthOnly");
	mBSplineDraw_DepthOnlyAlphaClip = mValues->mCPSurfacesFX->GetTechniqueByName("BSplineDraw_DepthOnlyAlphaClip");

	// And now the variables.
	mDirLightning = mValues->mCPSurfacesFX->GetVariableByName("gDirLight");
	mViewProj = mValues->mCPSurfacesFX->GetVariableByName("gViewProj")->AsMatrix();
	mEyePosW = mValues->mCPSurfacesFX->GetVariableByName("gEyePosW")->AsVector();
	mMaterial = mValues->mCPSurfacesFX->GetVariableByName("gMaterial");
	mCPVar = mValues->mCPSurfacesFX->GetVariableByName("gCP");
	mCenterVar = mValues->mCPSurfacesFX->GetVariableByName("gCenter")->AsVector();
	mNUVar = mValues->mCPSurfacesFX->GetVariableByName("gNU")->AsScalar();
	mNVVar = mValues->mCPSurfacesFX->GetVariableByName("gNV")->AsScalar();
	mShadowTransform = mValues->mCPSurfacesFX->GetVariableByName("gShadowTransform")->AsMatrix();
	mShadowMapSize = mValues->mCPSurfacesFX->GetVariableByName("gShadowMapSize")->AsScalar();

	mMaxTessDistanceVar = mValues->mCPSurfacesFX->GetVariableByName("gMaxTessDistance")->AsScalar();
	mMinTessDistanceVar = mValues->mCPSurfacesFX->GetVariableByName("gMinTessDistance")->AsScalar();
	mMinTessFactorVar = mValues->mCPSurfacesFX->GetVariableByName("gMinTessFactor")->AsScalar();
	mMaxTessFactorVar = mValues->mCPSurfacesFX->GetVariableByName("gMaxTessFactor")->AsScalar();

	mDiffuseMapVar = mValues->mCPSurfacesFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	//mNormalMap = mValues->mCPSurfacesFX->GetVariableByName("gNormalMap")->AsShaderResource();
	mShadowMapVar = mValues->mCPSurfacesFX->GetVariableByName("gShadowMap")->AsShaderResource();

	BuildGeometryBuffers(uSize, vSize);
	BuildVertexLayout();
	mMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMat.Reflect = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMat.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 5.0f);

	mRastState = mValues->mRenderStatesManager->GetRS(L"normal");
}

BSplineSurface::~BSplineSurface()
{
	mInputLayout->Release();
	mVertexBuffer->Release();
	mIndexBuffer->Release();
}

void *BSplineSurface::operator new(size_t size)
{
	void *storage = _aligned_malloc(size, 16);
	if (NULL == storage)
	{
		throw AllocationError(L"No free memory. (BSplineSurface.cpp)");
	}
	return storage;
}

void BSplineSurface::operator delete(void *pt)
{
	_aligned_free(pt);
}

void BSplineSurface::RebuildBoundingVolume()
{
	int cpNum = mUCP_Count * mVCP_Count;
	void *pData = &mCP[0];
	ComputeBoundingAxisAlignedBoxFromPoints(&mBoundingVolume, cpNum, (const XMFLOAT3 *)(pData), sizeof(XMFLOAT4));
}

void AverageXYZ(XMFLOAT3 &in1, XMFLOAT3 &in2, XMFLOAT4 *out)
{
	out->x = (in1.x + in2.x) * 0.5f;
	out->y = (in1.y + in2.y) * 0.5f;
	out->z = (in1.z + in2.z) * 0.5f;
}

void BSplineSurface::BuildGeometryBuffers(float uSize, float vSize)
{
	BasicMeshData mesh;
	GeometryGenerator::CreateGrid(uSize, vSize, mVCP_Count, mUCP_Count, mesh);
	vector<VertexBSpline> vertices;
	for (UINT i = 0; i < mesh.Vertices.size(); ++i)
	{
		VertexBSpline temp;
		mCP[i] = XMFLOAT4(mesh.Vertices[i].Position.x, mesh.Vertices[i].Position.y, mesh.Vertices[i].Position.z, 1.0f);

		// In this implementation every knot is influenced by 2 control points except the ones
		// at the beginning and at the end of the B-Spline. Therefore stretching occurs and the
		// easiest way of handling this is by altering the control points at the border of the
		// surface.

		// Borders
		if (i / mUCP_Count == 0) // first row
			AverageXYZ(mesh.Vertices[i].Position, mesh.Vertices[i + mUCP_Count].Position, &mCP[i]);
		if (i / mUCP_Count == (mVCP_Count - 1)) // last row
			AverageXYZ(mesh.Vertices[i].Position, mesh.Vertices[i - mUCP_Count].Position, &mCP[i]);
		if (i % mUCP_Count == 0) // first column
			AverageXYZ(mesh.Vertices[i].Position, mesh.Vertices[i + 1].Position, &mCP[i]);
		if (i % mUCP_Count == (mUCP_Count - 1)) // last column
			AverageXYZ(mesh.Vertices[i].Position, mesh.Vertices[i - 1].Position, &mCP[i]);

		// Corners
		if ((i / mUCP_Count == 0) && (i % mUCP_Count == 0)) // upper left
			AverageXYZ(mesh.Vertices[i].Position, mesh.Vertices[i + mUCP_Count + 1].Position, &mCP[i]);
		if ((i / mUCP_Count == 0) && (i % mUCP_Count == (mUCP_Count - 1))) // upper right
			AverageXYZ(mesh.Vertices[i].Position, mesh.Vertices[i + mUCP_Count - 1].Position, &mCP[i]);
		if ((i / mUCP_Count == (mVCP_Count - 1)) && (i % mUCP_Count == 0)) // lower left
			AverageXYZ(mesh.Vertices[i].Position, mesh.Vertices[i - mUCP_Count + 1].Position, &mCP[i]);
		if ((i / mUCP_Count == (mVCP_Count - 1)) && (i % mUCP_Count == (mUCP_Count - 1))) // lower right
			AverageXYZ(mesh.Vertices[i].Position, mesh.Vertices[i - mUCP_Count - 1].Position, &mCP[i]);

		temp.Tex = mesh.Vertices[i].TexC;
		vertices.push_back(temp);
	}
	
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(VertexBSpline)* mUCP_Count * mVCP_Count;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	(mValues->md3dDevice->CreateBuffer(&vbd, &vinitData, &mVertexBuffer));
	mVertexCount = vertices.size();

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
	(mValues->md3dDevice->CreateBuffer(&ibd, &iinitData, &mIndexBuffer));
	mIndexCount = mesh.Indices.size();

	RebuildBoundingVolume();
}

void BSplineSurface::BuildVertexLayout()
{
	//Create input layout
	D3DX11_PASS_DESC passDesc;
	mBSplineDraw_FinalComplete->GetPassByIndex(0)->GetDesc(&passDesc);
	HRESULT hr = mValues->md3dDevice->CreateInputLayout(VertexBSplineDesc, 1, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create input layout (BSplineSurface::BuildVertexLayout())!");
}

size_t BSplineSurface::GetStride() { return sizeof(VertexBSpline); }