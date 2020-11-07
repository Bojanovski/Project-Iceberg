
#ifndef RENDERSTATESMANAGER_H
#define RENDERSTATESMANAGER_H

#include <string>
#include <vector>
#include "DirectX11Headers.h"
#include "GameComponent.h"

namespace Makina
{
	class RenderStatesManager : public GameComponent
	{
	public:
		__declspec(dllexport) RenderStatesManager(D3DAppValues *value);
		__declspec(dllexport) ~RenderStatesManager();

		__declspec(dllexport) void Update(float dt);
		__declspec(dllexport) void Draw(float dt);
		__declspec(dllexport) void OnResize();

		__declspec(dllexport) ID3D11RasterizerState *LoadAndGetRS(const wchar_t *RSName, D3D11_RASTERIZER_DESC &desc);
		__declspec(dllexport) ID3D11RasterizerState *GetRS(const wchar_t *RSName);
		__declspec(dllexport) void RemoveRS(ID3D11RasterizerState *RSPt);

		__declspec(dllexport) ID3D11BlendState *LoadAndGetBS(const wchar_t *BSName, D3D11_BLEND_DESC &desc);
		__declspec(dllexport) ID3D11BlendState *GetBS(const wchar_t *BSName);
		__declspec(dllexport) void RemoveBS(ID3D11BlendState *BSPt);

		__declspec(dllexport) ID3D11DepthStencilState *LoadAndGetDSS(const wchar_t *DSSName, D3D11_DEPTH_STENCIL_DESC &desc);
		__declspec(dllexport) ID3D11DepthStencilState *GetDSS(const wchar_t *DSSName);
		__declspec(dllexport) void RemoveDSS(ID3D11DepthStencilState *DSSPt);

	private:
		std::vector<ID3D11RasterizerState *> mRStates;
		std::vector<std::wstring> mRSNames;

		std::vector<ID3D11BlendState *> mBStates;
		std::vector<std::wstring> mBSNames;

		std::vector<ID3D11DepthStencilState *> mDSStates;
		std::vector<std::wstring> mDSSNames;
		
		__declspec(dllexport) bool ContainsValue(std::vector<std::wstring> &dataVec, const std::wstring &name, int &index);
	};
}

#endif
