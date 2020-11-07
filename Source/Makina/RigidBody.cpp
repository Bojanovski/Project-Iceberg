
#include "RigidBody.h"
#include "CollisionSkin.h"
#include "CollisionData.h"
#include "Exceptions.h"

#define MOTION_BASE_BIAS 0.5f

using namespace std;
using namespace Makina;
using namespace Physics_Makina;

float RigidBody::mSleepEpsilon = 0.2f;

RigidBody::RigidBody(bool isMovable, bool canSleep, bool isAwake,
	float restitutionCoefficient, float frictionCoefficient, float mass, XMFLOAT3 &cm, XMFLOAT3X3 &inertia,
	XMFLOAT3 &position, XMFLOAT4 &orientation, XMFLOAT3 &linVelocity)
: mIsMovable(isMovable),
mCanSleep(canSleep),
mIsAwake(isAwake),
mMotion(2.0f*mSleepEpsilon),
mRestitutionCoefficient(restitutionCoefficient),
mFrictionCoefficient(frictionCoefficient),
mInverseMass(1.0f / mass),
mLastUpdateAcceleration(XMFLOAT3(0.0f, 0.0f, 0.0f)),
mCm(cm),
mPosition(position),
mOrientation(orientation),
mLinVelocity(linVelocity),
mAngVelocity(XMFLOAT3(0.0f, 0.0f, 0.0f)),
mReg(false),
m_allocIndex(0)
{
	if (!mIsMovable)
	{ // Force this despite constructor caller.
		mIsAwake = false;
		mCanSleep = true;
	}

	XMVECTOR det;
	XMMATRIX M = XMMatrixInverse(&det, XMLoadFloat3x3(&inertia));
	XMStoreFloat3x3(&mInverseInertiaTensorBody, M);

	UpdateTransformationMatrix();
	mTransformMatrix_Rend = mTransformMatrix;
}

RigidBody::RigidBody(const RigidBody &obj)
: mIsMovable(obj.mIsMovable),
mCanSleep(obj.mCanSleep),
mIsAwake(obj.mIsAwake),
mMotion(obj.mMotion),
mRestitutionCoefficient(obj.mRestitutionCoefficient),
mFrictionCoefficient(obj.mFrictionCoefficient),
mInverseMass(obj.mInverseMass),
mLastUpdateAcceleration(obj.mLastUpdateAcceleration),
mCm(obj.mCm),
mPosition(obj.mPosition),
mOrientation(obj.mOrientation),
mLinVelocity(obj.mLinVelocity),
mAngVelocity(obj.mAngVelocity),
mReg(obj.mReg),
m_allocIndex(obj.m_allocIndex),
mTransformMatrix(obj.mTransformMatrix),
mTransformMatrix_Rend(obj.mTransformMatrix_Rend),
mInverseInertiaTensorBody(obj.mInverseInertiaTensorBody),
mInverseInertiaTensorWorld(obj.mInverseInertiaTensorWorld),
mForceAcc(obj.mForceAcc),
mTorqueAcc(obj.mTorqueAcc)
{
	for (UINT i = 0; i < COLLISION_SKINS_PER_RIGID_BODY; ++i)
		mSkinInstances[i] = obj.mSkinInstances[i];
}

RigidBody::~RigidBody()
{
	lock_guard<mutex> lk(mLock);
}

void RigidBody::ForceUpdateOfTransformationMatrix() {
	lock_guard<mutex> lk(mLock); 
	UpdateTransformationMatrix(); 
	mTransformMatrix_Rend = mTransformMatrix;
}

void RigidBody::AddCollisionSkin(CollisionSkin *collSkin)
{
	if (COLLISION_SKINS_PER_RIGID_BODY <= (m_allocIndex + 1)) throw InvalidOperation(L"Cannot add more collision skins to this rigid body! (RigidBody::AddCollisionSkin)");

	CollisionSkin_RigidBody_Instance instance;
	instance.mCollisionSkin = collSkin;
	instance.mRigidBody = this;

	if (!mReg)
		mSkinInstances[m_allocIndex++] = instance;
	else
		throw Makina::InvalidOperation(L"Unable to add a skin to a already registered body! (RigidBody::AddCollisionSkin)");
}

void RigidBody::Integrate(float dt)
{
	if (!mIsAwake || !mIsMovable) return;

	// Calculate linear acceleration from force inputs.
	XMVECTOR linearAcc = XMLoadFloat3(&mForceAcc) * mInverseMass;
	XMStoreFloat3(&mLastUpdateAcceleration, linearAcc);

	// Calculate angular acceleration from torque inputs.gg
	XMVECTOR angularAcc = XMVector3Transform(XMLoadFloat3(&mTorqueAcc), XMLoadFloat3x3(&mInverseInertiaTensorWorld));

	// Adjust velocities
	// Update linear velocity from both acceleration and impulse.
	XMVECTOR linearVel = XMLoadFloat3(&mLinVelocity);
	linearVel += linearAcc * dt;
	// Update angular velocity from both acceleration and impulse.
	XMVECTOR angularVel = XMLoadFloat3(&mAngVelocity);
	angularVel += angularAcc * dt;

	// Impose drag.
	linearVel *= pow(0.5f, dt);
	angularVel *= pow(0.5f, dt);

	// Adjust positions
	// Update linear position.
	XMVECTOR linearPos = XMLoadFloat3(&mPosition);
	linearPos += linearVel * dt;
	// Update angular position.
	XMVECTOR orientation = XMLoadFloat4(&mOrientation);
	XMVECTOR angularVelQ = XMVectorSet(XMVectorGetX(angularVel), XMVectorGetY(angularVel), XMVectorGetZ(angularVel), 0);
	orientation += XMQuaternionMultiply(orientation, angularVelQ * dt * 0.5f);

	//orientation = XMQuaternionMultiply(orientation, XMQuaternionRotationNormal(XMVector3Normalize(angularVel), XMVectorGetX(XMVector3Length(angularVel)) * dt));
	//orientation = XMQuaternionNormalize(orientation);

	orientation = XMQuaternionNormalize(orientation);

	// Store everything
	XMStoreFloat3(&mLinVelocity, linearVel);
	XMStoreFloat3(&mAngVelocity, angularVel);
	XMStoreFloat3(&mPosition, linearPos);
	XMStoreFloat4(&mOrientation, orientation);

	// Calculate motion for putting body to sleep
	float currentMotion = XMVectorGetX(XMVector3Dot(linearVel, linearVel) + XMVector3Dot(angularVel, angularVel));
	float bias = pow(MOTION_BASE_BIAS, dt);
	mMotion = bias*mMotion + (1.0f - bias)*currentMotion;
	if (mMotion > 10 * mSleepEpsilon) mMotion = 10 * mSleepEpsilon;
}

void RigidBody::CheckForSleep()
{
	if (mMotion < mSleepEpsilon)
		SetAwake(false);
}

void RigidBody::UpdateTransformationMatrix()
{
	// position is where center of mass is
	XMVECTOR pos = XMLoadFloat3(&mPosition);
	XMVECTOR ori = XMLoadFloat4(&mOrientation);

	XMMATRIX M = XMMatrixTranslation(-mCm.x, -mCm.y, -mCm.z) *
		XMMatrixAffineTransformation(
		XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
		ori,
		pos);
	XMStoreFloat4x4(&mTransformMatrix, M);

	XMMATRIX R = XMMatrixRotationQuaternion(ori);
	XMMATRIX iR = XMMatrixTranspose(R);
	XMMATRIX iitb = XMLoadFloat3x3(&mInverseInertiaTensorBody);
	XMMATRIX W = iR * iitb * R;

	XMStoreFloat3x3(&mInverseInertiaTensorWorld, W);
}

XMFLOAT3& operator+= (XMFLOAT3 &left, const XMFLOAT3 &right)
{
	left = XMFLOAT3(left.x + right.x, left.y + right.y, left.z + right.z);
	return left;
}

void RigidBody::ClearAccumulators()
{
	mForceAcc = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mTorqueAcc = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

void RigidBody::AddForce(const XMFLOAT3 &force)
{
	mForceAcc += force;
}

void RigidBody::AddTorque(const XMFLOAT3 &torque)
{
	mTorqueAcc += torque;
}

void RigidBody::AddForceAtBodyPoint(const XMFLOAT3 &force, const XMFLOAT3 &point)
{
	// Convert to coordinates relative to the center of mass.
	XMVECTOR pt = XMLoadFloat3(&point) + XMLoadFloat3(&mCm);
	pt = XMVector3Transform(pt, XMLoadFloat4x4(&mTransformMatrix));
	XMFLOAT3 ptF;
	XMStoreFloat3(&ptF, pt);
	AddForceAtPoint(force, ptF);
}

void RigidBody::AddForceAtPoint(const XMFLOAT3 &force, const XMFLOAT3 &point)
{
	// Convert to coordinates relative to the center of mass.
	XMVECTOR ptRelativeToBody = XMLoadFloat3(&point) - XMLoadFloat3(&mPosition);

	XMVECTOR vecForce = XMLoadFloat3(&force);

	// Torque
	XMVECTOR t = XMVector3Cross(ptRelativeToBody, vecForce);

	if (XMVectorGetX(XMVectorIsInfinite(t)))
		t = XMVectorZero();

	// Force
	XMVECTOR f;
	if (XMVectorGetX(XMVector3LengthSq(ptRelativeToBody))) // ptRelativeToBody has length
	{
		XMVECTOR posDir = XMVector3Normalize(ptRelativeToBody);
		f = posDir * XMVectorGetX(XMVector3Dot(posDir, vecForce));
	}
	else // ptRelativeToBody is zero
	{
		f = vecForce;
	}

	XMFLOAT3 fR, tR;
	XMStoreFloat3(&tR, t);
	XMStoreFloat3(&fR, vecForce);

	AddTorque(tR);
	AddForce(fR);
}

void RigidBody::SetAwake(const bool awake)
{
	lock_guard<mutex> lk(mLock);

	if (awake && mIsMovable || !mCanSleep)
	{
		mIsAwake = true;
		// Add a bit of motion to avoid it falling asleep immediately.
		mMotion = mSleepEpsilon*1.1f;
		// Reset any build up velocities from collision reslover (asleep-awake case contact velocity resolving).
		mLinVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
		mAngVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
	else
	{
		mIsAwake = false;
		mLinVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
		mAngVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
}