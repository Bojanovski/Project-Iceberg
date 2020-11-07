
#include "MeshAnimationData.h"
#include "Exceptions.h"

using namespace Makina;
using namespace std;

MeshAnimationData::MeshAnimationData()
{

}

MeshAnimationData::~MeshAnimationData()
{

}

void MeshAnimationData::AddBone(const string &name, const string &parentName, const XMFLOAT4X4 &b)
{
	Bone bone;
	bone.mBind = b;
	bone.mName = name;
	bone.mParent = -1;
	mBones.push_back(bone);
	mBoneParents.push_back(parentName);
}

void MeshAnimationData::CalculateBoneHierarchy()
{
	for (unsigned int boneI = 0; boneI < mBones.size(); ++boneI)
	{
		// find parent index
		int parentI = GetBoneIndex(mBoneParents[boneI]);
		if (parentI == -1)
		{
			if (mBoneParents[boneI].size() > 0)
				throw UnexpectedError(L"No parent bone found for a non-root bone. (MeshAnimationData::CalculateHierarchy)");
			else // this is the root bone
				mRootBoneIndex = boneI;
		}
		else
			mBones[boneI].mParent = parentI;
	}

	mBoneParents.clear();
}

int MeshAnimationData::GetBoneIndex(const std::string &boneName)
{
	for (unsigned int i = 0; i < mBones.size(); ++i)
	{
		if (mBones[i].mName.compare(boneName) == 0)
		{
			return i;
		}
	}
	return -1;
}

bool MeshAnimationData::HasChildren(UINT boneIndex) const
{
	for (unsigned int i = 0; i < mBones.size(); ++i)
	{
		if (mBones[i].mParent == boneIndex)
		{
			return true;
		}
	}
	return false;
}