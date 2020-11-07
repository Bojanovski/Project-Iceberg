
#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include "PhysicsHeaders.h"

namespace Physics_Makina
{
	/********************************************************************************
		A class that contains all physical details of object in space such as 
		mass, inertia tensor, center of mass, applied forces,
		list of collision skins, collision data...

		Also, it is important to know that this class serves as an interface so that
		program read what is the rotation/position of body in space at current time.
	********************************************************************************/
	class RigidBody
	{
		friend class World;
		friend class ForceGenerator;
		friend class CollisionDetector;
		friend class CollisionResolver;
		friend class BinnedCollisionSpace;
		friend class Joint;
		friend class UniversalJoint;
		friend class HingeJoint;

	public:
		__declspec(dllexport) RigidBody(bool isMovable, bool canSleep, bool isAwake, float restitutionCoefficient, float frictionCoefficient, float mass, XMFLOAT3 &cm, XMFLOAT3X3 &inertia,
			XMFLOAT3 &position, XMFLOAT4 &orientation, XMFLOAT3 &velocity);
		__declspec(dllexport) RigidBody(const RigidBody &obj);
		__declspec(dllexport) ~RigidBody();

		void GetTransformation(XMFLOAT4X4 *tMatrix) { std::lock_guard<std::mutex> lk(mLock); *tMatrix = mTransformMatrix_Rend; }
		void GetPosition(XMFLOAT3 *pos) { std::lock_guard<std::mutex> lk(mLock); *pos = mPosition; }
		float GetMass() { return 1.0f/mInverseMass; }
		bool HasFiniteMass() const { return (_finite(mInverseMass) != 0) ? true : false; }
		bool IsMovable() const { return mIsMovable; }	
		static float GetSleepEpsilon() { return mSleepEpsilon; }
		static void SetSleepEpsilon(float sleepEpsilon) { mSleepEpsilon = sleepEpsilon; }	
		__declspec(dllexport) void SetAwake(const bool awake);
		bool IsAwake() { return mIsAwake; }

		void SetPosition(CXMVECTOR pos) { std::lock_guard<std::mutex> lk(mLock); XMStoreFloat3(&mPosition, pos); }
		void SetOrientation(CXMVECTOR ori) { std::lock_guard<std::mutex> lk(mLock); XMStoreFloat4(&mOrientation, ori); }
		// Updates transformation matrices, both the renderable and the primary one.
		__declspec(dllexport) void ForceUpdateOfTransformationMatrix();

		// skins
		__declspec(dllexport) void AddCollisionSkin(CollisionSkin *collSkin);
		CollisionSkin_RigidBody_Instance &GetCollisionSkin_RigidBody_Instance(int skinIndex) { return mSkinInstances[skinIndex]; }
		int GetCollisionSkin_RigidBody_InstanceCount() { return m_allocIndex; }

	private:	
		void Integrate(float dt);
		void CheckForSleep();
		void UpdateTransformationMatrix();

		void ClearAccumulators();
		void AddForce(const XMFLOAT3 &force);
		void AddTorque(const XMFLOAT3 &torque);
		void AddForceAtBodyPoint(const XMFLOAT3 &force, const XMFLOAT3 &point);
		void AddForceAtPoint(const XMFLOAT3 &force, const XMFLOAT3 &point);

		// It should be set as high as possible before strange mid - motion freezes become apparent.
		static float mSleepEpsilon;

		// A lot of optimizations take place if object is not movable.
		bool mIsMovable;

		// Some bodies may never be allowed to fall asleep.
		// User-controlled bodies, for example, should be
		// always awake.
		bool mCanSleep;

		// A body can be put to sleep to avoid it being updated
		// by the integration functions or affected by collisions
		// with the world.
		bool mIsAwake;

		// Holds the amount of motion of the body. This is a recency-
		// weighted mean that can be used to put a body to sleep.
		float mMotion;

		float mRestitutionCoefficient;
		float mFrictionCoefficient;
		float mInverseMass;
		XMFLOAT3 mLastUpdateAcceleration;
		XMFLOAT3 mCm;
		XMFLOAT3X3 mInverseInertiaTensorBody;
		XMFLOAT3X3 mInverseInertiaTensorWorld;

		XMFLOAT3 mPosition;
		XMFLOAT4 mOrientation;

		// accumulators
		XMFLOAT3 mForceAcc;
		XMFLOAT3 mTorqueAcc;

		// linear and angular velocities
		XMFLOAT3 mLinVelocity;
		XMFLOAT3 mAngVelocity;

		// cache data
		XMFLOAT4X4 mTransformMatrix;
		XMFLOAT4X4 mTransformMatrix_Rend;

		// collision skins and data
		UINT m_allocIndex;
		CollisionSkin_RigidBody_Instance mSkinInstances[COLLISION_SKINS_PER_RIGID_BODY];

		// registration with World class
		bool mReg;

		// Multithreading
		std::mutex mLock;
	};
}

#endif
