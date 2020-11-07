
#include "BlurEffect.h"
#include "Exceptions.h"
#include "Resource.h"
#include <string>

using namespace Makina;
using namespace std;

BlurEffect::BlurEffect(D3DAppValues *values, const DXGI_FORMAT &Format)
	: Effect(values, L"Makina.dll", ID_BLUR),
	mFormat(Format),
	mBlurredOutputTexSRV(NULL),
	mBlurredOutputTexUAV(NULL)
{
	mHorzBlurTech = mFX->GetTechniqueByName("HorzBlur");
	mVertBlurTech = mFX->GetTechniqueByName("VertBlur");

	mWeights      = mFX->GetVariableByName("gWeights")->AsScalar();
	mInputMap     = mFX->GetVariableByName("gInput")->AsShaderResource();
	mOutputMap    = mFX->GetVariableByName("gOutput")->AsUnorderedAccessView();

	OnResize();
	SetGaussianWeights(2);
}

BlurEffect::~BlurEffect()
{
	mBlurredOutputTexSRV->Release();
	mBlurredOutputTexUAV->Release();
}

void BlurEffect::OnResize()
{
	// Start fresh.
	if (mBlurredOutputTexSRV) mBlurredOutputTexSRV->Release();
	if (mBlurredOutputTexUAV) mBlurredOutputTexUAV->Release();

	// Note, compressed formats cannot be used for UAV.  We get error like:
	// ERROR: ID3D11Device::CreateTexture2D: The format (0x4d, BC3_UNORM) 
	// cannot be bound as an UnorderedAccessView, or cast to a format that
	// could be bound as an UnorderedAccessView.  Therefore this format 
	// does not support D3D11_BIND_UNORDERED_ACCESS.

	D3D11_TEXTURE2D_DESC blurredTexDesc;
	blurredTexDesc.Width     = mValues->mClientWidth;
	blurredTexDesc.Height    = mValues->mClientHeight;
	blurredTexDesc.MipLevels = 1;
	blurredTexDesc.ArraySize = 1;
	blurredTexDesc.Format    = mFormat;
	blurredTexDesc.SampleDesc.Count   = 1;
	blurredTexDesc.SampleDesc.Quality = 0;
	blurredTexDesc.Usage     = D3D11_USAGE_DEFAULT;
	blurredTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	blurredTexDesc.CPUAccessFlags = 0;
	blurredTexDesc.MiscFlags      = 0;

	HRESULT hr;
	ID3D11Texture2D* blurredTex = 0;
	hr = mValues->md3dDevice->CreateTexture2D(&blurredTexDesc, 0, &blurredTex);
	if(FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to create texture! (BlufEffect)"));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = mFormat;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	hr = mValues->md3dDevice->CreateShaderResourceView(blurredTex, &srvDesc, &mBlurredOutputTexSRV);
	if(FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to create shader resource view!"));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = mFormat;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	hr = mValues->md3dDevice->CreateUnorderedAccessView(blurredTex, &uavDesc, &mBlurredOutputTexUAV);
	if(FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to create unordered access view!"));

	// Views save a reference to the texture so we can release our reference.
	blurredTex->Release();
}

void BlurEffect::SetGaussianWeights(float sigma)
{
	UINT length = 11;
	float d = 2.0f*sigma*sigma;

	float *weights = new float[length];
	float sum = 0.0f;
	for(UINT i = 0; i < length; ++i)
	{
		float x = (float)i - (float)(length / 2);
		weights[i] = expf(-x*x/d);

		sum += weights[i];
	}

	// Divide by the sum so all the weights add up to 1.0.
	for(UINT i = 0; i < length; ++i)
	{
		weights[i] /= sum;
	}

	mWeights->SetFloatArray(weights, 0, length);
	delete [] weights;
}

void BlurEffect::Blur(ID3D11ShaderResourceView* input, ID3D11UnorderedAccessView* output, int blurCount)
{
	ID3D11DeviceContext *deviceContext = mValues->md3dImmediateContext;

	//
	// Run the compute shader to blur the offscreen texture.
	// 

	for(int i = 0; i < blurCount; ++i)
	{
		// HORIZONTAL blur pass.
		D3DX11_TECHNIQUE_DESC techDesc;
		mHorzBlurTech->GetDesc( &techDesc );
		for(UINT p = 0; p < techDesc.Passes; ++p)
		{
			mInputMap->SetResource(input);
			mOutputMap->SetUnorderedAccessView(mBlurredOutputTexUAV);
			mHorzBlurTech->GetPassByIndex(p)->Apply(0, deviceContext);

			// How many groups do we need to dispatch to cover a row of pixels, where each
			// group covers 256 pixels (the 256 is defined in the ComputeShader).
			UINT numGroupsX = (UINT)ceilf(mValues->mClientWidth / 256.0f);
			deviceContext->Dispatch(numGroupsX, mValues->mClientHeight, 1);
		}
	
		// Unbind the input texture from the CS for good housekeeping.
		ID3D11ShaderResourceView* nullSRV[1] = { 0 };
		deviceContext->CSSetShaderResources( 0, 1, nullSRV );

		// Unbind output from compute shader (we are going to use this output as an input in the next pass, 
		// and a resource cannot be both an output and input at the same time.
		ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
		deviceContext->CSSetUnorderedAccessViews( 0, 1, nullUAV, 0 );
	
		// VERTICAL blur pass.
		mVertBlurTech->GetDesc( &techDesc );
		for(UINT p = 0; p < techDesc.Passes; ++p)
		{
			mInputMap->SetResource(mBlurredOutputTexSRV);
			mOutputMap->SetUnorderedAccessView(output);
			mVertBlurTech->GetPassByIndex(p)->Apply(0, deviceContext);

			// How many groups do we need to dispatch to cover a column of pixels, where each
			// group covers 256 pixels  (the 256 is defined in the ComputeShader).
			UINT numGroupsY = (UINT)ceilf(mValues->mClientHeight / 256.0f);
			deviceContext->Dispatch(mValues->mClientWidth, numGroupsY, 1);
		}
	
		deviceContext->CSSetShaderResources( 0, 1, nullSRV );
		deviceContext->CSSetUnorderedAccessViews( 0, 1, nullUAV, 0 );
	}

	// Disable compute shader.
	deviceContext->CSSetShader(0, 0, 0);
}