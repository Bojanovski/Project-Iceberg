#include "TextureLoader.h"
#include "Exceptions.h"
#include <fstream>
#include <vector>

using namespace Makina;
using namespace std;

TextureLoader::TextureLoader(D3DAppValues *values)
: mValues(values),
mLoaded()
{

}

TextureLoader::~TextureLoader()
{
	for (auto texIt = mLoaded.begin(); texIt != mLoaded.end(); ++texIt)
	{ // Release all loaded textures.
		texIt->second->Release();
	}
}

void TextureLoader::LoadSRV(const wchar_t *filePath)
{
	// First check if texture is already loaded.
	if (ContainsTexture(filePath))
		return;

	// Not loaded.
	//ifstream fin(filePath, ios::binary);
	//if (!fin)
	//	throw FileNotFound(wstring(L"Failed to load texture ") + filePath + L"!");

	//fin.seekg(0, ios_base::end);
	//int size = (int)fin.tellg();
	//fin.seekg(0, ios_base::beg);
	//vector<char> texture(size);
	//fin.read(&texture[0], size);
	//fin.close();

	ID3D11ShaderResourceView *textureView;

	HRESULT hr;
	//hr = D3DX11CreateShaderResourceViewFromMemory(mValues->md3dDevice, &texture[0], size, 0, 0, &textureView, 0);
	hr = D3DX11CreateShaderResourceViewFromFile(mValues->md3dDevice, filePath, 0, 0, &textureView, 0);

	if(FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to load texture '") + filePath + L"'! (TextureLoader::LoadSRV)");

	// Everything is ok!
	mLoaded[filePath] = textureView;
}

void TextureLoader::LoadSRVArray(const wchar_t *name, vector<std::wstring> &filePaths)
{
	// First check if array is already loaded.
	if (ContainsTexture(name))
		return;

	//
	// Load the texture elements individually from file.  These textures
	// won't be used by the GPU (0 bind flags), they are just used to 
	// load the image data from file.  We use the STAGING usage so the
	// CPU can read the resource.
	//

	HRESULT hr;
	vector<ID3D11Texture2D *> srcTex;
	for (UINT i = 0; i < filePaths.size(); i++)
	{
		//ifstream fin(filePaths[i], ios::binary);
		//if (!fin)
		//	throw FileNotFound(wstring(L"Failed to load texture ") + filePaths[i] + L"!");

		//fin.seekg(0, ios_base::end);
		//int size = (int)fin.tellg();
		//fin.seekg(0, ios_base::beg);
		//vector<char> texture(size);
		//fin.read(&texture[0], size);
		//fin.close();

		D3DX11_IMAGE_LOAD_INFO loadInfo;

		loadInfo.Width  = D3DX11_FROM_FILE;
		loadInfo.Height = D3DX11_FROM_FILE;
		loadInfo.Depth  = D3DX11_FROM_FILE;
		loadInfo.FirstMipLevel = 0;
		loadInfo.MipLevels = D3DX11_FROM_FILE;
		loadInfo.Usage = D3D11_USAGE_STAGING;
		loadInfo.BindFlags = 0;
		loadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		loadInfo.MiscFlags = 0;
		loadInfo.Format = DXGI_FORMAT_FROM_FILE;
		loadInfo.Filter = D3DX11_FROM_FILE;
		loadInfo.MipFilter = D3DX11_FROM_FILE;
		loadInfo.pSrcInfo  = 0;

		ID3D11Texture2D *texPt;
		//hr = D3DX11CreateTextureFromMemory(mValues->md3dDevice, &texture[0], size, &loadInfo, 0, (ID3D11Resource**)&texPt, 0);
		hr = D3DX11CreateTextureFromFile(mValues->md3dDevice, &(filePaths[i])[0], &loadInfo, 0, (ID3D11Resource**)&texPt, 0);

		if(FAILED(hr))
			throw UnexpectedError(wstring(L"Failed to load texture '") + filePaths[i] + L"'! (TextureLoader::LoadSRVArray)");
		else
			srcTex.push_back(texPt);
	}


	//
	// Create the texture array.  Each element in the texture 
	// array has the same format/dimensions.
	//

	D3D11_TEXTURE2D_DESC texElementDesc;
	srcTex[0]->GetDesc(&texElementDesc);

	D3D11_TEXTURE2D_DESC texArrayDesc;
	texArrayDesc.Width              = texElementDesc.Width;
	texArrayDesc.Height             = texElementDesc.Height;
	texArrayDesc.MipLevels          = texElementDesc.MipLevels;
	texArrayDesc.ArraySize          = srcTex.size();
	texArrayDesc.Format             = texElementDesc.Format;
	texArrayDesc.SampleDesc.Count   = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage              = D3D11_USAGE_DEFAULT;
	texArrayDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags     = 0;
	texArrayDesc.MiscFlags          = 0;

	ID3D11Texture2D* texArray = 0;
	hr = mValues->md3dDevice->CreateTexture2D(&texArrayDesc, 0, &texArray);

	if(FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to create texture array! (TextureLoader::LoadSRVArray)"));

	//
	// Copy individual texture elements into texture array.
	//

	// for each texture element...
	for(UINT texElement = 0; texElement < srcTex.size(); ++texElement)
	{
		// for each mipmap level...
		for(UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel)
		{
			D3D11_MAPPED_SUBRESOURCE mappedTex2D;
			hr = mValues->md3dImmediateContext->Map(srcTex[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D);
			if(FAILED(hr))
				throw UnexpectedError(wstring(L"Failed to map subresource mip maps! (TextureLoader::LoadSRVArray)"));

			mValues->md3dImmediateContext->UpdateSubresource(texArray, D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels),
				0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);

			mValues->md3dImmediateContext->Unmap(srcTex[texElement], mipLevel);
		}
	}

	//
	// Create a resource view to the texture array.
	//

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize = srcTex.size();

	ID3D11ShaderResourceView* texArraySRV = 0;
	hr = mValues->md3dDevice->CreateShaderResourceView(texArray, &viewDesc, &texArraySRV);

	//
	// Cleanup--we only need the resource view.
	//
	texArray->Release();
	for(UINT i = 0; i < srcTex.size(); ++i)
	{
		srcTex[i]->Release();
		srcTex[i] = NULL;
	}

	// Everything is ok!
	mLoaded[name] = texArraySRV;
}

ID3D11ShaderResourceView *TextureLoader::GetSRV(const wchar_t *stringIdentifier)
{
	if (mLoaded.find(stringIdentifier) == mLoaded.end())
	{
		// not found
		throw UnexpectedError(wstring(L"Shader resource view associated with identifier '") + stringIdentifier + L"' does not exist!! (TextureLoader::GetSRV)");
	}
	else
	{
		return mLoaded[stringIdentifier];
	}
}

void TextureLoader::RemoveTexture(ID3D11ShaderResourceView *viewPt)
{
	bool removed = false;

	for (auto texIt = mLoaded.begin(); texIt != mLoaded.end();)
	{
		if (texIt->second == viewPt)
		{
			texIt->second->Release();
			texIt = mLoaded.erase(texIt);
			removed = true;
		}
		else ++texIt;
	}

	if (!removed) throw UnexpectedError(L"Texture does not exist. (TextureLoader::RemoveTexture)");
}

void TextureLoader::RemoveTexture(const wstring &filePath)
{
	auto texIt = mLoaded.find(filePath);
	if (texIt == mLoaded.end())
		throw UnexpectedError(L"Texture does not exist. (TextureLoader::RemoveTexture)");
	else
	{
		texIt->second->Release();
		mLoaded.erase(texIt);
	}
}

