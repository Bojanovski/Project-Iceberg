
#ifndef SKINNEDMODEL_H
#define SKINNEDMODEL_H

#include "Model.h"
#include "Subset.h"
#include "MeshAnimationData.h"
#include "MeshSimulationData.h"
#include <vector>

namespace Makina
{
	class SkinnedModel : public Model
	{
	public:
		__declspec(dllexport) SkinnedModel(D3DAppValues *values);
		__declspec(dllexport) ~SkinnedModel();

		// Dynamic allocation must be aligned to the 16, therefore custom 'new' and 'delete' operators are needed.
		__declspec(dllexport) void *operator new(size_t size);
		__declspec(dllexport) void operator delete(void *pt);

		__declspec(dllexport) void Draw(ID3DX11EffectPass *pass, Frustum &localSpaceCamFrustum, bool distortingVertices = false);
		__declspec(dllexport) void Draw(ID3DX11EffectPass *pass, OrientedBox &localSpaceCamOBB, bool distortingVertices = false);

		__declspec(dllexport) void AddSubset(SkinnedMeshData &mesh, Material &material, ID3D11ShaderResourceView *dMap, ID3D11ShaderResourceView *bMap);
		__declspec(dllexport) void AddSubset(SkinnedMeshData &mesh);
		__declspec(dllexport) void UpdateChanges();
		__declspec(dllexport) bool ObjectSpaceRayIntersects(FXMVECTOR origin, FXMVECTOR dir, float *iDist);

		__declspec(dllexport) OrientedBox const *GetBoundingVolume(bool distortingVertices = false) const;
		__declspec(dllexport) Subset<SkinnedMeshData> const *GetSubset(int index) const;
		int GetSubsetCount() const { return mParts.size(); }
		MeshAnimationData *GetMeshAnimationData() { return &mAnimData; }
		MeshSimulationData *GetMeshSimulationData() { return &mSimData; }

	private:
		std::vector<Subset<SkinnedMeshData> *> mParts;
		OrientedBox mObjectBoundingVolume;
		OrientedBox mObjectBoundingVolume_ForAnimation;
		MeshAnimationData mAnimData;
		MeshSimulationData mSimData;
	};
}

#endif