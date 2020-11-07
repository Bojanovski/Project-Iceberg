#include "Fire.h"
#include "D3DUtilities.h"
#include "Resource.h"
#include "TextureGenerator.h"
#include <string>

using namespace Makina;
using namespace std;

Fire::Fire(D3DAppValues *values, ID3D11ShaderResourceView* randomTexSRV, ID3D11ShaderResourceView* fireTexArraySRV)
: ParticleSystem(values, randomTexSRV, 3000),
mFireTexArraySRV(fireTexArraySRV)
{
	mParticleUpdate = mValues->mParticlesFX->GetTechniqueByName("FireUpdate");
	mParticleDraw = mValues->mParticlesFX->GetTechniqueByName("FireDraw");

	BuildVertexLayout();
}

Fire::~Fire()
{
	mInputLayout->Release();
}

void Fire::Draw()
{
	ParticleSystem::UpdateParameters();
	mTexArrayVar->SetResource(mFireTexArraySRV);

	//
	// Set IA stage.
	//
	mValues->md3dImmediateContext->IASetInputLayout(mInputLayout);
	mValues->md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(VertexParticle);
	UINT offset = 0;

	// On the first pass, use the initialization VB.  Otherwise, use
	// the VB that contains the current particle list.
	if (mFirstRun)
		mValues->md3dImmediateContext->IASetVertexBuffers(0, 1, &mInitVB, &stride, &offset);
	else
		mValues->md3dImmediateContext->IASetVertexBuffers(0, 1, &mDrawVB, &stride, &offset);

	//
	// Draw the current particle list using stream-out only to update them.  
	// The updated vertices are streamed-out to the target VB. 
	//
	mValues->md3dImmediateContext->SOSetTargets(1, &mStreamOutVB, &offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	mParticleUpdate->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mParticleUpdate->GetPassByIndex(p)->Apply(0, mValues->md3dImmediateContext);

		if (mFirstRun)
		{
			mValues->md3dImmediateContext->Draw(1, 0);
			mFirstRun = false;
		}
		else
		{
			mValues->md3dImmediateContext->DrawAuto();
		}
	}

	// done streaming-out--unbind the vertex buffer
	ID3D11Buffer* bufferArray[1] = { 0 };
	mValues->md3dImmediateContext->SOSetTargets(1, bufferArray, &offset);

	// ping-pong the vertex buffers
	swap(mDrawVB, mStreamOutVB);

	//
	// Draw the updated particle system we just streamed-out. 
	//
	mValues->md3dImmediateContext->IASetVertexBuffers(0, 1, &mDrawVB, &stride, &offset);

	mParticleDraw->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mParticleDraw->GetPassByIndex(p)->Apply(0, mValues->md3dImmediateContext);

		mValues->md3dImmediateContext->DrawAuto();
	}
}

void Fire::BuildVertexLayout()
{
	//Create input layout
	D3DX11_PASS_DESC passDesc;
	mParticleUpdate->GetPassByIndex(0)->GetDesc(&passDesc);
	HRESULT hr = mValues->md3dDevice->CreateInputLayout(ParticleDesc, 5, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create input layout in ParticleSystem!");
}