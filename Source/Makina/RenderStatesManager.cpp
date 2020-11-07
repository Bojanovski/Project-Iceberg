#include "RenderStatesManager.h"

using namespace Makina;
using namespace std;

RenderStatesManager::RenderStatesManager(D3DAppValues *value) : GameComponent(value)
{
	// Rasterizer states
	D3D11_RASTERIZER_DESC wireframe, normal;
	ZeroMemory(&wireframe, sizeof(D3D11_RASTERIZER_DESC));
	wireframe.FillMode = D3D11_FILL_WIREFRAME;
	wireframe.CullMode = D3D11_CULL_BACK;
	wireframe.FrontCounterClockwise = false;
	wireframe.DepthClipEnable = true;

	ZeroMemory(&normal, sizeof(D3D11_RASTERIZER_DESC));
	normal.FillMode = D3D11_FILL_SOLID;
	normal.CullMode = D3D11_CULL_BACK;
	normal.FrontCounterClockwise = false;
	normal.DepthClipEnable = true;

	LoadAndGetRS(L"wireframe", wireframe);
	LoadAndGetRS(L"normal", normal);

	// Blend states
	D3D11_BLEND_DESC transparentDesc = {0};
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;

	transparentDesc.RenderTarget[0].BlendEnable = true;
	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	LoadAndGetBS(L"alphaBlend", transparentDesc);

	//Depth stencil states
	D3D11_DEPTH_STENCIL_DESC depthDisabled;
	depthDisabled.DepthEnable = false;
	depthDisabled.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthDisabled.DepthFunc = D3D11_COMPARISON_LESS;
	depthDisabled.StencilEnable = false;
	LoadAndGetDSS(L"depthDisabled", depthDisabled);

	D3D11_DEPTH_STENCIL_DESC depthStencilUI;
	depthStencilUI.DepthEnable = true;
	depthStencilUI.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilUI.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilUI.StencilEnable = false;
	LoadAndGetDSS(L"depthStencilUI", depthStencilUI);
}

RenderStatesManager::~RenderStatesManager()
{
	for (UINT i = 0; i < mRStates.size(); i++)
	{
		mRStates[i]->Release();
	}

	for (UINT i = 0; i < mBStates.size(); i++)
	{
		mBStates[i]->Release();
	}

	for (UINT i = 0; i < mDSStates.size(); i++)
	{
		mDSStates[i]->Release();
	}
}

void RenderStatesManager::Update(float dt)
{

}

void RenderStatesManager::Draw(float dt)
{

}

void RenderStatesManager::OnResize()
{

}

ID3D11RasterizerState *RenderStatesManager::LoadAndGetRS(const wchar_t *RSName, D3D11_RASTERIZER_DESC &desc)
{
	int index = -1;

	// First check if rasterizer state is already created.
	if (ContainsValue(mRSNames, RSName, index))
		return mRStates[index];

	ID3D11RasterizerState *newRS;
	mD3DAppValues->md3dDevice->CreateRasterizerState(&desc, &newRS);

	// Everything is ok!
	index = mRStates.size();

	mRStates.push_back(newRS);
	mRSNames.push_back(RSName);

	return mRStates[index];
}

ID3D11RasterizerState *RenderStatesManager::GetRS(const wchar_t *RSName)
{
	int index = -1;

	// First check if texture is already loaded.
	if (ContainsValue(mRSNames, RSName, index))
		return mRStates[index];
	else
		throw NotFound(wstring(L"Rasterizer State ") + RSName + L" not found.");
}

void RenderStatesManager::RemoveRS(ID3D11RasterizerState *RSPt)
{
	for (UINT i = 0; i < mRStates.size(); i++)
		if (mRStates[i] == RSPt)
		{
			mRStates[i]->Release();
			mRStates.erase(mRStates.begin() + i);
			mRSNames.erase(mRSNames.begin() + i);
		}
}

ID3D11BlendState *RenderStatesManager::LoadAndGetBS(const wchar_t *BSName, D3D11_BLEND_DESC &desc)
{
	int index = -1;

	// First check if blend state is already created.
	if (ContainsValue(mBSNames, BSName, index))
		return mBStates[index];

	ID3D11BlendState *newBS;
	mD3DAppValues->md3dDevice->CreateBlendState(&desc, &newBS);

	// Everything is ok!
	index = mBStates.size();

	mBStates.push_back(newBS);
	mBSNames.push_back(BSName);

	return mBStates[index];
}

ID3D11BlendState *RenderStatesManager::GetBS(const wchar_t *BSName)
{
	int index = -1;

	// First check if texture is already loaded.
	if (ContainsValue(mBSNames, BSName, index))
		return mBStates[index];
	else
		throw NotFound(wstring(L"Blend State ") + BSName + L" not found.");
}

void RenderStatesManager::RemoveBS(ID3D11BlendState *BSPt)
{
	for (UINT i = 0; i < mBStates.size(); i++)
		if (mBStates[i] == BSPt)
		{
			mBStates[i]->Release();
			mBStates.erase(mBStates.begin() + i);
			mBSNames.erase(mBSNames.begin() + i);
		}
}

ID3D11DepthStencilState *RenderStatesManager::LoadAndGetDSS(const wchar_t *DSSName, D3D11_DEPTH_STENCIL_DESC &desc)
{
	int index = -1;

	// First check if blend state is already created.
	if (ContainsValue(mDSSNames, DSSName, index))
		return mDSStates[index];

	ID3D11DepthStencilState *newDSS;
	mD3DAppValues->md3dDevice->CreateDepthStencilState(&desc, &newDSS);

	// Everything is ok!
	index = mDSStates.size();

	mDSStates.push_back(newDSS);
	mDSSNames.push_back(DSSName);

	return mDSStates[index];
}

ID3D11DepthStencilState *RenderStatesManager::GetDSS(const wchar_t *DSSName)
{
	int index = -1;

	// First check if texture is already loaded.
	if (ContainsValue(mDSSNames, DSSName, index))
		return mDSStates[index];
	else
		throw NotFound(wstring(L"Depth Stencil State ") + DSSName + L" not found.");
}

void RenderStatesManager::RemoveDSS(ID3D11DepthStencilState *DSSPt)
{
	for (UINT i = 0; i < mDSStates.size(); i++)
		if (mDSStates[i] == DSSPt)
		{
			mDSStates[i]->Release();
			mDSStates.erase(mDSStates.begin() + i);
			mDSSNames.erase(mDSSNames.begin() + i);
		}
}

bool RenderStatesManager::ContainsValue(vector<wstring> &dataVec, const wstring &name, int &index)
{
	for (unsigned int i = 0; i < dataVec.size(); i++)
		if (dataVec[i] == name)
		{
			index = i;
			return true;
		}

	return false;
}
