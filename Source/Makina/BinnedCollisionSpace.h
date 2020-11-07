
#ifndef BINNED_COLLISION_SPACE_H
#define BINNED_COLLISION_SPACE_H

#include "PhysicsHeaders.h"
#include "CollisionSkin.h"
#include "CollisionBin.h"
#include "CollisionDetector.h"

namespace Physics_Makina
{
	class BinnedCollisionSpace
	{
		class AsyncColDetPerformer : public Makina::AsyncWorker
		{
		public:
			AsyncColDetPerformer(){}
			~AsyncColDetPerformer(){}

		private:
			void Work();
		};

	public:
		__declspec(dllexport) BinnedCollisionSpace(const XMFLOAT3 &numberOfBins, const XMFLOAT3 &spaceCenter, const XMFLOAT3 &spaceDimensionsExtents);
		__declspec(dllexport) BinnedCollisionSpace(const BinnedCollisionSpace &obj);
		__declspec(dllexport) ~BinnedCollisionSpace();

		__declspec(dllexport) void Update();

		__declspec(dllexport) void RegisterSkinInstance(CollisionSkin_RigidBody_Instance *skinInstance);
		__declspec(dllexport) void RemoveSkinInstance(CollisionSkin_RigidBody_Instance *skinInstance);

		__declspec(dllexport) void PerformCollisionDetection(CollisionDetector *colDet);

		__declspec(dllexport) void AddToCollisionIgnore(CollisionSkin_RigidBody_Instance *first, CollisionSkin_RigidBody_Instance *second);

	private:
		//-----------------------------------------------------------------------------
		// Return values: 0 = volume is outside the AxisAlignedBox,
		//                1 = volume intersects the AxisAlignedBox,
		//                2 = volume is inside the AxisAlignedBox 
		//-----------------------------------------------------------------------------
		int IntersectOrientedBoxAxisAlignedBox( const Makina::OrientedBox &obb, Makina::AxisAlignedBox &aabb, char &adjacentContainers);

		// Returns false if position is outside binned collision space
		bool IsCenterInsideBinnedSpace(const XMFLOAT3 &center, UINT index[3]);

		// This method is called recursively, it adds skin instance to bin (if not already added) and calls the same function for all bins sharing this skin
		void Add_Skin_Instance_To_Bin(CollisionSkin_RigidBody_Instance *skinInstance, Makina::OrientedBox *oob, UINT index[3], bool *isChecked);

		// Helper method for RegisterSkinInstance() and RemoveSkinInstance() methods.
		void AfterRegisterRemove();

		// Looks into table (mUpdateCodes) and returns true if pair was already tested, otherwise it returns false and updates the table.
		// Also check if this pair is registered via mCollisionIgnore.
		bool SkipTest(CollisionSkin_RigidBody_Instance *a, CollisionSkin_RigidBody_Instance *b);

		// Will return -1 if index is outside bonds.
		UINT GetIndexFrom3D(UINT x, UINT y, UINT z);
		UINT GetIndexFrom3D(UINT index[3]) {return GetIndexFrom3D(index[0], index[1], index[2]);}

		XMFLOAT3 mNumberOfBins, mBinExtents, mSpaceCenter, mSpaceDimensionsExtents;
		UINT mBinCount; // = numberOfBins.x * numberOfBins.y * numberOfBins.z
		CollisionBin *mBins;

		// Each bin performs its async collision detection using this class.
		AsyncColDetPerformer *mAsyncColDetPerformer;

		// Holds information whether current bin has already been checked while calling recursive function Add_Skin_Instance_To_Bin(...).
		bool *mBinCache;

		// All skin instances in entire binned collision space.
		std::list<CollisionSkin_RigidBody_Instance *> mSkinInstances;

		// One pair of skin instances can be checked multiple times if both skin instances are contained in multiple common bins.
		// This table of update codes prevents that. Length is 1/2 * mSkinInstances * (mSkinInstances - 1)
		UCHAR *mUpdateCodes;
		std::mutex *mUpdateCodesLocks;

		// Used to define collision skin instance pairs that can collide
		bool *mCanCollide;
		std::vector<std::pair<CollisionSkin_RigidBody_Instance *, CollisionSkin_RigidBody_Instance *>> mCollisionIgnore;

		// Current update code
		UCHAR mCurentUpdateCode;
	};
}

#endif
