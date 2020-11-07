#include "InitD3DApp.h"
#include <D3DUtilities.h>
#include <cassert>	
#include <sstream>
#include "Resource.h"

using namespace Makina;
using namespace std;

InitD3DApp::InitD3DApp(HINSTANCE hInstance, int mWidth, int mHeight, HWND mhParentWnd)
	: D3DApp(hInstance, mWidth, mHeight, mhParentWnd)
{
	mD3DAppValues.mEnable4xMsaa = true;
}

InitD3DApp::~InitD3DApp()
{
	//delete sparks;
}

//int SparksChangePos(lua_State *luaState)
//{
//	// get number of arguments
//	int n = Lua::GetTop(luaState);
//	int i;
//
//	float coord[3];
//
//	// loop through each argument
//	for (i = 1; i <= n; ++i)
//	{
//		coord[i - 1] = (float)Lua::ToNumber(luaState, i);
//	}
//
//	sparks->SetEmitPos(XMFLOAT3(coord[0], coord[1], coord[2]));
//
//	// return the number of results
//	return 0;
//}

bool InitD3DApp::Init()
{
	if (!D3DApp::Init())
		return false;

	//sparks = new Sparks(GetD3DAppValues());
	//sparks->SetEmitDir(XMFLOAT3(0.0f, 0.0f, -2.0f));
	//sparks->SetEmitPos(XMFLOAT3(2.0f, 0.5f, 0.0f));
	//sparks->SetWind(XMFLOAT3(5.0f, 0.0f, 0.0f));

	return true;
}

void InitD3DApp::OnResize()
{
	D3DApp::OnResize();
}

void InitD3DApp::UpdateScene(float dt)
{
	//sparks->SetEmitPos(XMFLOAT3(sparks->GetEmitPos().x + dt, 8.0f, 0.0f));

	for (UINT i = 0; i < mGameComponents.size(); i++)
		if (mGameComponents[i]->Enabled)
			mGameComponents[i]->Update(dt);
}

void InitD3DApp::DrawScene(float dt)
{
	for (UINT i = 0; i < mGameComponents.size(); i++)
	if (mGameComponents[i]->Visible)
		mGameComponents[i]->Draw(dt, GAME_COMPONENT_DRAW_TYPE_NORMAL | GAME_COMPONENT_DRAW_TYPE_EFFECT);


	HRESULT hr = mD3DAppValues.mSwapChain->Present(0, 0);
	//mSwapChain->SetFullscreenState(true, 0);
	if(FAILED(hr))
		throw UnexpectedError(L"Swap chain error!");
}

LRESULT InitD3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return D3DApp::MsgProc(hwnd, msg, wParam, lParam);
}
