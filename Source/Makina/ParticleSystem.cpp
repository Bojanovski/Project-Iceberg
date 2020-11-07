#include "ParticleSystem.h"
#include "D3DUtilities.h"
#include "Resource.h"
#include <string>
#include "GameTimer.h"

using namespace Makina;
using namespace std;

ParticleSystem::ParticleSystem(D3DAppValues *values, ID3D11ShaderResourceView* randomTexSRV, UINT maxParticles)
	: mValues(values),
	mRandomTexSRV(randomTexSRV),
	mCam((Camera *) values->mCamera),
	mInitVB(0),
	mDrawVB(0),
	mStreamOutVB(0),
	mMaxParticles(maxParticles),
	mWind(0.0f, 0.0f, 0.0f),
	mGravity(0.0f, -9.81f, 0.0f)
{
	mFirstRun = true;
	mEmitting = true;
	mGameTime = 0.0f;
	mTimeStep = 0.0f;
	mAge      = 0.0f;

	mEmitPosW = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mEmitDirW = XMFLOAT3(0.0f, 2.0f, 0.0f);

	BuildGeometryBuffers();

	mEmit				 = mValues->mParticlesFX->GetVariableByName("gEmitting")->AsScalar();
	mViewProjVar		 = mValues->mParticlesFX->GetVariableByName("gViewProj")->AsMatrix();
	mGameTimeVar		 = mValues->mParticlesFX->GetVariableByName("gGameTime")->AsScalar();
	mTimeStepVar		 = mValues->mParticlesFX->GetVariableByName("gTimeStep")->AsScalar();
	mEyePosWVar			 = mValues->mParticlesFX->GetVariableByName("gEyePosW")->AsVector();
	mEmitPosWVar		 = mValues->mParticlesFX->GetVariableByName("gEmitPosW")->AsVector();
	mEmitDirWVar		 = mValues->mParticlesFX->GetVariableByName("gEmitDirW")->AsVector();
	mWindVar			 = mValues->mParticlesFX->GetVariableByName("gWind")->AsVector();
	mGravityVar			 = mValues->mParticlesFX->GetVariableByName("gGravity")->AsVector();
	mTexArrayVar		 = mValues->mParticlesFX->GetVariableByName("gTexArray")->AsShaderResource();
	mRandomTexVar		 = mValues->mParticlesFX->GetVariableByName("gRandomTex")->AsShaderResource();
}

ParticleSystem::~ParticleSystem()
{
	mInitVB->Release();
	mDrawVB->Release();
	mStreamOutVB->Release();
}

void ParticleSystem::Reset()
{
	mFirstRun = true;
	mAge      = 0.0f;
}

void ParticleSystem::UpdateParameters()
{
	XMMATRIX vp = mCam->ViewProj();
	mGameTime = mValues->mGameTimer->TotalTime();
	mTimeStep = mValues->mGameTimer->DeltaTime();

	mAge += ((GameTimer *)mValues->mGameTimer)->DeltaTime();

	//
	// Set constants.
	//
	mViewProjVar->SetMatrix(reinterpret_cast<float*>(&vp));
	mGameTimeVar->SetFloat(mGameTime);
	mTimeStepVar->SetFloat(mTimeStep);
	mEmit->SetBool(mEmitting);
	mEyePosWVar->SetRawValue(&mCam->GetPosition(), 0, sizeof(XMFLOAT3));
	mEmitPosWVar->SetRawValue(&mEmitPosW, 0, sizeof(XMFLOAT3));
	mEmitDirWVar->SetRawValue(&mEmitDirW, 0, sizeof(XMFLOAT3));
	mWindVar->SetRawValue(&mWind, 0, sizeof(XMFLOAT3));
	mGravityVar->SetRawValue(&mGravity, 0, sizeof(XMFLOAT3));
	mRandomTexVar->SetResource(mRandomTexSRV);
}

void ParticleSystem::BuildGeometryBuffers()
{
	//
	// Create the buffer to kick-off the particle system.
	//

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(VertexParticle) * 1;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// The initial particle emitter has type 0 and age 0.  The rest
	// of the particle attributes do not apply to an emitter.
	VertexParticle p;
	ZeroMemory(&p, sizeof(VertexParticle));
	p.Age  = 0.0f;
	p.Type = 0; 

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &p;

	HRESULT hr;
	hr = mValues->md3dDevice->CreateBuffer(&vbd, &vinitData, &mInitVB);
	if(FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to create mInitVB vertex buffer in ParticleSystem!"));

	//
	// Create the ping-pong buffers for stream-out and drawing.
	//
	vbd.ByteWidth = sizeof(VertexParticle) * mMaxParticles;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;

	hr = mValues->md3dDevice->CreateBuffer(&vbd, 0, &mDrawVB);
	if(FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to create mInitVB vertex buffer in ParticleSystem!"));


	hr = mValues->md3dDevice->CreateBuffer(&vbd, 0, &mStreamOutVB);
	if(FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to create mInitVB vertex buffer in ParticleSystem!"));
}