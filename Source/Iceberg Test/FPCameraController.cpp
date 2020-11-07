#include "FPCameraController.h"
#include <InputHandler.h>
#include <Camera.h>
#include <MathHelper.h>

using namespace Makina;

FPCameraController::FPCameraController(InitD3DApp *appPt, std::vector<Object *> *objPt)
	: CameraController(appPt->GetD3DAppValues()),
	objPt(objPt),
	mSpeed(1.0f),
	mPickingIndex(-1)
{
	Makina::MouseState curr, prev;
	mInput->GetMouseStates(&curr, &prev);
	mInitialWheelValue = curr.wheelValue;
}

FPCameraController::~FPCameraController()
{

}

void FPCameraController::Update(float dt)
{
	// KEYBOARD
	float pressedTime;

	// forward
	if (mInput->IsPressed(0x57, &pressedTime))
	{
		mCam->Walk(mSpeed * dt);
	}

	// backward
	if (mInput->IsPressed(0x53, &pressedTime))
	{
		mCam->Walk(-mSpeed * dt);
	}

	// left
	if (mInput->IsPressed(0x41, &pressedTime))
	{
		mCam->Strafe(-mSpeed * dt);
	}

	// right
	if (mInput->IsPressed(0x44, &pressedTime))
	{
		mCam->Strafe(mSpeed * dt);
	}

	//MOUSE
	Makina::MouseState curr, prev;
	mInput->GetMouseStates(&curr, &prev);
	if (curr.IsRMBDown)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(curr.x - prev.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(curr.y - prev.y));

		mCam->Pitch(dy);
		mCam->RotateY(dx);
	}

	//wheel
	mSpeed = pow(1.5f, curr.wheelValue - mInitialWheelValue);
	if (mSpeed < 0) mSpeed = 0;

	XMVECTOR origin, dir;
	GetPickRayInWorldSpace(&origin, &dir);

	// picking
	if (curr.IsLMBDown == true && prev.IsLMBDown == false)
	{
		float minDist = MathHelper::Infinity;
		int j = -1;
		for(UINT i = 0; i < objPt->size(); i++)
		{
			float dist;
			if (objPt->at(i)->RayIntersects(origin, dir, &dist))
				if (dist < minDist)
				{
					minDist = dist;
					j = i;
				}
		}

		if (j >= 0)
			mPickingIndex = j;
	}
	else if (curr.IsLMBDown == false && prev.IsLMBDown == true)
	{
		float minDist = MathHelper::Infinity;
		int j = -1;
		for(UINT i = 0; i < objPt->size(); i++)
		{
			float dist;
			if (objPt->at(i)->RayIntersects(origin, dir, &dist))
				if (dist < minDist)
				{
					minDist = dist;
					j = i;
				}
		}

		//if (j >= 0 && mPickingIndex == j)
		//	objPt->at(j)->ToogleColor();

		mPickingIndex = -1;
	}
}

void FPCameraController::Draw(float dt)
{

}

void FPCameraController::OnResize()
{

}