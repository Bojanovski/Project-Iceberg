
#ifndef COLLISION_DATA_H
#define COLLISION_DATA_H
#define COLLISION_DATA_MAX_CONTACT_POINTS 8
#define COLLISION_DATA_MAX_CONTACT_EDGES 8

#include "PhysicsHeaders.h"

namespace Physics_Makina
{
	class CollisionSkin;

	/********************************************************************************
	This struct contains all data needed to describe a contact.
	********************************************************************************/
	struct Contact
	{
		// Holds the position of the contact in local coordinates.
		XMFLOAT3 mContactPoint;

		// Holds the direction of the contact in local coordinates.
		XMFLOAT3 mContactNormal;

		// Holds the depth of penetration at the contact point. If both
		// bodies are specified then the contact point should be midway
		// between the inter-penetrating points.
		float mInterpenetration;

		// Index for obtaining collision skin from CollisionData::mSkinInstance[2] that contains contact point vertice.
		char mContactPointHolderSkinI;

		// A transform matrix that converts coordinates in the contact’s
		// frame of reference to world coordinates. The columns of this
		// matrix form an orthonormal set of vectors.
		XMFLOAT4X4 mContactToWorld;

		// Holds the closing velocity at the point of contact. This is
		// set when the calculateInternals function is run.
		XMFLOAT3 mContactVelocity;

		// Holds the required change in velocity for this contact to be
		// resolved.
		float mDesiredDeltaVelocity;

		// Holds the world space position of the contact point
		// relative to the center of each body. This is set when
		// the calculateInternals function is run.
		XMFLOAT3 mRelativeContactPosition[2];

		// This holds vertex and face index if Contact is a point 
		// or two edge indices if contact is edge-edge.
		UINT mIndexData[2];
	};

	/********************************************************************************
	A helper structures that contains information for the detector to use in
	building its contact data.
	********************************************************************************/
	enum CollisionDataType { MeshMesh, CapsuleMesh };

	struct CollisionData
	{
		// This structure will be given to a CollisionDetector for collision resolution.
		// Both RigidBodies are needed, so here are two pointers to a CollisionSkin_RigidBody_Instance
		// which has a pointer of type RigidBody.
		// This is also needed when CollisionDetector is deciding should new Contact be
		// added to new Collision data or old one.
		CollisionSkin_RigidBody_Instance const *mSkinInstance[2];
		Makina::HostedList<Contact> mContacts;
		CollisionDataType mType;

		virtual bool HasNoContacts() = 0;
		bool BothAsleep();
	};

	struct CollisionData_MeshMesh : public CollisionData
	{
		bool HasNoContacts();	
		
		// Holds the contact index list.
		std::list<char> mContactPointsI;
		std::list<char> mContactEdgesI;
	};

	struct CollisionData_CapsuleMesh : public CollisionData
	{
		CollisionData_CapsuleMesh(char capsuleIndex);
		bool HasNoContacts();
		CollisionSkin_RigidBody_Instance const *GetCapsuleSkinInstance() { return mSkinInstance[mCapsuleIndex]; }
		CollisionSkin_RigidBody_Instance const *GetMeshSkinInstance() { return mSkinInstance[1 - mCapsuleIndex]; }

		// Holds the contact index list.
		std::list<char> mContactPointsI;

	private:
		char mCapsuleIndex;
	};
}

#endif
