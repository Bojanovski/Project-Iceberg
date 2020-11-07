
#ifndef COLLISION_DETECTOR_H
#define COLLISION_DETECTOR_H

#include "PhysicsHeaders.h"
#include "CollisionData.h"
#include "CollisionSkin.h"

namespace Physics_Makina
{
	/********************************************************************************
		This class contains all collision detection algorithms.
	********************************************************************************/
	class CollisionDetector
	{
		friend class CollisionResolver;

	public:
		__declspec(dllexport) CollisionDetector(float interpenetrationEpsilon,  float vectorComparisonEpsilon);
		__declspec(dllexport) ~CollisionDetector();

		__declspec(dllexport) void DetectContacts(const CollisionSkin_RigidBody_Instance *first, const CollisionSkin_RigidBody_Instance *second);

		// In one pass, only one Contact point and edge can be added to the CollisionData, so in order to realize multiple contact in single collData,
		// like in, for example, face-face collision data (needs 3 or more contact points), it is needed to save old contact, revalidate them
		// using some epsilon value and (if they pass) add them to the newely created contact.
		__declspec(dllexport) void RevalidatePreviousContacts();

		// Functions to handle adding and removing mesh collision skin data.
		__declspec(dllexport) void AddCollisionSkin(CollisionSkin *skin);
		__declspec(dllexport) void RemoveCollisionSkin(CollisionSkin *skin);

		// Get two closest points on two lines. Returns false if lines are parallel or coplanar.
		// Vector n is direction of a line that connects two closest points.
		__declspec(dllexport) static bool GetClosestPoints_Lines(FXMVECTOR LineA_p0, FXMVECTOR LineA_p1, FXMVECTOR LineB_p0, CXMVECTOR LineB_p1,
			float *LineA_r, float *LineB_r, XMVECTOR *n, XMVECTOR *cA, XMVECTOR *cB);

		// Get interpenetration, return false if there is no interpenetration.
		__declspec(dllexport) static bool GetInterpenetration(FXMVECTOR pos, float mInterpenetrationEpsilon, float *interpenetration, UINT *faceIndex, const Mesh_CS *meshSkin);

	private:
		// Specific revalidation methods:
		__declspec(dllexport) inline void RevalidateMeshMesh(CollisionData_MeshMesh *data);
		__declspec(dllexport) inline void RevalidateCapsuleMesh(CollisionData_CapsuleMesh *data);

		// Returns true if collision is found.
		__declspec(dllexport) bool PolytopePointFaceTesting(const Mesh_CS *A, const Mesh_CS *B,
			CXMMATRIX fromAtoB, CXMMATRIX fromBtoA, CXMMATRIX worldA, CXMMATRIX worldB, CollisionData_MeshMesh *data);

		// Returns true if collision is found.
		__declspec(dllexport) bool PolytopeEdgeEdgeTesting(const Mesh_CS *A, const Mesh_CS *B,
			CXMMATRIX fromAtoB, CXMMATRIX fromBtoA, CXMMATRIX worldA, CXMMATRIX worldB, CollisionData_MeshMesh *data);

		// Checks if point is inside mesh
		__declspec(dllexport) bool IsPointInside(FXMVECTOR pos, bool useEpsilon, const Mesh_CS *meshSkin);

		// Project mesh onto axis with position. Returns projection with smallest and largest value.
		__declspec(dllexport) void GetProjection(FXMVECTOR pos, XMVECTOR axis, const Mesh_CS *meshSkin, float *min, float *max);

		// Checks whether there is already collision data between this two skins, if not, it creates one.
		// There should not exist two pairs of same skins but in different order. Reason for this is "bigger first check" in the beginning of DetectContacts(...). 
		__declspec(dllexport) void GetCollisionData(const CollisionSkin_RigidBody_Instance *first, const CollisionSkin_RigidBody_Instance *second, CollisionData **data);

		// Checks whether there is already contact like conPnt, if so, it returns false, if not, it returns true.
		__declspec(dllexport) bool IsThereAlreadyThisContact(Contact *conPnt, CollisionData_MeshMesh *data, bool points, bool edges);

		// Real number (typicaly very small) that describes how much certain point
		// can be above of previous penetration plane and still be considered as contact point.
		float mInterpenetrationEpsilon;

		// Used in IsThereAlreadyThisContact(). 
		float mVectorComparisonEpsilon;

		// All collisions are contained here.
		std::list<CollisionData *> mColData; // this can be improved with memory pools
		std::mutex mColDataLock;

		// Stores all mesh collision skin data.
		std::vector<CollisionSkin *> mCollisionSkins;
	};
}

#endif