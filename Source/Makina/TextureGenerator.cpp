#include "TextureGenerator.h"
#include "Resource.h"
#include "Exceptions.h"
#include "MathHelper.h"
#include <vector>

#define CELLS_DIMENSION 8
#define GRADIENTS_DIMENSION (CELLS_DIMENSION + 1)

#define CUBES_DIMENSION 4
#define GRADIENTS_3D_DIMENSION (CUBES_DIMENSION + 1)

using namespace Makina;
using namespace std;

// 2D
void TextureGenerator::GenerateGradients(int seed, float *graPt)
{
	srand(seed);
	rand();
	rand();

	for (int i = 0; i < GRADIENTS_DIMENSION; i++) // y
		for (int j = 0; j < GRADIENTS_DIMENSION; j++) // x
		{
			float x = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
			float y = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
			float length = sqrt(x*x + y*y);
			x /= length;
			y /= length;

			graPt[i*GRADIENTS_DIMENSION*2 + j*2 + 0] = x;
			graPt[i*GRADIENTS_DIMENSION*2 + j*2 + 1] = y;
		}
}

void TextureGenerator::TileV(float *graPt, int height)
{
	for (int j = 0; j < GRADIENTS_DIMENSION; j++)
	{
		graPt[j*GRADIENTS_DIMENSION*2 + 2*(height-1) + 0] = graPt[j*GRADIENTS_DIMENSION*2 + 0];
		graPt[j*GRADIENTS_DIMENSION*2 + 2*(height-1) + 1] = graPt[j*GRADIENTS_DIMENSION*2 + 1];
	}
}

void TextureGenerator::TileU(float *graPt, int width)
{
	for (int i = 0; i < GRADIENTS_DIMENSION; i++)
	{
		graPt[(width-1)*(GRADIENTS_DIMENSION)*2 + i*2 + 0] = graPt[i*2 + 0];
		graPt[(width-1)*(GRADIENTS_DIMENSION)*2 + i*2 + 1] = graPt[i*2 + 1];
	}
}

// 3D
void TextureGenerator::Generate3DGradients(int seed, float *graPt)
{
	srand(seed);
	rand();
	rand();

	for (int i = 0; i < GRADIENTS_3D_DIMENSION; i++) // z
		for (int j = 0; j < GRADIENTS_3D_DIMENSION; j++) // y
			for (int k = 0; k < GRADIENTS_3D_DIMENSION; k++) // x
			{
				float x = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
				float y = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
				float z = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
				float length = sqrt(x*x + y*y + z*z);
				x /= length;
				y /= length;
				z /= length;

				graPt[i*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + j*GRADIENTS_3D_DIMENSION*3 + k*3 + 0] = x;
				graPt[i*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + j*GRADIENTS_3D_DIMENSION*3 + k*3 + 1] = y;
				graPt[i*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + j*GRADIENTS_3D_DIMENSION*3 + k*3 + 2] = z;
			}
}

void TextureGenerator::TileY(float *graPt, int y)
{
	for (int i = 0; i < GRADIENTS_3D_DIMENSION; i++) // z
		for (int k = 0; k < GRADIENTS_3D_DIMENSION; k++) // x
		{
			graPt[i*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + (y - 1)*GRADIENTS_3D_DIMENSION*3 + k*3 + 0] =
				graPt[i*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + k*3 + 0];

			graPt[i*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + (y - 1)*GRADIENTS_3D_DIMENSION*3 + k*3 + 1] =
				graPt[i*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + k*3 + 1];

			graPt[i*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + (y - 1)*GRADIENTS_3D_DIMENSION*3 + k*3 + 2] =
				graPt[i*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + k*3 + 2];
		}
}

void TextureGenerator::TileX(float *graPt, int x)
{
	for (int i = 0; i < GRADIENTS_3D_DIMENSION; i++) // z
		for (int j = 0; j < GRADIENTS_3D_DIMENSION; j++) // y
		{
			graPt[i*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + j*GRADIENTS_3D_DIMENSION*3 + (x - 1)*3 + 0] =
				graPt[i*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + j*GRADIENTS_3D_DIMENSION*3 + 0];

			graPt[i*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + j*GRADIENTS_3D_DIMENSION*3 + (x - 1)*3 + 1] =
				graPt[i*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + j*GRADIENTS_3D_DIMENSION*3 + 1];

			graPt[i*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + j*GRADIENTS_3D_DIMENSION*3 + (x - 1)*3 + 2] =
				graPt[i*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + j*GRADIENTS_3D_DIMENSION*3 + 2];
		}
}

void TextureGenerator::TileZ(float *graPt, int z)
{
	for (int j = 0; j < GRADIENTS_3D_DIMENSION; j++) // y
		for (int k = 0; k < GRADIENTS_3D_DIMENSION; k++) // x
		{
			graPt[(z - 1)*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + j*GRADIENTS_3D_DIMENSION*3 + k*3 + 0] =
				graPt[j*GRADIENTS_3D_DIMENSION*3 + k*3 + 0];

			graPt[(z - 1)*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + j*GRADIENTS_3D_DIMENSION*3 + k*3 + 1] =
				graPt[j*GRADIENTS_3D_DIMENSION*3 + k*3 + 1];

			graPt[(z - 1)*GRADIENTS_3D_DIMENSION*GRADIENTS_3D_DIMENSION*3 + j*GRADIENTS_3D_DIMENSION*3 + k*3 + 2] =
				graPt[j*GRADIENTS_3D_DIMENSION*3 + k*3 + 2];
		}
}

TextureGenerator::TextureGenerator(D3DAppValues *values)
	: Effect(values, L"Makina.dll", ID_TEXTUREGENERATOR)
{	
	mOutput							= mFX->GetVariableByName("gOutput")->AsUnorderedAccessView();
	mOutput3D						= mFX->GetVariableByName("gOutput3D")->AsUnorderedAccessView();
	mTexDimensions					= mFX->GetVariableByName("gTexDimensions")->AsScalar();
	mSeed							= mFX->GetVariableByName("gSeed")->AsScalar();
	mLacunarity						= mFX->GetVariableByName("gLacunarity")->AsScalar();
	mOctaves						= mFX->GetVariableByName("gOctaves")->AsScalar();
	mVoronoiPointsNum				= mFX->GetVariableByName("gVoronoiPointsNum")->AsScalar();

	// 2D
	mVoronoi2D						= mFX->GetTechniqueByName("Voronoi2D");
	mPerlinTech						= mFX->GetTechniqueByName("PerlinTech");
	mRidgedTech						= mFX->GetTechniqueByName("RidgedTech");
	mRidgedPerlinMix1Tech			= mFX->GetTechniqueByName("RidgedPerlinMix1Tech");

	mVoronoi2DPoints				= mFX->GetVariableByName("gVoronoi2DPoints")->AsShaderResource();
	mGradients						= mFX->GetVariableByName("gGradients")->AsScalar();
	mGradientsToUse					= mFX->GetVariableByName("gGradientsToUse")->AsScalar();

	// 3D
	mSphericalPerlinTech			= mFX->GetTechniqueByName("SphericalPerlinTech");
	mSphericalRidgedTech			= mFX->GetTechniqueByName("SphericalRidgedTech");
	mSphericalRidgedPerlinMix1Tech	= mFX->GetTechniqueByName("SphericalRidgedPerlinMix1Tech");
	m3DRidgedPerlinMix1Tech			= mFX->GetTechniqueByName("RidgedPerlinMix1Tech3D");
	m3DSphericalGradientTech		= mFX->GetTechniqueByName("SphericalGradientTech3D");

	m3DGradients					= mFX->GetVariableByName("g3DGradients")->AsScalar();
	m3DGradientsToUse				= mFX->GetVariableByName("g3DGradientsToUse")->AsScalar();
}

TextureGenerator::~TextureGenerator()
{

}

// Some useful textures
void TextureGenerator::GenerateGenericNormalMap(ID3D11ShaderResourceView **gNMapSRV)
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 256;
	texDesc.Height = 256;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {0};
	initData.SysMemPitch = 256*sizeof(XMCOLOR);

	XMCOLOR color[256*256];
	for(int i = 0; i < 256; ++i)
	{
		for(int j = 0; j < 256; ++j)
		{
			XMFLOAT3 v(MathHelper::RandF(), MathHelper::RandF(), MathHelper::RandF());

			color[i*256+j] = XMCOLOR(1.0f, 0.5f, 0.5f, 1.0f);
		}
	}

	initData.pSysMem = color;

	ID3D11Texture2D* tex = 0;
	HRESULT hr = mValues->md3dDevice->CreateTexture2D(&texDesc, &initData, &tex);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create texture (GenerateGenericNormalMap).");

	hr = mValues->md3dDevice->CreateShaderResourceView(tex, 0, gNMapSRV);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create texture SRV (GenerateGenericNormalMap).");

	// view saves a reference.
	ReleaseCOM(tex);
}

void TextureGenerator::GenerateGenericDiffuseMap(const XMCOLOR &color, ID3D11ShaderResourceView **gMapSRV)
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 1;
	texDesc.Height = 1;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {0};
	initData.SysMemPitch = sizeof(XMCOLOR);
	XMCOLOR colorData[] = {color};
	initData.pSysMem = colorData;

	ID3D11Texture2D* tex = 0;
	HRESULT hr = mValues->md3dDevice->CreateTexture2D(&texDesc, &initData, &tex);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create texture (GenerateGenericDiffuseMap).");

	hr = mValues->md3dDevice->CreateShaderResourceView(tex, 0, gMapSRV);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create texture SRV (GenerateGenericDiffuseMap).");

	// view saves a reference.
	ReleaseCOM(tex);
}

void TextureGenerator::GenerateColorPaletteMap(std::vector<XMCOLOR> &colors, ID3D11ShaderResourceView **gMapSRV)
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = colors.size();
	texDesc.Height = 1;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.SysMemPitch = colors.size() * sizeof(XMCOLOR);
	initData.pSysMem = &colors[0];

	ID3D11Texture2D* tex = 0;
	HRESULT hr = mValues->md3dDevice->CreateTexture2D(&texDesc, &initData, &tex);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create texture (GenerateGenericDiffuseMap).");

	hr = mValues->md3dDevice->CreateShaderResourceView(tex, 0, gMapSRV);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create texture SRV (GenerateGenericDiffuseMap).");

	// view saves a reference.
	ReleaseCOM(tex);
}

// Effects
void TextureGenerator::Generate2DVoronoiMap(UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **texSRV, vector<Voronoi2DPoint> &points)
{
	// points structured buffer
	D3D11_BUFFER_DESC inputDescPoints;
	inputDescPoints.Usage = D3D11_USAGE_DEFAULT;
	inputDescPoints.ByteWidth = sizeof(Voronoi2DPoint)* points.size();
	inputDescPoints.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	inputDescPoints.CPUAccessFlags = 0;
	inputDescPoints.StructureByteStride = sizeof(Voronoi2DPoint);
	inputDescPoints.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	D3D11_SUBRESOURCE_DATA vinitDataPoints;
	vinitDataPoints.pSysMem = &points[0];
	ID3D11Buffer* bufferPoints = 0;
	mValues->md3dDevice->CreateBuffer(&inputDescPoints, &vinitDataPoints, &bufferPoints);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.BufferEx.Flags = 0;
	srvDesc.BufferEx.NumElements = points.size();
	ID3D11ShaderResourceView *inputPointsSRV;
	mValues->md3dDevice->CreateShaderResourceView(bufferPoints, &srvDesc, &inputPointsSRV);

	// set as shader resource variable
	mVoronoi2DPoints->SetResource(inputPointsSRV);

	ID3D11ShaderResourceView *SRV = 0; //DXGI_FORMAT_R16_FLOAT
	ID3D11UnorderedAccessView *UAV = 0;
	BuildTextures(DXGI_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, &SRV, &UAV);

	// Set variables
	float texDimensions[] = { (float)texWidth, (float)texHeight };
	mTexDimensions->SetFloatArray(texDimensions, 0, 2);
	float texAspectRatio = (float)(texWidth) / (float)(texHeight);
	mVoronoiPointsNum->SetInt(points.size());

	// passes
	D3DX11_TECHNIQUE_DESC techDesc;
	mVoronoi2D->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mOutput->SetUnorderedAccessView(UAV);
		mVoronoi2D->GetPassByIndex(p)->Apply(0, mValues->md3dImmediateContext);

		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).
		UINT numGroupsX = (UINT)ceilf(texWidth / 256.0f);
		mValues->md3dImmediateContext->Dispatch(numGroupsX, texHeight, 1);
	}

	// Unbind the output texture from the CS for good housekeeping.
	ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
	mValues->md3dImmediateContext->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Unbind the input texture from the CS for good housekeeping.
	ID3D11ShaderResourceView* nullSRV[1] = { 0 };
	mValues->md3dImmediateContext->CSSetShaderResources(0, 1, nullSRV);

	ReleaseCOM(UAV);
	ReleaseCOM(bufferPoints);
	ReleaseCOM(inputPointsSRV);
	*texSRV = SRV;
}

void TextureGenerator::Generate2DVoronoiMap(UINT texWidth, UINT texHeight, ID3D11UnorderedAccessView **texUAV, std::vector<Voronoi2DPoint> &points)
{
	// points structured buffer
	D3D11_BUFFER_DESC inputDescPoints;
	inputDescPoints.Usage = D3D11_USAGE_DEFAULT;
	inputDescPoints.ByteWidth = sizeof(Voronoi2DPoint)* points.size();
	inputDescPoints.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	inputDescPoints.CPUAccessFlags = 0;
	inputDescPoints.StructureByteStride = sizeof(Voronoi2DPoint);
	inputDescPoints.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	D3D11_SUBRESOURCE_DATA vinitDataPoints;
	vinitDataPoints.pSysMem = &points[0];
	ID3D11Buffer* bufferPoints = 0;
	mValues->md3dDevice->CreateBuffer(&inputDescPoints, &vinitDataPoints, &bufferPoints);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.BufferEx.Flags = 0;
	srvDesc.BufferEx.NumElements = points.size();
	ID3D11ShaderResourceView *inputPointsSRV;
	mValues->md3dDevice->CreateShaderResourceView(bufferPoints, &srvDesc, &inputPointsSRV);

	// set as shader resource variable
	mVoronoi2DPoints->SetResource(inputPointsSRV);

	// Set variables
	float texDimensions[] = { (float)texWidth, (float)texHeight };
	mTexDimensions->SetFloatArray(texDimensions, 0, 2);
	float texAspectRatio = (float)(texWidth) / (float)(texHeight);
	mVoronoiPointsNum->SetInt(points.size());

	// passes
	D3DX11_TECHNIQUE_DESC techDesc;
	mVoronoi2D->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mOutput->SetUnorderedAccessView(*texUAV);
		mVoronoi2D->GetPassByIndex(p)->Apply(0, mValues->md3dImmediateContext);

		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).
		UINT numGroupsX = (UINT)ceilf(texWidth / 256.0f);
		mValues->md3dImmediateContext->Dispatch(numGroupsX, texHeight, 1);
	}

	// Unbind the output texture from the CS for good housekeeping.
	ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
	mValues->md3dImmediateContext->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Unbind the input texture from the CS for good housekeeping.
	ID3D11ShaderResourceView* nullSRV[1] = { 0 };
	mValues->md3dImmediateContext->CSSetShaderResources(0, 1, nullSRV);

	ReleaseCOM(bufferPoints);
	ReleaseCOM(inputPointsSRV);
}


// 2D noise
void TextureGenerator::GeneratePerlinNoise(UINT seed, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **perlinNoiseSRV)
{
	GenerateNoise(seed, texWidth, texHeight, perlinNoiseSRV, mPerlinTech);
}

void TextureGenerator::GenerateRidgedNoise(UINT seed, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **ridgedNoiseSRV)
{
	GenerateNoise(seed, texWidth, texHeight, ridgedNoiseSRV, mRidgedTech);
}

void TextureGenerator::GenerateRidgedPerlinMix1(UINT seed, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **noiseSRV)
{
	GenerateNoise(seed, texWidth, texHeight, noiseSRV, mRidgedPerlinMix1Tech);
}

void TextureGenerator::GenerateNoise(UINT seed, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **noiseSRV, ID3DX11EffectTechnique *tech)
{
	ID3D11ShaderResourceView *SRV = 0; //DXGI_FORMAT_R16_FLOAT
	ID3D11UnorderedAccessView *UAV = 0;
	BuildTextures(DXGI_FORMAT_R16_FLOAT, texWidth, texHeight, &SRV, &UAV);

	// Set variables
	float texDimensions[] = {(float)texWidth, (float)texHeight};
	mTexDimensions->SetFloatArray(texDimensions, 0, 2);
	mSeed->SetInt(seed);
	mLacunarity->SetFloat(2.0f);
	mOctaves->SetInt(10);
	float texAspectRatio = (float)(texWidth)/(float)(texHeight);
	int gradientsToUse[] = {GRADIENTS_DIMENSION, GRADIENTS_DIMENSION};
	if (texAspectRatio >= 1.0f)
		gradientsToUse[1] = (int)(GRADIENTS_DIMENSION / texAspectRatio);
	else
		gradientsToUse[0] = (int)(GRADIENTS_DIMENSION * texAspectRatio);
	mGradientsToUse->SetIntArray(gradientsToUse, 0, 2);

	float gradients[GRADIENTS_DIMENSION][GRADIENTS_DIMENSION][2];
	float *graPt = &gradients[0][0][0];
	GenerateGradients(seed, graPt);
	TileU(graPt, gradientsToUse[0]);
	TileV(graPt, gradientsToUse[1]);

	mGradients->SetFloatArray(graPt, 0, GRADIENTS_DIMENSION*GRADIENTS_DIMENSION*2);

	// First pass
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		mOutput->SetUnorderedAccessView(UAV);
		tech->GetPassByIndex(p)->Apply(0, mValues->md3dImmediateContext);

		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).
		UINT numGroupsX = (UINT)ceilf(texWidth / 256.0f);
		mValues->md3dImmediateContext->Dispatch(numGroupsX, texHeight, 1);
	}

	// Unbind the output texture from the CS for good housekeeping.
	ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
	mValues->md3dImmediateContext->CSSetUnorderedAccessViews( 0, 1, nullUAV, 0 );

	// Unbind the input texture from the CS for good housekeeping.
	ID3D11ShaderResourceView* nullSRV[1] = { 0 };
	mValues->md3dImmediateContext->CSSetShaderResources( 0, 1, nullSRV );

	ReleaseCOM(UAV);
	*noiseSRV = SRV;
}

void TextureGenerator::GenerateRandomTexture1D(ID3D11ShaderResourceView **randomSRV, UINT width)
{
	// 
	// Create the random data.
	//
	vector<XMFLOAT4> rndValues;

	for(UINT i = 0; i < width; ++i)
	{
		XMFLOAT4 a;
		a.x = MathHelper::RandF(-1.0f, 1.0f);
		a.y = MathHelper::RandF(-1.0f, 1.0f);
		a.z = MathHelper::RandF(-1.0f, 1.0f);
		a.w = MathHelper::RandF(-1.0f, 1.0f);

		rndValues.push_back(a);
	}

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = &rndValues[0];
	initData.SysMemPitch = width*sizeof(XMFLOAT4);
    initData.SysMemSlicePitch = 0;

	//
	// Create the texture.
	//
    D3D11_TEXTURE1D_DESC texDesc;
    texDesc.Width = width;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.ArraySize = 1;

	ID3D11Texture1D* randomTex = 0;
	HRESULT hr;
    hr = mValues->md3dDevice->CreateTexture1D(&texDesc, &initData, &randomTex);
	if(FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to create texture! (TextureGenerator::GenerateRandomTexture1D)"));

	//
	// Create the resource view.
	//
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texDesc.Format;
    viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
    viewDesc.Texture1D.MipLevels = texDesc.MipLevels;
	viewDesc.Texture1D.MostDetailedMip = 0;
	
    hr = mValues->md3dDevice->CreateShaderResourceView(randomTex, &viewDesc, randomSRV);
	if(FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to create SRV! (TextureGenerator::GenerateRandomTexture1D)"));

	ReleaseCOM(randomTex);
}

// 3D noise
void TextureGenerator::GenerateSphericalPerlinNoise(UINT seed, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **perlinNoiseSRV)
{
	GenerateSphericalNoise(seed, texWidth, texHeight, perlinNoiseSRV, mSphericalPerlinTech);
}

void TextureGenerator::GenerateSphericalRidgedNoise(UINT seed, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **perlinNoiseSRV)
{
	GenerateSphericalNoise(seed, texWidth, texHeight, perlinNoiseSRV, mSphericalRidgedTech);
}

void TextureGenerator::GenerateSphericalRidgedPerlinMix1(UINT seed, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **perlinNoiseSRV)
{
	GenerateSphericalNoise(seed, texWidth, texHeight, perlinNoiseSRV, mSphericalRidgedPerlinMix1Tech);
}

void TextureGenerator::Generate3DRidgedPerlinMix1(UINT seed, UINT texWidth, UINT texHeight, UINT texDepth, ID3D11ShaderResourceView **noiseSRV)
{
	Generate3DNoise(seed, texWidth, texHeight, texDepth, noiseSRV, m3DRidgedPerlinMix1Tech);
}

void TextureGenerator::Generate3DSphericalGradient(UINT seed, UINT texWidth, UINT texHeight, UINT texDepth, ID3D11ShaderResourceView **noiseSRV)
{
	ID3D11ShaderResourceView *SRV = 0; //DXGI_FORMAT_R16_FLOAT
	ID3D11UnorderedAccessView *UAV = 0;
	BuildTextures3D(DXGI_FORMAT_R16_FLOAT, texWidth, texHeight, texDepth, &SRV, &UAV);

	// Set variables
	float texDimensions[] = { (float)texWidth, (float)texHeight, (float)texDepth };
	mTexDimensions->SetFloatArray(texDimensions, 0, 3);
	mSeed->SetInt(seed);
	mLacunarity->SetFloat(2.0f);
	mOctaves->SetInt(10);

	int gradientsToUse[] = { GRADIENTS_3D_DIMENSION, GRADIENTS_3D_DIMENSION, GRADIENTS_3D_DIMENSION };
	m3DGradientsToUse->SetIntArray(gradientsToUse, 0, 3);

	float gradients[GRADIENTS_3D_DIMENSION][GRADIENTS_3D_DIMENSION][GRADIENTS_3D_DIMENSION][3];
	float *graPt = &gradients[0][0][0][0];
	Generate3DGradients(seed, graPt);
	TileX(graPt, gradientsToUse[0]);
	TileY(graPt, gradientsToUse[1]);
	TileZ(graPt, gradientsToUse[2]);

	m3DGradients->SetFloatArray(graPt, 0, GRADIENTS_3D_DIMENSION * GRADIENTS_3D_DIMENSION * GRADIENTS_3D_DIMENSION * 3);

	// First pass
	D3DX11_TECHNIQUE_DESC techDesc;
	m3DSphericalGradientTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mOutput3D->SetUnorderedAccessView(UAV);
		m3DSphericalGradientTech->GetPassByIndex(p)->Apply(0, mValues->md3dImmediateContext);

		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).
		UINT numGroupsX = (UINT)ceilf(texWidth / 256.0f);
		mValues->md3dImmediateContext->Dispatch(numGroupsX, texHeight, texDepth);
	}

	// Unbind the output texture from the CS for good housekeeping.
	ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
	mValues->md3dImmediateContext->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Unbind the input texture from the CS for good housekeeping.
	ID3D11ShaderResourceView* nullSRV[1] = { 0 };
	mValues->md3dImmediateContext->CSSetShaderResources(0, 1, nullSRV);

	ReleaseCOM(UAV);
	*noiseSRV = SRV;
}

void TextureGenerator::GenerateSphericalNoise(UINT seed, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **noiseSRV, ID3DX11EffectTechnique *tech)
{
	ID3D11ShaderResourceView *SRV = 0; //DXGI_FORMAT_R16_FLOAT
	ID3D11UnorderedAccessView *UAV = 0;
	BuildTextures(DXGI_FORMAT_R16_FLOAT, texWidth, texHeight, &SRV, &UAV);

	// Set variables
	float texDimensions[] = {(float)texWidth, (float)texHeight};
	mTexDimensions->SetFloatArray(texDimensions, 0, 2);
	mSeed->SetInt(seed);
	mLacunarity->SetFloat(2.0f);
	mOctaves->SetInt(10);

	int gradientsToUse[] = {GRADIENTS_3D_DIMENSION, GRADIENTS_3D_DIMENSION, GRADIENTS_3D_DIMENSION};
	m3DGradientsToUse->SetIntArray(gradientsToUse, 0, 3);

	float gradients[GRADIENTS_3D_DIMENSION][GRADIENTS_3D_DIMENSION][GRADIENTS_3D_DIMENSION][3];
	float *graPt = &gradients[0][0][0][0];
	Generate3DGradients(seed, graPt);
	TileX(graPt, gradientsToUse[0]);
	TileY(graPt, gradientsToUse[1]);
	TileZ(graPt, gradientsToUse[2]);

	m3DGradients->SetFloatArray(graPt, 0, GRADIENTS_3D_DIMENSION * GRADIENTS_3D_DIMENSION * GRADIENTS_3D_DIMENSION * 3);

	// First pass
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		mOutput->SetUnorderedAccessView(UAV);
		tech->GetPassByIndex(p)->Apply(0, mValues->md3dImmediateContext);

		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).
		UINT numGroupsX = (UINT)ceilf(texWidth / 256.0f);
		mValues->md3dImmediateContext->Dispatch(numGroupsX, texHeight, 1);
	}

	// Unbind the output texture from the CS for good housekeeping.
	ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
	mValues->md3dImmediateContext->CSSetUnorderedAccessViews( 0, 1, nullUAV, 0 );

	// Unbind the input texture from the CS for good housekeeping.
	ID3D11ShaderResourceView* nullSRV[1] = { 0 };
	mValues->md3dImmediateContext->CSSetShaderResources( 0, 1, nullSRV );

	ReleaseCOM(UAV);
	*noiseSRV = SRV;
}

void TextureGenerator::Generate3DNoise(UINT seed, UINT texWidth, UINT texHeight, UINT texDepth, ID3D11ShaderResourceView **noiseSRV, ID3DX11EffectTechnique *tech)
{
	ID3D11ShaderResourceView *SRV = 0; //DXGI_FORMAT_R16_FLOAT
	ID3D11UnorderedAccessView *UAV = 0;
	BuildTextures3D(DXGI_FORMAT_R16_FLOAT, texWidth, texHeight, texDepth, &SRV, &UAV);

	// Set variables
	float texDimensions[] = { (float)texWidth, (float)texHeight, (float)texDepth };
	mTexDimensions->SetFloatArray(texDimensions, 0, 3);
	mSeed->SetInt(seed);
	mLacunarity->SetFloat(2.0f);
	mOctaves->SetInt(10);

	int gradientsToUse[] = { GRADIENTS_3D_DIMENSION, GRADIENTS_3D_DIMENSION, GRADIENTS_3D_DIMENSION };
	m3DGradientsToUse->SetIntArray(gradientsToUse, 0, 3);

	float gradients[GRADIENTS_3D_DIMENSION][GRADIENTS_3D_DIMENSION][GRADIENTS_3D_DIMENSION][3];
	float *graPt = &gradients[0][0][0][0];
	Generate3DGradients(seed, graPt);
	TileX(graPt, gradientsToUse[0]);
	TileY(graPt, gradientsToUse[1]);
	TileZ(graPt, gradientsToUse[2]);

	m3DGradients->SetFloatArray(graPt, 0, GRADIENTS_3D_DIMENSION * GRADIENTS_3D_DIMENSION * GRADIENTS_3D_DIMENSION * 3);

	// First pass
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mOutput3D->SetUnorderedAccessView(UAV);
		tech->GetPassByIndex(p)->Apply(0, mValues->md3dImmediateContext);

		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).
		UINT numGroupsX = (UINT)ceilf(texWidth / 256.0f);
		mValues->md3dImmediateContext->Dispatch(numGroupsX, texHeight, texDepth);
	}

	// Unbind the output texture from the CS for good housekeeping.
	ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
	mValues->md3dImmediateContext->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Unbind the input texture from the CS for good housekeeping.
	ID3D11ShaderResourceView* nullSRV[1] = { 0 };
	mValues->md3dImmediateContext->CSSetShaderResources(0, 1, nullSRV);

	ReleaseCOM(UAV);
	*noiseSRV = SRV;
}

void TextureGenerator::BuildTextures(const DXGI_FORMAT &format, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **SRV, ID3D11UnorderedAccessView **UAV)
{
	// Note, compressed formats cannot be used for UAV.  We get error like:
	// ERROR: ID3D11Device::CreateTexture2D: The format (0x4d, BC3_UNORM) 
	// cannot be bound as an UnorderedAccessView, or cast to a format that
	// could be bound as an UnorderedAccessView.  Therefore this format 
	// does not support D3D11_BIND_UNORDERED_ACCESS.

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width     = texWidth;
	texDesc.Height    = texHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format    = format;
	texDesc.SampleDesc.Count   = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage     = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags      = 0;

	HRESULT hr;
	ID3D11Texture2D* tex = 0;
	hr = mValues->md3dDevice->CreateTexture2D(&texDesc, 0, &tex);
	if(FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to create texture! (TextureGenerator::BuildTextures)"));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	hr = mValues->md3dDevice->CreateShaderResourceView(tex, &srvDesc, SRV);
	if(FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to create shader resource view! (TextureGenerator::BuildTextures)"));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	hr = mValues->md3dDevice->CreateUnorderedAccessView(tex, &uavDesc, UAV);
	if(FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to create unordered access view! (TextureGenerator::BuildTextures)"));

	// Views save a reference to the texture so we can release our reference.
	ReleaseCOM(tex);
}

void TextureGenerator::BuildTextures3D(const DXGI_FORMAT &format, UINT texWidth, UINT texHeight, UINT texDepth, ID3D11ShaderResourceView **SRV, ID3D11UnorderedAccessView **UAV)
{
	// Note, compressed formats cannot be used for UAV.  We get error like:
	// ERROR: ID3D11Device::CreateTexture3D: The format (0x4d, BC3_UNORM) 
	// cannot be bound as an UnorderedAccessView, or cast to a format that
	// could be bound as an UnorderedAccessView.  Therefore this format 
	// does not support D3D11_BIND_UNORDERED_ACCESS.

	D3D11_TEXTURE3D_DESC texDesc;
	texDesc.Width = texWidth;
	texDesc.Height = texHeight;
	texDesc.Depth = texDepth;
	texDesc.MipLevels = 1;
	texDesc.Format = format;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	HRESULT hr;
	ID3D11Texture3D* tex = 0;
	hr = mValues->md3dDevice->CreateTexture3D(&texDesc, 0, &tex);
	if (FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to create texture! (TextureGenerator::BuildTextures3D)"));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Texture3D.MostDetailedMip = 0;
	srvDesc.Texture3D.MipLevels = 1;
	hr = mValues->md3dDevice->CreateShaderResourceView(tex, &srvDesc, SRV);
	if (FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to create shader resource view! (TextureGenerator::BuildTextures3D)"));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Texture3D.MipSlice = 0;
	uavDesc.Texture3D.FirstWSlice = 0;
	uavDesc.Texture3D.WSize = texDepth;
	hr = mValues->md3dDevice->CreateUnorderedAccessView(tex, &uavDesc, UAV);
	if (FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to create unordered access view! (TextureGenerator::BuildTextures3D)"));

	// Views save a reference to the texture so we can release our reference.
	ReleaseCOM(tex);
}