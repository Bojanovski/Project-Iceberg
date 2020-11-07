
#include "ParticleSystemManager.h"
#include "TextureGenerator.h"
#include "TextureLoader.h"

using namespace Makina;
using namespace std;

ParticleSystemManager::ParticleSystemManager(D3DAppValues *values, ContentLoader *cl)
: mValues(values),
mCL(cl)
{
	TextureGenerator texGen(values);
	texGen.GenerateRandomTexture1D(&mRandomTexSRV, 1024);
}

ParticleSystemManager::~ParticleSystemManager()
{
	for (auto ps : mPS) delete ps;

	mRandomTexSRV->Release();
	if (mCL->GetTextureLoader()->ContainsTexture(L"fireTexArray"))
		mCL->GetTextureLoader()->RemoveTexture(L"fireTexArray");
}

void ParticleSystemManager::Draw()
{
	for (auto ps : mPS) ps->Draw();
}

Sparks *ParticleSystemManager::AddSparks()
{ 
	Sparks *sPt = new Sparks(mValues, mRandomTexSRV);
	mPS.push_back(sPt);
	return sPt;
}

Fire *ParticleSystemManager::AddFire(vector<wstring> &paths)
{
	mCL->GetTextureLoader()->LoadSRVArray(L"fireTexArray", paths);
	ID3D11ShaderResourceView *texArraySRV = mCL->GetTextureLoader()->GetSRV(L"fireTexArray");
	Fire *sPt = new Fire(mValues, mRandomTexSRV, texArraySRV);
	mPS.push_back(sPt);
	return sPt;
}