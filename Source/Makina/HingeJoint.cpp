
#include "HingeJoint.h"
#include "RigidBody.h"

#define SIDEWAYS_BEND_EPSILON 0.001f
#define ORIENTATION_EPSILON 0.001f

using namespace Physics_Makina;
using namespace Makina;
using namespace Makina::MathHelper;

HingeJoint::HingeJoint(RigidBody *parent, RigidBody *child, const XMFLOAT3 &anchor, const XMFLOAT3 &hingeAxis, float fwdAngle, float bckAngle)
: Joint(parent, child, anchor)
{
	XMVECTOR hingeAxisWorld = XMLoadFloat3(&hingeAxis);
	Initialize(hingeAxisWorld, fwdAngle, bckAngle);
}

HingeJoint::HingeJoint(RigidBody *parent, RigidBody *child, const XMFLOAT3 &anchor, float fwdAngle, float bckAngle)
: Joint(parent, child, anchor)
{
	// Calculate hinge axis
	XMMATRIX world_parent = XMLoadFloat4x4(&parent->mTransformMatrix);
	XMVECTOR det;
	XMMATRIX toLocalParent = XMMatrixInverse(&det, world_parent);
	XMVECTOR pos_anch = XMLoadFloat3(&anchor);
	XMVECTOR pos_parent = XMLoadFloat3(&mParent->mPosition);
	XMVECTOR pos_child = XMLoadFloat3(&mChild->mPosition);
	XMVECTOR anchDir_parent = XMVector3Normalize(pos_anch - pos_parent);
	XMVECTOR anchDir_child = XMVector3Normalize(pos_anch - pos_child);
	XMVECTOR hingeAxisWorld = XMVector3Cross(anchDir_child, anchDir_parent);
	if (XMVectorGetX(XMVector3LengthSq(hingeAxisWorld)) == 0)
	{	// anchDir_child and anchDir_parent are parallel
		// try (1, 0, 0)
		XMVECTOR tryVec = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		if (abs(XMVectorGetX(XMVector3Dot(tryVec, anchDir_parent))) == 1.0f)
			tryVec = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // try (0, 1, 0)

		tryVec = XMVector3Cross(anchDir_parent, tryVec);
		hingeAxisWorld = XMVector3Cross(tryVec, anchDir_parent);
	}

	Initialize(hingeAxisWorld, fwdAngle, bckAngle);
}

void HingeJoint::Initialize(FXMVECTOR hingeAxisWorld, float fwdAng, float bckAng)
{
	XMVECTOR det;
	XMMATRIX world_parent = XMLoadFloat4x4(&mParent->mTransformMatrix);
	XMMATRIX toLocalParent = XMMatrixInverse(&det, world_parent);
	XMMATRIX world_child = XMLoadFloat4x4(&mChild->mTransformMatrix);
	XMMATRIX toLocalChild = XMMatrixInverse(&det, world_child);
	XMVECTOR anchDir_child = XMLoadFloat3(&mAnchorCLC);
	anchDir_child = XMVector3TransformNormal(anchDir_child, world_child);
	XMVECTOR anchDir_parent = XMLoadFloat3(&mAnchorPLC);
	anchDir_parent = XMVector3TransformNormal(anchDir_parent, world_parent);

	// Calculate and store hinge axis in local space of the parent
	XMVECTOR hingeAxis_parentLocal = XMVector3TransformNormal(hingeAxisWorld, toLocalParent);
	hingeAxis_parentLocal = XMVector3Normalize(hingeAxis_parentLocal);
	XMStoreFloat3(&mHingeAxisPLC, hingeAxis_parentLocal);

	// Calculate and store modified copy of hinge axis perpendicular to the child dir, in child's local space
	XMVECTOR hingeAxisWorld_MC = XMVector3Cross(hingeAxisWorld, anchDir_child);
	hingeAxisWorld_MC = XMVector3Cross(anchDir_child, hingeAxisWorld_MC);
	XMVECTOR hingeAxis_MC_childLocal = XMVector3TransformNormal(hingeAxisWorld_MC, toLocalChild);
	hingeAxis_MC_childLocal = XMVector3Normalize(hingeAxis_MC_childLocal);
	XMStoreFloat3(&mHingeAxisCLC, hingeAxis_MC_childLocal);

	// Now create vector that will help with flexion and extension constraint (mFE_Dir).
	float fe_dir_angle = (fwdAng - bckAng) * 0.5f;
	mFEBendFreedom = fwdAng - fe_dir_angle;
	XMVECTOR fe_dir = XMVector3Rotate(anchDir_parent, XMQuaternionRotationAxis(hingeAxisWorld, fe_dir_angle));
	XMVECTOR fe_dir_PLC = XMVector3TransformNormal(fe_dir, toLocalParent);
	XMStoreFloat3(&mFE_Dir, fe_dir_PLC);
}

HingeJoint::~HingeJoint()
{

}

float HingeJoint::GetPosErrorSeverity()
{
	return mPosErrorSeverity + mSidewaysBendErrorSeverity + mOriErrorSeverity + mFEBendErrorSeverity;
}

float HingeJoint::GetVelErrorSeverity()
{
	return mVelErrorSeverity + mSidewaysBendErrorVelSeverity + mRotErrorVelSeverity + mFEBendErrorVelSeverity;
}

void HingeJoint::CalculatePositionErrorSeverity(float dt)
{
	// from anchor displacement
	XMMATRIX world[2];
	world[0] = XMLoadFloat4x4(&mParent->mTransformMatrix);
	world[1] = XMLoadFloat4x4(&mChild->mTransformMatrix);
	XMVECTOR anchPWC = XMVector3TransformCoord(XMLoadFloat3(&mAnchorPLC), world[0]);
	XMVECTOR anchCWC = XMVector3TransformCoord(XMLoadFloat3(&mAnchorCLC), world[1]);
	XMVECTOR errorVec = anchPWC - anchCWC;
	mPosErrorSeverity = XMVectorGetX(XMVector3Length(errorVec));

	// from sideways bending
	XMVECTOR hingeAxis = XMVector3TransformNormal(XMLoadFloat3(&mHingeAxisPLC), world[0]);
	XMVECTOR pos_parent = XMLoadFloat3(&mParent->mPosition);
	XMVECTOR pos_child = XMLoadFloat3(&mChild->mPosition);
	XMVECTOR anchRelPos_parent = anchPWC - pos_parent;
	XMVECTOR anchRelPos_child = anchPWC - pos_child;
	XMVECTOR dir_parent = XMVector3Normalize(anchRelPos_parent);
	XMVECTOR dir_child = XMVector3Normalize(anchRelPos_child);
	float projToHingeAxis = XMVectorGetX(XMVector3Dot(dir_child, hingeAxis));
	XMVECTOR rotationPlaneProjection = XMVector3Normalize(dir_child - hingeAxis*projToHingeAxis);
	mSidewaysBendActualAngle = GetAngleSafeXMVECTOR(rotationPlaneProjection, dir_child);
	if (mSidewaysBendActualAngle > SIDEWAYS_BEND_EPSILON)
	{
		XMMATRIX inverseInertiaTensor_parent = XMLoadFloat3x3(&mParent->mInverseInertiaTensorWorld);
		XMMATRIX inverseInertiaTensor_child = XMLoadFloat3x3(&mChild->mInverseInertiaTensorWorld);
		XMVECTOR dir = XMVector3Normalize(XMVector3Cross(rotationPlaneProjection, -dir_child));
		XMStoreFloat3(&mSidewaysBendErrorDir, dir);
		float angInertia_parent = 0.0f;
		if (mParent->mIsMovable) angInertia_parent += XMVectorGetX(XMVector3Length(XMVector3Transform(dir, inverseInertiaTensor_parent)));
		float angInertia_child = 0.0f;
		if (mChild->mIsMovable)	angInertia_child += XMVectorGetX(XMVector3Length(XMVector3Transform(dir, inverseInertiaTensor_child)));
		float totalAngInertia = angInertia_parent + angInertia_child;
		float inverseAngInertia = 1.0f / totalAngInertia;
		mAmountOfSidewaysBendingToApplyToParent = angInertia_parent * inverseAngInertia;
		mAmountOfSidewaysBendingToApplyToChild = angInertia_child * inverseAngInertia;
		
		float bendErrorAngle = mSidewaysBendActualAngle - SIDEWAYS_BEND_EPSILON;
		mSidewaysBendErrorSeverity = bendErrorAngle * 0.5f * (mAnchParentDist*mAmountOfSidewaysBendingToApplyToParent +
			mAnchChildDist*mAmountOfSidewaysBendingToApplyToChild);
	}
	else mSidewaysBendErrorSeverity = 0.0f;

	// from flexion and extension constraint
	XMVECTOR fe_dir = XMVector3TransformNormal(XMLoadFloat3(&mFE_Dir), world[0]);
	mFEBendErrorActualAngle = GetAngleSafeXMVECTOR(fe_dir, -dir_child);
	if (mFEBendErrorActualAngle > mFEBendFreedom)
	{
		XMMATRIX inverseInertiaTensor_parent = XMLoadFloat3x3(&mParent->mInverseInertiaTensorWorld);
		XMMATRIX inverseInertiaTensor_child = XMLoadFloat3x3(&mChild->mInverseInertiaTensorWorld);
		XMVECTOR dir = XMVector3Normalize(XMVector3Cross(-dir_child, fe_dir));
		XMStoreFloat3(&mFEBendErrorDir, dir);
		float angInertia_parent = 0.0f;
		if (mParent->mIsMovable) angInertia_parent += XMVectorGetX(XMVector3Length(XMVector3Transform(dir, inverseInertiaTensor_parent)));
		float angInertia_child = 0.0f;
		if (mChild->mIsMovable)	angInertia_child += XMVectorGetX(XMVector3Length(XMVector3Transform(dir, inverseInertiaTensor_child)));
		float totalAngInertia = angInertia_parent + angInertia_child;
		float inverseAngInertia = 1.0f / totalAngInertia;
		mAmountOfFEBendingToApplyToParent = angInertia_parent * inverseAngInertia;
		mAmountOfFEBendingToApplyToChild = angInertia_child * inverseAngInertia;

		float bendErrorAngle = mFEBendErrorActualAngle - mFEBendFreedom;
		mFEBendErrorSeverity = bendErrorAngle * (mAnchParentDist*mAmountOfFEBendingToApplyToParent +
			mAnchChildDist*mAmountOfFEBendingToApplyToChild);
	}
	else mFEBendErrorSeverity = 0.0f;

	// from orientational constraint
	XMVECTOR hingeAxis_child = XMVector3TransformNormal(XMLoadFloat3(&mHingeAxisCLC), world[1]);
	XMVECTOR hingeAxis_parent_to_child = XMVector3Cross(hingeAxis, dir_child);
	float dL = XMVectorGetX(XMVector3Length(hingeAxis_parent_to_child));
	if (dL == 0.0f) hingeAxis_parent_to_child = -dir_parent;
	else hingeAxis_parent_to_child = XMVector3Cross(dir_child, hingeAxis_parent_to_child);
	hingeAxis_parent_to_child = XMVector3Normalize(hingeAxis_parent_to_child);
	mOriActualAngle = GetAngleSafeXMVECTOR(hingeAxis_child, hingeAxis_parent_to_child);
	if (mOriActualAngle > ORIENTATION_EPSILON)
	{
		XMMATRIX inverseInertiaTensor_parent = XMLoadFloat3x3(&mParent->mInverseInertiaTensorWorld);
		XMMATRIX inverseInertiaTensor_child = XMLoadFloat3x3(&mChild->mInverseInertiaTensorWorld);
		XMVECTOR dir = XMVector3Normalize(XMVector3Cross(hingeAxis_child, hingeAxis_parent_to_child));
		XMStoreFloat3(&mOriErrorDir, dir);
		float angInertia_parent = 0.0f;
		if (mParent->mIsMovable) angInertia_parent += XMVectorGetX(XMVector3Length(XMVector3Transform(dir, inverseInertiaTensor_parent)));
		float angInertia_child = 0.0f;
		if (mChild->mIsMovable)	angInertia_child += XMVectorGetX(XMVector3Length(XMVector3Transform(dir, inverseInertiaTensor_child)));
		float totalAngInertia = angInertia_parent + angInertia_child;
		float inverseAngInertia = 1.0f / totalAngInertia;
		mAmountOfRotationToApplyToParent = angInertia_parent * inverseAngInertia;
		mAmountOfRotationToApplyToChild = angInertia_child * inverseAngInertia;

		float rotationErrorAngle = mOriActualAngle - ORIENTATION_EPSILON;
		mOriErrorSeverity = rotationErrorAngle * (mAnchParentDist*mAmountOfRotationToApplyToParent +
			mChildRadiusApprox*mAmountOfRotationToApplyToChild); // it is intentional to use mAnchParentDist and mChildRadiusApprox
	}
	else mOriErrorSeverity = 0.0f;
}

void HingeJoint::CalculateVeclocityErrorSeverity(float dt)
{
	XMMATRIX world[2];
	world[0] = XMLoadFloat4x4(&mParent->mTransformMatrix);
	world[1] = XMLoadFloat4x4(&mChild->mTransformMatrix);
	XMVECTOR anchPWC = XMVector3TransformCoord(XMLoadFloat3(&mAnchorPLC), world[0]);
	XMVECTOR anchCWC = XMVector3TransformCoord(XMLoadFloat3(&mAnchorCLC), world[1]);
	XMVECTOR parentPos = XMLoadFloat3(&mParent->mPosition);
	XMVECTOR childPos = XMLoadFloat3(&mChild->mPosition);
	XMVECTOR dir_parent = XMVector3Normalize(anchPWC - parentPos);
	XMVECTOR dir_child = XMVector3Normalize(anchCWC - childPos);
	XMVECTOR bodyAngVelocity_parent = XMLoadFloat3(&mParent->mAngVelocity);
	XMVECTOR bodyAngVelocity_child = XMLoadFloat3(&mChild->mAngVelocity);

	// calculate velocities of anchor
	XMVECTOR anchVelocity_parent = XMVectorZero();
	if (mParent->mIsMovable)
	{
		anchVelocity_parent = XMVector3Cross(bodyAngVelocity_parent, dir_parent);
		anchVelocity_parent += XMLoadFloat3(&mParent->mLinVelocity);
	}
	XMVECTOR anchVelocity_child = XMVectorZero();
	if (mChild->mIsMovable)
	{
		anchVelocity_child = XMVector3Cross(bodyAngVelocity_child, dir_child);
		anchVelocity_child += XMLoadFloat3(&mChild->mLinVelocity);
	}
	// difference of those two vectors is mVelErrorSeverity
	XMVECTOR velDiff = anchVelocity_parent - anchVelocity_child;
	mVelErrorSeverity = XMVectorGetX(XMVector3Length(velDiff));
	XMStoreFloat3(&mVelKill, velDiff);

	// Sideways bend velocity
	mSidewaysBendErrorVel = 0.0f;
	if (mSidewaysBendActualAngle >= SIDEWAYS_BEND_EPSILON)
	{
		XMVECTOR dir = XMLoadFloat3(&mSidewaysBendErrorDir);
		float velProjParent = XMVectorGetX(XMVector3Dot(dir, bodyAngVelocity_parent));
		float velProjChild = XMVectorGetX(XMVector3Dot(dir, bodyAngVelocity_child));
		mSidewaysBendErrorVel = velProjChild - velProjParent;
		mSidewaysBendErrorVelSeverity = abs(mSidewaysBendErrorVel) * (mAnchParentDist*mAmountOfSidewaysBendingToApplyToParent +
			mAnchChildDist*mAmountOfSidewaysBendingToApplyToChild);
	}

	// FE bend velocity
	mFEBendErrorVel = 0.0f;
	if (mFEBendErrorActualAngle >= mFEBendFreedom)
	{
		XMVECTOR dir = XMLoadFloat3(&mFEBendErrorDir);
		float velProjParent = XMVectorGetX(XMVector3Dot(dir, bodyAngVelocity_parent));
		float velProjChild = XMVectorGetX(XMVector3Dot(dir, bodyAngVelocity_child));
		mFEBendErrorVel = velProjChild - velProjParent;
		mFEBendErrorVelSeverity = abs(mFEBendErrorVel) * (mAnchParentDist*mAmountOfFEBendingToApplyToParent +
			mAnchChildDist*mAmountOfFEBendingToApplyToChild);
	}

	// Now handle rotational velocity
	mRotErrorVel = 0.0f;
	if (mOriActualAngle >= ORIENTATION_EPSILON)
	{
		XMVECTOR rotDir = XMLoadFloat3(&mOriErrorDir);
		float velProjParent = XMVectorGetX(XMVector3Dot(-rotDir, bodyAngVelocity_parent));
		float velProjChild = XMVectorGetX(XMVector3Dot(rotDir, bodyAngVelocity_child));
		mRotErrorVel = velProjParent + velProjChild;
		mRotErrorVelSeverity = abs(mRotErrorVel) * (mAnchParentDist*mAmountOfRotationToApplyToParent +
			mChildRadiusApprox*mAmountOfRotationToApplyToChild); // it is intentional to use mAnchParentDist and mChildRadiusApprox.
	}
}

void HingeJoint::ApplyPositionChange(float relaxation, float dt)
{
	XMMATRIX world[2];
	world[0] = XMLoadFloat4x4(&mParent->mTransformMatrix);
	world[1] = XMLoadFloat4x4(&mChild->mTransformMatrix);
	XMVECTOR anchPWC = XMVector3TransformCoord(XMLoadFloat3(&mAnchorPLC), world[0]);
	XMVECTOR anchCWC = XMVector3TransformCoord(XMLoadFloat3(&mAnchorCLC), world[1]);
	XMVECTOR errorVec = anchPWC - anchCWC;
	float errorVecLen = XMVectorGetX(XMVector3Length(errorVec));
	XMVECTOR errorVecN = errorVec / errorVecLen;
	XMVECTOR pos_parent = XMLoadFloat3(&mParent->mPosition);
	XMVECTOR pos_child = XMLoadFloat3(&mChild->mPosition);
	XMVECTOR relativeAnchPos_parent = anchPWC - pos_parent;
	XMVECTOR relativeAnchPos_child = anchCWC - pos_child;
	XMVECTOR dir_parent = XMVector3Normalize(relativeAnchPos_parent);
	XMVECTOR dir_child = XMVector3Normalize(relativeAnchPos_child);

	XMVECTOR ori_parent = XMLoadFloat4(&mParent->mOrientation);
	XMVECTOR ori_child = XMLoadFloat4(&mChild->mOrientation);
	XMMATRIX inverseInertiaTensor_parent = XMLoadFloat3x3(&mParent->mInverseInertiaTensorWorld);
	XMMATRIX inverseInertiaTensor_child = XMLoadFloat3x3(&mChild->mInverseInertiaTensorWorld);

	//********************************************************************
	//	Now we calculate how much do each rigid body change it's velocity
	//	(lin and ang) with impulse that is 1 in units of measurement.
	//********************************************************************
	if (mPosErrorSeverity > 0.0f)
	{
		float angularInertia_parent = 0;
		if (mParent->mIsMovable)
		{
			XMVECTOR angularInertiaVec_parent = XMVector3Cross(relativeAnchPos_parent, errorVecN);
			angularInertiaVec_parent = XMVector3Transform(angularInertiaVec_parent, inverseInertiaTensor_parent);
			angularInertiaVec_parent = XMVector3Cross(angularInertiaVec_parent, relativeAnchPos_parent);
			angularInertiaVec_parent = XMVector3Dot(angularInertiaVec_parent, errorVecN);
			angularInertia_parent = XMVectorGetX(angularInertiaVec_parent);
		}
		float angularInertia_child = 0;
		if (mChild->mIsMovable)
		{
			XMVECTOR angularInertiaVec_child = XMVector3Cross(relativeAnchPos_child, errorVecN);
			angularInertiaVec_child = XMVector3Transform(angularInertiaVec_child, inverseInertiaTensor_child);
			angularInertiaVec_child = XMVector3Cross(angularInertiaVec_child, relativeAnchPos_child);
			angularInertiaVec_child = XMVector3Dot(angularInertiaVec_child, errorVecN);
			angularInertia_child = XMVectorGetX(angularInertiaVec_child);
		}
		float linearInertia_parent = (mParent->mIsMovable) ? mParent->mInverseMass : 0;
		float linearInertia_child = (mChild->mIsMovable) ? mChild->mInverseMass : 0;

		// calculate total inertia
		float totalInertia = angularInertia_parent + angularInertia_child + linearInertia_parent + linearInertia_child;
		float inverseInertia = 1.0f / totalInertia;
		float linearMove_parent = -errorVecLen * linearInertia_parent * inverseInertia;
		float linearMove_child = errorVecLen * linearInertia_child * inverseInertia;
		float angularMove_parent = -errorVecLen * angularInertia_parent * inverseInertia;
		float angularMove_child = errorVecLen * angularInertia_child * inverseInertia;

		XMVECTOR linearChange[2];
		XMVECTOR angularChange[2];
		//	We now need to calculate the desired rotation (vector)
		if (angularMove_parent == 0)
			angularChange[0] = XMVectorZero();
		else
		{
			XMVECTOR impulsiveTorque = XMVector3Cross(relativeAnchPos_parent, errorVecN);
			angularChange[0] = XMVector3Transform(impulsiveTorque, inverseInertiaTensor_parent) * (angularMove_parent / angularInertia_parent) * relaxation;
		}
		if (angularMove_child == 0)
			angularChange[1] = XMVectorZero();
		else
		{
			XMVECTOR impulsiveTorque = XMVector3Cross(relativeAnchPos_child, errorVecN);
			angularChange[1] = XMVector3Transform(impulsiveTorque, inverseInertiaTensor_child) * (angularMove_child / angularInertia_child) * relaxation;
		}
		// linear velocity
		linearChange[0] = errorVecN * linearMove_parent * relaxation;
		linearChange[1] = errorVecN * linearMove_child * relaxation;

		// Apply the linear movement
		pos_parent += linearChange[0];
		pos_child += linearChange[1];

		// And the change in orientation
		if (angularMove_parent != 0)
		{
			XMVECTOR angularChangeQ = XMVectorSet(XMVectorGetX(angularChange[0]), XMVectorGetY(angularChange[0]), XMVectorGetZ(angularChange[0]), 0);
			ori_parent += XMQuaternionMultiply(ori_parent, angularChangeQ*0.5f);
		}
		if (angularMove_child != 0)
		{
			XMVECTOR angularChangeQ = XMVectorSet(XMVectorGetX(angularChange[1]), XMVectorGetY(angularChange[1]), XMVectorGetZ(angularChange[1]), 0);
			ori_child += XMQuaternionMultiply(ori_child, angularChangeQ*0.5f);
		}
	}

	if (mSidewaysBendErrorSeverity)
	{
		XMVECTOR hingeAxis = XMVector3TransformNormal(XMLoadFloat3(&mHingeAxisPLC), world[0]);
		float projToHingeAxis = XMVectorGetX(XMVector3Dot(dir_child, hingeAxis));
		XMVECTOR rotationPlaneProjection = dir_child - hingeAxis*projToHingeAxis;
		float bendAngle = GetAngleSafeXMVECTOR(rotationPlaneProjection, dir_child);
		float bendErrorAngle = bendAngle - SIDEWAYS_BEND_EPSILON;

		XMVECTOR rot = XMLoadFloat3(&mSidewaysBendErrorDir);
		float angChange_parent = -bendErrorAngle * mAmountOfSidewaysBendingToApplyToParent;
		float angChange_child = bendErrorAngle * mAmountOfSidewaysBendingToApplyToChild;

		if (angChange_parent != 0.0f)
		{
			XMVECTOR rot_parent = rot * angChange_parent * relaxation;
			XMVECTOR angularChangeQ = XMVectorSet(XMVectorGetX(rot_parent), XMVectorGetY(rot_parent), XMVectorGetZ(rot_parent), 0);
			ori_parent += XMQuaternionMultiply(ori_parent, angularChangeQ*0.5f);
		}
		if (angChange_child != 0.0f)
		{
			XMVECTOR rot_child = rot * angChange_child * relaxation;
			XMVECTOR angularChangeQ = XMVectorSet(XMVectorGetX(rot_child), XMVectorGetY(rot_child), XMVectorGetZ(rot_child), 0);
			ori_child += XMQuaternionMultiply(ori_child, angularChangeQ*0.5f);
		}
	}

	if (mFEBendErrorSeverity)
	{
		XMVECTOR fe_dir = XMVector3TransformNormal(XMLoadFloat3(&mFE_Dir), world[0]);
		float bendAngle = GetAngleSafeXMVECTOR(fe_dir, -dir_child);
		float bendErrorAngle = bendAngle - mFEBendFreedom;

		XMVECTOR rot = XMLoadFloat3(&mFEBendErrorDir);
		float angChange_parent = -bendErrorAngle * mAmountOfFEBendingToApplyToParent;
		float angChange_child = bendErrorAngle * mAmountOfFEBendingToApplyToChild;

		if (angChange_parent != 0.0f)
		{
			XMVECTOR rot_parent = rot * angChange_parent * relaxation;
			XMVECTOR angularChangeQ = XMVectorSet(XMVectorGetX(rot_parent), XMVectorGetY(rot_parent), XMVectorGetZ(rot_parent), 0);
			ori_parent += XMQuaternionMultiply(ori_parent, angularChangeQ*0.5f);
		}
		if (angChange_child != 0.0f)
		{
			XMVECTOR rot_child = rot * angChange_child * relaxation;
			XMVECTOR angularChangeQ = XMVectorSet(XMVectorGetX(rot_child), XMVectorGetY(rot_child), XMVectorGetZ(rot_child), 0);
			ori_child += XMQuaternionMultiply(ori_child, angularChangeQ*0.5f);
		}
	}

	if (mOriErrorSeverity)
	{
		XMVECTOR rot = XMLoadFloat3(&mOriErrorDir);
		float rotationErrorAngle = mOriActualAngle - ORIENTATION_EPSILON;
		float angChange_parent = -rotationErrorAngle * mAmountOfRotationToApplyToParent;
		float angChange_child = rotationErrorAngle * mAmountOfRotationToApplyToChild;

		if (angChange_parent != 0.0f)
		{
			XMVECTOR rot_parent = rot * angChange_parent * relaxation;
			XMVECTOR angularChangeQ = XMVectorSet(XMVectorGetX(rot_parent), XMVectorGetY(rot_parent), XMVectorGetZ(rot_parent), 0);
			ori_parent += XMQuaternionMultiply(ori_parent, angularChangeQ*0.5f);
		}
		if (angChange_child != 0.0f)
		{
			XMVECTOR rot_child = rot * angChange_child * relaxation;
			XMVECTOR angularChangeQ = XMVectorSet(XMVectorGetX(rot_child), XMVectorGetY(rot_child), XMVectorGetZ(rot_child), 0);
			ori_child += XMQuaternionMultiply(ori_child, angularChangeQ*0.5f);
		}
	}

	// Normalize and store changes
	XMStoreFloat3(&mParent->mPosition, pos_parent);
	XMStoreFloat3(&mChild->mPosition, pos_child);
	ori_parent = XMQuaternionNormalize(ori_parent);
	ori_child = XMQuaternionNormalize(ori_child);
	XMStoreFloat4(&mParent->mOrientation, ori_parent);
	XMStoreFloat4(&mChild->mOrientation, ori_child);

	// And update transform matrix as final step.
	mParent->UpdateTransformationMatrix();
	mChild->UpdateTransformationMatrix();
}

void HingeJoint::ApplyVelocityChange(float relaxation, float dt)
{
	XMMATRIX world[2];
	world[0] = XMLoadFloat4x4(&mParent->mTransformMatrix);
	world[1] = XMLoadFloat4x4(&mChild->mTransformMatrix);
	XMVECTOR anchPWC = XMVector3TransformCoord(XMLoadFloat3(&mAnchorPLC), world[0]);
	XMVECTOR anchCWC = XMVector3TransformCoord(XMLoadFloat3(&mAnchorCLC), world[1]);
	XMVECTOR parentPos = XMLoadFloat3(&mParent->mPosition);
	XMVECTOR childPos = XMLoadFloat3(&mChild->mPosition);
	XMVECTOR dir_parent = XMVector3Normalize(anchPWC - parentPos);
	XMVECTOR dir_child = XMVector3Normalize(anchCWC - childPos);
	XMVECTOR rotDir = XMLoadFloat3(&mOriErrorDir); // from rotational error
	XMVECTOR sidewaysBendDir = XMLoadFloat3(&mSidewaysBendErrorDir); // from sideways bending error
	XMVECTOR feBendDir = XMLoadFloat3(&mFEBendErrorDir); // from fe bending error

	//********************************************************************
	// Build the matrix to convert joint impulse to change in velocity
	// in world coordinates.
	//********************************************************************
	XMMATRIX deltaVelWorld = ZeroMatrix();
	// Velocity change for parent
	XMMATRIX inverseInertiaTensor_parent = XMLoadFloat3x3(&mParent->mInverseInertiaTensorWorld);
	if (mParent->mIsMovable)
	{
		// This is in world space.
		XMMATRIX skew = MathHelper::SkewSymmetric(dir_parent);
		XMMATRIX deltaVelWorld_parent = skew * XMMatrixScaling(-1.0f, -1.0f, -1.0f); // this is inverse cross product (reason for XMMatrixScaling(-1)).
		deltaVelWorld_parent *= inverseInertiaTensor_parent;
		deltaVelWorld_parent *= skew; // normal cross product
		deltaVelWorld += deltaVelWorld_parent;

		// Combine the linear motion with the angular motion we already have.
		deltaVelWorld._11 += mParent->mInverseMass;
		deltaVelWorld._22 += mParent->mInverseMass;
		deltaVelWorld._33 += mParent->mInverseMass;
	}
	// Velocity change for child
	XMMATRIX inverseInertiaTensor_child = XMLoadFloat3x3(&mChild->mInverseInertiaTensorWorld);
	if (mChild->mIsMovable)
	{
		// This is in world space.
		XMMATRIX skew = MathHelper::SkewSymmetric(dir_child);
		XMMATRIX deltaVelWorld_child = skew * XMMatrixScaling(-1.0f, -1.0f, -1.0f); // this is inverse cross product (reason for XMMatrixScaling(-1)).
		deltaVelWorld_child *= inverseInertiaTensor_child;
		deltaVelWorld_child *= skew; // normal cross product
		deltaVelWorld += deltaVelWorld_child;

		// Combine the linear motion with the angular motion we already have.
		deltaVelWorld._11 += mChild->mInverseMass;
		deltaVelWorld._22 += mChild->mInverseMass;
		deltaVelWorld._33 += mChild->mInverseMass;
	}
	deltaVelWorld._44 = 1.0f;

	XMVECTOR det; // Invert to get the impulse needed per unit velocity.
	XMMATRIX impulseMatrix = XMMatrixInverse(&det, deltaVelWorld);
	// Find the target velocities to kill.
	XMVECTOR killVel = -XMLoadFloat3(&mVelKill);
	// Find the impulse to kill target velocities.
	XMVECTOR impulseContact = XMVector3TransformNormal(killVel, impulseMatrix)*relaxation;

	// Applying the impulse
	if (mParent->mIsMovable)
	{
		XMVECTOR impulse = impulseContact;
		XMVECTOR velocityChange = impulse * mParent->mInverseMass;
		XMVECTOR impulsiveTorque = XMVector3Cross(dir_parent, impulse);
		XMVECTOR rotationChange = XMVector3TransformNormal(impulsiveTorque, inverseInertiaTensor_parent);

		// Change in rotation from sideways bending constraint
		XMVECTOR bendErrorVelKill = sidewaysBendDir * mSidewaysBendErrorVel * mAmountOfSidewaysBendingToApplyToParent * relaxation;
		rotationChange += bendErrorVelKill;

		// Change in rotation from FE bending constraint
		XMVECTOR feBendErrorVelKill = feBendDir * mFEBendErrorVel * mAmountOfFEBendingToApplyToParent * relaxation;
		rotationChange += feBendErrorVelKill;

		// Change in rotation from rotational constraint
		XMVECTOR rotErrorCorrection = rotDir * mRotErrorVel * mAmountOfRotationToApplyToParent * relaxation;
		rotationChange += rotErrorCorrection;

		// Apply and store
		XMVECTOR linVelocity = XMLoadFloat3(&mParent->mLinVelocity) + velocityChange;
		XMVECTOR angVelocity = XMLoadFloat3(&mParent->mAngVelocity) + rotationChange;
		XMStoreFloat3(&mParent->mLinVelocity, linVelocity);
		XMStoreFloat3(&mParent->mAngVelocity, angVelocity);
	}
	if (mChild->mIsMovable)
	{
		XMVECTOR impulse = -impulseContact;
		XMVECTOR velocityChange = impulse * mChild->mInverseMass;
		XMVECTOR impulsiveTorque = XMVector3Cross(dir_child, impulse);
		XMVECTOR rotationChange = XMVector3TransformNormal(impulsiveTorque, inverseInertiaTensor_child);

		// Change in rotation from sideways bending constraint
		XMVECTOR bendErrorVelKill = -sidewaysBendDir * mSidewaysBendErrorVel * mAmountOfSidewaysBendingToApplyToChild * relaxation;
		rotationChange += bendErrorVelKill;

		// Change in rotation from FE bending constraint
		XMVECTOR feBendErrorVelKill = -feBendDir * mFEBendErrorVel * mAmountOfFEBendingToApplyToChild * relaxation;
		rotationChange += feBendErrorVelKill;

		// Change in rotation from rotational constraint
		XMVECTOR rotErrorCorrection = -rotDir * mRotErrorVel * mAmountOfRotationToApplyToChild * relaxation;
		rotationChange += rotErrorCorrection;

		// Apply and store
		XMVECTOR linVelocity = XMLoadFloat3(&mChild->mLinVelocity) + velocityChange;
		XMVECTOR angVelocity = XMLoadFloat3(&mChild->mAngVelocity) + rotationChange;
		XMStoreFloat3(&mChild->mLinVelocity, linVelocity);
		XMStoreFloat3(&mChild->mAngVelocity, angVelocity);
	}
}