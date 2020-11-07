#ifndef FULLSCREENQUAD_H
#define FULLSCREENQUAD_H

#include "Object2D.h"

namespace Makina
{
	class FullScreenQuad : public Object2D
	{
	public:
		__declspec(dllexport) FullScreenQuad(D3DAppValues *values);
		__declspec(dllexport) ~FullScreenQuad();

		__declspec(dllexport) void Draw();

	private:
		__declspec(dllexport) void BuildGeometryBuffers();
		__declspec(dllexport) void CreateInputLayout();
	};
}

#endif