
#ifndef FONT_H
#define FONT_H

#include "D3DAppValues.h"
#include <vector>

namespace Makina
{
	struct ItemCoords
	{
		int charId;
		int x;
		int y;
		int width;
		int height;
		int xOffset;
		int yOffset;
		int xAdvance;
		int page;
		int chnl;
	};

	class Font
	{
		friend class Text;

	public:
		__declspec(dllexport) Font(D3DAppValues *values, wchar_t *fontFolderPath);
		__declspec(dllexport) ~Font();

	private:
		__declspec(dllexport) void ParseFontCoords(wchar_t *fontCoordsPath);

		// Buffer containing all coordinates and sizes to properly read from texture and display characters.
		std::vector<ItemCoords> mBuffer;
		int mNumElements;
		UINT mTexW;
		UINT mTexH;
		UINT mHighestLetter;
		int mSmallestYOffset;

		// Texture view
		ID3D11ShaderResourceView *mFontTexSRV;
	};
}

#endif