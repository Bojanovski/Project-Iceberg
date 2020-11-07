#ifndef BASEFORM_H
#define BASEFORM_H

#include <Windows.h>
#include "D3DApp.h"

namespace Makina
{
	class BaseForm
	{	
	public:
		__declspec(dllexport) BaseForm(HINSTANCE hInstance, int width, int height, int nCmdShow, LPCWSTR menuName);
		__declspec(dllexport) virtual ~BaseForm() = 0;

		__declspec(dllexport) virtual LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		__declspec(dllexport) HWND GetHWND();

		void SetD3DApp(D3DApp *appPt) {mAppPt = appPt;}

	protected:
		HWND hwnd;
		HINSTANCE hInstance;
		D3DApp *mAppPt;

	};
}

#endif
