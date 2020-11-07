#include "Text.h"
#include "Geometry.h"
#include "Exceptions.h"

using namespace Makina;
using namespace std;

#define MAX_TC_LEN 300

Text::Text(D3DAppValues *values, Font *font, const wchar_t *text, float posX, float posY, float size, FXMVECTOR color)
	: Object2D(values, posX, posY, size, size, color),
	mFont(font)
{
	// Copy color
	XMStoreFloat4(&mColor, color);

	// Copy text
	mText = wstring(text);	
	if (mText.length() > TEXT_MAX_LEN)
		throw UnexpectedError(wstring(L"Text must be less than ") + std::to_wstring(TEXT_MAX_LEN) + wstring(L" characters!"));

	// Get technique and variables
	mFXTechnique = values->m2DGraphicsFX->GetTechniqueByName("Text");

	// Get texture srv
	mTextureSRV = font->mFontTexSRV;

	// Create geometry.
	BuildGeometryBuffers();
	CreateInputLayout();
}

Text::~Text()
{

}

void Text::Draw()
{
	mD3DAppValues->md3dImmediateContext->IASetInputLayout(mInputLayout);
	mD3DAppValues->md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(VertexText2D);
	UINT offset = 0;
	mD3DAppValues->md3dImmediateContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);

	mD3DAppValues->md3dImmediateContext->OMSetDepthStencilState(mDepthDisabled, 0);
	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
	mD3DAppValues->md3dImmediateContext->OMSetBlendState(mBlendState, blendFactor, 0xffffffff);
	mD3DAppValues->md3dImmediateContext->RSSetState(NULL);
	
	// Set variables
	mTextureVar->SetResource(mTextureSRV);
	XMVECTOR offsetAndSize = XMVectorSet(mPosX, mPosY, mSizeX, mSizeY);
	mOffsetAndSizeVar->SetFloatVector((float *)&offsetAndSize);
	mColorVar->SetFloatVector((float *)&mColor);

	D3DX11_TECHNIQUE_DESC techDesc;
	mFXTechnique->GetDesc(&techDesc);

	for (UINT i = 0; i < techDesc.Passes; i++)
	{
		mFXTechnique->GetPassByIndex(i)->Apply(0, mD3DAppValues->md3dImmediateContext);

		//6 indices for the quad
		mD3DAppValues->md3dImmediateContext->Draw(mVertexCount, 0);
	}

	//// Set shader resource to null because we will use this texture later.
	//ID3D11ShaderResourceView *pSRV[1] = {NULL};
	//mD3DAppValues->md3dImmediateContext->PSSetShaderResources(0, 1, pSRV);
}

void Text::ChangeProp(float posX, float posY, float size, FXMVECTOR color)
{
	XMVECTOR offsetAndSize = XMVectorSet(posX, posY, size, size);
	mOffsetAndSizeVar->SetFloatVector((float *)&offsetAndSize);
	mColorVar->SetFloatVector((float *)&color);
}

void Text::ChangeText(const wchar_t *newText)
{
	// Copy text
	mText = newText;	
	if (mText.length() > TEXT_MAX_LEN )
		throw UnexpectedError(wstring(L"Text must be smaller than ") + std::to_wstring(TEXT_MAX_LEN) + wstring(L" characters!"));

	// Now change vertex buffer.
	D3D11_MAPPED_SUBRESOURCE mappedData;
	mD3DAppValues->md3dImmediateContext->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

	VertexText2D *vert = reinterpret_cast<VertexText2D *>(mappedData.pData);
	float xOffset = 0, yOffset = 0;
	for (UINT i = 0; i < mText.length(); i++)
	{
		if (mText[i] == '\n')
		{
			xOffset = 0;
			yOffset += mFont->mHighestLetter;
		}
		else
			SetValues(mText[i], &vert[i], &xOffset, &yOffset);
	}

	mD3DAppValues->md3dImmediateContext->Unmap(mVertexBuffer, 0);
	mVertexCount = mText.length();
}

void Text::OnResize()
{
	ChangeText(&mText[0]);
}

void Text::BuildGeometryBuffers()
{
	//Release if necessary
	if (mVertexBuffer)
	{
		mVertexBuffer->Release();
		mVertexBuffer = NULL;
	}
	if (mIndexBuffer)
	{
		mIndexBuffer->Release();
		mIndexBuffer = NULL;
	}

	float xOffset = 0, yOffset = 0;
	// Not vector<VertexText2D> vertices(mText.length()) because we need access to enough system memory for entire vertex buffer.
	// Remember that buffer is this big -> sizeof(VertexText2D) * TEXT_MAX_LEN;
	vector<VertexText2D> vertices(TEXT_MAX_LEN);

	for (UINT i = 0; i < mText.length(); i++)
	{
		if (mText[i] == '\n')
		{ // new line
			xOffset = 0;
			yOffset += mFont->mHighestLetter;
		}
		else
			SetValues(mText[i], &vertices[i], &xOffset, &yOffset);
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(VertexText2D) * TEXT_MAX_LEN;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HRESULT hr = mD3DAppValues->md3dDevice->CreateBuffer(&vbd, &vinitData, &mVertexBuffer);
	if(FAILED(hr)) throw UnexpectedError(wstring(L"Failed to create vertex buffer! (Text::BuildGeometryBuffers)"));
	mVertexCount = mText.length();
}

void Text::CreateInputLayout()
{
	D3DX11_PASS_DESC passDesc;
	mFXTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	HRESULT hr = mD3DAppValues->md3dDevice->CreateInputLayout(VertexText2DDesc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout);
	if(FAILED(hr)) throw UnexpectedError(L"Failed to create input layout! (Text.cpp)");
}

void Text::SetValues(int charId, void *vert, float *xOffset, float *yOffset)
{
	ItemCoords *item;
	bool found = false;
	for (int i = 0; i < mFont->mNumElements; i++)
	{
		if (mFont->mBuffer.at(i).charId == charId)
		{
			found = true;
			item = &mFont->mBuffer.at(i);
			break;
		}
	}

	if (!found)
		throw UnexpectedError(wstring(L"Character '") + (wchar_t)charId + L"' not supported by this font.");

	float xPos = *xOffset + item->xOffset;
	float yPos = *yOffset + item->yOffset - mFont->mSmallestYOffset;

	((VertexText2D *)vert)->PosRec = XMFLOAT4(
		xPos / (float)mD3DAppValues->mClientWidth,
		yPos / (float)mD3DAppValues->mClientHeight,
		(xPos + item->width) / (float)mD3DAppValues->mClientWidth,
		(yPos + item->height) / (float)mD3DAppValues->mClientHeight);

	((VertexText2D *)vert)->TexRec = XMFLOAT4(
		item->x / (float)mFont->mTexW,
		item->y / (float)mFont->mTexH,
		(item->x + item->width) / (float)mFont->mTexW,
		(item->y + item->height) / (float)mFont->mTexH);

	*xOffset += (float)item->xAdvance;
}