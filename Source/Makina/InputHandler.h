
#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include "GameComponent.h"
#include "MouseState.h"
#include <map>

namespace Makina
{
	class InputHandler : public GameComponent
	{
		friend class D3DApp;

	public:
		__declspec(dllexport) InputHandler(D3DAppValues *values, HWND hwnd);
		__declspec(dllexport) ~InputHandler();

		__declspec(dllexport) void Update(float dt);
		__declspec(dllexport) void Draw(float dt);
		__declspec(dllexport) void OnResize();
		__declspec(dllexport) void GetMouseStates(MouseState *curr, MouseState *prev);
		__declspec(dllexport) void GetMouseCoords(int *sx, int *sy);
		__declspec(dllexport) bool IsPressed(UINT keyCode, float *pressedTime);

	private:
		MouseState mPrevMouseState, mCurrMouseState, mNextMouseState;
		HWND hwnd;

		// Virtual key codes http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx
		std::map<UINT, float> currKeyboardState;

		__declspec(dllexport) void Win32MsgProc(MSG *msg);
	};
}

#endif