
#ifndef MESHSIMULATIONDATA_H
#define MESHSIMULATIONDATA_H

#include "DirectX11Headers.h"
#include <vector>
#include <string>

namespace Makina
{
	struct BoneRigidBody
	{
		// index of a bone in MeshAnimationData::mBones
		UINT mIndex;
		// bone space in rigid body local space
		XMFLOAT4X4 mBoneSpace;
		// initial position of the bone rigid body
		XMFLOAT3 mPos;
		// length of the bone
		float mLength;
		// thickness
		XMFLOAT2 mThickness;
		// direction of the bone
		XMFLOAT3 mDir;
	};

	struct BoneJoint
	{
		// index of a parent bone in MeshSimulationData::mBoneBodies
		UINT mIParent;
		// index of a child bone in MeshSimulationData::mBoneBodies
		UINT mIChild;
		// initial position of the joint anchor
		XMFLOAT3 mAnchorPos;
		// type of the joint (hinge or universal)
		char mType;
		// first and second parameter (their usage depends on type)
		float mP1, mP2;
	};

	class MeshAnimationData;

	class MeshSimulationData
	{
		friend class XFileLoader;
		friend class RagdollAnimationPlayer;

	public:
		__declspec(dllexport) MeshSimulationData(MeshAnimationData *animData);
		__declspec(dllexport) ~MeshSimulationData();

		// Checks whether two bones have the same parent bone.
		__declspec(dllexport) bool AreSiblings(UINT boneIndex1, UINT boneIndex2) const;

	private:
		void GenerateData();
		void AddBoneRigidBody(UINT childBoneIndex, bool addBone, UINT boneIndex, CXMVECTOR pos);
		void CalculateBoneSpace(BoneRigidBody &brb);
		void CalculateAllBoneSpaceMatrices();

		MeshAnimationData *mAnimData;
		std::vector<BoneRigidBody> mBoneBodies;
		std::vector<BoneJoint> mBoneJoints;
	};
}

#endif
