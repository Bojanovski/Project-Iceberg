
#ifndef BASICEFFECT_H
#define BASICEFFECT_H

#include "D3DUtilities.h"
#include "Camera.h"
#include "Effect.h"
#include "d3dx11Effect.h"

namespace Makina
{
	class BasicEffect : public Effect
	{
	public:
		__declspec(dllexport) BasicEffect(D3DAppValues *values);
		__declspec(dllexport) ~BasicEffect();
		__declspec(dllexport) void UpdateVariables();

	public:	
		ID3DX11EffectTechnique *mDepthOnlyTech;
		ID3DX11EffectTechnique *mDepthOnlyAlphaClipTech;
		ID3DX11EffectTechnique *mNormalAndDepth16BitTech;
		ID3DX11EffectTechnique *mNormalAndDepthAlphaClip16BitTech;
		ID3DX11EffectTechnique *mNormalAndDepth8BitTech;
		ID3DX11EffectTechnique *mNormalAndDepthAlphaClip8BitTech;
		ID3DX11EffectTechnique *mGouraudTech;
		ID3DX11EffectTechnique *mGouraudFastTech;
		ID3DX11EffectTechnique *mLightFullTech;
		ID3DX11EffectTechnique *mLightDirTech;
		ID3DX11EffectTechnique *mLightDirShadowTech;
		ID3DX11EffectTechnique *mLightDirShadowAndSSAOTech;
		ID3DX11EffectTechnique *mLightDirShadowReflectionAndSSAOTech;
		ID3DX11EffectTechnique *mLightDirTessTech;
		ID3DX11EffectTechnique *mLightReflectionTech;
		ID3DX11EffectTechnique *mLightAndNTB;
		ID3DX11EffectTechnique *mJustTexture;
		ID3DX11EffectTechnique *mSkinnedDepthOnlyTech;
		ID3DX11EffectTechnique *mSkinnedDepthOnlyAlphaClipTech;
		ID3DX11EffectTechnique *mSkinnedNormalAndDepth16BitTech;
		ID3DX11EffectTechnique *mSkinnedNormalAndDepthAlphaClip16BitTech;
		ID3DX11EffectTechnique *mSkinnedNormalAndDepth8BitTech;
		ID3DX11EffectTechnique *mSkinnedNormalAndDepthAlphaClip8BitTech;
		ID3DX11EffectTechnique *mSkinnedLightDirShadowAndSSAOTech;

		ID3DX11EffectMatrixVariable *mWorld;
		ID3DX11EffectMatrixVariable *mWorldInvTranspose;
		ID3DX11EffectMatrixVariable *mWorldViewProj;
		ID3DX11EffectMatrixVariable *mWorldViewProjTex;
		ID3DX11EffectMatrixVariable *mWorldInvTransposeView;
		ID3DX11EffectMatrixVariable *mWorldView;
		ID3DX11EffectVariable *mMaterial;
		ID3DX11EffectMatrixVariable *mTexTransform;
		ID3DX11EffectMatrixVariable *mShadowTransform;
		ID3DX11EffectScalarVariable *mShadowMapSize;
		ID3DX11EffectScalarVariable *mReflection;
		ID3DX11EffectMatrixVariable* mBoneTransforms;

		ID3DX11EffectShaderResourceVariable *mDiffuseMap;
		ID3DX11EffectShaderResourceVariable *mNormalMap;
		ID3DX11EffectShaderResourceVariable *mTexArray;
		ID3DX11EffectShaderResourceVariable *mCubeMap;
		ID3DX11EffectShaderResourceVariable *mShadowMap;
		ID3DX11EffectShaderResourceVariable *mSSAOMap;

		DirectionalLight &DirLight() {return mDirLight;}
		ID3D11InputLayout *GetVertexFullInputLayout() {return mVertexFullInputLayout;}
		ID3D11InputLayout *GetSkinnedVertexFullInputLayout() { return mSkinnedVertexFullInputLayout; }
		void SetGenericDiffuseMap() {mDiffuseMap->SetResource(mGenericDiffuseMapSRV);}
		void SetGenericNormalMap() {mNormalMap->SetResource(mGenericNormalMapSRV);}

	private:
		__declspec(dllexport) void BuildInputLayouts();

		//
		// Lights
		//
		DirectionalLight mDirLight;
		SpotLight mSpotLight;
		PointLight mPointLight;

		// Camera
		Camera *mCamera;

		//
		// Input layouts
		//
		ID3D11InputLayout *mVertexFullInputLayout;
		ID3D11InputLayout *mSkinnedVertexFullInputLayout;

		//
		// Variables
		//
		ID3DX11EffectVariable *mDirLightning;
		ID3DX11EffectVariable *mPointLightning;
		ID3DX11EffectVariable *mSpotLightning;
		ID3DX11EffectVectorVariable *mEyePosW;
		ID3DX11EffectMatrixVariable *mViewProj;
		ID3DX11EffectScalarVariable *mZNear;
		ID3DX11EffectScalarVariable *mZFar;

		ID3DX11EffectScalarVariable *mHeightScale;
		ID3DX11EffectScalarVariable *mMaxTessDistance;
		ID3DX11EffectScalarVariable *mMinTessDistance;
		ID3DX11EffectScalarVariable *mMinTessFactor;
		ID3DX11EffectScalarVariable *mMaxTessFactor;

		//
		// For models without SRVs.
		//
		ID3D11ShaderResourceView *mGenericDiffuseMapSRV;
		ID3D11ShaderResourceView *mGenericNormalMapSRV;
	};
}

#endif