#include "d3dApp.h"
#include <WindowsX.h>
#include <psapi.h>
#include <TCHAR.h>
#include <pdh.h>
#include <sstream>
#include <cassert>
#include <string>
#include "Resource.h"
#include "d3dx11Effect.h"
#include "BasicEffect.h"
#include "Camera.h"
#include "GameStatesManager.h"
#include "InputHandler.h"
#include "RenderStatesManager.h"

#pragma comment(lib, "pdh.lib")

using namespace Makina;
using namespace std;

namespace Makina
{
	static PDH_HQUERY cpuQuery;
	static PDH_HCOUNTER cpuTotal;
	static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
	static int numProcessors;
	static HANDLE self;

	// This is just used to forward Windows messages from a global window
	// procedure to our member function window procedure because we cannot
	// assign a member function to WNDCLASS::lpfnWndProc.
	D3DApp* gd3dApp = 0;

	LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// Forward hwnd on because we can get messages (e.g., WM_CREATE)
		// before CreateWindow returns, and thus before mhMainWnd is valid.
		return Makina::gd3dApp->MsgProc(hwnd, msg, wParam, lParam);
	}

	D3DApp::D3DApp(HINSTANCE hInstance, int Width, int Height, HWND hParentWnd)
		:	mhAppInst(hInstance),
		mMainWndCaption(L"Window title"),
		mhMainWnd(0),
		mhParentWnd(hParentWnd),
		mAppPaused(false),
		mMinimized(false),
		mMaximized(false),
		mResizing(false),
		m4xMsaaQuality(0)
	{
		// Process and system data
		PdhOpenQuery(NULL, NULL, &cpuQuery);
		PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
		PdhCollectQueryData(cpuQuery);
		SYSTEM_INFO sysInfo;
		FILETIME ftime, fsys, fuser;
		GetSystemInfo(&sysInfo);
		numProcessors = sysInfo.dwNumberOfProcessors;
		GetSystemTimeAsFileTime(&ftime);
		memcpy(&lastCPU, &ftime, sizeof(FILETIME));
		self = GetCurrentProcess();
		GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
		memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
		memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);
		mProcessAndSystemData.mTotalVirtualMem = memInfo.ullTotalPageFile;
		mProcessAndSystemData.mVirtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		mProcessAndSystemData.mVirtualMemUsedByMe = pmc.PrivateUsage;
		mProcessAndSystemData.mTotalPhysMem = memInfo.ullTotalPhys;
		mProcessAndSystemData.mPhysMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
		mProcessAndSystemData.mPhysMemUsedByMe = pmc.WorkingSetSize;
		mD3DAppValues.mProcessAndSystemData = &mProcessAndSystemData;

		// Add values to D3DAppValues struct
		mD3DAppValues.md3dDevice = 0;
		mD3DAppValues.md3dImmediateContext = 0;
		mD3DAppValues.mSwapChain = 0;	
		mD3DAppValues.mDepthStencilBuffer = 0;
		mD3DAppValues.mRenderTargetView = 0;
		mD3DAppValues.mDepthStencilView = 0;

		mD3DAppValues.md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
		ZeroMemory(&mD3DAppValues.mScreenViewport, sizeof(D3D11_VIEWPORT));
		mD3DAppValues.mClientHeight = Height;
		mD3DAppValues.mClientWidth = Width;
		mD3DAppValues.mEnable4xMsaa = false;
		mD3DAppValues.mCamera = 0;
		mD3DAppValues.mRenderStatesManager = 0;

		// Effects
		mD3DAppValues.m2DGraphicsFX = 0;
		mD3DAppValues.mParticlesFX = 0;
		mD3DAppValues.mBasicEffect = 0;

		// Load bitmap
		HMODULE hMod = LoadLibraryEx(L"Makina.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
		mResizingBmp = NULL;
		mResizingBmp =  LoadBitmap(hMod, MAKEINTRESOURCE(ID_RESIZE));

		if(mResizingBmp == NULL)
		{
			MessageBox(NULL, L"Could not load Resizing Bitmap!", L"Error", MB_OK | MB_ICONEXCLAMATION);
			exit(0);
		}

		// Get a pointer to the application object so we can forward 
		// Windows messages to the object's window procedure through
		// the global window procedure.
		Makina::gd3dApp = this;
	}

	D3DApp::~D3DApp()
	{
		ReleaseCOM(mD3DAppValues.mRenderTargetView);
		ReleaseCOM(mD3DAppValues.mDepthStencilView);
		ReleaseCOM(mD3DAppValues.mSwapChain);
		ReleaseCOM(mD3DAppValues.mDepthStencilBuffer);

		// Restore all default settings.
		if( mD3DAppValues.md3dImmediateContext )
			mD3DAppValues.md3dImmediateContext->ClearState();


		ReleaseCOM(mD3DAppValues.md3dImmediateContext);
		ReleaseCOM(mD3DAppValues.md3dDevice);

		// Effects
		ReleaseCOM(mD3DAppValues.m2DGraphicsFX);
		ReleaseCOM(mD3DAppValues.mParticlesFX);
		if (mD3DAppValues.mBasicEffect) delete mD3DAppValues.mBasicEffect;

		// Game components
		for (unsigned int i = 0; i < mGameComponents.size(); i++)
			delete mGameComponents[i];
	}

	HINSTANCE D3DApp::AppInst()const
	{
		return mhAppInst;
	}

	HWND D3DApp::MainWnd()const
	{
		return mhMainWnd;
	}

	int D3DApp::Run()
	{
		MSG msg = {0};
		mTimer.Reset();

		while(msg.message != WM_QUIT)
		{
			// If there are Window messages then process them.
			if(PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
				((InputHandler *)mD3DAppValues.mInputHandler)->Win32MsgProc(&msg);
			}
			// Otherwise, do animation/game stuff.
			else
			{	
				mTimer.Tick();

				if( !mAppPaused )
				{
					UpdateProcessAndSystemData();
					UpdateScene(mTimer.DeltaTime());	
					DrawScene(mTimer.DeltaTime());
				}
				else
				{
					Sleep(100);
				}
			}
		}

		return (int)msg.wParam;
	}

	void D3DApp::SetMainWndCaption(const wstring &mainWndCaption)
	{
		mMainWndCaption = mainWndCaption;

		if (mhParentWnd)
			SetWindowText(mhParentWnd, &mMainWndCaption[0]);
		else
			SetWindowText(mhMainWnd, &mMainWndCaption[0]);
	}

	bool D3DApp::Init()
	{
		//try
		//{
			if(!InitMainWindow())
				return false;

			if(!InitDirect3D())
				return false;

			// Time
			mD3DAppValues.mGameTimer = &mTimer;

			// Crucial game components.
			mD3DAppValues.mInputHandler = new InputHandler(GetD3DAppValues(), mhMainWnd);
			AddGameComponent(((InputHandler *)mD3DAppValues.mInputHandler)); // input handler goes first

			mD3DAppValues.mCamera = new Camera(GetD3DAppValues());
			((Camera *)mD3DAppValues.mCamera)->SetLens(0.25f * XM_PI, mD3DAppValues.GetAspectRatio(), CAMERA_ZN, CAMERA_ZF);
			XMVECTOR pos = XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
			XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			mD3DAppValues.mCamera->LookAt(pos, target, up);
			AddGameComponent(mD3DAppValues.mCamera);

			mD3DAppValues.mGameStatesManager = new GameStatesManager(GetD3DAppValues());
			AddGameComponent(mD3DAppValues.mGameStatesManager);

			mD3DAppValues.mRenderStatesManager = new RenderStatesManager(GetD3DAppValues());
			AddGameComponent(mD3DAppValues.mRenderStatesManager);	

			if(!InitFX())
				return false;

			return true;

		//}
		//catch (Exception &ex)
		//{
		//	MessageBox(NULL, ex.Message(), ex.Type(), MB_ICONERROR|MB_OK);
		//	exit(0);
		//}
	}

	void D3DApp::OnResize()
	{
		assert(mD3DAppValues.md3dImmediateContext);
		assert(mD3DAppValues.md3dDevice);
		assert(mD3DAppValues.mSwapChain);

		// Release the old views, as they hold references to the buffers we
		// will be destroying.  Also release the old depth/stencil buffer.

		ReleaseCOM(mD3DAppValues.mRenderTargetView);
		ReleaseCOM(mD3DAppValues.mDepthStencilView);
		ReleaseCOM(mD3DAppValues.mDepthStencilBuffer);

		// Resize the swap chain and recreate the render target view.

		/*HR*/(mD3DAppValues.mSwapChain->ResizeBuffers(1, mD3DAppValues.mClientWidth, mD3DAppValues.mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
		ID3D11Texture2D* backBuffer;
		/*HR*/(mD3DAppValues.mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
		/*HR*/(mD3DAppValues.md3dDevice->CreateRenderTargetView(backBuffer, 0, &mD3DAppValues.mRenderTargetView));

		ReleaseCOM(backBuffer);

		// Create the depth/stencil buffer and view.

		D3D11_TEXTURE2D_DESC depthStencilDesc;

		depthStencilDesc.Width     = mD3DAppValues.mClientWidth;
		depthStencilDesc.Height    = mD3DAppValues.mClientHeight;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;

		// Use 4X MSAA? --must match swap chain MSAA values.
		if( mD3DAppValues.mEnable4xMsaa )
		{
			depthStencilDesc.SampleDesc.Count   = 4;
			depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality-1;
		}
		// No MSAA
		else
		{
			depthStencilDesc.SampleDesc.Count   = 1;
			depthStencilDesc.SampleDesc.Quality = 0;
		}

		depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0; 
		depthStencilDesc.MiscFlags      = 0;

		/*HR*/(mD3DAppValues.md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &mD3DAppValues.mDepthStencilBuffer));
		/*HR*/(mD3DAppValues.md3dDevice->CreateDepthStencilView(mD3DAppValues.mDepthStencilBuffer, 0, &mD3DAppValues.mDepthStencilView));

		// Bind the render target view and depth/stencil view to the pipeline.
		mD3DAppValues.md3dImmediateContext->OMSetRenderTargets(1, &mD3DAppValues.mRenderTargetView, mD3DAppValues.mDepthStencilView);

		// Set the viewport transform.
		mD3DAppValues.mScreenViewport.TopLeftX = 0;
		mD3DAppValues.mScreenViewport.TopLeftY = 0;
		mD3DAppValues.mScreenViewport.Width    = static_cast<float>(mD3DAppValues.mClientWidth);
		mD3DAppValues.mScreenViewport.Height   = static_cast<float>(mD3DAppValues.mClientHeight);
		mD3DAppValues.mScreenViewport.MinDepth = 0.0f;
		mD3DAppValues.mScreenViewport.MaxDepth = 1.0f;

		mD3DAppValues.md3dImmediateContext->RSSetViewports(1, &mD3DAppValues.mScreenViewport);

		// Fire the appropriate methods
		for (unsigned int i = 0; i < mGameComponents.size(); i++)
			mGameComponents[i]->OnResize();
	}

	LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch( msg )
		{
			// WM_ACTIVATE is sent when the window is activated or deactivated.  
			// We pause the game when the window is deactivated and unpause it 
			// when it becomes active.  
		case WM_ACTIVATE:
			if( LOWORD(wParam) == WA_INACTIVE )
			{
				mAppPaused = true;
				mTimer.Stop();
			}
			else
			{
				mAppPaused = false;
				mTimer.Start();
			}
			return 0;

			// WM_SIZE is sent when the user resizes the window.  
		case WM_SIZE:
			// Save the new client area dimensions.
			mD3DAppValues.mClientWidth  = LOWORD(lParam);
			mD3DAppValues.mClientHeight = HIWORD(lParam);
			if( mD3DAppValues.md3dDevice )
			{
				if( wParam == SIZE_MINIMIZED )
				{
					mAppPaused = true;
					mMinimized = true;
					mMaximized = false;
				}
				else if( wParam == SIZE_MAXIMIZED )
				{
					mAppPaused = false;
					mMinimized = false;
					mMaximized = true;
					OnResize();
				}
				else if( wParam == SIZE_RESTORED )
				{

					// Restoring from minimized state?
					if( mMinimized )
					{
						mAppPaused = false;
						mMinimized = false;
						OnResize();
					}

					// Restoring from maximized state?
					else if( mMaximized )
					{
						mAppPaused = false;
						mMaximized = false;
						OnResize();
					}
					else if( mResizing )
					{
						// If user is dragging the resize bars, we do not resize 
						// the buffers here because as the user continuously 
						// drags the resize bars, a stream of WM_SIZE messages are
						// sent to the window, and it would be pointless (and slow)
						// to resize for each WM_SIZE message received from dragging
						// the resize bars.  So instead, we reset after the user is 
						// done resizing the window and releases the resize bars, which 
						// sends a WM_EXITSIZEMOVE message.
					}
					else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
					{
						OnResize();
					}
				}
			}
			return 0;

			// WM_ENTERSIZEMOVE is sent when the user grabs the resize bars.
		case WM_ENTERSIZEMOVE:
			mAppPaused = true;
			mResizing  = true;
			mTimer.Stop();
			return 0;

			// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
			// Here we reset everything based on the new window dimensions.
		case WM_EXITSIZEMOVE:
			mAppPaused = false;
			mResizing  = false;
			mTimer.Start();
			OnResize();
			return 0;

			// Just paint the little resize bitmap on window.
		case WM_PAINT:
			{
				RECT rcClient;
				GetClientRect(mhMainWnd, &rcClient);
				BITMAP bm;
				PAINTSTRUCT ps;

				HDC hdc = BeginPaint(hwnd, &ps);
				HDC hdcMem = CreateCompatibleDC(hdc);

				HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, mResizingBmp);
				GetObject(mResizingBmp, sizeof(bm), &bm);

				BitBlt(hdc, rcClient.right / 2 - bm.bmWidth / 2, rcClient.bottom / 2 - bm.bmHeight / 2, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
				SelectObject(hdcMem, hbmOld);

				DeleteDC(hdcMem);
				EndPaint(hwnd, &ps);
			}
			return 0;

			// Catch this message so to prevent the window from becoming too small.
		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200; 
			return 0;

			// WM_DESTROY is sent when the window is being destroyed.
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

			// The WM_MENUCHAR message is sent when a menu is active and the user presses 
			// a key that does not correspond to any mnemonic or accelerator key. 
		case WM_MENUCHAR:
			// Don't beep when we alt-enter.
			return MAKELRESULT(0, MNC_CLOSE);

		case WM_COPYDATA:
			// Sending text using COPYDATASTRUCT 
			COPYDATASTRUCT* cds = (COPYDATASTRUCT*)lParam;
			if (cds->dwData == 1)
			{
				//copydata message
				string s((char *)cds->lpData, cds->cbData);
				wstring ws(s.begin(), s.end());
				//SetMainWndCaption(ws);
			}
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	void D3DApp::AddGameComponent(GameComponent *gameComponentPt, int drawOrder)
	{
		bool contains = false;
		for (UINT i = 0; i < mGameComponents.size(); ++i)
		{
			if (mGameComponents[i] == gameComponentPt)
			{
				contains = true;
				break;
			}
		}

		// Compare
		if (contains)
			throw InvalidOperation(L"Game component is already on the list!");
		else
		{
			mGameComponents.push_back(gameComponentPt);
		}
	}

	void D3DApp::RemoveGameComponent(GameComponent *gameComponentPt)
	{
		// first get index
		int index = -1;
		for (UINT i = 0; i < mGameComponents.size(); ++i)
		{
			if (mGameComponents[i] == gameComponentPt)
			{
				index = i;
				break;
			}
		}

		if (index == -1)
			throw InvalidOperation(L"Game component is not on the list!");

		// now erase
		mGameComponents.erase(mGameComponents.begin() + index);
	}

	bool D3DApp::InitMainWindow()
	{
		WNDCLASS wc;
		wc.style         = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc   = MainWndProc; 
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = mhAppInst;
		wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
		wc.hCursor       = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); //(HBRUSH)GetStockObject(NULL_BRUSH);
		wc.lpszMenuName  = 0;
		wc.lpszClassName = L"D3DWndClassName";

		if( !RegisterClass(&wc) )
			throw UnexpectedError(L"RegisterClass Failed.");

		if (mhParentWnd == NULL)
		{
			// Compute window rectangle dimensions based on requested client area dimensions.
			RECT R = { 0, 0, mD3DAppValues.mClientWidth, mD3DAppValues.mClientHeight };
			AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
			int width  = R.right - R.left;
			int height = R.bottom - R.top;
			mhMainWnd = CreateWindowEx(WS_EX_WINDOWEDGE, L"D3DWndClassName", mMainWndCaption.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0);
		}
		else
		{
			mhMainWnd = CreateWindowEx(WS_EX_CLIENTEDGE, L"D3DWndClassName", NULL, WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, mD3DAppValues.mClientWidth, mD3DAppValues.mClientHeight, mhParentWnd, (HMENU)ID_D3D11_CONTROL, mhAppInst, 0);
			SetWindowText(mhParentWnd, mMainWndCaption.c_str());
		}

		if( !mhMainWnd )
			throw UnexpectedError(L"CreateWindow Failed.");

		ShowWindow(mhMainWnd, SW_SHOW);
		UpdateWindow(mhMainWnd);

		return true;
	}

	bool D3DApp::InitDirect3D()
	{
		// Create the device and device context.

		UINT createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)  
		//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_FEATURE_LEVEL featureLevel;
		HRESULT hr = D3D11CreateDevice(
			0,                 // default adapter
			mD3DAppValues.md3dDriverType,
			0,                 // no software device
			createDeviceFlags, 
			0, 0,              // default feature level array
			D3D11_SDK_VERSION,
			&mD3DAppValues.md3dDevice,
			&featureLevel,
			&mD3DAppValues.md3dImmediateContext);

		if( FAILED(hr) )
			throw UnexpectedError(L"D3D11CreateDevice Failed.");

		if( featureLevel != D3D_FEATURE_LEVEL_11_0 )
			throw UnexpectedError(L"Direct3D Feature Level 11 unsupported.");

		// Check 4X MSAA quality support for our back buffer format.
		// All Direct3D 11 capable devices support 4X MSAA for all render 
		// target formats, so we only need to check quality support.

		/*HR*/(mD3DAppValues.md3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality));
		assert( m4xMsaaQuality > 0 );

		// Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.
		DXGI_SWAP_CHAIN_DESC sd;
		sd.BufferDesc.Width  = mD3DAppValues.mClientWidth;
		sd.BufferDesc.Height = mD3DAppValues.mClientHeight;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		// Use 4X MSAA? 
		if( mD3DAppValues.mEnable4xMsaa )
		{
			sd.SampleDesc.Count   = 4;
			sd.SampleDesc.Quality = m4xMsaaQuality-1;
		}
		// No MSAA
		else
		{
			sd.SampleDesc.Count   = 1;
			sd.SampleDesc.Quality = 0;
		}

		sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount  = 1;
		sd.OutputWindow = mhMainWnd;
		sd.Windowed     = true;
		sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags        = 0;

		// To correctly create the swap chain, we must use the IDXGIFactory that was
		// used to create the device.  If we tried to use a different IDXGIFactory instance
		// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
		// This function is being called with a device from a different IDXGIFactory."

		IDXGIDevice* dxgiDevice = 0;
		/*HR*/(mD3DAppValues.md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

		IDXGIAdapter* dxgiAdapter = 0;
		/*HR*/(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

		IDXGIFactory* dxgiFactory = 0;
		/*HR*/(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

		/*HR*/(dxgiFactory->CreateSwapChain(mD3DAppValues.md3dDevice, &sd, &mD3DAppValues.mSwapChain));

		//prevent Alt + Enter from setting application into full screen
		//dxgiFactory->MakeWindowAssociation(mhMainWnd, DXGI_MWA_NO_WINDOW_CHANGES);

		ReleaseCOM(dxgiDevice);
		ReleaseCOM(dxgiAdapter);
		ReleaseCOM(dxgiFactory);


		// The remaining steps that need to be carried out for d3d creation
		// also need to be executed every time the window is resized.  So
		// just call the OnResize method here to avoid code duplication.

		OnResize();

		return true;
	}

	bool D3DApp::InitFX()
	{
		//
		// Load effects from .dll resources
		//

		HMODULE hMod = LoadLibraryEx(L"Makina.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
		if (NULL != hMod)
		{
			// First the 2DGraphics.fxo
			HRSRC hRes = FindResource(hMod, MAKEINTRESOURCE(ID_2DGRAPHICS),  MAKEINTRESOURCE(TID_FX));
			if (NULL != hRes)
			{
				HGLOBAL hgbl = LoadResource(hMod, hRes);
				void *data = LockResource(hgbl);
				UINT32  size = SizeofResource(hMod, hRes);

				HRESULT hr = D3DX11CreateEffectFromMemory(data, size, 0, mD3DAppValues.md3dDevice, &mD3DAppValues.m2DGraphicsFX);
				if(FAILED(hr))
					throw UnexpectedError(wstring(L"Cannot load FX from Makina.dll.!"));
			}

			// then Particles.fxo
			hRes = FindResource(hMod, MAKEINTRESOURCE(ID_PARTICLES),  MAKEINTRESOURCE(TID_FX));
			if (NULL != hRes)
			{
				HGLOBAL hgbl = LoadResource(hMod, hRes);
				void *data = LockResource(hgbl);
				UINT32  size = SizeofResource(hMod, hRes);

				HRESULT hr = D3DX11CreateEffectFromMemory(data, size, 0, mD3DAppValues.md3dDevice, &mD3DAppValues.mParticlesFX);
				if(FAILED(hr))
					throw UnexpectedError(wstring(L"Cannot load FX from Makina.dll.!"));
			}

			// then CPSurfaces.fxo
			hRes = FindResource(hMod, MAKEINTRESOURCE(ID_CPSURFACES), MAKEINTRESOURCE(TID_FX));
			if (NULL != hRes)
			{
				HGLOBAL hgbl = LoadResource(hMod, hRes);
				void *data = LockResource(hgbl);
				UINT32  size = SizeofResource(hMod, hRes);

				HRESULT hr = D3DX11CreateEffectFromMemory(data, size, 0, mD3DAppValues.md3dDevice, &mD3DAppValues.mCPSurfacesFX);
				if (FAILED(hr))
					throw UnexpectedError(wstring(L"Cannot load FX from Makina.dll.!"));
			}

			// Free .dll module.
			FreeLibrary(hMod);

		}

		mD3DAppValues.mBasicEffect = new BasicEffect(&mD3DAppValues);
		return true;
	}

	void D3DApp::UpdateProcessAndSystemData()
	{
		static int frameCnt = 0;
		static float timeElapsed = 0.0f;
		frameCnt++;

		// Compute averages over one second period.
		if ((mTimer.TotalTime() - timeElapsed) < 1.0f) return;

		// Code computes the average frames per second, and also the 
		// average time it takes to render one frame.  These stats 
		// are appended to the window caption bar.

		mProcessAndSystemData.mFPS = (float)frameCnt; // fps = frameCnt / 1
		mProcessAndSystemData.m_mSPF = 1000.0f / mProcessAndSystemData.mFPS;

		//std::wostringstream outs;
		//outs.precision(6);
		//outs << mMainWndCaption << L"    "
		//	<< L"FPS: " << mProcessAndSystemData.mFPS << L"    "
		//	<< L"Frame Time: " << mProcessAndSystemData.m_mSPF << L" (ms)";

		//if (mhParentWnd)
		//	SetWindowText(mhParentWnd, outs.str().c_str());
		//else
		//	SetWindowText(mhMainWnd, outs.str().c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;

		// Process and system data
		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);
		//mProcessAndSystemData.mTotalVirtualMem = memInfo.ullTotalPageFile;
		mProcessAndSystemData.mVirtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
		PROCESS_MEMORY_COUNTERS_EX pmc;
		pmc.cb = sizeof(pmc);
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, pmc.cb);
		mProcessAndSystemData.mVirtualMemUsedByMe = pmc.PrivateUsage;
		//mProcessAndSystemData.mTotalPhysMem = memInfo.ullTotalPhys;
		mProcessAndSystemData.mPhysMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
		mProcessAndSystemData.mPhysMemUsedByMe = pmc.WorkingSetSize;
		PDH_FMT_COUNTERVALUE counterVal;
		PdhCollectQueryData(cpuQuery);
		PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
		mProcessAndSystemData.mCPU_usage = counterVal.doubleValue;
		FILETIME ftime, fsys, fuser;
		ULARGE_INTEGER now, sys, user;
		double percent;
		GetSystemTimeAsFileTime(&ftime);
		memcpy(&now, &ftime, sizeof(FILETIME));
		GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
		memcpy(&sys, &fsys, sizeof(FILETIME));
		memcpy(&user, &fuser, sizeof(FILETIME));
		percent = (double)((sys.QuadPart - lastSysCPU.QuadPart) + (user.QuadPart - lastUserCPU.QuadPart));
		percent /= (now.QuadPart - lastCPU.QuadPart);
		percent /= numProcessors;
		lastCPU = now;
		lastUserCPU = user;
		lastSysCPU = sys;
		mProcessAndSystemData.mCPU_usageByMe = percent * 100;
	}
}