#include "Subset.h"
#include "Exceptions.h"

using namespace Makina;

// Definitions of template classes
template class Subset<BasicMeshData>;
template class Subset<SkinnedMeshData>;

template <typename MeshDataType>
Subset<MeshDataType>::Subset(D3DAppValues *values, MeshDataType &mesh, Material &mat, ID3D11ShaderResourceView *dMap, ID3D11ShaderResourceView *bMap)
	: mMesh(mesh),
	mMat(mat),
	mDMap(dMap),
	mBMap(bMap),
	mVertexBuffer(NULL),
	mIndexBuffer(NULL),
	mIndexCount(0),
	mVertexCount(0)
{
	BuildGeometryBuffers(values);
}

template <typename MeshDataType>
Subset<MeshDataType>::Subset(D3DAppValues *values, MeshDataType &mesh)
	: mMesh(mesh),
	mDMap(0),
	mBMap(0),
	mVertexBuffer(NULL),
	mIndexBuffer(NULL),
	mIndexCount(0),
	mVertexCount(0)
{
	BuildGeometryBuffers(values);

	mMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMat.Reflect = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMat.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 5.0f);
}

template <typename MeshDataType>
Subset<MeshDataType>::~Subset()
{
	if (mDMap) mDMap->Release();
	if (mBMap) mBMap->Release();

	if (mVertexBuffer) mVertexBuffer->Release();
	if (mIndexBuffer) mIndexBuffer->Release();
}

template <typename MeshDataType>
void *Subset<MeshDataType>::operator new(size_t size)
{
	void *storage = _aligned_malloc(size, 16);
	if (NULL == storage)
	{
		throw AllocationError(L"No free memory. (Subset.cpp)");
	}
	return storage;
}

template <typename MeshDataType>
void Subset<MeshDataType>::operator delete(void *pt)
{
	_aligned_free(pt);
}

template <typename MeshDataType>
void Subset<MeshDataType>::BuildGeometryBuffers(D3DAppValues *values)
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = mMesh.SizeOfVertexElement() * mMesh.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &mMesh.Vertices[0];
	(values->md3dDevice->CreateBuffer(&vbd, &vinitData, &mVertexBuffer));
	mVertexCount = mMesh.Vertices.size();
	
	// Create the index buffer
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mMesh.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &mMesh.Indices[0];
	(values->md3dDevice->CreateBuffer(&ibd, &iinitData, &mIndexBuffer));
	mIndexCount = mMesh.Indices.size();

	// Bounding volume
	ComputeBoundingOrientedBoxFromPoints(&mSubsetBoundingVolume, mMesh.Vertices.size(), &(mMesh.Vertices[0].Position), mMesh.SizeOfVertexElement());
}