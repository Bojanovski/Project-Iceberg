
#include "ContentLoader.h"

using namespace Makina;

ContentLoader::ContentLoader(D3DAppValues *values)
	: mTextureLoader(values),
	mOBJFileLoader(values, L"Textures"),
	mXFileLoader(values, L"Textures")
{

}

ContentLoader::~ContentLoader()
{

}