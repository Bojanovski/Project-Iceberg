#ifndef PARTICLESYSTEMMANAGER_H
#define PARTICLESYSTEMMANAGER_H

#include "D3DAppValues.h"
#include "ContentLoader.h"
#include "Camera.h"
#include "Effect.h"
#include "ParticleSystem.h"
#include "Sparks.h"
#include "Fire.h"
#include <vector>

namespace Makina
{
	class ParticleSystemManager
	{
	public:
		__declspec(dllexport) ParticleSystemManager(D3DAppValues *values, ContentLoader *cl);
		__declspec(dllexport) ~ParticleSystemManager();

		__declspec(dllexport) void Draw();
		__declspec(dllexport) Sparks *AddSparks();
		__declspec(dllexport) Fire *AddFire(std::vector<std::wstring> &paths);

	private:
		D3DAppValues *mValues;
		// Content loader
		ContentLoader *mCL;

		ID3D11ShaderResourceView* mRandomTexSRV;

		// All the particle systems.
		std::vector<ParticleSystem *> mPS;
	};
}

#endif