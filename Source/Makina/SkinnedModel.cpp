#include "SkinnedModel.h"
#include "MathHelper.h"
#include "BasicEffect.h"

using namespace Makina;
using namespace std;

SkinnedModel::SkinnedModel(D3DAppValues *values)
: Model(values),
mSimData(&mAnimData)
{

}

SkinnedModel::~SkinnedModel()
{
	for (UINT i = 0; i < mParts.size(); ++i)
		delete mParts[i];

}

void *SkinnedModel::operator new(size_t size)
{
	void *storage = _aligned_malloc(size, 16);
	if (NULL == storage)
	{
		throw AllocationError(L"No free memory. (SkinnedModel.cpp)");
	}
	return storage;
}

void SkinnedModel::operator delete(void *pt)
{
	_aligned_free(pt);
}

void SkinnedModel::Draw(ID3DX11EffectPass *pass, Frustum &localSpaceCamFrustum, bool distortingVertices)
{
	mValues->md3dImmediateContext->IASetInputLayout(mValues->mBasicEffect->GetSkinnedVertexFullInputLayout());

	for (auto &subset : mParts)
	{
		// check subset bounding volume
		if (distortingVertices == false && IntersectOrientedBoxFrustum(&subset->mSubsetBoundingVolume, &localSpaceCamFrustum) == 0)
			continue;

		mValues->mBasicEffect->mMaterial->SetRawValue(&subset->GetMaterial(), 0, sizeof(Material));
		if (subset->GetDiffuseMap()) mValues->mBasicEffect->mDiffuseMap->SetResource(subset->GetDiffuseMap());
		else mValues->mBasicEffect->SetGenericDiffuseMap();

		if (subset->GetNormalMap()) mValues->mBasicEffect->mNormalMap->SetResource(subset->GetNormalMap());
		else mValues->mBasicEffect->SetGenericNormalMap();

		mValues->mBasicEffect->mReflection->SetFloat(1.0f);

		UINT stride = subset->GetStride();
		UINT offset = 0;
		mValues->md3dImmediateContext->IASetVertexBuffers(0, 1, &subset->mVertexBuffer, &stride, &offset);
		mValues->md3dImmediateContext->IASetIndexBuffer(subset->mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		pass->Apply(0, mValues->md3dImmediateContext);
		mValues->md3dImmediateContext->DrawIndexed(subset->mIndexCount, 0, 0);
	}
}

void SkinnedModel::Draw(ID3DX11EffectPass *pass, OrientedBox &localSpaceCamOBB, bool distortingVertices)
{
	mValues->md3dImmediateContext->IASetInputLayout(mValues->mBasicEffect->GetSkinnedVertexFullInputLayout());

	for (auto &subset : mParts)
	{
		// check subset bounding volume
		if (distortingVertices == false && IntersectOrientedBoxOrientedBox(&subset->mSubsetBoundingVolume, &localSpaceCamOBB) == 0)
			continue;

		mValues->mBasicEffect->mMaterial->SetRawValue(&subset->GetMaterial(), 0, sizeof(Material));

		UINT stride = subset->GetStride();
		UINT offset = 0;
		mValues->md3dImmediateContext->IASetVertexBuffers(0, 1, &subset->mVertexBuffer, &stride, &offset);
		mValues->md3dImmediateContext->IASetIndexBuffer(subset->mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		pass->Apply(0, mValues->md3dImmediateContext);
		mValues->md3dImmediateContext->DrawIndexed(subset->mIndexCount, 0, 0);
	}
}

void SkinnedModel::AddSubset(SkinnedMeshData &mesh, Material &material, ID3D11ShaderResourceView *dMap, ID3D11ShaderResourceView *bMap)
{
	mParts.push_back(new Subset<SkinnedMeshData>(mValues, mesh, material, dMap, bMap));
}

void SkinnedModel::AddSubset(SkinnedMeshData &mesh)
{
	mParts.push_back(new Subset<SkinnedMeshData>(mValues, mesh));
}

void SkinnedModel::UpdateChanges()
{
	// Recalculate object OBB
	vector<XMFLOAT3> allVertices;

	for (UINT i = 0; i < mParts.size(); ++i)
	{
		for (UINT j = 0; j < mParts[i]->mMesh.Vertices.size(); j++)
		{
			allVertices.push_back(mParts[i]->mMesh.Vertices[j].Position);
		}
	}

	if (allVertices.size())
	{
		ComputeBoundingOrientedBoxFromPoints(&mObjectBoundingVolume, allVertices.size(), &(allVertices[0]), sizeof(XMFLOAT3));
		mObjectBoundingVolume_ForAnimation.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
		mObjectBoundingVolume_ForAnimation.Extents = XMFLOAT3(
			mObjectBoundingVolume.Extents.x * 2.0f,
			mObjectBoundingVolume.Extents.y * 2.0f,
			mObjectBoundingVolume.Extents.z * 2.0f);
		mObjectBoundingVolume_ForAnimation.Orientation = mObjectBoundingVolume.Orientation;
	}
}

bool SkinnedModel::ObjectSpaceRayIntersects(FXMVECTOR origin, FXMVECTOR dir, float *iDist)
{
	// first check object bounding volume
	if (IntersectRayOrientedBox(origin, dir, &mObjectBoundingVolume, iDist))
	{// continue to mesh testing

		for (UINT i = 0; i < mParts.size(); ++i)
		{
			// check subset bounding volume
			if (IntersectRayOrientedBox(origin, dir, &mParts[i]->mSubsetBoundingVolume, iDist))
			{// continue to subset mesh testing

				// Find the nearest ray/triangle intersection.
				*iDist = MathHelper::Infinity;
				UINT mPickedTriangle = -1;
				for (UINT j = 0; j < mParts[i]->mMesh.Indices.size() / 3; ++j)
				{
					// Indices for this triangle.
					UINT i0 = mParts[i]->mMesh.Indices[j * 3 + 0];
					UINT i1 = mParts[i]->mMesh.Indices[j * 3 + 1];
					UINT i2 = mParts[i]->mMesh.Indices[j * 3 + 2];

					// Vertices for this triangle.
					XMVECTOR v0 = XMLoadFloat3(&mParts[i]->mMesh.Vertices[i0].Position);
					XMVECTOR v1 = XMLoadFloat3(&mParts[i]->mMesh.Vertices[i1].Position);
					XMVECTOR v2 = XMLoadFloat3(&mParts[i]->mMesh.Vertices[i2].Position);

					// We have to iterate over all the triangles in order to find the nearest intersection.
					float t = 0.0f;
					if (IntersectRayTriangle(origin, dir, v0, v1, v2, &t))
					{
						if (t < *iDist)
						{
							// This is the new nearest picked triangle.
							*iDist = t;
							mPickedTriangle = j;
						}
					}
				}
				if (mPickedTriangle >= 0)
					return true;
			}
		}
	}
	// if code reaches to this point, there is no intersection
	return false;
}

OrientedBox const *SkinnedModel::GetBoundingVolume(bool distortingVertices) const
{
	if (!distortingVertices)
	{
		return &mObjectBoundingVolume;
	}
	else
	{
		return &mObjectBoundingVolume_ForAnimation;
	}
}

Subset<SkinnedMeshData> const *SkinnedModel::GetSubset(int index) const
{
	return mParts[index];
}