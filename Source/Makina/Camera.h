
#ifndef CAMERA_H
#define CAMERA_H
#define CAMERA_ZN 0.1f
#define CAMERA_ZF 1000.0f

#include <Windows.h>
#include <xnamath.h>
#include "XnaCollision.h"
#include "GameComponent.h"

namespace Makina
{
	class Camera : public GameComponent
	{
	public:
		__declspec(dllexport) Camera(D3DAppValues *values);
		__declspec(dllexport) ~Camera();

		// Dynamic allocation must be aligned to the 16, therefore custom 'new' and 'delete' operators are needed.
		__declspec(dllexport) void *operator new(size_t size);
		__declspec(dllexport) void operator delete(void *pt);

		__declspec(dllexport) void Update(float dt);
		__declspec(dllexport) void Draw(float dt);
		__declspec(dllexport) void OnResize();

		// Get / Set world camera position.
		XMVECTOR GetPositionVector() const { return XMLoadFloat3(&mPosition); }
		XMFLOAT3 GetPosition() const { return mPosition; }
		__declspec(dllexport) void SetPosition(float x, float y, float z);
		__declspec(dllexport) void SetPosition(const XMFLOAT3 &v);

		//Get camera basis vectors.
		XMVECTOR GetRightVector() const { return XMLoadFloat3(&mRight); }
		XMFLOAT3 GetRightFloat3() const { return mRight; }
		XMVECTOR GetUpVector() const { return XMLoadFloat3(&mUp); }
		XMFLOAT3 GetUpFloat3() const { return mUp; }
		XMVECTOR GetLookVector() const{ return XMLoadFloat3(&mLook); }
		XMFLOAT3 GetLookFloat3() const { return mLook; }

		// Get frustum properties.
		float GetNearZ() const { return mNearZ; }
		float GetFarZ() const { return mFarZ; }
		float GetAspect() const { return mD3DAppValues->GetAspectRatio(); }
		float GetFovY() const { return mFovY; }
		__declspec(dllexport) float GetFovX() const;
		Frustum GetFrustum() { return mWorldFrustum; }

		// Get near and far plane dimensions in view space coordinates.
		__declspec(dllexport) float GetNearWindowWidth() const;
		float GetNearWindowHeight() const { return mNearWindowHeight; }
		__declspec(dllexport) float GetFarWindowWidth() const;
		float GetFarWindowHeight() const { return mFarWindowHeight; }

		// Set frustum.
		__declspec(dllexport) void SetLens(float fovY, float aspect, float zn, float zf);

		// Define camera space via LookAt parameters.
		__declspec(dllexport) void LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp);
		__declspec(dllexport) void LookAt(const XMFLOAT3 &pos, const XMFLOAT3 &target, const XMFLOAT3 &worldUp);

		// Get View / Proj matrices.
		XMMATRIX View() const { return XMLoadFloat4x4(&mView); }
		XMMATRIX Proj() const { return XMLoadFloat4x4(&mProj); }
		XMMATRIX ViewProj() const { return XMLoadFloat4x4(&mView) * XMLoadFloat4x4(&mProj); }

		// Strafe/Walk the camera a distance d.
		__declspec(dllexport) void Strafe(float d);
		__declspec(dllexport) void Walk(float d);

		// Rotate the camera.
		__declspec(dllexport) void Pitch(float angle);
		__declspec(dllexport) void RotateY(float angle);

		// After modifying camera position/orientation, call to rebuild the view matrix once per frame.
		__declspec(dllexport) void UpdateViewMatrix();

	private:

		//Camera coordinate system with coordinates relative to world space.
		XMFLOAT3 mPosition; // view space origin
		XMFLOAT3 mRight;    // view space x-axis
		XMFLOAT3 mUp;       // view space y-axis
		XMFLOAT3 mLook;     // view space z-axis

		// Cache frustum properties.
		float mNearZ;
		float mFarZ;
		float mFovY;
		float mNearWindowHeight;
		float mFarWindowHeight;
		Frustum mCamFrustum;
		Frustum mWorldFrustum;

		// Cache View/Proj matrices.
		XMFLOAT4X4 mView;
		XMFLOAT4X4 mProj;
	};
}

#endif