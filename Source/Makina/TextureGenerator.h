
#ifndef TEXTUREGENERATOR_H
#define TEXTUREGENERATOR_H

#include "Effect.h"
#include "D3DAppValues.h"
#include <vector>

namespace Makina
{
	struct Voronoi2DPoint
	{
		XMFLOAT3 mColor;
		XMFLOAT2 mPos;
	};

	class TextureGenerator : public Effect
	{
	public:
		__declspec(dllexport) TextureGenerator(D3DAppValues *values);
		__declspec(dllexport) ~TextureGenerator();

		__declspec(dllexport) void GenerateGenericNormalMap(ID3D11ShaderResourceView **gNMapSRV);
		__declspec(dllexport) void GenerateGenericDiffuseMap(const XMCOLOR &color, ID3D11ShaderResourceView **gMapSRV);
		__declspec(dllexport) void GenerateColorPaletteMap(std::vector<XMCOLOR> &colors, ID3D11ShaderResourceView **gMapSRV);

		// Generates new 2D Voronoi map on newly allocated space (texture).
		__declspec(dllexport) void Generate2DVoronoiMap(UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **texSRV, std::vector<Voronoi2DPoint> &points);
		// Generates new 2D Voronoi map on an old texture.
		__declspec(dllexport) void Generate2DVoronoiMap(UINT texWidth, UINT texHeight, ID3D11UnorderedAccessView **texUAV, std::vector<Voronoi2DPoint> &points);

		__declspec(dllexport) void GeneratePerlinNoise(UINT seed, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **perlinNoiseSRV);
		__declspec(dllexport) void GenerateRidgedNoise(UINT seed, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **ridgedNoiseSRV);
		__declspec(dllexport) void GenerateRidgedPerlinMix1(UINT seed, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **noiseSRV);

		__declspec(dllexport) void GenerateRandomTexture1D(ID3D11ShaderResourceView **randomSRV, UINT width);

		__declspec(dllexport) void GenerateSphericalPerlinNoise(UINT seed, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **perlinNoiseSRV);
		__declspec(dllexport) void GenerateSphericalRidgedNoise(UINT seed, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **ridgedNoiseSRV);
		__declspec(dllexport) void GenerateSphericalRidgedPerlinMix1(UINT seed, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **noiseSRV);
		__declspec(dllexport) void Generate3DRidgedPerlinMix1(UINT seed, UINT texWidth, UINT texHeight, UINT texDepth, ID3D11ShaderResourceView **noiseSRV);
		__declspec(dllexport) void Generate3DSphericalGradient(UINT seed, UINT texWidth, UINT texHeight, UINT texDepth, ID3D11ShaderResourceView **noiseSRV);

		__declspec(dllexport) void BuildTextures(const DXGI_FORMAT &format, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **SRV, ID3D11UnorderedAccessView **UAV);
		__declspec(dllexport) void BuildTextures3D(const DXGI_FORMAT &format, UINT texWidth, UINT texHeight, UINT texDepth, ID3D11ShaderResourceView **SRV, ID3D11UnorderedAccessView **UAV);

		// 2D
		__declspec(dllexport) static void GenerateGradients(int seed, float *graPt);
		__declspec(dllexport) static void TileV(float *graPt, int height);
		__declspec(dllexport) static void TileU(float *graPt, int width);

		// 3D
		__declspec(dllexport) static void Generate3DGradients(int seed, float *graPt);
		__declspec(dllexport) static void TileY(float *graPt, int y);
		__declspec(dllexport) static void TileX(float *graPt, int x);
		__declspec(dllexport) static void TileZ(float *graPt, int z);

	private:
		__declspec(dllexport) void GenerateNoise(UINT seed, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **noiseSRV, ID3DX11EffectTechnique *tech);
		__declspec(dllexport) void GenerateSphericalNoise(UINT seed, UINT texWidth, UINT texHeight, ID3D11ShaderResourceView **noiseSRV, ID3DX11EffectTechnique *tech);
		__declspec(dllexport) void Generate3DNoise(UINT seed, UINT texWidth, UINT texHeight, UINT texDepth, ID3D11ShaderResourceView **noiseSRV, ID3DX11EffectTechnique *tech);

		ID3DX11EffectUnorderedAccessViewVariable* mOutput;
		ID3DX11EffectUnorderedAccessViewVariable* mOutput3D;
		ID3DX11EffectScalarVariable *mTexDimensions;
		ID3DX11EffectScalarVariable *mSeed;
		ID3DX11EffectScalarVariable *mLacunarity;
		ID3DX11EffectScalarVariable *mOctaves;
		ID3DX11EffectScalarVariable *mVoronoiPointsNum;

		// 2D
		ID3DX11EffectTechnique* mVoronoi2D;
		ID3DX11EffectTechnique* mPerlinTech;
		ID3DX11EffectTechnique* mRidgedTech;
		ID3DX11EffectTechnique* mRidgedPerlinMix1Tech;
		ID3DX11EffectShaderResourceVariable *mVoronoi2DPoints;
		ID3DX11EffectScalarVariable *mGradients;
		ID3DX11EffectScalarVariable *mGradientsToUse;

		// 3D
		ID3DX11EffectTechnique* mSphericalPerlinTech;
		ID3DX11EffectTechnique* mSphericalRidgedTech;
		ID3DX11EffectTechnique* mSphericalRidgedPerlinMix1Tech;
		ID3DX11EffectTechnique* m3DRidgedPerlinMix1Tech;
		ID3DX11EffectTechnique* m3DSphericalGradientTech;
		ID3DX11EffectScalarVariable *m3DGradients;
		ID3DX11EffectScalarVariable *m3DGradientsToUse;
	};
}

#endif
