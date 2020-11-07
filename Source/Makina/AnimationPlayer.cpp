
#include "AnimationPlayer.h"
#include "Exceptions.h"

using namespace Makina;
using namespace std;

AnimationPlayer::AnimationPlayer(MeshAnimationData *data, CXMMATRIX world)
: mAnmData(data),
mTime(0),
mCurrentAnimSet(0),
mState(AnimationPlayerState::Paused),
mOffset(0.0f, 0.0f, 0.0f)
{
	mBoneLocal.resize(data->mBones.size());
	mBoneFinal.resize(data->mBones.size());

	// for each bone
	for (UINT i = 0; i < mAnmData->mBones.size(); ++i)
	{
		XMMATRIX boneFinal = XMMatrixIdentity();
		XMStoreFloat4x4(&mBoneFinal[i], boneFinal);
	}
}

AnimationPlayer::~AnimationPlayer()
{

}

int GetNextKey(int key, int numKeys)
{
	int ret = key + 1;
	if (ret >= numKeys) ret = 0;
	return ret;
}

void AnimationPlayer::Update(float dt, XMFLOAT4X4 *world)
{
	if (mState != AnimationPlayerState::Playing) return;
	mTime += (UINT)(dt * mAnmData->mAnimTicksPerSecond);
	while (mTime > mAnmData->mAnimationSets[mCurrentAnimSet].mTotalTime)
		mTime -= mAnmData->mAnimationSets[mCurrentAnimSet].mTotalTime;

	// for each bone and it's animation
	for (UINT i = 0; i < mAnmData->mAnimationSets[mCurrentAnimSet].mAnim.size(); ++i)
	{
		UINT bone = mAnmData->mAnimationSets[mCurrentAnimSet].mAnim[i].mBone;
		int key = -1;
		int keysNum = (int)mAnmData->mAnimationSets[mCurrentAnimSet].mAnim[i].mAnimKeys.size();
		for (int j = 0; j < keysNum; ++j)
		{
			if (mAnmData->mAnimationSets[mCurrentAnimSet].mAnim[i].mAnimKeys[j].mT <= mTime) key = j;
			else break;
		}
		XMMATRIX wNext = XMLoadFloat4x4(&mAnmData->mAnimationSets[mCurrentAnimSet].mAnim[i].mAnimKeys[GetNextKey(key, keysNum)].mW);
		XMMATRIX wCurr = XMLoadFloat4x4(&mAnmData->mAnimationSets[mCurrentAnimSet].mAnim[i].mAnimKeys[key].mW);
		XMVECTOR sN, rN, tN, sC, rC, tC;
		XMMatrixDecompose(&sN, &rN, &tN, wNext);
		XMMatrixDecompose(&sC, &rC, &tC, wCurr);
		float timeNext = (float)mAnmData->mAnimationSets[mCurrentAnimSet].mAnim[i].mAnimKeys[GetNextKey(key, keysNum)].mT;
		float timeCurr = (float)mAnmData->mAnimationSets[mCurrentAnimSet].mAnim[i].mAnimKeys[key].mT;
		float lif = (mTime - timeCurr) / (timeNext - timeCurr);
		XMVECTOR t = XMVectorLerp(tC, tN, lif);
		XMVECTOR r = XMQuaternionSlerp(rC, rN, lif);
		XMMATRIX finalW = XMMatrixRotationQuaternion(r) * XMMatrixTranslationFromVector(t);
		XMStoreFloat4x4(&mBoneLocal[bone], finalW);
	}

	XMMATRIX I = XMMatrixIdentity();
	XMVECTOR offset = XMLoadFloat3(&mOffset);
	UpdateBoneRoot(I, mAnmData->mRootBoneIndex, &offset);
	offset /= (float)mAnmData->mBones.size();
	XMStoreFloat3(&mOffset, offset);
}

void AnimationPlayer::Play(const string &animName)
{
	mCurrentAnimSet = -1;
	for (UINT i = 0; i < mAnmData->mAnimationSets.size(); ++i)
	if (mAnmData->mAnimationSets[i].mName.compare(animName) == 0)
		mCurrentAnimSet = i;

	if (mCurrentAnimSet == -1)
		throw UnexpectedError(L"Wrong animation name passed. (AnimationPlayer::Play)");

	mTime = 0;
	mState = AnimationPlayerState::Playing;
}

void AnimationPlayer::SetBoneTransform(CXMMATRIX t, UINT boneIndex)
{
	XMMATRIX boneBind = XMLoadFloat4x4(&mAnmData->mBones[boneIndex].mBind);
	XMMATRIX boneFinal = boneBind * t;
	XMStoreFloat4x4(&mBoneFinal[boneIndex], boneFinal);
}

void AnimationPlayer::UpdateBoneRoot(CXMMATRIX parentRoot, UINT boneIndex, XMVECTOR *offset)
{
	XMMATRIX boneBind = XMLoadFloat4x4(&mAnmData->mBones[boneIndex].mBind);
	XMMATRIX boneLocal = XMLoadFloat4x4(&mBoneLocal[boneIndex]);
	XMMATRIX boneRoot = boneLocal * parentRoot;
	XMMATRIX boneFinal = boneBind * boneRoot;
	XMStoreFloat4x4(&mBoneFinal[boneIndex], boneFinal);

	// add to offset
	*offset += XMVector3Transform(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), boneRoot);

	// for every child of this bone
	for (UINT i = 0; i < mAnmData->mBones.size(); ++i)
	if (mAnmData->mBones[i].mParent == boneIndex)
		UpdateBoneRoot(boneRoot, i, offset);
}