#include "InputHandler.h"
#include <WindowsX.h>

using namespace Makina;
using namespace std;

InputHandler::InputHandler(D3DAppValues *values, HWND hwnd) : GameComponent(values)
{
	this->hwnd = hwnd;
}

InputHandler::~InputHandler()
{

}

void InputHandler::Update(float dt)
{
	// Keyboard
	map<UINT, float>::iterator ite = currKeyboardState.begin();

	while (ite != currKeyboardState.end())
	{
		ite->second += dt;
		ite++;
	}

	// Mouse
	mPrevMouseState = mCurrMouseState;
	mCurrMouseState = mNextMouseState;
}

void InputHandler::Draw(float dt)
{

}

void InputHandler::OnResize()
{

}

void InputHandler::GetMouseStates(MouseState *curr, MouseState *prev)
{
	*curr = mCurrMouseState;
	*prev = mPrevMouseState;
}

void InputHandler::GetMouseCoords(int *sx, int *sy)
{
	*sx = mCurrMouseState.x;
	*sy = mCurrMouseState.y;
}

bool InputHandler::IsPressed(UINT keyCode, float *pressedTime)
{
	map<UINT, float>::iterator ite = currKeyboardState.find(keyCode);
	*pressedTime = 0;

	if (ite == currKeyboardState.end()) // There is no such key in this map.
		return false;
	else
	{
		*pressedTime = ite->second;
		return true;
	}
}

void InputHandler::Win32MsgProc(MSG *msg)
{
	switch (msg->message)
	{
	case WM_KEYDOWN:
		if (currKeyboardState.find(msg->wParam) == currKeyboardState.end()) // There is no such key in this map.
			currKeyboardState.insert(pair<UINT, float>(msg->wParam, 0.0f));
		return;
	case WM_KEYUP:
		currKeyboardState.erase(msg->wParam);
		return;
	case WM_MOUSEWHEEL:
		{
			mNextMouseState.wheelValue += GET_WHEEL_DELTA_WPARAM(msg->wParam) / WHEEL_DELTA;
		}
		return;
	}

	if (msg->hwnd == hwnd)
	{
		switch (msg->message)
		{
		case WM_LBUTTONDOWN:
			mNextMouseState.IsLMBDown = true;
			return;
		case WM_MBUTTONDOWN:
			mNextMouseState.IsMMBDown = true;
			return;
		case WM_RBUTTONDOWN:
			mNextMouseState.IsRMBDown = true;
			return;
		case WM_LBUTTONUP:
			mNextMouseState.IsLMBDown = false;
			return;
		case WM_MBUTTONUP:
			mNextMouseState.IsMMBDown = false;
			return;
		case WM_RBUTTONUP:		
			mNextMouseState.IsRMBDown = false;
			return;
		case WM_MOUSEMOVE:
			{
				mNextMouseState.x = GET_X_LPARAM(msg->lParam);
				mNextMouseState.y = GET_Y_LPARAM(msg->lParam);
			}
		default:
			{
				//Check the mouse left button is pressed or not
				if ((GetKeyState(VK_LBUTTON) & 0x80) != 0)
					mNextMouseState.IsLMBDown = true;
				else
					mNextMouseState.IsLMBDown = false;

				//Check the mouse right button is pressed or not
				if ((GetKeyState(VK_RBUTTON) & 0x80) != 0)
					mNextMouseState.IsRMBDown = true;
				else
					mNextMouseState.IsRMBDown = false;

				//Check the mouse middle button is pressed or not
				if ((GetKeyState(VK_MBUTTON) & 0x80) != 0)
					mNextMouseState.IsMMBDown = true;
				else
					mNextMouseState.IsMMBDown = false;
			}
			return;
		}
	}
}