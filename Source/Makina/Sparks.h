#ifndef SPARKS_H
#define SPARKS_H

#include "ParticleSystem.h"

namespace Makina
{
	class Sparks : public ParticleSystem
	{
	public:
		__declspec(dllexport) Sparks(D3DAppValues *values, ID3D11ShaderResourceView* randomTexSRV);
		__declspec(dllexport) ~Sparks();

		__declspec(dllexport) void Draw();

	private:
		__declspec(dllexport) void BuildVertexLayout();
	};
}

#endif