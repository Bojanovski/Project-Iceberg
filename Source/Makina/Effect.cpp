#include "Effect.h"
#include <fstream>
#include <vector>
#include "Exceptions.h"
#include "Resource.h"

using namespace Makina;
using namespace std;

Effect::Effect(D3DAppValues *values, const wchar_t *file) 
	: mFX(NULL),
	mValues(values)
{
	LoadEffect(file);
}

Effect::Effect(D3DAppValues *values, const wchar_t *library, UINT resourceId)
	: mFX(NULL),
	mValues(values)
{
	LoadEffect(library, resourceId);
}

Effect::~Effect()
{
	mFX->Release();
}

void Effect::LoadEffect(const wchar_t *file)
{
	ifstream fin(file, ios::binary);
	if (!fin)
		throw UnexpectedError(wstring(L"Effect not loaded properly ") + file + L"!");

	fin.seekg(0, ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, ios_base::beg);
	vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();

	HRESULT hr = D3DX11CreateEffectFromMemory(&compiledShader[0], size, 0, mValues->md3dDevice, &mFX);
	if(FAILED(hr))
		throw UnexpectedError(wstring(L"Effect not loaded properly ") + file + L"!");
}

void Effect::LoadEffect(const wchar_t *library, UINT resourceId)
{
	// Load effect from .dll resource
	HMODULE hMod = LoadLibraryEx(library, NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (NULL != hMod)
	{
		HRSRC hRes = FindResource(hMod, MAKEINTRESOURCE(resourceId),  MAKEINTRESOURCE(TID_FX));
		if (NULL != hRes)
		{
			HGLOBAL hgbl = LoadResource(hMod, hRes);
			void *data = LockResource(hgbl);
			UINT32  size = SizeofResource(hMod, hRes);

			HRESULT hr = D3DX11CreateEffectFromMemory(data, size, 0, mValues->md3dDevice, &mFX);
			if(FAILED(hr))
				throw UnexpectedError(wstring(L"Cannot load FX from Makina.dll.!"));
		}

		// Free .dll module.
		FreeLibrary(hMod);
	}
}