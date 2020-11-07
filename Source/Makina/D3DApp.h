
#ifndef D3DAPP_H
#define D3DAPP_H
#define ID_D3D11_CONTROL 10000

#include "DirectX11Headers.h"
#include "GameTimer.h"
#include "GameComponent.h"
#include "D3DAppValues.h"
#include "ProcessAndSystemData.h"
#include <string>
#include <vector>

namespace Makina
{
	class D3DApp
	{
	public:
		__declspec(dllexport) D3DApp(HINSTANCE hInstance, int Width, int Height, HWND hParentWnd = NULL);
		__declspec(dllexport) virtual ~D3DApp();

		__declspec(dllexport) HINSTANCE AppInst()const;
		__declspec(dllexport) HWND      MainWnd()const;

		__declspec(dllexport) int Run();
		__declspec(dllexport) void SetMainWndCaption(const std::wstring &mainWndCaption);

		// Framework methods.  Derived client class overrides these methods to 
		// implement specific application requirements.

		__declspec(dllexport) virtual bool Init();
		__declspec(dllexport) virtual void OnResize(); 
		__declspec(dllexport) virtual void UpdateScene(float dt)=0;
		__declspec(dllexport) virtual void DrawScene(float dt)=0; 
		__declspec(dllexport) virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		// Game Components
		__declspec(dllexport) void AddGameComponent(GameComponent *gameComponentPt, int drawOrder);
		void AddGameComponent(GameComponent *gameComponentPt) {AddGameComponent(gameComponentPt, GetGameComponentsLength());}
		__declspec(dllexport) void RemoveGameComponent(GameComponent *gameComponentPt);
		GameComponent *GetGameComponents(int index) {return mGameComponents[index];}
		int GetGameComponentsLength() {return mGameComponents.size();}


		ID3D11Device *GetDevice(){ return mD3DAppValues.md3dDevice; }
		ID3D11DeviceContext *GetImmediateContext(){ return mD3DAppValues.md3dImmediateContext; }
		D3DAppValues *GetD3DAppValues() {return &mD3DAppValues;}
		UINT Get4xMsaaQuality() { return m4xMsaaQuality; }

	protected:
		__declspec(dllexport) bool InitMainWindow();
		__declspec(dllexport) bool InitDirect3D();
		__declspec(dllexport) bool InitFX();

		__declspec(dllexport) void UpdateProcessAndSystemData();

	protected:
		HINSTANCE mhAppInst;
		HWND      mhMainWnd;
		HWND      mhParentWnd;
		bool      mAppPaused;
		bool      mMinimized;
		bool      mMaximized;
		bool      mResizing;
		UINT      m4xMsaaQuality;	
		std::wstring mMainWndCaption;

		// Nice bitmap for resizing
		HBITMAP mResizingBmp;

		// Tick tock :)
		GameTimer mTimer;

		ProcessAndSystemData mProcessAndSystemData;

		// Game components
		std::vector<GameComponent *> mGameComponents;

		// All graphics and system related values.
		D3DAppValues mD3DAppValues;
	};
}

#endif
