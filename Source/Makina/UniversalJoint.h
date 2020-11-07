
#ifndef UNIVERSAL_JOINT_H
#define UNIVERSAL_JOINT_H

#include "Joint.h"

namespace Physics_Makina
{
	/****************************************************************************
		Also known as the "ball and socket". Used in joint such as 
		shoulders and hips.
	****************************************************************************/
	class UniversalJoint : public Joint
	{
	public:
		__declspec(dllexport) UniversalJoint(RigidBody *parent, RigidBody *child, const XMFLOAT3 &anchor, float rotationalFreedom, float bendFreedom);
		__declspec(dllexport) ~UniversalJoint();

		__declspec(dllexport) float GetPosErrorSeverity();
		__declspec(dllexport) float GetVelErrorSeverity();
		__declspec(dllexport) void CalculatePositionErrorSeverity(float dt);
		__declspec(dllexport) void CalculateVeclocityErrorSeverity(float dt);
		__declspec(dllexport) void ApplyPositionChange(float relaxation, float dt);
		__declspec(dllexport) void ApplyVelocityChange(float relaxation, float dt);

	private:
		// Bending
		float mBendFreedom;

		// Rotation
		float mRotFreedom;

		// Vectors used to determine how much are parent and child rotated one relative to another
		XMFLOAT3 mRotationVec_parent;
		XMFLOAT3 mRotationVec_child;

		// Based on moment of inertia, apply different amounts of rotation and bending
		float mAmountOfRotationToApplyToParent;
		float mAmountOfRotationToApplyToChild;
		float mAmountOfBendingToApplyToParent;
		float mAmountOfBendingToApplyToChild;

		// Measurements of error and cache
		float mPosErrorSeverity;
		float mBendErrorSeverity;
		float mBendActualAngle;
		float mOriErrorSeverity;
		float mOriErrorAngle;
		float mOriActualAngle;
		float mVelErrorSeverity;
		float mBendErrorVelSeverity;
		float mBendErrorVel;
		XMFLOAT3 mBendErrorDir;
		float mRotErrorVelSeverity;
		float mRotErrorVel;
		XMFLOAT3 mVelKill;
	};
}

#endif
