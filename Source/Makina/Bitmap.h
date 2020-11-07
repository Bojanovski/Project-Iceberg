
#ifndef BITMAP_H
#define BITMAP_H

#include "Object2D.h"

namespace Makina
{
	class Bitmap : public Object2D
	{
	public:
		__declspec(dllexport) Bitmap(D3DAppValues *values, ID3D11ShaderResourceView *bitmapTexSRV, float posX, float posY, float sizeX, float sizeY, FXMVECTOR color);
		__declspec(dllexport) ~Bitmap();

		__declspec(dllexport) void Draw();
		__declspec(dllexport) void ChangeProp(float posX, float posY, float sizeX, float sizeY, FXMVECTOR color);

	private:
		__declspec(dllexport) void BuildGeometryBuffers();
		__declspec(dllexport) void CreateInputLayout();
	};
}

#endif
