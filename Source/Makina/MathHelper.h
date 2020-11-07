
#ifndef MATHHELPER_H
#define MATHHELPER_H

#include <Windows.h>
#include <xnamath.h>
#include <float.h>

namespace Makina
{
	namespace MathHelper
	{
		const float Infinity = FLT_MAX;
		const float Pi       = 3.1415926535f;

		__declspec(dllexport) float AngleFromXY(float x, float y);
		__declspec(dllexport) XMMATRIX InverseTranspose(CXMMATRIX M);
		__declspec(dllexport) XMMATRIX SkewSymmetric(FXMVECTOR v);
		__declspec(dllexport) XMMATRIX ZeroMatrix();
		__declspec(dllexport) XMMATRIX& operator+= (XMMATRIX &left, CXMMATRIX right);
		__declspec(dllexport) XMVECTOR CreateQuaternionFromDir(FXMVECTOR forward);

		// Returns random float in [0, 1).
		static float RandF()
		{
			return (float)(rand()) / (float)RAND_MAX;
		}

		// Returns random float in [a, b).
		static float RandF(float a, float b)
		{
			return a + RandF()*(b-a);
		}

		// Returns sign.
		template <typename T> inline int sgn(T val)
		{
			return (T(0) < val) - (val < T(0));
		}

		// Safely returns angle between two normal vectors.
		inline static float GetAngleSafeXMVECTOR(FXMVECTOR v1, FXMVECTOR v2)
		{
			float dot = XMVectorGetX(XMVector3Dot(v1, v2));
			// This can happen due to floating point precision error.
			dot = min(dot, 1.0f);
			dot = max(dot, -1.0f);
			return acos(dot);
		}

		inline bool operator== (XMFLOAT2 &left, XMFLOAT2 &right)
		{
			if ((left.x == right.x) && (left.y == right.y))
				return true;
			else
				return false;
		}

		inline bool operator== (XMFLOAT3 &left, XMFLOAT3 &right)
		{
			if ((left.x == right.x) && (left.y == right.y) && (left.z == right.z))
				return true;
			else
				return false;
		}

		inline XMFLOAT3& operator+= (XMFLOAT3 &left, XMFLOAT3 &right)
		{
			left = XMFLOAT3(left.x + right.x, left.y + right.y, left.z + right.z);
			return left;
		}
	}
}

#endif
