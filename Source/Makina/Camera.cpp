#include "Camera.h"

using namespace Makina;

Camera::Camera(D3DAppValues *values) 
	: GameComponent(values)
{

}

Camera::~Camera()
{

}

void *Camera::operator new(size_t size)
{
	void *storage = _aligned_malloc(size, 16);
	if (NULL == storage)
	{
		throw AllocationError(L"No free memory. (Camera.cpp)");
	}
	return storage;
}

void Camera::operator delete(void *pt)
{
	_aligned_free(pt);
}

void Camera::Update(float dt)
{

}

void Camera::Draw(float dt)
{

}

void Camera::OnResize()
{
	SetLens(mFovY, mD3DAppValues->GetAspectRatio(), mNearZ, mFarZ);
	UpdateViewMatrix();
}

float Camera::GetFovX() const
{
	float halfWidth = 0.5f * GetNearWindowWidth();
	return 2.0f * atan(halfWidth / mNearZ);
}

float Camera::GetNearWindowWidth() const
{
	return mD3DAppValues->GetAspectRatio() * mNearWindowHeight;
}

float Camera::GetFarWindowWidth() const
{
	return mD3DAppValues->GetAspectRatio() * mFarWindowHeight;
}

void Camera::SetLens(float fovY, float aspect, float zn, float zf)
{
	// cache properties
	mFovY = fovY;
	mNearZ = zn;
	mFarZ = zf;

	mNearWindowHeight = 2.0f * mNearZ * tanf(0.5f * mFovY);
	mFarWindowHeight = 2.0f * mFarZ * tanf(0.5f * mFovY);

	XMMATRIX P = XMMatrixPerspectiveFovLH(mFovY, aspect, mNearZ, mFarZ);
	XMStoreFloat4x4(&mProj, P);
	
	// Frustum culling
	ComputeFrustumFromProjection(&mCamFrustum, &Proj());
}

void Camera::SetPosition(float x, float y, float z)
{
	mPosition = XMFLOAT3(x, y, z);
}

void Camera::SetPosition(const XMFLOAT3& v)
{
	mPosition = v;
}

void Camera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
{
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	XMVECTOR U = XMVector3Cross(L, R);

	XMStoreFloat3(&mPosition, pos);
	XMStoreFloat3(&mLook, L);
	XMStoreFloat3(&mRight, R);
	XMStoreFloat3(&mUp, U);
	
	// Update
	UpdateViewMatrix();
}

void Camera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&up);

	LookAt(P, T, U);
}

void Camera::Strafe(float d)
{
	// mPosition += d * mRight;
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat3(&mRight);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, r, p));

	// Update
	UpdateViewMatrix();
}

void Camera::Walk(float d)
{
	// mPosition += d * mRight;
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR l = XMLoadFloat3(&mLook);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, l, p));
	
	// Update
	UpdateViewMatrix();
}

void Camera::Pitch(float angle)
{
	// Rotate up and look vector about the right vector.
	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mRight), angle);

	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));
	
	// Update
	UpdateViewMatrix();
}

void Camera::RotateY(float angle)
{
	// Rotate the basis vectors about the world y-axis.
	XMMATRIX R = XMMatrixRotationY(angle);

	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));
	
	// Update
	UpdateViewMatrix();
}
#include "InputHandler.h"
void Camera::UpdateViewMatrix()
{
	XMVECTOR r = XMLoadFloat3(&mRight);
	XMVECTOR u = XMLoadFloat3(&mUp);
	XMVECTOR l = XMLoadFloat3(&mLook);
	XMVECTOR p = XMLoadFloat3(&mPosition);

	// Orthonormalize the right, up and look vectors.
	l = XMVector3Normalize(l);
	u = XMVector3Normalize(XMVector3Cross(l, r));
	r = XMVector3Cross(u, l);

	// Fill in the view matrix entries.
	float x = -XMVectorGetX(XMVector3Dot(p, r));
	float y = -XMVectorGetX(XMVector3Dot(p, u));
	float z = -XMVectorGetX(XMVector3Dot(p, l));

	XMStoreFloat3(&mRight, r);
	XMStoreFloat3(&mUp, u);
	XMStoreFloat3(&mLook, l);

	mView(0,0) = mRight.x;
	mView(1,0) = mRight.y;
	mView(2,0) = mRight.z;
	mView(3,0) = x;

	mView(0,1) = mUp.x;
	mView(1,1) = mUp.y;
	mView(2,1) = mUp.z;
	mView(3,1) = y;

	mView(0,2) = mLook.x;
	mView(1,2) = mLook.y;
	mView(2,2) = mLook.z;
	mView(3,2) = z;

	mView(0,3) = 0.0f;
	mView(1,3) = 0.0f;
	mView(2,3) = 0.0f;
	mView(3,3) = 1.0f;

	// Frustum culling
	XMVECTOR detView;
	XMMATRIX invView = XMMatrixInverse(&detView, View());
	XMVECTOR scale, rotQuat, translation;
	XMMatrixDecompose(&scale, &rotQuat, &translation, invView);

	float sfa;
	if (!mD3DAppValues->mInputHandler->IsPressed(0x43, &sfa))
		TransformFrustum(&mWorldFrustum, &mCamFrustum, XMVectorGetX(scale), rotQuat, translation);
}