
#ifndef SUBSET_H
#define SUBSET_H

#include "DirectX11Headers.h"
#include "D3DAppValues.h"
#include "Geometry.h"
#include "XnaCollision.h"

namespace Makina
{
	template <typename MeshDataType>
	class Subset
	{
		friend class BasicModel;
		friend class SkinnedModel;

	public:
		__declspec(dllexport) Subset(D3DAppValues *values, MeshDataType &mesh, Material &mat, ID3D11ShaderResourceView *dMap, ID3D11ShaderResourceView *bMap);
		__declspec(dllexport) Subset(D3DAppValues *values, MeshDataType &mesh);
		__declspec(dllexport) ~Subset();

		// Dynamic allocation must be aligned to the 16, therefore custom 'new' and 'delete' operators are needed.
		__declspec(dllexport) void *operator new(size_t size);
		__declspec(dllexport) void operator delete(void *pt);

		Material &GetMaterial() {return mMat;}
		ID3D11ShaderResourceView *GetDiffuseMap() const {return mDMap;}
		ID3D11ShaderResourceView *GetNormalMap() const {return mBMap;}

		__declspec(dllexport) OrientedBox const *GetBoundingVolume() const { return &mSubsetBoundingVolume; }
		__declspec(dllexport) MeshDataType const *GetMesh() const { return &mMesh; }
		
	private:
		void BuildGeometryBuffers(D3DAppValues *values);

		UINT GetStride() { return mMesh.SizeOfVertexElement(); }

		MeshDataType mMesh;

		Material mMat;

		ID3D11ShaderResourceView *mDMap;
		ID3D11ShaderResourceView *mBMap;

		ID3D11Buffer *mVertexBuffer;
		ID3D11Buffer *mIndexBuffer;
		UINT mIndexCount;
		UINT mVertexCount;

		OrientedBox mSubsetBoundingVolume;
	};
}

#endif