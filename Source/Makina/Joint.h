
#ifndef JOINT_H
#define JOINT_H

#include "PhysicsHeaders.h"

namespace Physics_Makina
{
	class Joint
	{
		friend class CollisionResolver;

	public:
		Joint(RigidBody *parent, RigidBody *child, const XMFLOAT3 &anchor);
		virtual ~Joint();

		bool AtLeastOneAwake();
		virtual float GetPosErrorSeverity() = 0;
		virtual float GetVelErrorSeverity() = 0;
		virtual void CalculatePositionErrorSeverity(float dt) = 0;
		virtual void CalculateVeclocityErrorSeverity(float dt) = 0;
		virtual void ApplyPositionChange(float relaxation, float dt) = 0;
		virtual void ApplyVelocityChange(float relaxation, float dt) = 0;

	protected:
		RigidBody *mParent;
		RigidBody *mChild;

		// Anchor of the joint in parent rigid body's local coordinates.
		XMFLOAT3 mAnchorPLC;

		// Anchor of the joint in child rigid body's local coordinates.
		XMFLOAT3 mAnchorCLC;

		// Distances
		float mAnchParentDist;
		float mAnchChildDist;

		// Radius
		float mParentRadiusApprox;
		float mChildRadiusApprox;
	};
}

#endif
