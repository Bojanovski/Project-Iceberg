
#ifndef SCENEVORONOIZATOR_H
#define SCENEVORONOIZATOR_H

#include <D3DAppValues.h>
#include <Bitmap.h>
#include <TextureGenerator.h>
#include <vector>

class SceneVoronoizator
{
public:
	SceneVoronoizator(Makina::D3DAppValues *values);
	~SceneVoronoizator();

	void Draw(ID3D11ShaderResourceView *colorSRV, ID3D11ShaderResourceView *depthSRV);
	void OnResize();

private:
	
	Makina::D3DAppValues *mValues;
	Makina::TextureGenerator *mTexGen;
	Makina::Bitmap *mBmp;
	std::vector<Makina::Voronoi2DPoint> mPoints;

	ID3D11ShaderResourceView *mVoronoiSRV;
	ID3D11UnorderedAccessView *mVoronoiUAV;

	ID3D11Texture2D* mStagingColorTex;
	ID3D11Texture2D* mStagingDepthTex;
	ID3D11Texture2D* mStagingVoronoiTex;
};

#endif
