
#ifndef D3DVALUES_H
#define D3DVALUES_H

#include "DirectX11Headers.h"

namespace Makina
{
	/************************************************************

	This structure holds all graphic and system related values.
	It is ment to be initialzed
	only once (in graphic initialization class) and then
	passed to game components via pointer.

	*************************************************************/

	class GameTimer;
	class Camera;
	class InputHandler;
	class RenderStatesManager;
	class BasicEffect;
	class GameStatesManager;
	struct ProcessAndSystemData;

	struct D3DAppValues
	{
		ID3D11Device *md3dDevice;
		ID3D11DeviceContext *md3dImmediateContext;
		IDXGISwapChain *mSwapChain;
		ID3D11Texture2D *mDepthStencilBuffer;
		ID3D11RenderTargetView *mRenderTargetView;
		ID3D11DepthStencilView *mDepthStencilView;
		D3D_DRIVER_TYPE md3dDriverType;
		D3D11_VIEWPORT mScreenViewport;
		UINT mClientWidth;
		UINT mClientHeight;
		bool mEnable4xMsaa;

		// Everything important
		ProcessAndSystemData const * mProcessAndSystemData;

		// Time.
		GameTimer *mGameTimer;

		// Game states.
		GameStatesManager *mGameStatesManager;

		// Camera component.
		Camera *mCamera;

		// Input stuff
		InputHandler *mInputHandler;

		// Important for managing render states (blend and rasterizer states).
		RenderStatesManager *mRenderStatesManager;

		// Methods
		float GetAspectRatio() {return static_cast<float>(mClientWidth) / mClientHeight;}

		// Effects
		ID3DX11Effect *m2DGraphicsFX;
		ID3DX11Effect *mParticlesFX;
		ID3DX11Effect *mCPSurfacesFX;
		BasicEffect *mBasicEffect;
	};
}

#endif
