#include "MathHelper.h"

using namespace Makina;

float MathHelper::AngleFromXY(float x, float y)
{
	float theta = 0.0f;

	// Quadrant I or IV
	if (x >= 0.0f)
	{
		// If x = 0, then atanf(y/x) = +pi/2 if y > 0
		//                atanf(y/x) = -pi/2 if y < 0
		theta = atanf(y / x); // in [-pi/2, +pi/2]

		if (theta < 0.0f)
			theta += 2.0f * XM_PI; // in [0, 2*pi).
	}

	// Quadrant II or III
	else
		theta = atanf(y / x) + XM_PI; // in [0, 2*pi).

	return theta;
}

XMMATRIX MathHelper::InverseTranspose(CXMMATRIX M)
{
	// Inverse-transpose is just applied to normals.  So zero out 
	// translation row so that it doesn't get into our inverse-transpose
	// calculation--we don't want the inverse-transpose of the translation.
	XMMATRIX A = M;
	A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	XMVECTOR det;
	return XMMatrixTranspose(XMMatrixInverse(&det, A));
}

XMMATRIX MathHelper::SkewSymmetric(FXMVECTOR v)
{
	XMMATRIX skewSymetric = XMMatrixSet(
		0.0f, -XMVectorGetZ(v), XMVectorGetY(v), 0.0f,
		XMVectorGetZ(v), 0.0f, -XMVectorGetX(v), 0.0f,
		-XMVectorGetY(v), XMVectorGetX(v), 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f);

	return skewSymetric;
}

XMMATRIX MathHelper::ZeroMatrix()
{
	XMMATRIX zero = XMMatrixSet(
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f);

	return zero;
}

XMMATRIX& MathHelper::operator += (XMMATRIX &left, CXMMATRIX right)
{
	left.r[0] = left.r[0] + right.r[0];
	left.r[1] = left.r[1] + right.r[1];
	left.r[2] = left.r[2] + right.r[2];
	left.r[3] = left.r[3] + right.r[3];

	return left;
}

XMVECTOR MathHelper::CreateQuaternionFromDir(FXMVECTOR forward)
{
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR right = XMVector3Cross(up, forward);
	if (XMVectorGetX(XMVector3LengthSq(right)) > 0.0f)
	{
		right = XMVector3Normalize(right);
		up = XMVector3Cross(forward, right);
	}
	else
	{
		right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		up = XMVector3Cross(forward, right);
		up = XMVector3Normalize(up);
		right = XMVector3Cross(up, forward);
	}
	XMFLOAT3 i, j, k;
	XMStoreFloat3(&i, right);
	XMStoreFloat3(&j, up);
	XMStoreFloat3(&k, forward);
	XMMATRIX W = XMMatrixSet(
		i.x, i.y, i.z, 0.0f,
		j.x, j.y, j.z, 0.0f,
		k.x, k.y, k.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR q = XMQuaternionRotationMatrix(W);
	return q;
}