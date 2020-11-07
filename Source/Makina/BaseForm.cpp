#include "BaseForm.h"

using namespace Makina;

namespace MainFormHelperSpace
{
	BaseForm *mfPt;

	LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return mfPt->MainWndProc(hwnd, msg, wParam, lParam);
	}
}

BaseForm::BaseForm(HINSTANCE hInstance, int width, int height, int nCmdShow, LPCWSTR menuName)
{
	MainFormHelperSpace::mfPt = this;

	this->hInstance = hInstance;

	WNDCLASSEX wcMain;
	wcMain.cbSize = sizeof(WNDCLASSEX);
	wcMain.style = 0;
	wcMain.lpfnWndProc = MainFormHelperSpace::MainWndProc;
	wcMain.cbClsExtra = 0;
	wcMain.cbWndExtra = 0;
	wcMain.hInstance = hInstance;
	wcMain.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	wcMain.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcMain.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcMain.lpszMenuName = menuName;
	wcMain.lpszClassName = L"MainWnd";
	wcMain.hIconSm = LoadIcon (NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&wcMain))
	{
		MessageBox(NULL, L"Window Registration Failed!", L"Error!",
			MB_ICONEXCLAMATION | MB_OK);
		exit(0);
	}

	hwnd = CreateWindowEx(WS_EX_WINDOWEDGE, L"MainWnd", L"The title of my window", WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);

	if(hwnd == NULL)
	{
		MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		exit(0);
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
}

BaseForm::~BaseForm()
{

}

LRESULT CALLBACK BaseForm::MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		// Forward to child d3dApp window
	case WM_ENTERSIZEMOVE:
	case WM_EXITSIZEMOVE:
		SendMessage(mAppPt->MainWnd(), msg, wParam, lParam);
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

HWND BaseForm::GetHWND(){return hwnd;}