
#include "Font.h"
#include <fstream>
#include "Exceptions.h"

using namespace Makina;
using namespace std;

#define MAX_TC_LEN 300

int GetNumAfterEq(fstream *input)
{
	int num = 0;
	char sign = 1;
	char currChar;
	// skip to '=' char
	while (input->get() != '=');
	// continue
	while (currChar = input->get())
	{
		if (currChar == ' ' || currChar == '\n')
			break;

		if (currChar == '-')
		{
			sign = -1;
			continue;
		}

		num *= 10;
		num += (currChar - 48) * sign;
	}

	return num;
}

Font::Font(D3DAppValues *values, wchar_t *fontFolderPath)
: mFontTexSRV(NULL)
{
	// Load the texture and get its dimensions
	wstring fontTexPath = wstring(L"Fonts\\") + fontFolderPath + wstring(L"\\") + wstring(L"tex.dds");

	HRESULT hr;
	hr = D3DX11CreateShaderResourceViewFromFile(values->md3dDevice, &fontTexPath[0], 0, 0, &mFontTexSRV, 0);
	if (FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to load texture ") + fontTexPath + L"! (Font::Font)");

	ID3D11Resource* res;
	mFontTexSRV->GetResource(&res);
	D3D11_TEXTURE2D_DESC texDesc;
	reinterpret_cast<ID3D11Texture2D*>(res)->GetDesc(&texDesc);
	mTexW = texDesc.Width;
	mTexH = texDesc.Height;

	// Parse coordinates
	wstring fontCoordsPath = wstring(L"Fonts\\") + fontFolderPath + wstring(L"\\") + wstring(L"coord.fnt");
	ParseFontCoords(&fontCoordsPath[0]);
}

Font::~Font()
{
	if (mFontTexSRV) mFontTexSRV->Release();
}

void Font::ParseFontCoords(wchar_t *fontCoordsPath)
{
	fstream input;
	input.open(fontCoordsPath);
	if (!input)
		throw FileNotFound(fontCoordsPath);

	char text[MAX_TC_LEN + 1];
	mSmallestYOffset = INT_MAX;
	mHighestLetter = 0;

	// read first three lines
	input.getline(text, MAX_TC_LEN);
	input.getline(text, MAX_TC_LEN);
	input.getline(text, MAX_TC_LEN);

	// Get number of chars
	mNumElements = GetNumAfterEq(&input);

	// Populate items vector
	ItemCoords item;
	for (int i = 0; i < mNumElements; i++)
	{
		item.charId = GetNumAfterEq(&input);
		item.x = GetNumAfterEq(&input);
		item.y = GetNumAfterEq(&input);
		item.width = GetNumAfterEq(&input);
		item.height = GetNumAfterEq(&input);
		item.xOffset = GetNumAfterEq(&input);
		item.yOffset = GetNumAfterEq(&input);
		item.xAdvance = GetNumAfterEq(&input);
		item.page = GetNumAfterEq(&input);
		item.chnl = GetNumAfterEq(&input);

		mBuffer.push_back(item);

		if (mSmallestYOffset > item.yOffset)
			mSmallestYOffset = item.yOffset;

		if (mHighestLetter < (UINT)item.height)
			mHighestLetter = item.height;
	}

	input.close();
}