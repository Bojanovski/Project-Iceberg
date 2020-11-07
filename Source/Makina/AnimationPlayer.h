
#ifndef ANIMATIONPLAYER_H
#define ANIMATIONPLAYER_H

#include "MeshAnimationData.h"

namespace Makina
{
	enum AnimationPlayerState { Playing, Paused, Other };

	class AnimationPlayer
	{
	public:
		__declspec(dllexport) AnimationPlayer(MeshAnimationData *data, CXMMATRIX world);
		__declspec(dllexport) virtual ~AnimationPlayer();

		// Updates the bone matrices.
		// World parameter might be changed depending on the type of the animation (e.g. ragdoll will change it).
		__declspec(dllexport) virtual void Update(float dt, XMFLOAT4X4 *world);
		__declspec(dllexport) virtual void Play(const std::string &animName);

		__declspec(dllexport) void SetBoneTransform(CXMMATRIX t, UINT boneIndex);
		XMFLOAT4X4 const *GetFinalTransforms() { return &mBoneFinal[0]; }
		UINT GetFinalTransformsCount() { return mBoneFinal.size(); }
		XMFLOAT3 const *GetOffset() { return &mOffset; }

	protected:
		// Current state of the animation.
		AnimationPlayerState mState;
		// Animation of a bone in root space (combining bone bind transformation).
		std::vector<XMFLOAT4X4> mBoneFinal;

	private:
		void UpdateBoneRoot(CXMMATRIX parentRoot, UINT boneIndex, XMVECTOR *offset);

		// All the data for animation
		MeshAnimationData *mAnmData;
		// Current time of the player in time stamps.
		UINT mTime;
		// What animation is currently active.
		int mCurrentAnimSet;
		// Animation of a bone in it's local space.
		std::vector<XMFLOAT4X4> mBoneLocal;	
		// Animation offset. Used to send information how much offset did animation/simulation make, to the rendering object.
		XMFLOAT3 mOffset;
	};
}

#endif