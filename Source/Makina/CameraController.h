
#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <Windows.h>
#include <xnamath.h>
#include "GameComponent.h"
#include "InputHandler.h"
#include "Camera.h"

namespace Makina
{
	class CameraController : public GameComponent
	{
	public:
		__declspec(dllexport) CameraController(D3DAppValues *values);
		__declspec(dllexport) ~CameraController();

		virtual void Update(float dt) = 0;
		virtual void Draw(float dt) = 0;
		virtual void OnResize() = 0;

		__declspec(dllexport) void GetPickRayInWorldSpace(XMVECTOR *origin, XMVECTOR *direction);

	protected:
		InputHandler *mInput;
		Camera *mCam;
	};
}

#endif
