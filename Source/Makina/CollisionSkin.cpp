#include "CollisionSkin.h"
#include "Exceptions.h"

#define P_EPSILON 0.00001f // plane-point distance epsilon

using namespace Makina;
using namespace Physics_Makina;
using namespace std;

CollisionSkin::CollisionSkin(OrientedBox const *obb)
{
	// copy
	mOBB.Center = obb->Center;
	mOBB.Extents = obb->Extents;
	mOBB.Orientation = obb->Orientation;
}

CollisionSkin::~CollisionSkin()
{

}

void *CollisionSkin::operator new(size_t size)
{
	void *storage = _aligned_malloc(size, 16);
	if (NULL == storage)
	{
		throw AllocationError(L"No free memory. (CollisionSkin::operator new)");
	}
	return storage;
}

void CollisionSkin::operator delete(void *pt)
{
	_aligned_free(pt);
}

Mesh_CS::Mesh_CS(OrientedBox const *obb, BasicMeshData const *mesh)
: CollisionSkin(obb)
{
	mMesh.Vertices = mesh->Vertices;
	mMesh.Indices = mesh->Indices;

	XMVECTOR centerOfMesh = XMVectorZero();

	UINT verticesN = mMesh.Vertices.size();
	for (UINT i = 0; i < verticesN; ++i)
	{
		centerOfMesh += XMLoadFloat3(&mMesh.Vertices[i].Position);
	}
	centerOfMesh = XMVectorScale(centerOfMesh, 1.0f / verticesN);
	XMStoreFloat3(&mCenterOfMesh, centerOfMesh);

	DefineEdgesFacesAndVertices();
}

Mesh_CS::~Mesh_CS()
{

}

bool operator== (const XMFLOAT3 &left, const XMFLOAT3 &right)
{
	if ((abs(left.x - right.x) < P_EPSILON) && (abs(left.y - right.y) < P_EPSILON) && (abs(left.z - right.z) < P_EPSILON))
		return true;
	else
		return false;
}

void Mesh_CS::DefineEdgesFacesAndVertices()
{
	UINT indicesN = mMesh.Indices.size();

	// create new index array (because some vertices have the same coordinates,
	// but are different in other properties and we are only interested in coordinates so with this we make sure there is no repetition).
	vector<UINT> newIndexArray;
	newIndexArray.reserve(indicesN);
	for (UINT i = 0; i < indicesN; ++i)
	{
		XMFLOAT3 vertice = mMesh.Vertices[mMesh.Indices[i]].Position;
		UINT newIndex = mMesh.Indices[i];

		for (UINT j = 0; j < newIndexArray.size(); ++j)
		if ((i != j) && (vertice == mMesh.Vertices[newIndexArray[j]].Position))
		{
			newIndex = mMesh.Indices[j];
			break;
		}

		newIndexArray.push_back(newIndex);
	}

	// finally define edges and faces
	vector<XMFLOAT4> planes;
	UINT planesN = 0;
	for (UINT i = 0; i < newIndexArray.size(); i += 3)
	{
		UINT i0 = newIndexArray[i + 0];
		UINT i1 = newIndexArray[i + 1];
		UINT i2 = newIndexArray[i + 2];

		// Basically this checks if there are some triangles with zero area. We cannot work with those.
		if (i0 == i1 || i0 == i2 || i1 == i2)
		{
			newIndexArray.erase(newIndexArray.begin() + i + 2);
			newIndexArray.erase(newIndexArray.begin() + i + 1);
			newIndexArray.erase(newIndexArray.begin() + i + 0);
			i -= 3;
			continue; //throw UnexpectedError(L"Invalid geometry! (Mesh_CS::DefineEdgesFacesAndVertices)");
		}

		XMFLOAT3 fP0 = mMesh.Vertices[i0].Position;
		XMFLOAT3 fP1 = mMesh.Vertices[i1].Position;
		XMFLOAT3 fP2 = mMesh.Vertices[i2].Position;
		XMVECTOR p0 = XMLoadFloat3(&fP0);
		XMVECTOR p1 = XMLoadFloat3(&fP1);
		XMVECTOR p2 = XMLoadFloat3(&fP2);
		XMVECTOR normal = XMVector3Normalize(XMVector3Cross(p1 - p0, p2 - p0));
		XMFLOAT3 fNormal;
		XMStoreFloat3(&fNormal, normal);
		XMVECTOR plane = XMPlaneFromPointNormal(p0, normal);
		UINT currentSFIndex;

		bool planeFlag = true; // indicates if there is not a plane like the one we want to add.
		UINT planeIndex = -1; // also mFaces index
		for (UINT j = 0; j < planesN; ++j)
		{
			XMVECTOR cPlane = XMLoadFloat4(&planes[j]);

			float d0 = abs(XMVectorGetX(XMPlaneDotCoord(cPlane, p0)));
			float d1 = abs(XMVectorGetX(XMPlaneDotCoord(cPlane, p1)));
			float d2 = abs(XMVectorGetX(XMPlaneDotCoord(cPlane, p2)));

			if ((d0 < P_EPSILON) && (d1 < P_EPSILON) && (d2 < P_EPSILON))
			{
				planeFlag = false;
				planeIndex = j;
			}
		}

		if (planeFlag)
		{
			XMFLOAT4 fPlane;
			XMStoreFloat4(&fPlane, plane);
			planes.push_back(fPlane);
			++planesN;
		}

		//
		// FACES
		//

		if (!planeFlag) // *** there is already a plane (face) for this triangle
		{
			bool add0 = true, add1 = true, add2 = true; // first check which vertices are not inside already
			for (vector<UINT>::iterator it = mFaces[planeIndex].mP.begin(); it != mFaces[planeIndex].mP.end();)
			{
				if (mMesh.Vertices[*it].Position == fP0) add0 = false;
				if (mMesh.Vertices[*it].Position == fP1) add1 = false;
				if (mMesh.Vertices[*it].Position == fP2) add2 = false;
				++it;
			}
			if (add0) mFaces[planeIndex].mP.push_back(i0);
			if (add1) mFaces[planeIndex].mP.push_back(i1);
			if (add2) mFaces[planeIndex].mP.push_back(i2);

			currentSFIndex = planeIndex;
		}
		else // *** there is not, create new face
		{
			CollisionSkinFace face;
			XMStoreFloat3(&face.mNormal, normal);
			face.mP.push_back(i0);
			face.mP.push_back(i1);
			face.mP.push_back(i2);
			mFaces.push_back(face);

			currentSFIndex = mFaces.size() - 1;
		}

		//
		// EDGES
		//

		// first edge
		CollisionSkinEdge ed01(i0, i1);
		ed01.mFaceIndex[0] = currentSFIndex;
		vector<CollisionSkinEdge>::iterator edIt;
		if ((edIt = find(mEdges.begin(), mEdges.end(), ed01)) == mEdges.end())
			mEdges.push_back(ed01);
		else // update existing one
		{
			if (edIt->mFaceIndex[0] != currentSFIndex)
				edIt->mFaceIndex[1] = currentSFIndex;
		}

		// second edge
		CollisionSkinEdge ed12(i1, i2);
		ed12.mFaceIndex[0] = currentSFIndex;
		if ((edIt = find(mEdges.begin(), mEdges.end(), ed12)) == mEdges.end())
			mEdges.push_back(ed12);
		else // update existing one
		{
			if (edIt->mFaceIndex[0] != currentSFIndex)
				edIt->mFaceIndex[1] = currentSFIndex;
		}

		// third edge
		CollisionSkinEdge ed20(i2, i0);
		ed20.mFaceIndex[0] = currentSFIndex;
		if ((edIt = find(mEdges.begin(), mEdges.end(), ed20)) == mEdges.end())
			mEdges.push_back(ed20);
		else // update existing one
		{
			if (edIt->mFaceIndex[0] != currentSFIndex)
				edIt->mFaceIndex[1] = currentSFIndex;
		}

		//
		// VERTICES
		//
		bool v0 = true, v1 = true, v2 = true;
		for (vector<UINT>::iterator indexIt = mVertexIndices.begin(); indexIt != mVertexIndices.end(); ++indexIt)
		{
			if (fP0 == mMesh.Vertices[*indexIt].Position) v0 = false;
			if (fP1 == mMesh.Vertices[*indexIt].Position) v1 = false;
			if (fP2 == mMesh.Vertices[*indexIt].Position) v2 = false;
		}
		if (v0) mVertexIndices.push_back(i0);
		if (v1) mVertexIndices.push_back(i1);
		if (v2) mVertexIndices.push_back(i2);
	}

	// now remove the ones that lie on only one plane (edges like diagonal edge for quad face)
	UINT edgesN = mEdges.size();
	vector<USHORT> pMember; // inside how many planes is this edge (minimum is 1, and maximum is 2)
	pMember.resize(edgesN);

	for (UINT i = 0; i < edgesN; ++i)
	{
		for (UINT j = 0; j < planesN; ++j)
		{
			XMVECTOR p0 = XMLoadFloat3(&mMesh.Vertices[mEdges[i].mP0].Position);
			XMVECTOR p1 = XMLoadFloat3(&mMesh.Vertices[mEdges[i].mP1].Position);

			XMVECTOR cPlane = XMLoadFloat4(&planes[j]);
			float d0 = abs(XMVectorGetX(XMPlaneDotCoord(cPlane, p0)));
			float d1 = abs(XMVectorGetX(XMPlaneDotCoord(cPlane, p1)));
			if ((d0 < P_EPSILON) && (d1 < P_EPSILON))
				++pMember[i];
		}
	}

	int lookupIndex = 0;
	for (vector<CollisionSkinEdge>::iterator it = mEdges.begin(); it != mEdges.end();)
	{
		if (pMember[lookupIndex++] < 2)
			it = mEdges.erase(it);
		else
			++it;
	}

	// Calculate mAngle
	for (vector<CollisionSkinEdge>::iterator it = mEdges.begin(); it != mEdges.end();)
	{
		//CalculateEdgeAngle(&(*it), mMesh);
		XMVECTOR normal_1 = XMLoadFloat3(&mFaces[it->mFaceIndex[0]].mNormal);
		XMVECTOR normal_2 = XMLoadFloat3(&mFaces[it->mFaceIndex[1]].mNormal);
		it->mAngle = XM_PI - acos(XMVectorGetX(XMVector3Dot(normal_1, normal_2)));
		++it;
	}

}

Capsule_CS::Capsule_CS(OrientedBox const *obb, float height, float radius)
: CollisionSkin(obb),
mH(height),
mH_DIV_2(height * 0.5f),
mR(radius)
{

}

Capsule_CS::~Capsule_CS()
{

}
