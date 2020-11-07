
#include "Joint.h"
#include "RigidBody.h"

using namespace Physics_Makina;

Joint::Joint(RigidBody *parent, RigidBody *child, const XMFLOAT3 &anchor)
: mParent(parent),
mChild(child)
{
	/*************************************************************************************************
	Goal: Find out how much does a point traverse when the body is rotated (mParentRadiusApprox, mChildRadiusApprox).
	Here we calculate how much impulse is needed to achieve change in angular velocity of unit vector.
	Now let's consider the same problem for cylinder: L = I * w, where I is not an inertia tensor,
	but simply a moment of inertia along the central axis (I = 0.5 * M * R^2) and w = 1.
	Therefore: L = 0.5 * M * R^2  -->  R = sqrt(2 * L / M).
	This radius gives us a good approximation of the actual distance between average surface point and
	the rotating axis of the body.
	*************************************************************************************************/
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

	// Radius
	mParentRadiusApprox = sqrt(2.0f * angImpulse_parentF * mParent->mInverseMass);
	mChildRadiusApprox = sqrt(2.0f * angImpulse_childF * mChild->mInverseMass);

	XMMATRIX world[2];
	world[0] = XMLoadFloat4x4(&parent->mTransformMatrix);
	world[1] = XMLoadFloat4x4(&child->mTransformMatrix);
	XMMATRIX toLocalChild = XMMatrixInverse(&det, world[1]);
	XMMATRIX toLocalParent = XMMatrixInverse(&det, world[0]);
	XMVECTOR childAnchorRelPos = XMVector3TransformCoord(pos_anch, toLocalChild);
	XMVECTOR parentAnchorRelPos = XMVector3TransformCoord(pos_anch, toLocalParent);

	// finally, anchor in local coordinates
	XMStoreFloat3(&mAnchorCLC, childAnchorRelPos);
	XMStoreFloat3(&mAnchorPLC, parentAnchorRelPos);

	// Distance from anchor
	mAnchParentDist = XMVectorGetX(XMVector3Length(pos_anch - pos_parent));
	mAnchChildDist = XMVectorGetX(XMVector3Length(pos_anch - pos_child));
}

Joint::~Joint()
{

}

bool Joint::AtLeastOneAwake()
{
	return mParent->IsAwake() || mChild->IsAwake();
}