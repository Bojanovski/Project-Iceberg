
#ifndef MESHANIMATIONDATA_H
#define MESHANIMATIONDATA_H

#include "DirectX11Headers.h"
#include <vector>
#include <string>

namespace Makina
{
	struct Bone {
		// Name of the bone
		std::string mName;
		// Index of the bone's parent (root has -1)
		int mParent;
		// Transforms from world to bind space of a bone
		XMFLOAT4X4 mBind;
	};
	struct AnimationKey {
		// Animation time in time stamps
		UINT mT;
		// Transformation matrix
		XMFLOAT4X4 mW;
	};
	struct Animation {
		// Index of a bone this animation is for
		UINT mBone;
		// All animation keys for this bone and this animation
		std::vector<AnimationKey> mAnimKeys;
	};
	// All the data for one animation
	struct AnimationSet {
		// name of the animation
		std::string mName;
		// Animations for specific bones in the skeleton that,
		// when combined, create animation set.
		std::vector<Animation> mAnim;
		// Length (in in time stamps) of the longest animation.
		UINT mTotalTime;
	};

	class MeshAnimationData
	{
		friend class XFileLoader;
		friend class AnimationPlayer;
		friend class MeshSimulationData;

	public:
		__declspec(dllexport) MeshAnimationData();
		__declspec(dllexport) ~MeshAnimationData();

	private:
		void AddBone(const std::string &name, const std::string &parentName, const XMFLOAT4X4 &b);
		void CalculateBoneHierarchy();
		int GetBoneIndex(const std::string &boneName);	
		// Checks whether bone has any children.
		bool HasChildren(UINT boneIndex) const;

		// A scale factor to convert animation time stamps to global time.
		UINT mAnimTicksPerSecond;

		// All the bones int the animation file
		std::vector<Bone> mBones;

		// Index of the root bone
		int mRootBoneIndex;

		// Here are the names of bone parents.
		// They are used as cache and later replaced with indices in Bone structure
		std::vector<std::string> mBoneParents;

		// All the animations
		std::vector<AnimationSet> mAnimationSets;
	};
}

#endif