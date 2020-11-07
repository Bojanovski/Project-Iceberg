#include "CollisionDetector.h"
#include "RigidBody.h"
#include "Geometry.h"

#define ANGLE_EPSILON_FOR_EDGE_EDGE_TESTING 0.000002f

#include <string>
#include <sstream>

using namespace Makina;
using namespace Physics_Makina;
using namespace std;

CollisionDetector::CollisionDetector(float interpenetrationEpsilon, float vectorComparisonEpsilon)
: mInterpenetrationEpsilon(interpenetrationEpsilon),
mVectorComparisonEpsilon(vectorComparisonEpsilon)
{

}

CollisionDetector::~CollisionDetector()
{
	for (auto &collisionSkin : mCollisionSkins)
		delete collisionSkin;
}

inline void CollisionDetector::RevalidateMeshMesh(CollisionData_MeshMesh *data)
{
	const Mesh_CS *skin[2];
	skin[0] = dynamic_cast<const Mesh_CS *>(data->mSkinInstance[0]->mCollisionSkin);
	skin[1] = dynamic_cast<const Mesh_CS *>(data->mSkinInstance[1]->mCollisionSkin);

	XMMATRIX worldA = XMLoadFloat4x4(&data->mSkinInstance[0]->mRigidBody->mTransformMatrix);
	XMVECTOR detA;
	XMMATRIX toLocalA = XMMatrixInverse(&detA, worldA);
	XMMATRIX worldB = XMLoadFloat4x4(&data->mSkinInstance[1]->mRigidBody->mTransformMatrix);
	XMVECTOR detB;
	XMMATRIX toLocalB = XMMatrixInverse(&detB, worldB);
	XMMATRIX fromLToL[2];
	fromLToL[0] = worldA * toLocalB; // from A to B
	fromLToL[1] = worldB * toLocalA; // from B to A
	XMVECTOR centerOfMeshA = XMLoadFloat3(&skin[0]->mCenterOfMesh);
	XMVECTOR centerOfMeshB = XMLoadFloat3(&skin[1]->mCenterOfMesh);

	//*******************************************************
	//	First points.
	//*******************************************************
	list<char> *cPnt = &data->mContactPointsI;
	for (auto pointIt = cPnt->begin(); pointIt != cPnt->end();)
	{
		// get vertice responsible for Contact point
		XMVECTOR vertPos = XMLoadFloat3(&data->mContacts[*pointIt].mContactPoint);
		// transform it to appropriate local space
		XMVECTOR vertPosLocal = XMVector3TransformCoord(vertPos, fromLToL[data->mContacts[*pointIt].mContactPointHolderSkinI]);

		// get fresh depth of interpenetration
		UINT faceIndex;
		float newInterpenetration;
		bool passed = GetInterpenetration(vertPosLocal, mInterpenetrationEpsilon, &newInterpenetration, &faceIndex, skin[1 - data->mContacts[*pointIt].mContactPointHolderSkinI]);

		// now the critical part for contact point
		if (newInterpenetration < mInterpenetrationEpsilon && passed)
		{// keep the point
			data->mContacts[*pointIt].mInterpenetration = newInterpenetration; // new interpenetration value

			XMVECTOR axis = XMVector3TransformNormal(XMLoadFloat3(&skin[1 - data->mContacts[*pointIt].mContactPointHolderSkinI]->mFaces[faceIndex].mNormal),
				fromLToL[1 - data->mContacts[*pointIt].mContactPointHolderSkinI]);

			data->mContacts[*pointIt].mIndexData[1] = faceIndex; // new face
			XMStoreFloat3(&data->mContacts[*pointIt].mContactNormal, axis); // new normal
			++pointIt;
		}
		else
		{// bye bye contact point
			data->mContacts.Remove(*pointIt);
			pointIt = cPnt->erase(pointIt); // will redirect iterator to item after erased one
		}
	}

	//*******************************************************
	//	Now edges.
	//*******************************************************
	cPnt = &data->mContactEdgesI;
	for (auto pointIt = cPnt->begin(); pointIt != cPnt->end();)
	{
		XMVECTOR edgeA_p0 = XMLoadFloat3(&skin[0]->mMesh.Vertices[skin[0]->mEdges[data->mContacts[*pointIt].mIndexData[0]].mP0].Position);
		XMVECTOR edgeA_p1 = XMLoadFloat3(&skin[0]->mMesh.Vertices[skin[0]->mEdges[data->mContacts[*pointIt].mIndexData[0]].mP1].Position);
		edgeA_p0 = XMVector3TransformCoord(edgeA_p0, fromLToL[0]); // Now go to B's local space
		edgeA_p1 = XMVector3TransformCoord(edgeA_p1, fromLToL[0]);
		XMVECTOR AC_in_BLS = XMVector3TransformCoord(centerOfMeshA, fromLToL[0]); // A center in B local space
		float edgeALength = XMVectorGetX(XMVector3Length(edgeA_p1 - edgeA_p0));

		XMVECTOR edgeB_p0 = XMLoadFloat3(&skin[1]->mMesh.Vertices[skin[1]->mEdges[data->mContacts[*pointIt].mIndexData[1]].mP0].Position);
		XMVECTOR edgeB_p1 = XMLoadFloat3(&skin[1]->mMesh.Vertices[skin[1]->mEdges[data->mContacts[*pointIt].mIndexData[1]].mP1].Position);
		float edgeBLength = XMVectorGetX(XMVector3Length(edgeB_p1 - edgeB_p0));

		// run four tests
		bool passed = true;

		float edgeA_s, edgeB_s;
		XMVECTOR cA, cB, normal;
		// 1. test
		if (!GetClosestPoints_Lines(edgeA_p0, edgeA_p1, edgeB_p0, edgeB_p1, &edgeA_s, &edgeB_s, &normal, &cA, &cB))
			passed = false; // edges are parallel

		// 2. test
		if ((edgeA_s < 0.0f) || (edgeA_s > edgeALength) || (edgeB_s < 0.0f) || (edgeB_s > edgeBLength))
			passed = false; // closest points are not on edges

		XMVECTOR lineA_r = edgeA_p0 + XMVectorScale(cA, edgeA_s);
		XMVECTOR lineB_r = edgeB_p0 + XMVectorScale(cB, edgeB_s);
		XMVECTOR r = lineB_r - lineA_r;
		normal = XMVector3Normalize(r); // this normal is pointing in right direction
		XMVECTOR contactPos = lineA_r;

		// project A center, B center and B collision point (lineB_r) onto normal, with center lineA_r.
		float AC_projection = XMVectorGetX(XMVector3Dot(normal, AC_in_BLS - lineA_r));
		float BC_projection = XMVectorGetX(XMVector3Dot(normal, centerOfMeshB - lineA_r));
		float lineB_r_projection = XMVectorGetX(XMVector3Dot(normal, lineB_r - lineA_r));
		float penetrationDepth_AinB = XMVectorGetX(XMVector3Length(r));

		// 3. test -> three very important conditions to check, think about them (drawing helps).
		bool penetrationIsPositive = false;
		if ((AC_projection < 0.0f) || (BC_projection > 0.0f) || (lineB_r_projection > AC_projection))
		{// Here is extra tolerance if point is outside of mesh but closer than mInterpenetrationEpsilon.
			penetrationIsPositive = true;
			if (penetrationDepth_AinB >= mInterpenetrationEpsilon)
				passed = false;
		}

		XMVECTOR newPosition = XMVector3TransformCoord(contactPos, fromLToL[1]); // contactPos was in B's local space!
		XMVECTOR newNormal = XMVector3TransformNormal(normal, fromLToL[1]); // normal was in B's local space!

		if (passed)
		{// keep the point
			data->mContacts[*pointIt].mInterpenetration = penetrationDepth_AinB * ((penetrationIsPositive) ? 1.0f : -1.0f); // new interpenetration value
			XMStoreFloat3(&data->mContacts[*pointIt].mContactPoint, newPosition); // new position
			XMStoreFloat3(&data->mContacts[*pointIt].mContactNormal, newNormal * ((penetrationIsPositive) ? -1.0f : 1.0f)); // new normal
			++pointIt;
		}
		else
		{// bye bye
			data->mContacts.Remove(*pointIt);
			pointIt = cPnt->erase(pointIt); // will redirect iterator to item after erased one
		}
	}
}

inline void CollisionDetector::RevalidateCapsuleMesh(CollisionData_CapsuleMesh *data)
{

}

void CollisionDetector::RevalidatePreviousContacts()
{
	// for each CollisionData in mColData
	for (auto dataIt = mColData.begin(); dataIt != mColData.end();)
	{
		switch ((*dataIt)->mType)
		{
			{
		case (CollisionDataType::MeshMesh) :
			CollisionData_MeshMesh *data_mm = dynamic_cast<CollisionData_MeshMesh *>(*dataIt);
			RevalidateMeshMesh(data_mm);
			break;
			}

			{
		case (CollisionDataType::CapsuleMesh) :
			CollisionData_CapsuleMesh *data_cm = dynamic_cast<CollisionData_CapsuleMesh *>(*dataIt);
			RevalidateCapsuleMesh(data_cm);
			break;
		}
		default:
			break;
		}

		//*******************************************************
		// delete CollisionData if there are no Contact points
		// and no Contact edges left in it. Also delete it if
		// both rigid bodies (owners) are asleep.
		//*******************************************************
		if ((*dataIt)->HasNoContacts() || (*dataIt)->BothAsleep())
		{
			delete (*dataIt);
			dataIt = mColData.erase(dataIt); // will redirect iterator to item after erased one
		}
		else
			++dataIt;
	}
}

void CollisionDetector::AddCollisionSkin(CollisionSkin *skin)
{
	mCollisionSkins.push_back(skin);
}

void CollisionDetector::RemoveCollisionSkin(CollisionSkin *skin)
{
	// erase from mMeshCollData
	int index = -1;
	UINT size = mCollisionSkins.size();
	for (UINT i = 0; i < size; ++i)
	{
		if (mCollisionSkins[i] == skin)
		{
			index = i;
			break;
		}
	}

	if (index == -1) throw InvalidOperation(L"Skin is not on the list! (CollisionDetector::RemoveMeshCollisionSkinData)");
	mCollisionSkins.erase(mCollisionSkins.begin() + index);
}

void CollisionDetector::DetectContacts(const CollisionSkin_RigidBody_Instance *first, const CollisionSkin_RigidBody_Instance *second)
{
	CollisionData *data = 0;

	// bigger first check
	if (first < second)
	{	// switch
		const CollisionSkin_RigidBody_Instance *temp = first;
		first = second;
		second = temp;
	}

	XMMATRIX worldA = XMLoadFloat4x4(&first->mRigidBody->mTransformMatrix);
	XMMATRIX worldB = XMLoadFloat4x4(&second->mRigidBody->mTransformMatrix);
	XMVECTOR detB;
	XMMATRIX toLocalB = XMMatrixInverse(&detB, worldB);
	XMVECTOR detA;
	XMMATRIX toLocalA = XMMatrixInverse(&detA, worldA);
	XMMATRIX fromBtoA = worldB * toLocalA;
	XMMATRIX fromAtoB = worldA * toLocalB;

	// get pointer to CollisionData structures in list
	GetCollisionData(first, second, &data);

	switch (data->mType)
	{
		{
	case (CollisionDataType::MeshMesh) :
		CollisionData_MeshMesh *data_mm = dynamic_cast<CollisionData_MeshMesh *>(data);
		const Mesh_CS *A = dynamic_cast<const Mesh_CS *>(first->mCollisionSkin), *B = dynamic_cast<const Mesh_CS *>(second->mCollisionSkin);

		// actual testing
		PolytopePointFaceTesting(A, B, fromAtoB, fromBtoA, worldA, worldB, data_mm);
		PolytopeEdgeEdgeTesting(A, B, fromAtoB, fromBtoA, worldA, worldB, data_mm);
		break;
		}

		{
	case (CollisionDataType::CapsuleMesh) :
		CollisionData_CapsuleMesh *data_cm = dynamic_cast<CollisionData_CapsuleMesh *>(data);
		const Capsule_CS *C = dynamic_cast<const Capsule_CS *>(data_cm->GetCapsuleSkinInstance()->mCollisionSkin);
		const Mesh_CS *M = dynamic_cast<const Mesh_CS *>(data_cm->GetMeshSkinInstance()->mCollisionSkin);

		// actual testing
		break;
	}
	default:
#if defined(DEBUG) || defined(_DEBUG)  
		wstring msg = wstring(L"WARNING: CollisionDetector::DetectContacts() had some CollisionSkins it couldn't solve!\n");
		OutputDebugString(&msg[0]);
#endif
		break;
	}

	if ((typeid(*first->mCollisionSkin) == typeid(const Mesh_CS)) && (typeid(*second->mCollisionSkin) == typeid(const Mesh_CS)))
	{
		// dynamic cast to Mesh_CS (mesh collision skin)
		const Mesh_CS *A = dynamic_cast<const Mesh_CS *>(first->mCollisionSkin), *B = dynamic_cast<const Mesh_CS *>(second->mCollisionSkin);
		CollisionData_MeshMesh *data_m = dynamic_cast<CollisionData_MeshMesh *>(data);

		// actual testing
		PolytopePointFaceTesting(A, B, fromAtoB, fromBtoA, worldA, worldB, data_m);
		PolytopeEdgeEdgeTesting(A, B, fromAtoB, fromBtoA, worldA, worldB, data_m);
	}
	else
	{
#if defined(DEBUG) || defined(_DEBUG)  
		wstring msg = wstring(L"WARNING: CollisionDetector::DetectContacts() had some CollisionSkins it couldn't solve!\n");
		OutputDebugString(&msg[0]);
#endif
	}


	//#if defined(DEBUG) || defined(_DEBUG) 
	//	wstringstream wss;
	//	wss << L"***********************************************************" << endl;
	//	wss << L"REPORT:  coldet has:  " << mColData.size() << endl;
	//	list<CollisionData>::iterator pointIt = mColData.begin();
	//	for (UINT i = 0; i < mColData.size(); ++i)
	//	{
	//		wss << i << L". has " << (pointIt)->mContactPoints.size() << L" contact points.";
	//		for (list<Contact>::iterator cnPtIt = pointIt->mContactPoints.begin(); cnPtIt != pointIt->mContactPoints.end(); ++cnPtIt)
	//			wss << L" { " << cnPtIt->mContactPoint.x << L" ; " << cnPtIt->mContactPoint.y << L" ; " << cnPtIt->mContactPoint.z << L" ; " << cnPtIt->mIAmFromAMesh << L" ; " << cnPtIt->mInterpenetration << L" } ";
	//		wss << endl;
	//		++pointIt;
	//	}
	//
	//	pointIt = mColData.begin();
	//	for (UINT i = 0; i < mColData.size(); ++i)
	//	{
	//		wss << i << L". has " << (pointIt)->mContactEdges.size() << L" contact edges.";
	//		for (list<Contact>::iterator cnPtIt = pointIt->mContactEdges.begin(); cnPtIt != pointIt->mContactEdges.end(); ++cnPtIt)
	//			wss << L" { " << cnPtIt->mContactPoint.x << L" ; " << cnPtIt->mContactPoint.y << L" ; " << cnPtIt->mContactPoint.z << L" ; " << cnPtIt->mIAmFromAMesh << L" ; " << cnPtIt->mInterpenetration << L" } ";
	//		wss << endl;
	//		++pointIt;
	//	}
	//	wss << L"***********************************************************" << endl;
	//	wstring ws = wss.str();
	//	OutputDebugString(&ws[0]);
	//#endif
}

bool CollisionDetector::GetClosestPoints_Lines(FXMVECTOR LineA_p0, FXMVECTOR LineA_p1, FXMVECTOR LineB_p0, CXMVECTOR LineB_p1,
	float *LineA_r, float *LineB_r, XMVECTOR *n, XMVECTOR *cA, XMVECTOR *cB)
{
	// direction of line A
	*cA = XMVector3Normalize(LineA_p1 - LineA_p0);
	// direction of line B
	*cB = XMVector3Normalize(LineB_p1 - LineB_p0);

	*n = XMVector3Cross(*cA, *cB);
	if (XMVectorGetX(XMVector3LengthSq(*n)) == 0)
		return false; // lines are parallel

	XMVECTOR planeN_A = XMVector3Cross(*n, *cA);
	XMVECTOR planeN_B = XMVector3Cross(*n, *cB);
	float k_A = XMVectorGetX(XMVector3Dot(planeN_B, *cA));
	float k_B = XMVectorGetX(XMVector3Dot(planeN_A, *cB));

	if ((k_A == 0) || (k_B == 0))
		return false; // coplanar lines

	float s_A = XMVectorGetX(XMVector3Dot(planeN_B, LineB_p0 - LineA_p0));
	*LineA_r = s_A / k_A;
	float s_B = XMVectorGetX(XMVector3Dot(planeN_A, LineA_p0 - LineB_p0));
	*LineB_r = s_B / k_B;

	return true;
}

bool CollisionDetector::GetInterpenetration(FXMVECTOR pos, float mInterpenetrationEpsilon, float *interpenetration, UINT *faceIndex, const Mesh_CS *meshSkin)
{
	*interpenetration = -D3D11_FLOAT32_MAX;
	*faceIndex = -1;
	bool isInterpenetrating = false;

	UINT facesN = meshSkin->mFaces.size();
	for (UINT i = 0; i < facesN; ++i)
	{
		XMVECTOR faceNormal = XMLoadFloat3(&meshSkin->mFaces[i].mNormal);
		XMVECTOR vertPos = XMLoadFloat3(&meshSkin->mMesh.Vertices[meshSkin->mFaces[i].mP[0]].Position);
		XMVECTOR posDir = pos - vertPos;
		float dot = XMVectorGetX(XMVector3Dot(posDir, faceNormal));

		if (dot > mInterpenetrationEpsilon) // early exit
			return false;

		// The axis with the shallowest penetration is used (most positive value)
		if (*interpenetration < dot)
		{
			*interpenetration = dot;
			*faceIndex = i;
			isInterpenetrating = true;
		}
	}

	return isInterpenetrating;
}

bool CollisionDetector::IsPointInside(FXMVECTOR pos, bool useEpsilon, const Mesh_CS *meshSkin)
{
	float smallestDepth = (useEpsilon) ? mInterpenetrationEpsilon : 0;
	int facesN = meshSkin->mFaces.size();
	for (int i = 0; i < facesN; ++i)
	{
		XMVECTOR faceNormal = XMLoadFloat3(&meshSkin->mFaces[i].mNormal);
		XMVECTOR vertPos = XMLoadFloat3(&meshSkin->mMesh.Vertices[meshSkin->mFaces[i].mP[0]].Position);
		XMVECTOR posDir = pos - vertPos;
		float dot = XMVectorGetX(XMVector3Dot(posDir, faceNormal));

		if (dot > smallestDepth) // early exit
			return false;
	}

	return true;
}

void CollisionDetector::GetProjection(FXMVECTOR pos, XMVECTOR axis, const Mesh_CS *meshSkin, float *min, float *max)
{
	*max = -D3D11_FLOAT32_MAX;
	*min = D3D11_FLOAT32_MAX;

	int vertexIndicesN = meshSkin->mVertexIndices.size();
	for (int i = 0; i < vertexIndicesN; ++i)
	{
		XMVECTOR vertPos = XMLoadFloat3(&meshSkin->mMesh.Vertices[meshSkin->mVertexIndices[i]].Position);
		XMVECTOR posDir = vertPos - pos;
		float dot = XMVectorGetX(XMVector3Dot(posDir, axis));

		if (dot < *min)	*min = dot;
		if (dot > *max)	*max = dot;
	}
}

bool CollisionDetector::PolytopePointFaceTesting(const Mesh_CS *A, const Mesh_CS *B,
	CXMMATRIX fromAtoB, CXMMATRIX fromBtoA, CXMMATRIX worldA, CXMMATRIX worldB, CollisionData_MeshMesh *data)
{
	// This is Ian Millington's simple algorithm for point face collision detection:
	// 1. Consider each vertex of object A.
	// 2. Calculate the interpenetration of that vertex with object B.
	// 3. The deepest such interpenetration is retained (most negative value).
	// 4. Do the same with object B’s vertices against object A.
	// 5. The deepest interpenetration overall is retained (most negative value).

	XMVECTOR contactNormal = XMVectorZero();
	XMVECTOR contactPoint;
	float deepestInterpenetration = D3D11_FLOAT32_MAX;
	bool contactPointVerticeIsPartOf_A;
	char contactPointHolderSkinI;
	UINT vertI, faceI;

	// 1.
	int vertexIndicesN = A->mVertexIndices.size();
	for (int i = 0; i < vertexIndicesN; ++i)
	{
		// Load and transform position to local space of B
		XMVECTOR vertPos = XMLoadFloat3(&A->mMesh.Vertices[A->mVertexIndices[i]].Position);
		XMVECTOR vertPosLocalB = XMVector3TransformCoord(vertPos, fromAtoB);

		// 2.
		XMVECTOR resultAxis;
		UINT faceIndex;
		float interpenetration;
		if (GetInterpenetration(vertPosLocalB, mInterpenetrationEpsilon, &interpenetration, &faceIndex, B))
		{
			resultAxis = XMVector3TransformNormal(XMLoadFloat3(&B->mFaces[faceIndex].mNormal), fromBtoA);

			// 3.
			if (deepestInterpenetration > interpenetration)
			{
				deepestInterpenetration = interpenetration;
				contactNormal = resultAxis;
				contactPoint = vertPos;
				contactPointVerticeIsPartOf_A = true;
				contactPointHolderSkinI = 0;
				vertI = i;
				faceI = faceIndex;
			}
		}
	}

	// 4. & 5.
	vertexIndicesN = B->mVertexIndices.size();
	for (int i = 0; i < vertexIndicesN; ++i)
	{
		// Load and transform position to local space of A
		XMVECTOR vertPos = XMLoadFloat3(&B->mMesh.Vertices[B->mVertexIndices[i]].Position);
		XMVECTOR vertPosLocalA = XMVector3TransformCoord(vertPos, fromBtoA);

		XMVECTOR resultAxis;
		UINT faceIndex;
		float interpenetration;
		if (GetInterpenetration(vertPosLocalA, mInterpenetrationEpsilon, &interpenetration, &faceIndex, A))
		{
			resultAxis = XMVector3TransformNormal(XMLoadFloat3(&A->mFaces[faceIndex].mNormal), fromAtoB);

			if (deepestInterpenetration >= interpenetration)
			{
				deepestInterpenetration = interpenetration;
				contactNormal = resultAxis;
				contactPoint = vertPos;
				contactPointVerticeIsPartOf_A = false;
				contactPointHolderSkinI = 1;
				vertI = i;
				faceI = faceIndex;
			}
		}
	}
	if (deepestInterpenetration > 0) // no interpenetration
		return false;

	// lets save this
	Contact cPoint;
	XMStoreFloat3(&cPoint.mContactNormal, contactNormal);
	XMStoreFloat3(&cPoint.mContactPoint, contactPoint);
	cPoint.mInterpenetration = deepestInterpenetration;
	cPoint.mContactPointHolderSkinI = contactPointHolderSkinI;
	cPoint.mIndexData[0] = vertI;
	cPoint.mIndexData[1] = faceI;

	if (!IsThereAlreadyThisContact(&cPoint, data, true, false)) // name says it all
	if (data->mContactPointsI.size() < COLLISION_DATA_MAX_CONTACT_POINTS)
	{
		int index = data->mContacts.Add(cPoint);
		data->mContactPointsI.push_back(index);
	}

	return true;
}

bool CollisionDetector::PolytopeEdgeEdgeTesting(const Mesh_CS *A, const Mesh_CS *B,
	CXMMATRIX fromAtoB, CXMMATRIX fromBtoA, CXMMATRIX worldA, CXMMATRIX worldB, CollisionData_MeshMesh *data)
{
	// This is also from Ian's book but I changed a lot.
	// There is no search for biggest interpenetration, all interpenetrating edges are added.
	// There are four tests that determine whether edges interpenetrate.
	// I will not go into detail on what does certain test do and why.
	//
	// Unlike with PolytopePointFaceTesting, here it is only possible to measure actual
	// interpenetration (no positive distance from face), meaning
	// *deepestInterpenetration = 0 in beginning and it can only become negative if 
	// interpenetration occurs.

	XMVECTOR centerOfMeshA = XMLoadFloat3(&A->mCenterOfMesh);
	XMVECTOR centerOfMeshB = XMLoadFloat3(&B->mCenterOfMesh);

	int edgesN_A = A->mEdges.size();
	int edgesN_B = B->mEdges.size();
	for (int i = 0; i < edgesN_A; ++i)
	{
		XMVECTOR currentNormal, currentPosition;

		XMVECTOR edgeA_p0 = XMLoadFloat3(&A->mMesh.Vertices[A->mEdges[i].mP0].Position);
		XMVECTOR edgeA_p1 = XMLoadFloat3(&A->mMesh.Vertices[A->mEdges[i].mP1].Position);
		edgeA_p0 = XMVector3TransformCoord(edgeA_p0, fromAtoB); // Now go to B's local space
		edgeA_p1 = XMVector3TransformCoord(edgeA_p1, fromAtoB);
		XMVECTOR AC_in_BLS = XMVector3TransformCoord(centerOfMeshA, fromAtoB); // A center in B local space
		float edgeALength = XMVectorGetX(XMVector3Length(edgeA_p1 - edgeA_p0));

		// Get neighbouring planes of A edge
		const CollisionSkinFace *first = &A->mFaces[A->mEdges[i].mFaceIndex[0]];
		const CollisionSkinFace *second = &A->mFaces[A->mEdges[i].mFaceIndex[1]];
		XMVECTOR faceNormal = XMVector3TransformNormal(XMLoadFloat3(&first->mNormal), fromAtoB);
		XMVECTOR facePos = XMVector3TransformCoord(XMLoadFloat3(&A->mMesh.Vertices[first->mP[0]].Position), fromAtoB);
		XMVECTOR firstPlane_A = XMPlaneFromPointNormal(facePos, faceNormal);
		faceNormal = XMVector3TransformNormal(XMLoadFloat3(&second->mNormal), fromAtoB);
		facePos = XMVector3TransformCoord(XMLoadFloat3(&A->mMesh.Vertices[second->mP[0]].Position), fromAtoB);
		XMVECTOR secondPlane_A = XMPlaneFromPointNormal(facePos, faceNormal);


		for (int j = 0; j < edgesN_B; ++j)
		{
			XMVECTOR edgeB_p0 = XMLoadFloat3(&B->mMesh.Vertices[B->mEdges[j].mP0].Position);
			XMVECTOR edgeB_p1 = XMLoadFloat3(&B->mMesh.Vertices[B->mEdges[j].mP1].Position);
			float edgeBLength = XMVectorGetX(XMVector3Length(edgeB_p1 - edgeB_p0));
			float edgeA_s, edgeB_s;
			XMVECTOR cA, cB, normal;

			// 1. test
			if (!GetClosestPoints_Lines(edgeA_p0, edgeA_p1, edgeB_p0, edgeB_p1, &edgeA_s, &edgeB_s, &normal, &cA, &cB))
				continue; // edges are parallel

			// 2. test
			if ((edgeA_s < 0.0f) || (edgeA_s > edgeALength) || (edgeB_s < 0.0f) || (edgeB_s > edgeBLength))
				continue; // closest points are not on edges

			XMVECTOR lineA_r = edgeA_p0 + XMVectorScale(cA, edgeA_s);
			XMVECTOR lineB_r = edgeB_p0 + XMVectorScale(cB, edgeB_s);
			XMVECTOR r = lineB_r - lineA_r;
			normal = XMVector3Normalize(r); // this normal is pointing in right direction
			XMVECTOR contactPos = lineA_r;

			// project A center, B center and B collision point (lineB_r) onto normal, with center lineA_r.
			float AC_projection = XMVectorGetX(XMVector3Dot(normal, AC_in_BLS - lineA_r));
			float BC_projection = XMVectorGetX(XMVector3Dot(normal, centerOfMeshB - lineA_r));
			float lineB_r_projection = XMVectorGetX(XMVector3Dot(normal, lineB_r - lineA_r));

			// 3. test -> three very important conditions to check, think about them (drawing helps).
			if ((AC_projection < 0.0f) || (BC_projection > 0.0f) || (lineB_r_projection > AC_projection))
				continue;

			float penetrationDepth_AinB = XMVectorGetX(XMVector3Length(r));

			// Get neighbouring planes of B edge
			first = &B->mFaces[B->mEdges[j].mFaceIndex[0]];
			second = &B->mFaces[B->mEdges[j].mFaceIndex[1]];
			faceNormal = XMLoadFloat3(&first->mNormal);
			facePos = XMLoadFloat3(&B->mMesh.Vertices[first->mP[0]].Position);
			XMVECTOR firstPlane_B = XMPlaneFromPointNormal(facePos, faceNormal);
			faceNormal = XMLoadFloat3(&second->mNormal);
			facePos = XMLoadFloat3(&B->mMesh.Vertices[second->mP[0]].Position);
			XMVECTOR secondPlane_B = XMPlaneFromPointNormal(facePos, faceNormal);


			// Last test. It is 4 AM so I will be breif about this. Motivation is to check whether lineB_r and lineA_r are both inside another body.
			// This would be simple if there would be no sharp edges (mAngle value smaller than PIDIV2), so there is special scalar value that serves as tolerance
			// and it is a function of mAngle. The bigger the tolerance means that the point (lineB_r or lineA_r) can be more outside of another body and still be considered as contact point.
			float lineB_r_distance = XMVectorGetX(XMPlaneDotCoord(firstPlane_A, lineB_r));
			float temp = XMVectorGetX(XMPlaneDotCoord(secondPlane_A, lineB_r));
			if (temp > lineB_r_distance) lineB_r_distance = temp;

			float lineA_r_distance = XMVectorGetX(XMPlaneDotCoord(firstPlane_B, lineA_r));
			temp = XMVectorGetX(XMPlaneDotCoord(secondPlane_B, lineA_r));
			if (temp > lineA_r_distance) lineA_r_distance = temp;

			float lineB_r_tolerance = penetrationDepth_AinB * sin(XM_PIDIV2 - A->mEdges[i].mAngle + ANGLE_EPSILON_FOR_EDGE_EDGE_TESTING);
			if (lineB_r_tolerance < 0.0f) lineB_r_tolerance = 0.0f;
			float lineA_r_tolerance = penetrationDepth_AinB * sin(XM_PIDIV2 - B->mEdges[j].mAngle + ANGLE_EPSILON_FOR_EDGE_EDGE_TESTING);
			if (lineA_r_tolerance < 0.0f) lineA_r_tolerance = 0.0f;

			// 4. test
			if ((lineA_r_distance >= lineA_r_tolerance) || (lineB_r_distance >= lineB_r_tolerance))
				continue; // no interpenetration

			currentNormal = XMVector3TransformNormal(normal, fromBtoA); // normal was in B's local space!
			currentPosition = XMVector3TransformCoord(contactPos, fromBtoA); // contactPos was in B's local space!

			// Test to see if there is already this pair in data->mContactEdges.
			bool pairAlreadyExists = false;
			list<char> *cPnt = &data->mContactEdgesI;
			for (auto contactIt = cPnt->begin(); contactIt != cPnt->end();)
			{
				if ((data->mContacts[*contactIt].mIndexData[0] == i) && (data->mContacts[*contactIt].mIndexData[1] == j))
				{
					pairAlreadyExists = true;
					break;
				} else ++contactIt;
			}
			if (pairAlreadyExists) continue;

			// lets save this
			Contact cPoint;
			XMStoreFloat3(&cPoint.mContactNormal, currentNormal);
			XMStoreFloat3(&cPoint.mContactPoint, currentPosition);
			cPoint.mInterpenetration = -penetrationDepth_AinB;
			cPoint.mContactPointHolderSkinI = 0; // It always is 0 for edge-edge testing
			cPoint.mIndexData[0] = i; // A's edge index
			cPoint.mIndexData[1] = j; // B's edge index

			if (data->mContactEdgesI.size() < COLLISION_DATA_MAX_CONTACT_EDGES)
			{
				int index = data->mContacts.Add(cPoint);
				data->mContactEdgesI.push_back(index);
			}
		}
	}

	if (data->mContactEdgesI.size() == 0)
		return false;
	else
		return true;
}

void CollisionDetector::GetCollisionData(const CollisionSkin_RigidBody_Instance *first, const CollisionSkin_RigidBody_Instance *second, CollisionData **data)
{
	*data = 0;

	lock_guard<mutex> lock(mColDataLock);

	// check if there is already CollisionData between these two skins
	for (auto dataIt = mColData.begin(); dataIt != mColData.end(); dataIt++)
	{
		// first skin data?
		if (((*dataIt)->mSkinInstance[0] == first) && ((*dataIt)->mSkinInstance[1] == second))
		{
			*data = (*dataIt); // there is data
			return;
		}
	}

	if (*data == 0) // there was no such data in previous loop
	{	
		CollisionData *newColData;

		if ((typeid(*first->mCollisionSkin) == typeid(const Mesh_CS)) && (typeid(*second->mCollisionSkin) == typeid(const Mesh_CS)))
		{
			newColData = new CollisionData_MeshMesh();
			newColData->mType = CollisionDataType::MeshMesh;
		}
		if ((typeid(*first->mCollisionSkin) == typeid(const Capsule_CS)) && (typeid(*second->mCollisionSkin) == typeid(const Mesh_CS)))
		{
			newColData = new CollisionData_CapsuleMesh(0);
			newColData->mType = CollisionDataType::CapsuleMesh;
		}
		if ((typeid(*first->mCollisionSkin) == typeid(const Mesh_CS)) && (typeid(*second->mCollisionSkin) == typeid(const Capsule_CS)))
		{
			newColData = new CollisionData_CapsuleMesh(1);
			newColData->mType = CollisionDataType::CapsuleMesh;
		}

		newColData->mSkinInstance[0] = first;
		newColData->mSkinInstance[1] = second;
		mColData.push_back(newColData); // and if no contact points are detected, it is deleted right away in resolution method (could be improved)
		*data = (*--mColData.end());
	}
}

bool CollisionDetector::IsThereAlreadyThisContact(Contact *conPnt, CollisionData_MeshMesh *data, bool points, bool edges)
{
	XMVECTOR conPnt1 = XMLoadFloat3(&conPnt->mContactPoint);
	XMMATRIX world[2];
	world[0] = XMLoadFloat4x4(&data->mSkinInstance[0]->mRigidBody->mTransformMatrix);
	world[1] = XMLoadFloat4x4(&data->mSkinInstance[1]->mRigidBody->mTransformMatrix);
	XMVECTOR conPnt1_W = XMVector3TransformNormal(conPnt1, world[conPnt->mContactPointHolderSkinI]);

	// points first
	list<char> *cPnt = &data->mContactPointsI;
	if (points)
	for (auto contactIt = cPnt->begin(); contactIt != cPnt->end();)
	{
		XMVECTOR conPnt2_W = XMVector3TransformNormal(XMLoadFloat3(&data->mContacts[*contactIt].mContactPoint), world[conPnt->mContactPointHolderSkinI]);
		if (XMVectorGetX(XMVector3Length(conPnt1_W - conPnt2_W)) < mVectorComparisonEpsilon) return true;
		else ++contactIt;
	}

	// now set to edges
	cPnt = &data->mContactEdgesI;
	if (edges)
	for (auto contactIt = cPnt->begin(); contactIt != cPnt->end();)
	{
		XMVECTOR conPnt2_W = XMVector3TransformNormal(XMLoadFloat3(&data->mContacts[*contactIt].mContactPoint), world[conPnt->mContactPointHolderSkinI]);
		if (XMVectorGetX(XMVector3Length(conPnt1_W - conPnt2_W)) < mVectorComparisonEpsilon) return true;
		else ++contactIt;
	}

	return false; // there are no contacts like conPnt in data
}