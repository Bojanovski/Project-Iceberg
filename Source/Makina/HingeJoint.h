
#ifndef HINGE_JOINT_H
#define HINGE_JOINT_H

#include "Joint.h"

namespace Physics_Makina
{
	/***************************************************************************
	*	Used in joints such as elbow and knee.
	****************************************************************************/
	class HingeJoint : public Joint
	{
	public:
		__declspec(dllexport) HingeJoint(RigidBody *parent, RigidBody *child, const XMFLOAT3 &anchor, const XMFLOAT3 &hingeAxis, float fwdAngle, float bckAngle);
		__declspec(dllexport) HingeJoint(RigidBody *parent, RigidBody *child, const XMFLOAT3 &anchor, float fwdAngle, float bckAngle);
		__declspec(dllexport) ~HingeJoint();

		__declspec(dllexport) float GetPosErrorSeverity();
		__declspec(dllexport) float GetVelErrorSeverity();
		__declspec(dllexport) void CalculatePositionErrorSeverity(float dt);
		__declspec(dllexport) void CalculateVeclocityErrorSeverity(float dt);
		__declspec(dllexport) void ApplyPositionChange(float relaxation, float dt);
		__declspec(dllexport) void ApplyVelocityChange(float relaxation, float dt);

	private:
		__declspec(dllexport) void Initialize(FXMVECTOR hingeAxisWorld, float fwdAng, float bckAng);

		// Hinge properties
		XMFLOAT3 mHingeAxisPLC;
		XMFLOAT3 mHingeAxisCLC;	
		float mFEBendFreedom;
		XMFLOAT3 mFE_Dir;

		// Measurements of error and cache
		float mPosErrorSeverity;
		float mVelErrorSeverity;
		XMFLOAT3 mVelKill;

		float mSidewaysBendErrorSeverity;
		float mSidewaysBendActualAngle;
		XMFLOAT3 mSidewaysBendErrorDir;
		float mAmountOfSidewaysBendingToApplyToParent;
		float mAmountOfSidewaysBendingToApplyToChild;
		float mSidewaysBendErrorVelSeverity;
		float mSidewaysBendErrorVel;
		float mOriErrorSeverity;
		float mOriActualAngle;
		XMFLOAT3 mOriErrorDir;
		float mRotErrorVelSeverity;
		float mRotErrorVel;
		float mAmountOfRotationToApplyToParent;
		float mAmountOfRotationToApplyToChild;
		float mFEBendErrorSeverity;
		float mFEBendErrorActualAngle;
		XMFLOAT3 mFEBendErrorDir;
		float mFEBendErrorVelSeverity;
		float mFEBendErrorVel;
		float mAmountOfFEBendingToApplyToParent;
		float mAmountOfFEBendingToApplyToChild;
	};
}

#endif
