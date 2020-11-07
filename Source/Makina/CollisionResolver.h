
#ifndef COLLISION_RESOLVER_H
#define COLLISION_RESOLVER_H

#define PREPARE_CONTACTS_ASYNCWORKER_N 16

#include "PhysicsHeaders.h"
#include "CollisionData.h"
#include "CollisionSkin.h"

namespace Physics_Makina
{
	class Joint;

	/********************************************************************************
		The contact resolution routine. One resolver instance
		can be shared for the whole simulation, as long as you need
		roughly the same parameters each time (which is normal).
	********************************************************************************/
	class CollisionResolver
	{
		class PrepareContactsAsyncWorker : public Makina::AsyncWorker
		{
		public:
			PrepareContactsAsyncWorker() {}
			~PrepareContactsAsyncWorker() {}

		private:
			void Work();
		};

	public:
		__declspec(dllexport) CollisionResolver(UINT positionIterations, UINT velocityIterations, float angularMoveLimitConstant, float velocityEpsilon, float interpenetrationRelaxation, float velocityRelaxation);
		__declspec(dllexport) ~CollisionResolver();

		// Resolves a set of contacts for both penetration and velocity.
		__declspec(dllexport) void ResolveContacts(CollisionDetector *colDet, float dt);

		// Adds a joint to the vector
		__declspec(dllexport) void AddJoint(Joint *joint) { mJoints.push_back(joint); }

		// Remove a joint from the vector
		__declspec(dllexport) void RemoveJoint(Joint *joint);

	private:
		// Prepares values in contacts: mRelativeContactPosition
		void CalculateContactInterpenetrationData(Contact *contact, const CollisionSkin_RigidBody_Instance *A, const CollisionSkin_RigidBody_Instance *B);

		PrepareContactsAsyncWorker mPrepareContactsAsyncWorkers[PREPARE_CONTACTS_ASYNCWORKER_N];

		// Prepares values in contacts: mContactToWorld
		void CalculateContactBasisMatrix(Contact *contact, const CollisionSkin_RigidBody_Instance *A, const CollisionSkin_RigidBody_Instance *B);
		
		// Prepares values in contacts: mDesiredDeltaVelocity, mContactVelocity
		void CalculateContactVelocityData(Contact *contact, const CollisionSkin_RigidBody_Instance *A, const CollisionSkin_RigidBody_Instance *B, float dt);

		// This method handles interpenetration.
		void AdjustPositions(CollisionDetector *colDet, float dt);

		// Helper method for AdjustPositions() method.
		void UpdateContactsAndJoints_pos(CollisionDetector *colDet, const RigidBody *rgdB1, const RigidBody *rgdB2, float dt);

		// Changes position and orientation of RigidBody object and outputs changes.
		void ApplyPositionChange(Contact *contact, const CollisionSkin_RigidBody_Instance *A, const CollisionSkin_RigidBody_Instance *B, XMVECTOR linearChange[2], XMVECTOR angularChange[2], float dt);

		// Special cases contact update method for after the contact resolution.
		void UpdateContact_MeshMesh(CollisionData_MeshMesh *colDat);

		// This method handles closing velocities.
		void AdjustVelocities(CollisionDetector *colDet, float dt);

		// Helper method for AdjustPositions() method.
		void UpdateContactsAndJoints_vel(CollisionDetector *colDet, const RigidBody *rgdB1, const RigidBody *rgdB2, float dt);

		// Changes position and orientation of RigidBody object and outputs changes.
		void ApplyVelocityChange(Contact *contact, const CollisionSkin_RigidBody_Instance *A, const CollisionSkin_RigidBody_Instance *B, XMVECTOR velocityChange[2], XMVECTOR rotationChange[2], float dt);

		UINT mPositionIterations;
		UINT mVelocityIterations;

		// Velocities that are less than this value will not bounce of regardless of restitution coefficient.
		float mVelocityEpsilon;

		// The amount of move (not rotation in radians) that is maximum for a body when resolving interpenetration.
		float mAngularMoveLimitConstant;

		// Relaxations. Relaxation resolves only a proportion of the interpenetration at one go.
		float mInterpenetrationRelaxation;
		float mVelocityRelaxation;

		// Joints :)
		std::vector<Joint *> mJoints;
	};
}

#endif
