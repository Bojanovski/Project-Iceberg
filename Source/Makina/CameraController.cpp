#include "CameraController.h"

using namespace Makina;

CameraController::CameraController(D3DAppValues *values)
	: GameComponent(values)
{
	mCam = (Camera *)mD3DAppValues->mCamera;
	mInput = (InputHandler *)mD3DAppValues->mInputHandler;
}

CameraController::~CameraController()
{

}

void CameraController::GetPickRayInWorldSpace(XMVECTOR *origin, XMVECTOR *direction)
{
	XMMATRIX P = mCam->Proj();

	int sx, sy;
	mInput->GetMouseCoords(&sx, &sy);

	// Compute picking ray in view space.
	float vx = (+2.0f*sx/mD3DAppValues->mClientWidth  - 1.0f)/P(0,0);
	float vy = (-2.0f*sy/mD3DAppValues->mClientHeight + 1.0f)/P(1,1);

	// Ray definition in view space.
	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR rayDir    = XMVectorSet(vx, vy, 1.0f, 0.0f);

	// Tranform ray to world space
	XMMATRIX V = mCam->View();
	XMVECTOR detView;
	XMMATRIX invView = XMMatrixInverse(&detView, V);

	*origin = XMVector3TransformCoord(rayOrigin, invView);
	*direction = XMVector3TransformNormal(rayDir, invView);
}