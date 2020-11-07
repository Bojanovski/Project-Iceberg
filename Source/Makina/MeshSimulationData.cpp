
#include "MeshSimulationData.h"
#include "MeshAnimationData.h"
#include "MathHelper.h"

using namespace Makina;
using namespace std;

MeshSimulationData::MeshSimulationData(MeshAnimationData *animData)
: mAnimData(animData)
{

}

MeshSimulationData::~MeshSimulationData()
{

}

bool operator==(const BoneRigidBody &left, const BoneRigidBody &right)
{
	return (left.mIndex == right.mIndex);
}

void MeshSimulationData::GenerateData()
{
	XMVECTOR t;
	XMMATRIX boneBindToWorld = XMLoadFloat4x4(&mAnimData->mBones[mAnimData->mRootBoneIndex].mBind);
	boneBindToWorld = XMMatrixInverse(&t, boneBindToWorld);
	t = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	t = XMVector3TransformCoord(t, boneBindToWorld);

	// for every child of the root bone
	bool firstToAdd = true;
	for (UINT i = 0; i < mAnimData->mBones.size(); ++i)
	if (mAnimData->mBones[i].mParent == mAnimData->mRootBoneIndex) 
	{
		// bone
		AddBoneRigidBody(i, firstToAdd, mAnimData->mRootBoneIndex, t); // add only the first bone
		firstToAdd = false;
		// joint
		if (mAnimData->HasChildren(i))
		{
			BoneJoint bj;
			bj.mIParent = mAnimData->mRootBoneIndex;
			bj.mIChild = i;
			bj.mType = 'u';
			bj.mP1 = bj.mP2 = 0.3f;
			XMVECTOR anchorPos;
			XMMATRIX boneBindToWorldChild = XMLoadFloat4x4(&mAnimData->mBones[i].mBind);
			boneBindToWorldChild = XMMatrixInverse(&anchorPos, boneBindToWorldChild);
			anchorPos = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			anchorPos = XMVector3TransformCoord(anchorPos, boneBindToWorldChild);
			XMStoreFloat3(&bj.mAnchorPos, anchorPos);
			mBoneJoints.push_back(bj);
		}
	}

	// mBoneJoints parent and child indices are currently pointing to the bodies in mAnimData->mBones
	// here they are rearranged so that they point to the mBoneBodies.
	for (UINT i = 0; i < mBoneJoints.size(); ++i)
	{	// correct parent index
		for (UINT j = 0; j < mBoneBodies.size(); ++j)
		{
			if (mBoneBodies[j].mIndex == mBoneJoints[i].mIParent)
			{
				mBoneJoints[i].mIParent = j;
				break;
			}
		}
		// correct child index
		for (UINT j = 0; j < mBoneBodies.size(); ++j)
		{
			if (mBoneBodies[j].mIndex == mBoneJoints[i].mIChild)
			{
				mBoneJoints[i].mIChild = j;
				break;
			}
		}
	}
}

void MeshSimulationData::AddBoneRigidBody(UINT childBoneIndex, bool addBone, UINT boneIndex, CXMVECTOR pos)
{
	XMVECTOR childPos;
	XMMATRIX boneBindToWorld = XMLoadFloat4x4(&mAnimData->mBones[childBoneIndex].mBind);
	boneBindToWorld = XMMatrixInverse(&childPos, boneBindToWorld);
	childPos = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	childPos = XMVector3TransformCoord(childPos, boneBindToWorld);
	if (addBone)
	{
		BoneRigidBody brb;
		brb.mIndex = boneIndex;
		XMVECTOR boneDir = childPos - pos;
		brb.mLength = XMVectorGetX(XMVector3Length(boneDir));
		brb.mThickness.x = brb.mLength * 0.5f;
		brb.mThickness.y = brb.mLength * 0.5f;
		boneDir = XMVector3Normalize(boneDir);
		XMVECTOR bonePos = (childPos + pos) * 0.5f;
		XMStoreFloat3(&brb.mPos, bonePos);
		XMStoreFloat3(&brb.mDir, boneDir);
		CalculateBoneSpace(brb);
		mBoneBodies.push_back(brb);
	}

	// for every child of this bone
	bool firstToAdd = true;
	for (UINT i = 0; i < mAnimData->mBones.size(); ++i)
	if (mAnimData->mBones[i].mParent == childBoneIndex)
	{
		// bone
		AddBoneRigidBody(i, firstToAdd, childBoneIndex, childPos);
		firstToAdd = false;
		// joint
		if (mAnimData->HasChildren(i))
		{
			BoneJoint bj;
			bj.mIParent = childBoneIndex;
			bj.mIChild = i;
			bj.mType = 'u';
			bj.mP1 = bj.mP2 = 0.3f;
			XMVECTOR anchorPos;
			XMMATRIX boneBindToWorldChild = XMLoadFloat4x4(&mAnimData->mBones[i].mBind);
			boneBindToWorldChild = XMMatrixInverse(&anchorPos, boneBindToWorldChild);
			anchorPos = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			anchorPos = XMVector3TransformCoord(anchorPos, boneBindToWorldChild);
			XMStoreFloat3(&bj.mAnchorPos, anchorPos);
			mBoneJoints.push_back(bj);
		}
	}
}

void MeshSimulationData::CalculateBoneSpace(BoneRigidBody &brb)
{
	XMVECTOR boneDir = XMLoadFloat3(&brb.mDir);
	XMVECTOR ori = MathHelper::CreateQuaternionFromDir(boneDir);
	XMVECTOR pos = XMLoadFloat3(&brb.mPos);

	XMMATRIX rigidBodyToWorld = XMMatrixAffineTransformation(
		XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
		ori,
		pos);

	XMVECTOR d;
	XMMATRIX worldToRigidBody = XMMatrixInverse(&d, rigidBodyToWorld);
	XMMATRIX boneBindToWorld = XMLoadFloat4x4(&mAnimData->mBones[brb.mIndex].mBind);
	boneBindToWorld = XMMatrixInverse(&d, boneBindToWorld);

	XMMATRIX boneInRigidBodyLocalSpace = boneBindToWorld * worldToRigidBody;
	XMStoreFloat4x4(&brb.mBoneSpace, boneInRigidBodyLocalSpace);
}

void MeshSimulationData::CalculateAllBoneSpaceMatrices()
{
	for (UINT i = 0; i < mBoneBodies.size(); ++i)
		CalculateBoneSpace(mBoneBodies[i]);
}

bool MeshSimulationData::AreSiblings(UINT boneIndex1, UINT boneIndex2) const
{
	if (boneIndex1 == boneIndex2)
		return false; // can not be sibling with yourself

	int parentIndex1 = -1;
	int parentIndex2 = -1;
	for (UINT i = 0; i < mBoneJoints.size(); ++i)
	{
		if (mBoneJoints[i].mIChild == boneIndex1)
			parentIndex1 = mBoneJoints[i].mIParent;

		if (mBoneJoints[i].mIChild == boneIndex2)
			parentIndex2 = mBoneJoints[i].mIParent;
	}

	if (parentIndex1 == -1 || parentIndex2 == -1)
		return false; // one or more bones do not have a parent :'(

	return (parentIndex1 == parentIndex2);
}