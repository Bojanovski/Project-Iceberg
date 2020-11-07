
#include "SceneVoronoizator.h"
#include <D3DUtilities.h>
#include <MathHelper.h>
#include <sstream>
#include <iomanip>

using namespace Makina;
using namespace Makina::MathHelper;
using namespace std;

SceneVoronoizator::SceneVoronoizator(D3DAppValues *values)
: mValues(values),
mTexGen(NULL),
mBmp(NULL),
mVoronoiSRV(NULL),
mVoronoiUAV(NULL),
mStagingColorTex(NULL),
mStagingDepthTex(NULL),
mStagingVoronoiTex(NULL)
{
	mTexGen = new TextureGenerator(values);
	mBmp = new Bitmap(values, NULL, 0.0f, 0.0f, 0.7f, 0.7f, Colors::White);

	OnResize();
}

SceneVoronoizator::~SceneVoronoizator()
{
	if (mTexGen) delete mTexGen;
	if (mBmp) delete mBmp;
	if (mVoronoiSRV) mVoronoiSRV->Release();
	if (mVoronoiUAV) mVoronoiUAV->Release();
	if (mStagingColorTex) mStagingColorTex->Release();
	if (mStagingDepthTex) mStagingDepthTex->Release();
	if (mStagingVoronoiTex) mStagingVoronoiTex->Release();
}

void SceneVoronoizator::OnResize()
{
	int numPoints = 2000;
	mPoints.clear();
	mPoints.reserve(numPoints);
	Voronoi2DPoint vPoint;
	for (int i = 0; i < numPoints; ++i)
	{
		vPoint.mColor = XMFLOAT3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
		vPoint.mPos = XMFLOAT2((float)rand() / RAND_MAX * mValues->mClientWidth, (float)rand() / RAND_MAX * mValues->mClientHeight);
		mPoints.push_back(vPoint);
	}

	if (mVoronoiSRV) mVoronoiSRV->Release();
	if (mVoronoiUAV) mVoronoiUAV->Release();
	mTexGen->BuildTextures(DXGI_FORMAT_R8G8B8A8_UNORM, mValues->mClientWidth, mValues->mClientHeight, &mVoronoiSRV, &mVoronoiUAV);

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = mValues->mClientWidth;
	texDesc.Height = mValues->mClientHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_STAGING;
	texDesc.BindFlags = 0;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	texDesc.MiscFlags = 0;

	if (mStagingColorTex) mStagingColorTex->Release();
	if (mStagingDepthTex) mStagingDepthTex->Release();
	if (mStagingVoronoiTex) mStagingVoronoiTex->Release();

	HRESULT hr;
	hr = mValues->md3dDevice->CreateTexture2D(&texDesc, 0, &mStagingColorTex);
	if (FAILED(hr)) throw;
	hr = mValues->md3dDevice->CreateTexture2D(&texDesc, 0, &mStagingDepthTex);
	if (FAILED(hr)) throw;
	hr = mValues->md3dDevice->CreateTexture2D(&texDesc, 0, &mStagingVoronoiTex);
	if (FAILED(hr)) throw;
}

void SceneVoronoizator::Draw(ID3D11ShaderResourceView *colorSRV, ID3D11ShaderResourceView *depthSRV)
{
	ID3D11Resource *srcDepth, *srcColor;
	depthSRV->GetResource(&srcDepth);
	colorSRV->GetResource(&srcColor);
	mValues->md3dImmediateContext->CopyResource(mStagingDepthTex, srcDepth);
	mValues->md3dImmediateContext->CopyResource(mStagingColorTex, srcColor);
	srcDepth->Release();
	srcColor->Release();

	D3D11_MAPPED_SUBRESOURCE mappedResourceDepth, mappedResourceColor;
	mValues->md3dImmediateContext->Map(mStagingDepthTex, 0, D3D11_MAP_READ, 0, &mappedResourceDepth);
	mValues->md3dImmediateContext->Map(mStagingColorTex, 0, D3D11_MAP_READ, 0, &mappedResourceColor);

	vector<Voronoi2DPoint> points;
	for (UINT i = 0; i < mPoints.size(); ++i)
	{
		unsigned char *colorRPt = (unsigned char *)mappedResourceDepth.pData;
		int x = (int)mPoints[i].mPos.x * 4;
		colorRPt += x + (int)mPoints[i].mPos.y * mappedResourceDepth.RowPitch;
		unsigned char colorR = *(colorRPt + 0);
		unsigned char colorG = *(colorRPt + 1);
		unsigned char colorB = *(colorRPt + 2);
		unsigned char colorA = *(colorRPt + 3);
		float depth = colorB + colorA / 255.0f;
		float probability = 1.0f / (depth * 0.01f);
		float randVal = ((int)((mPoints[i].mPos.x + mPoints[i].mPos.y) * 10000) % 40) * 0.4f;
		if (probability > randVal)
		{
			points.push_back(mPoints[i]);
		}
	}

	for (UINT i = 0; i < points.size(); ++i)
	{
		// map index of this point to the RGB channels
		int r = i % 100;
		int g = ((i % 10000) - (i % 100)) / 100;
		int b = (i - i % 10000) / 10000;
		points[i].mColor = XMFLOAT3( 
			r / 256.0f,
			g / 256.0f,
			b / 256.0f);
	}

	mTexGen->Generate2DVoronoiMap(mValues->mClientWidth, mValues->mClientHeight, &mVoronoiUAV, points);
	ID3D11Resource *voronoiTex;
	mVoronoiSRV->GetResource(&voronoiTex);




	D3D11_MAPPED_SUBRESOURCE mappedResourceVoronoi;
	mValues->md3dImmediateContext->CopyResource(mStagingVoronoiTex, voronoiTex);
	mValues->md3dImmediateContext->Map(mStagingVoronoiTex, 0, D3D11_MAP_READ, 0, &mappedResourceVoronoi);
	// clear points
	vector<UINT> pointsA;
	pointsA.resize(points.size());
	for (UINT i = 0; i < points.size(); ++i)
	{
		points[i].mColor = XMFLOAT3(0.0f, 0.0f, 0.0f);
		pointsA[i] = 0;
	}
	for (UINT x = 0; x < mValues->mClientWidth; ++x)
	{
		for (UINT y = 0; y < mValues->mClientHeight; ++y)
		{
			unsigned char *colorRPt = (unsigned char *)mappedResourceColor.pData;
			colorRPt += x * 4 + y * mappedResourceColor.RowPitch;
			unsigned char colorR = *colorRPt;
			unsigned char colorG = *(colorRPt + 1);
			unsigned char colorB = *(colorRPt + 2);

			XMFLOAT3 color = XMFLOAT3(
				colorR / 256.0f,
				colorG / 256.0f,
				colorB / 256.0f);

			colorRPt = (unsigned char *)mappedResourceVoronoi.pData;
			colorRPt += x * 4 + y * mappedResourceVoronoi.RowPitch;
			colorR = *colorRPt;
			colorG = *(colorRPt + 1);
			colorB = *(colorRPt + 2);
			int index = colorB * 10000 + colorG * 100 + colorR;
			if ((UINT)index < points.size())
			{
				points[index].mColor += color;
				++pointsA[index];
			}
		}
	}

	for (UINT i = 0; i < points.size(); ++i)
	{
		if (pointsA[i])
		{
			points[i].mColor.x /= pointsA[i];
			points[i].mColor.y /= pointsA[i];
			points[i].mColor.z /= pointsA[i];
		}
		else
		{
			points[i].mColor.x = 1.0f;
			points[i].mColor.y = 0.0f;
			points[i].mColor.z = 0.0f;
		}
	}
	mValues->md3dImmediateContext->Unmap(mStagingVoronoiTex, 0);

	mTexGen->Generate2DVoronoiMap(mValues->mClientWidth, mValues->mClientHeight, &mVoronoiUAV, points);
	mBmp->SetSRV(mVoronoiSRV);
	mBmp->Draw();

	//// Saving current frame as jpg.
	//string file = "C:\\Users\\bojan\\Desktop\\lol\\images\\img_";
	//static int num = 1;
	//stringstream ss;
	//ss << setw(10) << setfill('0') << num++;
	//file += ss.str() + ".jpg";
	//wstring fileWS;
	//fileWS.assign(file.begin(), file.end());
	//D3DX11SaveTextureToFile(mValues->md3dImmediateContext, voronoiTex, D3DX11_IFF_JPG, &fileWS[0]);
	//voronoiTex->Release();
}