
#ifndef INITD3DAPP
#define INITD3DAPP

#include <D3DApp.h>

class InitD3DApp : public Makina::D3DApp
{
public:
	InitD3DApp(HINSTANCE hInstance, int mWidth, int mHeight, HWND mhParentWnd = NULL);
	~InitD3DApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene(float dt);
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif
