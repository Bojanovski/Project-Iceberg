#include "Object2D.h"
#include "Exceptions.h"
#include "RenderStatesManager.h"
#include "Resource.h"
#include <fstream>
#include <vector>

using namespace Makina;
using namespace std;

Object2D::Object2D(D3DAppValues *values, float posX, float posY, float sizeX, float sizeY, FXMVECTOR color)
	: mD3DAppValues(values),
	mVertexBuffer(0),
	mIndexBuffer(0),
	mIndexCount(0),
	mVertexCount(0),
	mInputLayout(0),
	mFXTechnique(0),
	mTextureVar(0),
	mOffsetAndSizeVar(0),
	mColorVar(0),
	mTextureSRV(0),
	mDepthDisabled(0),
	mBlendState(0),
	mPosX(posX),
	mPosY(posY),
	mSizeX(sizeX),
	mSizeY(sizeY)

{
	//// First load effect from .dll resources
	//HMODULE hMod = LoadLibraryEx(L"Makina.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
	//if (NULL != hMod)
	//{
	//	HRSRC hRes = FindResource(hMod, MAKEINTRESOURCE(ID_2DGRAPHICS),  MAKEINTRESOURCE(TID_FX));
	//	if (NULL != hRes)
	//	{
	//		HGLOBAL hgbl = LoadResource(hMod, hRes);
	//		void *data = LockResource(hgbl);
	//		UINT32  size = SizeofResource(hMod, hRes);

	//		HRESULT hr = D3DX11CreateEffectFromMemory(data, size, 0, values->md3dDevice, &values->m2DGraphicsFX);
	//		if(FAILED(hr))
	//			throw UnexpectedError(wstring(L"Cannot load FX from Makina.dll.!"));
	//	}
	//	FreeLibrary(hMod);
	//}

	//wchar_t *file = L"FX\\2DGraphics.fxo";
	//ifstream fin(file, ios::binary);
	//if (!fin)
	//	throw UnexpectedError(wstring(L"Effect not loaded properly ") + file + L"!");

	//fin.seekg(0, ios_base::end);
	//int size = (int)fin.tellg();
	//fin.seekg(0, ios_base::beg);
	//vector<char> compiledShader(size);

	//fin.read(&compiledShader[0], size);
	//fin.close();

	//HRESULT hr = D3DX11CreateEffectFromMemory(&compiledShader[0], size, 0, values->md3dDevice, &mFX);
	//if(FAILED(hr))
	//	throw UnexpectedError(wstring(L"Effect not loaded properly ") + file + L"!");



	// Copy color
	XMStoreFloat4(&mColor, color);

	// Now get the variables.
	mTextureVar = values->m2DGraphicsFX->GetVariableByName("gTexture")->AsShaderResource();
	mOffsetAndSizeVar = values->m2DGraphicsFX->GetVariableByName("gOffsetAndSize")->AsVector();
	mColorVar = values->m2DGraphicsFX->GetVariableByName("gColor")->AsVector();

	// The rest
	mDepthDisabled = ((RenderStatesManager *)values->mRenderStatesManager)->GetDSS(L"depthStencilUI");

	// Get blend state
	mBlendState = ((RenderStatesManager *)values->mRenderStatesManager)->GetBS(L"alphaBlend");
}

Object2D::~Object2D()
{
	if(mVertexBuffer) mVertexBuffer->Release();
	if(mIndexBuffer) mIndexBuffer->Release();
	if(mInputLayout) mInputLayout->Release();
}

void Object2D::Draw()
{

}