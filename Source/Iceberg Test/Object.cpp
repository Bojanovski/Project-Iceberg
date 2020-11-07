#include "Object.h"
#include <RenderStatesManager.h>
#include <Camera.h>
#include <MathHelper.h>
#include <Exceptions.h>
#include <ShadowMap.h>
#include <DynamicCubeMap.h>
#include <SSAO.h>
#include <BasicModel.h>
#include "DirectX11Headers.h"
#include <SkinnedModel.h>
#include <RagdollAnimationPlayer.h>

using namespace Makina;
using namespace std;

Object::Object(D3DAppValues *values, Model *mdl, CXMMATRIX World)
	: mD3DAppValues(values),
	mMdl(mdl),
	mWireframe(false),
	mRefl(false),
	mFXPt(values->mBasicEffect)
{
	mCam = mD3DAppValues->mCamera;
	XMStoreFloat4x4(&mWorld, World);

	mBlendState = mD3DAppValues->mRenderStatesManager->GetBS(L"alphaBlend");
	mWireframeState = mD3DAppValues->mRenderStatesManager->GetRS(L"wireframe");
	mNormalState = mD3DAppValues->mRenderStatesManager->GetRS(L"normal");

	XMVECTOR p, r, s;
	XMMatrixDecompose(&s, &r, &p, World);
	if (typeid(*mdl) == typeid(SkinnedModel))
	{
		mSkinned = true;
#ifdef RAGDOLLANIMATIONPLAYER_DEBUG_MODE
		mAnimPlayer = new RagdollAnimationPlayer(values, static_cast<SkinnedModel *>(mdl)->GetMeshAnimationData(), World,
			static_cast<SkinnedModel *>(mdl)->GetMeshSimulationData(), s);
#else
		mAnimPlayer = new RagdollAnimationPlayer(static_cast<SkinnedModel *>(mdl)->GetMeshAnimationData(), World, 
			static_cast<SkinnedModel *>(mdl)->GetMeshSimulationData(), s);
#endif
	}
	else
	{
		mSkinned = false;
		mAnimPlayer = 0;
	}
}

Object::~Object()
{

}

void Object::Update(float dt)
{
	//static float lol = 0;
	//lol += dt;

	//D3D11_MAPPED_SUBRESOURCE vertices;
	//mImmediateContext->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vertices);
	//VertexPC *v = reinterpret_cast<VertexPC *>(vertices.pData);

	//for (UINT i = 0; i < mVertexCount; i++)
	//{
	//	v[i].Pos = XMFLOAT3(mesh.Vertices[i].Position.x, mesh.Vertices[i].Position.y + sin(lol + mesh.Vertices[i].Position.x) / 3, mesh.Vertices[i].Position.z);
	//	v[i].Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	//}

	//mImmediateContext->Unmap(mVertexBuffer, 0);

	if (mAnimPlayer)
	{
		mAnimPlayer->Update(dt, &mWorld);
	}
}

void Object::Draw(float dt)
{
	// First check frustum culling
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMVECTOR detWorld;
	XMMATRIX toLocal = XMMatrixInverse(&detWorld, world);
	Frustum transformedFrustum;
	XMVECTOR scale, rotQuat, translation;
	XMMatrixDecompose(&scale, &rotQuat, &translation, toLocal);
	TransformFrustum(&transformedFrustum, &mCam->GetFrustum(), XMVectorGetX(scale), rotQuat, translation);
	if (IntersectOrientedBoxFrustum(mMdl->GetBoundingVolume(mSkinned), &transformedFrustum) == 0)
		return;  // <--- PROBLEM

	// It's ok, let's countinue.

	//if (mNTB)
	//mD3DAppValues->md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	//else
	mD3DAppValues->md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mD3DAppValues->md3dImmediateContext->OMSetDepthStencilState(0, 0);
	mD3DAppValues->md3dImmediateContext->RSSetState((mWireframe) ? mWireframeState : mNormalState);
	mD3DAppValues->md3dImmediateContext->OMSetBlendState(0, 0, 0xffffffff);

	//set constants
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);

	XMMATRIX worldViewProj = world * mCam->ViewProj();

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX toTexSpace(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX worldViewProjTex = worldViewProj * toTexSpace;

	mFXPt->mWorld->SetMatrix(reinterpret_cast<float*>(&world));
	mFXPt->mWorldInvTranspose->SetMatrix(reinterpret_cast<float*>(&worldInvTranspose));
	mFXPt->mWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
	mFXPt->mWorldViewProjTex->SetMatrix(reinterpret_cast<float*>(&worldViewProjTex));
	mFXPt->mTexArray->SetResource(NULL);
	if (mSkinned)
	{
		mFXPt->mBoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(mAnimPlayer->GetFinalTransforms()),
			0, mAnimPlayer->GetFinalTransformsCount());
	}

	// shadows
	XMMATRIX worldViewProjShadow = world * mShadowMap->GetViewProjTransform();
	mFXPt->mShadowMap->SetResource(mShadowMap->GetSRV());
	mFXPt->mSSAOMap->SetResource(mSsaoMap->GetSRV());
	mFXPt->mShadowMapSize->SetFloat((float)mShadowMap->GetSize());
	mFXPt->mShadowTransform->SetMatrix(reinterpret_cast<float*>(&worldViewProjShadow));

	//reflection
	mFXPt->mCubeMap->SetResource(mDynamicCubeMap->GetSRV());

	XMMATRIX I = XMMatrixIdentity(); // = XMMatrixTranslation(-0.5f, -0.5f, 0.0f) * XMMatrixScaling(0.5f, 0.5f, 1) * XMMatrixRotationRollPitchYaw(0, 0, XM_PIDIV4) * XMMatrixTranslation(0.5f, 0.5f, 0.0f);
	XMVECTOR det;
	I = XMMatrixInverse(&det, I);
	mFXPt->mTexTransform->SetMatrix(reinterpret_cast<float*>(&I));
	if (mSkinned)
	{
		mFXPt->mBoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(mAnimPlayer->GetFinalTransforms()),
			0, mAnimPlayer->GetFinalTransformsCount());
	}

	ID3DX11EffectTechnique *technique;
	if (mSkinned )	technique = (mRefl) ? mFXPt->mSkinnedLightDirShadowAndSSAOTech : mFXPt->mSkinnedLightDirShadowAndSSAOTech;
	else			technique = (mRefl) ? mFXPt->mLightDirShadowReflectionAndSSAOTech : mFXPt->mLightDirShadowAndSSAOTech;
	D3DX11_TECHNIQUE_DESC techDesc;
	technique->GetDesc(&techDesc);

	for (UINT i = 0; i < techDesc.Passes; i++)
	{
		mMdl->Draw(technique->GetPassByIndex(i), transformedFrustum, mSkinned);
	}

	// FX sets tessellation stages, but it does not disable them.  So do that here
	// to turn off tessellation.
	mD3DAppValues->md3dImmediateContext->HSSetShader(0, 0, 0);
	mD3DAppValues->md3dImmediateContext->DSSetShader(0, 0, 0);
}

// need to update for skinned
void Object::DrawDepthOnly()
{
	if (mWireframe) return;

	// First check frustum or OBB culling	
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMVECTOR detWorld;
	XMMATRIX toLocal = XMMatrixInverse(&detWorld, world);
	XMVECTOR scale, rotQuat, translation;
	XMMatrixDecompose(&scale, &rotQuat, &translation, toLocal);	
	Frustum transformedFrustum;
	OrientedBox transformedBox;

	if (mShadowMap->GetType() == Projective)
	{
		TransformFrustum(&transformedFrustum, &mShadowMap->GetFrustum(), XMVectorGetX(scale), rotQuat, translation);
		if (IntersectOrientedBoxFrustum(mMdl->GetBoundingVolume(mSkinned), &transformedFrustum) == 0)
			return;
	}
	else if (mShadowMap->GetType() == Orthographic)
	{
		TransformOrientedBox(&transformedBox, &mShadowMap->GetOrientedBox(), XMVectorGetX(scale), rotQuat, translation);
		if (IntersectOrientedBoxOrientedBox(mMdl->GetBoundingVolume(mSkinned), &transformedBox) == 0)
			return;
	}

	// It's ok, let's countinue.

	mD3DAppValues->md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mD3DAppValues->md3dImmediateContext->OMSetDepthStencilState(0, 0);
	float blendFactor[] = {1.0f, 0.5f, 0.0f, 0.0f};
	mD3DAppValues->md3dImmediateContext->OMSetBlendState(0, 0, 0xffffffff);

	//set constants
	XMMATRIX worldViewProj = world * mShadowMap->GetViewProj();
	mFXPt->mWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
	if (mSkinned)
	{
		mFXPt->mBoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(mAnimPlayer->GetFinalTransforms()),
			0, mAnimPlayer->GetFinalTransformsCount());
	}

	ID3DX11EffectTechnique *technique;
	if (mSkinned)	technique = mFXPt->mSkinnedDepthOnlyTech;
	else			technique = mFXPt->mDepthOnlyTech;
	D3DX11_TECHNIQUE_DESC techDesc;
	technique->GetDesc(&techDesc);

	for (UINT i = 0; i < techDesc.Passes; i++)
	{
		if (mShadowMap->GetType() == Projective) mMdl->Draw(technique->GetPassByIndex(i), transformedFrustum, mSkinned);
		else if (mShadowMap->GetType() == Orthographic)	mMdl->Draw(technique->GetPassByIndex(i), transformedBox, mSkinned);
	}
}

void Object::DrawNormalAndDepth()
{
	if (mWireframe) return;

	// First check frustum culling
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMVECTOR detWorld;
	XMMATRIX toLocal = XMMatrixInverse(&detWorld, world);
	Frustum transformedFrustum;
	XMVECTOR scale, rotQuat, translation;
	XMMatrixDecompose(&scale, &rotQuat, &translation, toLocal);
	TransformFrustum(&transformedFrustum, &mCam->GetFrustum(), XMVectorGetX(scale), rotQuat, translation);
	if (IntersectOrientedBoxFrustum(mMdl->GetBoundingVolume(mSkinned), &transformedFrustum) == 0)
		return;

	// It's ok, let's countinue.

	mD3DAppValues->md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mD3DAppValues->md3dImmediateContext->OMSetDepthStencilState(0, 0);
	mD3DAppValues->md3dImmediateContext->OMSetBlendState(0, 0, 0xffffffff);
	//mD3DAppValues->md3dImmediateContext->RSSetState(mNormalState);

	//set constants
	XMMATRIX worldViewProj = world * mCam->ViewProj();
	mFXPt->mWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
	XMMATRIX worldInvTransposeView = MathHelper::InverseTranspose(world) * mCam->View();
	mFXPt->mWorldInvTransposeView->SetMatrix(reinterpret_cast<float*>(&worldInvTransposeView));
	XMMATRIX worldView = world * mCam->View();
	mFXPt->mWorldView->SetMatrix(reinterpret_cast<float*>(&worldView));
	if (mSkinned)
	{
		mFXPt->mBoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(mAnimPlayer->GetFinalTransforms()),
			0, mAnimPlayer->GetFinalTransformsCount());
	}

	ID3DX11EffectTechnique *technique;
	if (mSkinned)	technique = mFXPt->mSkinnedNormalAndDepth8BitTech;
	else			technique = mFXPt->mNormalAndDepth8BitTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	technique->GetDesc(&techDesc);

	for (UINT i = 0; i < techDesc.Passes; i++)
	{
		mMdl->Draw(technique->GetPassByIndex(i), transformedFrustum, mSkinned);
	}
}

void Object::OnResize()
{

}

bool Object::RayIntersects(FXMVECTOR origin, FXMVECTOR dir, float *iDist)
{
	// transform ray to local space of mesh
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMVECTOR detWorld;
	XMMATRIX toLocal = XMMatrixInverse(&detWorld, world);
	XMVECTOR localOrigin = XMVector3TransformCoord(origin, toLocal);
	XMVECTOR localDir = XMVector3TransformNormal(dir, toLocal);
	localDir = XMVector3Normalize(localDir);

	// test
	return mMdl->ObjectSpaceRayIntersects(localOrigin, localDir, iDist);
}