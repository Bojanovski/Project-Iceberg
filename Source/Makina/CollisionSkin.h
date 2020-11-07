
#ifndef COLLISION_SKIN_H
#define COLLISION_SKIN_H

#include "PhysicsHeaders.h"

namespace Physics_Makina
{
	class CollisionSkin
	{
		friend class BinnedCollisionSpace;
		friend class CollisionDetector;
		friend class CollisionResolver;
		friend class RigidBody;

	public:
		__declspec(dllexport) CollisionSkin(Makina::OrientedBox const *obb);
		__declspec(dllexport) virtual ~CollisionSkin();

		// Dynamic allocation must be aligned to the 16, therefore custom 'new' and 'delete' operators are needed.
		__declspec(dllexport) void *operator new(size_t size);
		__declspec(dllexport) void operator delete(void *pt);

	protected:
		Makina::OrientedBox mOBB;
	};

	// Helper struct for handling faces
	struct CollisionSkinFace
	{
		std::vector<UINT> mP;
		XMFLOAT3 mNormal;

		CollisionSkinFace() {}
	};

	// Helper struct for handling edges
	struct CollisionSkinEdge
	{
		// indices to vertex data
		UINT mP0;
		UINT mP1;
		float mAngle; // inside angle made by two planes (faces)
		// every two non parallel planes (faces) define a line, this line (edge) is no different
		UINT mFaceIndex[2];

		CollisionSkinEdge(UINT p0, UINT p1)
			: mP0(p0),
			mP1(p1)	{}

		bool operator== (const CollisionSkinEdge &edge)
		{
			return ((mP0 == edge.mP0) && (mP1 == edge.mP1)) ||
				((mP0 == edge.mP1) && (mP1 == edge.mP0));
		}
	};

	// Mesh collision skin
	class Mesh_CS : public CollisionSkin
	{
		friend class BinnedCollisionSpace;
		friend class CollisionDetector;
		friend class CollisionResolver;
		friend class RigidBody;

	public:
		__declspec(dllexport) Mesh_CS(Makina::OrientedBox const *obb, Makina::BasicMeshData const *mesh);
		__declspec(dllexport) ~Mesh_CS();

		Makina::BasicMeshData const *GetMesh() { return &mMesh; }

	private:
		// Construct index array for edges
		void DefineEdgesFacesAndVertices();

		Makina::BasicMeshData mMesh;
		std::vector<CollisionSkinEdge> mEdges;
		std::vector<CollisionSkinFace> mFaces;
		std::vector<UINT> mVertexIndices;
		XMFLOAT3 mCenterOfMesh;
	};

	// Capsule collision skin
	class Capsule_CS : public CollisionSkin
	{
		friend class BinnedCollisionSpace;
		friend class CollisionDetector;
		friend class CollisionResolver;
		friend class RigidBody;

	public:
		__declspec(dllexport) Capsule_CS(Makina::OrientedBox const *obb, float height, float radius);
		__declspec(dllexport) ~Capsule_CS();

		float GetHeight() { return mH; }
		float GetRadius() { return mR; }

	private:
		float mH;			// height
		float mH_DIV_2;		// height divided by two
		float mR;			// radius
	};
}

#endif
