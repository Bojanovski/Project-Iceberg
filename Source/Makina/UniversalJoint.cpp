
#include "UniversalJoint.h"
#include "RigidBody.h"

using namespace Physics_Makina;
using namespace Makina;
using namespace Makina::MathHelper;

UniversalJoint::UniversalJoint(RigidBody *parent, RigidBody *child, const XMFLOAT3 &anchor, float rotationalFreedom, float bendFreedom)
: Joint(parent, child, anchor),
mRotFreedom(rotationalFreedom),
mBendFreedom(bendFreedom)
{
	XMVECTOR det;
	XMMATRIX inverseInertiaTensor_parent = XMLoadFloat3x3(&mParent->mInverseInertiaTensorWorld);
	XMMATRIX inertiaTensor_parent = XMMatrixInverse(&det, inverseInertiaTensor_parent);
	XMMATRIX inverseInertiaTensor_child = XMLoadFloat3x3(&mChild->mInverseInertiaTensorWorld);
	XMMATRIX inertiaTensor_child = XMMatrixInverse(&det, inverseInertiaTensor_child);
	XMVECTOR pos_anch = XMLoadFloat3(&anchor);
	XMVECTOR pos_parent = XMLoadFloat3(&mParent->mPosition);
	XMVECTOR pos_child = XMLoadFloat3(&mChild->mPosition);
	XMVECTOR anchDir_parent = XMVector3Normalize(pos_anch - pos_parent);
	XMVECTOR anchDir_child = XMVector3Normalize(pos_anch - pos_child);
	XMVECTOR angImpulse_parent = XMVector3Transform(anchDir_parent, inertiaTensor_parent);
	XMVECTOR angImpulse_child = XMVector3Transform(anchDir_child, inertiaTensor_child);
	float angImpulse_parentF = XMVectorGetX(XMVector3Length(angImpulse_parent));
	float angImpulse_childF = XMVectorGetX(XMVector3Length(angImpulse_child));
	float totalAngImpulse = angImpulse_parentF + angImpulse_childF;
	// bigger impulse needed --> more inertia the body has
	if (mParent->mIsMovable && mChild->mIsMovable)
	{
		mAmountOfRotationToApplyToParent = angImpulse_childF / totalAngImpulse;
		mAmountOfRotationToApplyToChild = angImpulse_parentF / totalAngImpulse;
	}
	else if (mParent->mIsMovable && !mChild->mIsMovable)
	{
		mAmountOfRotationToApplyToParent = 1.0f;
		mAmountOfRotationToApplyToChild = 0.0f;
	}
	else if (!mParent->mIsMovable && mChild->mIsMovable)
	{
		mAmountOfRotationToApplyToParent = 0.0f;
		mAmountOfRotationToApplyToChild = 1.0f;
	}
	else if (!mParent->mIsMovable && !mChild->mIsMovable)
	{
		mAmountOfRotationToApplyToParent = 0.0f;
		mAmountOfRotationToApplyToChild = 0.0f;
	}

	// Now calculate rotation direction vectors in world space.
	XMMATRIX world[2];
	world[0] = XMLoadFloat4x4(&parent->mTransformMatrix);
	world[1] = XMLoadFloat4x4(&child->mTransformMatrix);
	XMMATRIX toLocalChild = XMMatrixInverse(&det, world[1]);
	XMMATRIX toLocalParent = XMMatrixInverse(&det, world[0]);
	// try cross(dir_p, dir_c)
	XMVECTOR tryVec = XMVector3Cross(anchDir_parent, -anchDir_child);
	XMVECTOR rotVec_parent = XMVector3Cross(tryVec, anchDir_parent);
	XMVECTOR rotVec_child = XMVector3Cross(tryVec, -anchDir_child);
	if (XMVectorGetX(XMVector3Length(rotVec_parent)) != 0 && XMVectorGetX(XMVector3Length(rotVec_child)) != 0)
	{
		XMVECTOR rotVec_parentLocal = XMVector3TransformNormal(rotVec_parent, toLocalParent);
		XMVECTOR rotVec_childLocal = XMVector3TransformNormal(rotVec_child, toLocalChild);
		XMStoreFloat3(&mRotationVec_parent, XMVector3Normalize(rotVec_parentLocal));
		XMStoreFloat3(&mRotationVec_child, XMVector3Normalize(rotVec_childLocal));
		return;
	}
	// if not, try (1, 0, 0)
	tryVec = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	rotVec_parent = XMVector3Cross(tryVec, anchDir_parent);
	rotVec_child = XMVector3Cross(tryVec, -anchDir_child);
	if (XMVectorGetX(XMVector3Length(rotVec_parent)) != 0 && XMVectorGetX(XMVector3Length(rotVec_child)) != 0)
	{
		XMVECTOR rotVec_parentLocal = XMVector3TransformNormal(rotVec_parent, toLocalParent);
		XMVECTOR rotVec_childLocal = XMVector3TransformNormal(rotVec_child, toLocalChild);
		XMStoreFloat3(&mRotationVec_parent, XMVector3Normalize(rotVec_parentLocal));
		XMStoreFloat3(&mRotationVec_child, XMVector3Normalize(rotVec_childLocal));
		return;
	}
	// if not, try (0, 1, 0)
	tryVec = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	rotVec_parent = XMVector3Cross(tryVec, anchDir_parent);
	rotVec_child = XMVector3Cross(tryVec, -anchDir_child);
	if (XMVectorGetX(XMVector3Length(rotVec_parent)) != 0 && XMVectorGetX(XMVector3Length(rotVec_child)) != 0)
	{
		XMVECTOR rotVec_parentLocal = XMVector3TransformNormal(rotVec_parent, toLocalParent);
		XMVECTOR rotVec_childLocal = XMVector3TransformNormal(rotVec_child, toLocalChild);
		XMStoreFloat3(&mRotationVec_parent, XMVector3Normalize(rotVec_parentLocal));
		XMStoreFloat3(&mRotationVec_child, XMVector3Normalize(rotVec_childLocal));
		return;
	}
	// if not, try (0, 0, 1)
	tryVec = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	rotVec_parent = XMVector3Cross(tryVec, anchDir_parent);
	rotVec_child = XMVector3Cross(tryVec, -anchDir_child);
	if (XMVectorGetX(XMVector3Length(rotVec_parent)) != 0 && XMVectorGetX(XMVector3Length(rotVec_child)) != 0)
	{
		XMVECTOR rotVec_parentLocal = XMVector3TransformNormal(rotVec_parent, toLocalParent);
		XMVECTOR rotVec_childLocal = XMVector3TransformNormal(rotVec_child, toLocalChild);
		XMStoreFloat3(&mRotationVec_parent, XMVector3Normalize(rotVec_parentLocal));
		XMStoreFloat3(&mRotationVec_child, XMVector3Normalize(rotVec_childLocal));
		return;
	}
}

UniversalJoint::~UniversalJoint()
{

}

float UniversalJoint::GetPosErrorSeverity()
{
	return mPosErrorSeverity + mOriErrorSeverity + mBendErrorSeverity;
}

float UniversalJoint::GetVelErrorSeverity()
{
	return mVelErrorSeverity + mRotErrorVelSeverity + mBendErrorVelSeverity;
}

void UniversalJoint::CalculatePositionErrorSeverity(float dt)
{
	// from anchor displacement
	XMMATRIX world[2];
	world[0] = XMLoadFloat4x4(&mParent->mTransformMatrix);
	world[1] = XMLoadFloat4x4(&mChild->mTransformMatrix);
	XMVECTOR anchPWC = XMVector3TransformCoord(XMLoadFloat3(&mAnchorPLC), world[0]);
	XMVECTOR anchCWC = XMVector3TransformCoord(XMLoadFloat3(&mAnchorCLC), world[1]);
	XMVECTOR errorVec = anchPWC - anchCWC;
	mPosErrorSeverity = XMVectorGetX(XMVector3Length(errorVec));

	// from bending constraint
	XMVECTOR pos_parent = XMLoadFloat3(&mParent->mPosition);
	XMVECTOR pos_child = XMLoadFloat3(&mChild->mPosition);
	XMVECTOR anchRelPos_parent = anchPWC - pos_parent;
	XMVECTOR anchRelPos_child = anchPWC - pos_child;
	XMVECTOR dir_parent = XMVector3Normalize(anchRelPos_parent);
	XMVECTOR dir_child = XMVector3Normalize(anchRelPos_child);
	mBendActualAngle = GetAngleSafeXMVECTOR(dir_parent, -dir_child);
	if (mBendActualAngle > mBendFreedom)
	{
		XMMATRIX inverseInertiaTensor_parent = XMLoadFloat3x3(&mParent->mInverseInertiaTensorWorld);
		XMMATRIX inverseInertiaTensor_child = XMLoadFloat3x3(&mChild->mInverseInertiaTensorWorld);
		XMVECTOR dir = XMVector3Normalize(XMVector3Cross(dir_parent, -dir_child));
		XMStoreFloat3(&mBendErrorDir, dir);
		float angInertia_parent = 0.0f;
		if (mParent->mIsMovable) angInertia_parent += XMVectorGetX(XMVector3Length(XMVector3Transform(dir, inverseInertiaTensor_parent)));
		float angInertia_child = 0.0f;
		if (mChild->mIsMovable)	angInertia_child += XMVectorGetX(XMVector3Length(XMVector3Transform(dir, inverseInertiaTensor_child)));
		float totalAngInertia = angInertia_parent + angInertia_child;
		float inverseAngInertia = 1.0f / totalAngInertia;
		mAmountOfBendingToApplyToParent = angInertia_parent * inverseAngInertia;
		mAmountOfBendingToApplyToChild = angInertia_child * inverseAngInertia;

		float bendErrorAngle = mBendActualAngle - mBendFreedom;
		mBendErrorSeverity = bendErrorAngle * (mAnchParentDist*mAmountOfBendingToApplyToParent +
			mAnchChildDist*mAmountOfBendingToApplyToChild);
	}
	else mBendErrorSeverity = 0.0f;

	// from orientational constraint
	XMVECTOR rotVec_parent = XMVector3TransformNormal(XMLoadFloat3(&mRotationVec_parent), world[0]);
	XMVECTOR rotVec_child = XMVector3TransformNormal(XMLoadFloat3(&mRotationVec_child), world[1]);

	XMVECTOR d = XMVector3Cross(rotVec_parent, dir_child);
	float dL = XMVectorGetX(XMVector3Length(d));
	if (dL == 0.0f) d = -dir_parent;
	else d = XMVector3Cross(dir_child, d);
	d = XMVector3Normalize(d);
	mOriActualAngle = GetAngleSafeXMVECTOR(d, rotVec_child);
	mOriErrorAngle = max(mOriActualAngle - mRotFreedom, 0.0f);
	mOriErrorSeverity = mOriErrorAngle*(mParentRadiusApprox*mAmountOfRotationToApplyToParent +
		mChildRadiusApprox*mAmountOfRotationToApplyToChild);
	XMVECTOR pp_dir_rotVec = XMVector3Cross(dir_child, rotVec_child);
	float pp_dir_rotVec_res = XMVectorGetX(XMVector3Dot(d, pp_dir_rotVec));
	if (pp_dir_rotVec_res < 0.0f) // fourth or third quadrant
	{
		mOriErrorAngle *= -1.0f;
		mOriActualAngle *= -1.0f;
	}
}

void UniversalJoint::CalculateVeclocityErrorSeverity(float dt)
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

	// Bend velocity
	mBendErrorVel = 0.0f;
	if (mBendActualAngle >= mBendFreedom)
	{
		XMVECTOR dir = XMLoadFloat3(&mBendErrorDir);
		float velProjParent = XMVectorGetX(XMVector3Dot(dir, bodyAngVelocity_parent));
		float velProjChild = XMVectorGetX(XMVector3Dot(dir, bodyAngVelocity_child));
		mBendErrorVel = velProjChild - velProjParent;
		mBendErrorVelSeverity = abs(mBendErrorVel) * (mAnchParentDist*mAmountOfBendingToApplyToParent +
			mAnchChildDist*mAmountOfBendingToApplyToChild);
	}

	// Now handle rotational velocity
	mRotErrorVel = 0.0f;
	if (abs(mOriActualAngle) >= mRotFreedom)
	{
		float velProjParent = XMVectorGetX(XMVector3Dot(dir_parent, bodyAngVelocity_parent));
		float velProjChild = XMVectorGetX(XMVector3Dot(dir_child, bodyAngVelocity_child));
		mRotErrorVel = velProjParent + velProjChild;
		mRotErrorVelSeverity = abs(mRotErrorVel) * (mParentRadiusApprox*mAmountOfRotationToApplyToParent +
			mChildRadiusApprox*mAmountOfRotationToApplyToChild);
	}
}

void UniversalJoint::ApplyPositionChange(float relaxation, float dt)
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

	if (mBendErrorSeverity)
	{
		float bendAngle = GetAngleSafeXMVECTOR(dir_parent, -dir_child);
		float bendErrorAngle = bendAngle - mBendFreedom;
		XMVECTOR rot = XMLoadFloat3(&mBendErrorDir);
		float angChange_parent = bendErrorAngle * mAmountOfBendingToApplyToParent;
		float angChange_child = -bendErrorAngle * mAmountOfBendingToApplyToChild;

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

	if (mOriErrorSeverity > 0.0f)
	{
		XMVECTOR rot_parent = XMVector3Normalize(relativeAnchPos_parent)*mAmountOfRotationToApplyToParent*mOriErrorAngle*relaxation;
		XMVECTOR rot_child = XMVector3Normalize(relativeAnchPos_child)*mAmountOfRotationToApplyToChild*mOriErrorAngle*relaxation;

		XMVECTOR angularChangeQ = XMVectorSet(XMVectorGetX(rot_parent), XMVectorGetY(rot_parent), XMVectorGetZ(rot_parent), 0);
		ori_parent += XMQuaternionMultiply(ori_parent, angularChangeQ*0.5f);

		angularChangeQ = XMVectorSet(XMVectorGetX(rot_child), XMVectorGetY(rot_child), XMVectorGetZ(rot_child), 0);
		ori_child += XMQuaternionMultiply(ori_child, angularChangeQ*0.5f);
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

void UniversalJoint::ApplyVelocityChange(float relaxation, float dt)
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

		// Change in rotation from bending constraint
		XMVECTOR bendErrorVelKill = XMLoadFloat3(&mBendErrorDir) * mBendErrorVel * mAmountOfBendingToApplyToParent * relaxation;
		rotationChange += bendErrorVelKill;

		// Change in rotation from rotational constraint
		XMVECTOR rotErrorCorrection = -dir_parent * mRotErrorVel * mAmountOfRotationToApplyToParent * relaxation;
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

		// Change in rotation from bending constraint
		XMVECTOR bendErrorVelKill = -XMLoadFloat3(&mBendErrorDir) * mBendErrorVel * mAmountOfBendingToApplyToChild * relaxation;
		rotationChange += bendErrorVelKill;

		// Change in rotation from rotational constraint
		XMVECTOR rotErrorCorrection = -dir_child * mRotErrorVel * mAmountOfRotationToApplyToChild * relaxation;
		rotationChange += rotErrorCorrection;

		// Apply and store
		XMVECTOR linVelocity = XMLoadFloat3(&mChild->mLinVelocity) + velocityChange;
		XMVECTOR angVelocity = XMLoadFloat3(&mChild->mAngVelocity) + rotationChange;
		XMStoreFloat3(&mChild->mLinVelocity, linVelocity);
		XMStoreFloat3(&mChild->mAngVelocity, angVelocity);
	}
}