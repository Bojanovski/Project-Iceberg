
#ifndef BASICMODEL_H
#define BASICMODEL_H

#include "Model.h"
#include "Subset.h"
#include <vector>

namespace Makina
{
	class BasicModel : public Model
	{
	public:
		__declspec(dllexport) BasicModel(D3DAppValues *values);
		__declspec(dllexport) ~BasicModel();

		// Dynamic allocation must be aligned to the 16, therefore custom 'new' and 'delete' operators are needed.
		__declspec(dllexport) void *operator new(size_t size);
		__declspec(dllexport) void operator delete(void *pt);

		__declspec(dllexport) void Draw(ID3DX11EffectPass *pass, Frustum &localSpaceCamFrustum, bool distortingVertices = false);
		__declspec(dllexport) void Draw(ID3DX11EffectPass *pass, OrientedBox &localSpaceCamOBB, bool distortingVertices = false);

		__declspec(dllexport) void AddSubset(BasicMeshData &mesh, Material &material, ID3D11ShaderResourceView *dMap, ID3D11ShaderResourceView *bMap);
		__declspec(dllexport) void AddSubset(BasicMeshData &mesh);
		__declspec(dllexport) void UpdateChanges();
		__declspec(dllexport) bool ObjectSpaceRayIntersects(FXMVECTOR origin, FXMVECTOR dir, float *iDist);

		__declspec(dllexport) OrientedBox const *GetBoundingVolume(bool distortingVertices = false) const;
		__declspec(dllexport) Subset<BasicMeshData> const *GetSubset(int index) const;
		__declspec(dllexport) int GetSubsetCount() const { return mParts.size(); }

	private:
		std::vector<Subset<BasicMeshData> *> mParts;
		OrientedBox mObjectBoundingVolume;
	};
}

#endif