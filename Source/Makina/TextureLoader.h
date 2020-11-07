
#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <string>
#include <map>
#include <vector>
#include "DirectX11Headers.h"
#include "D3DAppValues.h"

namespace Makina
{
	class TextureLoader
	{
	public:
		__declspec(dllexport) TextureLoader(D3DAppValues *values);
		__declspec(dllexport) ~TextureLoader();

		// Textures
		__declspec(dllexport) void LoadSRV(const wchar_t *filePath);

		// Texture Arrays
		__declspec(dllexport) void LoadSRVArray(const wchar_t *name, std::vector<std::wstring> &filePaths);

		__declspec(dllexport) ID3D11ShaderResourceView *GetSRV(const wchar_t *stringIdentifier);
		bool ContainsTexture(const std::wstring &filePath) { return (mLoaded.find(filePath) != mLoaded.end()); }
		__declspec(dllexport) void RemoveTexture(ID3D11ShaderResourceView *viewPt);
		__declspec(dllexport) void RemoveTexture(const std::wstring &filePath);

	private:

		D3DAppValues *mValues;
		std::map<std::wstring, ID3D11ShaderResourceView *> mLoaded;
	};
}

#endif
