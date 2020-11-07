#include "BasicEffect.h"
#include <fstream>
#include <vector>
#include "Resource.h"
#include "TextureGenerator.h"

using namespace Makina;
using namespace std;

BasicEffect::BasicEffect(D3DAppValues *values)
	: Effect(values, L"Makina.dll", ID_BASIC)
{
	// And now the variables.
	mDepthOnlyTech								= mFX->GetTechniqueByName("DepthOnlyTech");
	mDepthOnlyAlphaClipTech						= mFX->GetTechniqueByName("DepthOnlyAlphaClipTech");
	mNormalAndDepth16BitTech					= mFX->GetTechniqueByName("NormalAndDepth16BitTech");
	mNormalAndDepthAlphaClip16BitTech			= mFX->GetTechniqueByName("NormalAndDepthAlphaClip16BitTech");
	mNormalAndDepth8BitTech						= mFX->GetTechniqueByName("NormalAndDepth8BitTech");
	mNormalAndDepthAlphaClip8BitTech			= mFX->GetTechniqueByName("NormalAndDepthAlphaClip8BitTech");
	mGouraudTech								= mFX->GetTechniqueByName("GouraudTech");
	mGouraudFastTech							= mFX->GetTechniqueByName("GouraudFastTech");
	mLightFullTech								= mFX->GetTechniqueByName("LightFullTech");
	mLightDirTech								= mFX->GetTechniqueByName("LightDirTech");
	mLightDirShadowTech							= mFX->GetTechniqueByName("LightDirShadowTech");
	mLightDirShadowAndSSAOTech					= mFX->GetTechniqueByName("LightDirShadowAndSSAOTech");
	mLightDirShadowReflectionAndSSAOTech		= mFX->GetTechniqueByName("LightDirShadowReflectionAndSSAOTech");
	mLightDirTessTech							= mFX->GetTechniqueByName("LightDirTessTech");
	mLightReflectionTech						= mFX->GetTechniqueByName("LightReflectionTech");
	mLightAndNTB								= mFX->GetTechniqueByName("LightAndNTB");
	mJustTexture								= mFX->GetTechniqueByName("JustTexture");
	mSkinnedDepthOnlyTech						= mFX->GetTechniqueByName("SkinnedDepthOnlyTech");
	mSkinnedDepthOnlyAlphaClipTech				= mFX->GetTechniqueByName("SkinnedDepthOnlyAlphaClipTech");
	mSkinnedNormalAndDepth16BitTech				= mFX->GetTechniqueByName("SkinnedNormalAndDepth16BitTech");
	mSkinnedNormalAndDepthAlphaClip16BitTech	= mFX->GetTechniqueByName("SkinnedNormalAndDepthAlphaClip16BitTech");
	mSkinnedNormalAndDepth8BitTech				= mFX->GetTechniqueByName("SkinnedNormalAndDepth8BitTech");
	mSkinnedNormalAndDepthAlphaClip8BitTech		= mFX->GetTechniqueByName("SkinnedNormalAndDepthAlphaClip8BitTech");
	mSkinnedLightDirShadowAndSSAOTech			= mFX->GetTechniqueByName("SkinnedLightDirShadowAndSSAOTech");

	mDirLightning								= mFX->GetVariableByName("gDirLight");
	mPointLightning								= mFX->GetVariableByName("gPointLight");
	mSpotLightning								= mFX->GetVariableByName("gSpotLight");
	mEyePosW									= mFX->GetVariableByName("gEyePosW")->AsVector();
	mViewProj									= mFX->GetVariableByName("gViewProj")->AsMatrix();
	mZNear										= mFX->GetVariableByName("gZNear")->AsScalar();
	mZFar										= mFX->GetVariableByName("gZFar")->AsScalar();
	mWorld										= mFX->GetVariableByName("gWorld")->AsMatrix();
	mWorldInvTranspose							= mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	mWorldViewProj								= mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	mWorldViewProjTex							= mFX->GetVariableByName("gWorldViewProjTex")->AsMatrix();
	mWorldInvTransposeView						= mFX->GetVariableByName("gWorldInvTransposeView")->AsMatrix();
	mWorldView									= mFX->GetVariableByName("gWorldView")->AsMatrix();
	mMaterial									= mFX->GetVariableByName("gMaterial");
	mTexTransform								= mFX->GetVariableByName("gTexTransform")->AsMatrix();
	mShadowTransform							= mFX->GetVariableByName("gShadowTransform")->AsMatrix();
	mShadowMapSize								= mFX->GetVariableByName("gShadowMapSize")->AsScalar();
	mReflection									= mFX->GetVariableByName("gReflection")->AsScalar();
	mBoneTransforms								= mFX->GetVariableByName("gBoneTransforms")->AsMatrix();

	mHeightScale								= mFX->GetVariableByName("gHeightScale")->AsScalar();
	mMaxTessDistance							= mFX->GetVariableByName("gMaxTessDistance")->AsScalar();
	mMinTessDistance							= mFX->GetVariableByName("gMinTessDistance")->AsScalar();
	mMinTessFactor								= mFX->GetVariableByName("gMinTessFactor")->AsScalar();
	mMaxTessFactor								= mFX->GetVariableByName("gMaxTessFactor")->AsScalar();

	mDiffuseMap									= mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	mNormalMap									= mFX->GetVariableByName("gNormalMap")->AsShaderResource();
	mCubeMap									= mFX->GetVariableByName("gCubeMap")->AsShaderResource();
	mShadowMap									= mFX->GetVariableByName("gShadowMap")->AsShaderResource();
	mSSAOMap									= mFX->GetVariableByName("gSsaoMap")->AsShaderResource();
	mTexArray									= mFX->GetVariableByName("gTexArray")->AsShaderResource();

	mDirLight.Ambient							= XMFLOAT4(0.5f, 0.4f, 0.4f, 1.0f);
	mDirLight.Diffuse							= XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mDirLight.Direction							= XMFLOAT3(-0.2f, -0.3f, -0.707f); // = XMFLOAT3(0.57735f, -0.57735f, 0.57735f); // = XMFLOAT3(0.140028f, -0.980196f, 0.140028f);

	mDirLight.Direction.x /= sqrt(mDirLight.Direction.x*mDirLight.Direction.x + mDirLight.Direction.y*mDirLight.Direction.y + mDirLight.Direction.z*mDirLight.Direction.z);
	mDirLight.Direction.y /= sqrt(mDirLight.Direction.x*mDirLight.Direction.x + mDirLight.Direction.y*mDirLight.Direction.y + mDirLight.Direction.z*mDirLight.Direction.z);
	mDirLight.Direction.z /= sqrt(mDirLight.Direction.x*mDirLight.Direction.x + mDirLight.Direction.y*mDirLight.Direction.y + mDirLight.Direction.z*mDirLight.Direction.z);

	mDirLight.Pad								= 0.0f;
	mDirLight.Specular							= XMFLOAT4(0.7f, 0.6f, 0.5f, 1.0f);

	mSpotLight.Ambient							= XMFLOAT4(0.2f, 0.1f, 0.0f, 1.0f);
	mSpotLight.Att								= XMFLOAT3(0.0f, 0.2f, 0.02f);
	mSpotLight.Diffuse							= XMFLOAT4(0.6f, 0.5f, 0.4f, 1.0f);
	mSpotLight.Direction						= XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);
	mSpotLight.Pad								= 0.0f;
	mSpotLight.Position							= XMFLOAT3(3.0f, 3.0f, -3.0f);
	mSpotLight.Range							= 1000.0f;
	mSpotLight.Specular							= XMFLOAT4(0.4f, 0.3f, 0.2f, 1.0f);
	mSpotLight.Spot								= 20.0f;

	mPointLight.Ambient							= XMFLOAT4(0.1f, 0.05f, 0.0f, 1.0f);
	mPointLight.Att								= XMFLOAT3(0.0f, 0.2f, 0.02f);
	mPointLight.Diffuse							= XMFLOAT4(0.3f, 0.05f, 0.04f, 1.0f);
	mPointLight.Pad								= 0.0f;
	mPointLight.Position						= XMFLOAT3(1.0f, 1.0f, -2.0f);
	mPointLight.Range							= 1000.0f;
	mPointLight.Specular						= XMFLOAT4(0.2f, 0.05f, 0.05f, 1.0f);

	// Input layout
	BuildInputLayouts();

	// Camera
	mCamera = (Camera *)mValues->mCamera;

	// For models without SRVs.
	TextureGenerator gen(values);
	gen.GenerateGenericDiffuseMap(XMCOLOR(1.0f, 1.0f, 1.0f, 1.0f), &mGenericDiffuseMapSRV);
	gen.GenerateGenericNormalMap(&mGenericNormalMapSRV);
}

BasicEffect::~BasicEffect()
{
	mVertexFullInputLayout->Release();
	mSkinnedVertexFullInputLayout->Release();

	mGenericDiffuseMapSRV->Release();
	mGenericNormalMapSRV->Release();
}

void BasicEffect::UpdateVariables()
{
	mDirLightning->SetRawValue(&mDirLight, 0, sizeof(mDirLight));
	mSpotLightning->SetRawValue(&mSpotLight, 0, sizeof(mSpotLight));
	mPointLightning->SetRawValue(&mPointLight, 0, sizeof(mPointLight));
	mEyePosW->SetRawValue(&mCamera->GetPosition(), 0, sizeof(XMFLOAT3));
	mZNear->SetFloat(mCamera->GetNearZ());
	mZFar->SetFloat(mCamera->GetFarZ());

	XMMATRIX viewProj = mCamera->View() * mCamera->Proj();
	mViewProj->SetMatrix(reinterpret_cast<float*>(&viewProj));

	mHeightScale->SetFloat(0.1f);
	mMaxTessDistance->SetFloat(0.5f);
	mMinTessDistance->SetFloat(5.0f);
	mMinTessFactor->SetFloat(1.0f);
	mMaxTessFactor->SetFloat(64.0f);
}

void BasicEffect::BuildInputLayouts()
{
	//Create vertex full input layout
	D3DX11_PASS_DESC passDesc;
	mLightDirShadowAndSSAOTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HRESULT hr = mValues->md3dDevice->CreateInputLayout(VertexFullDesc, 4,
		passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mVertexFullInputLayout);
	if(FAILED(hr))
		throw UnexpectedError(L"Failed to create vertex full input layout! (BasicEffect.cpp)");	
	
	//Create skinned vertex full input layout
	mSkinnedLightDirShadowAndSSAOTech->GetPassByIndex(0)->GetDesc(&passDesc);
	hr = mValues->md3dDevice->CreateInputLayout(SkinnedVertexFullDesc, 6,
		passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mSkinnedVertexFullInputLayout);
	if (FAILED(hr))
		throw UnexpectedError(L"Failed to create skinned vertex full input layout! (BasicEffect.cpp)");
}