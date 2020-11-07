
#ifndef CONTENTLOADER_H
#define CONTENTLOADER_H

#include <string>
#include <vector>
#include "DirectX11Headers.h"
#include "TextureLoader.h"
#include "OBJFileLoader.h"
#include "XFileLoader.h"

namespace Makina
{
	class ContentLoader
	{
	public:
		__declspec(dllexport) ContentLoader(D3DAppValues *values);
		__declspec(dllexport) ~ContentLoader();

		TextureLoader *GetTextureLoader() { return &mTextureLoader; }
		OBJFileLoader *GetOBJFileLoader() { return &mOBJFileLoader; }
		XFileLoader *GetXFileLoader() { return &mXFileLoader; }

	private:
		TextureLoader mTextureLoader;
		OBJFileLoader mOBJFileLoader;
		XFileLoader mXFileLoader;
	};
}

#endif
