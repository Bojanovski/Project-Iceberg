#ifndef FIRE_H
#define FIRE_H

#include "ParticleSystem.h"

namespace Makina
{
	class Fire : public ParticleSystem
	{
	public:
		__declspec(dllexport) Fire(D3DAppValues *values, ID3D11ShaderResourceView* randomTexSRV, ID3D11ShaderResourceView* fireTexArraySRV);
		__declspec(dllexport) ~Fire();

		__declspec(dllexport) void Draw();

	private:
		__declspec(dllexport) void BuildVertexLayout();

		ID3D11ShaderResourceView* mFireTexArraySRV;
	};
}

#endif