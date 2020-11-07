
#ifndef OBJECT_H
#define OBJECT_H

#include <Windows.h>
#include <xnamath.h>
#include <D3DAppValues.h>

namespace Makina
{
	class ShadowMap;
	class DynamicCubeMap;
	class SSAO;
	class Model;
	class AnimationPlayer;
}

class Object
{
public:
	Object(Makina::D3DAppValues *values, Makina::Model *mdl, CXMMATRIX World);
	~Object();

	void Update(float dt);
	void Draw(float dt);
	void DrawDepthOnly();
	void DrawNormalAndDepth();
	void OnResize();

	bool RayIntersects(FXMVECTOR origin, FXMVECTOR dir, float *iDist);
	void SetCubeMap(Makina::DynamicCubeMap *dynamicCubeMap) { mDynamicCubeMap = dynamicCubeMap; }
	void SetShadowMap(Makina::ShadowMap *shadowMap) { mShadowMap = shadowMap; }
	void SetSsaoMap(Makina::SSAO *ssaoMap) { mSsaoMap = ssaoMap; }
	void SetWorld(XMFLOAT4X4 &world) { mWorld = world; }
	XMFLOAT4X4 &GetWorld() { return mWorld; }
	Makina::AnimationPlayer *GetAnimationPlayer() { return mAnimPlayer; }

private:
	bool normalColor;
	bool mSkinned;

public:
	bool mWireframe;
	bool mRefl;
	Makina::Camera *mCam;
	Makina::Model *mMdl;

private:
	Makina::D3DAppValues *mD3DAppValues;

	Makina::DynamicCubeMap *mDynamicCubeMap;
	Makina::BasicEffect *mFXPt;
	Makina::ShadowMap *mShadowMap;
	Makina::SSAO *mSsaoMap;

	XMFLOAT4X4 mWorld;

	ID3D11BlendState *mBlendState;
	ID3D11RasterizerState *mNormalState;
	ID3D11RasterizerState *mWireframeState;

	Makina::AnimationPlayer *mAnimPlayer;
};

#endif
