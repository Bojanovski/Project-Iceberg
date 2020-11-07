#include "BinnedCollisionSpace.h"
#include "Exceptions.h"
#include "RigidBody.h"

#define AABB_Front		0x001;
#define AABB_Back		0x002;
#define AABB_Right		0x004;
#define AABB_Left		0x008;
#define AABB_Bottom		0x010;
#define AABB_Up			0x020;

using namespace Makina;
using namespace Physics_Makina;
using namespace std;

struct AsyncColDetPerformerData
{
	CollisionDetector *colDet;
	BinnedCollisionSpace *mBinnedColSpace;
	CollisionBin *mBin;
};

void BinnedCollisionSpace::AsyncColDetPerformer::Work()
{
	AsyncColDetPerformerData mAsyncColDetPerformerData = *(AsyncColDetPerformerData *)mData;
	CollisionDetector *colDet = mAsyncColDetPerformerData.colDet;
	BinnedCollisionSpace *mBinnedColSpace = mAsyncColDetPerformerData.mBinnedColSpace;
	CollisionBin *mBin = mAsyncColDetPerformerData.mBin;

	// for each movable skin
	for (auto skinInstIt = mBin->mSkinInstances.begin(); skinInstIt != mBin->mSkinInstances.end(); ++skinInstIt)
	{
		XMMATRIX worldA = XMLoadFloat4x4(&(*skinInstIt)->mRigidBody->mTransformMatrix);
		XMVECTOR scale, rotQuat, translation;
		XMMatrixDecompose(&scale, &rotQuat, &translation, worldA);
		OrientedBox transformedBox_Skin;
		TransformOrientedBox(&transformedBox_Skin, &(*skinInstIt)->mCollisionSkin->mOBB, XMVectorGetX(scale), rotQuat, translation);

		// first check collision with all immovable skins
		if ((*skinInstIt)->mRigidBody->IsAwake())
		{
			for (auto secondSkinsIt = mBin->mImmovableSkinInstances.begin(); secondSkinsIt != mBin->mImmovableSkinInstances.end(); ++secondSkinsIt)
			{
				if (mBinnedColSpace->SkipTest(*skinInstIt, *secondSkinsIt)) continue;

				XMMATRIX worldB = XMLoadFloat4x4(&(*secondSkinsIt)->mRigidBody->mTransformMatrix);
				XMMatrixDecompose(&scale, &rotQuat, &translation, worldB);
				OrientedBox transformedBox_SecondSkin;
				TransformOrientedBox(&transformedBox_SecondSkin, &(*secondSkinsIt)->mCollisionSkin->mOBB, XMVectorGetX(scale), rotQuat, translation);

				if (IntersectOrientedBoxOrientedBox(&transformedBox_Skin, &transformedBox_SecondSkin))
					colDet->DetectContacts(*skinInstIt, *secondSkinsIt);
			}
		}

		// then check collision with all other movable skins (that are not already tested!! meaning -->  secondSkinsIt = skinsItNext)
		list<CollisionSkin_RigidBody_Instance *>::iterator skinInsItNext = skinInstIt;
		skinInsItNext++;

		// if this one is last, we have nothing to compare it with
		if (skinInsItNext == mBin->mSkinInstances.end()) break;

		for (auto secondSkinsIt = skinInsItNext; secondSkinsIt != mBin->mSkinInstances.end(); ++secondSkinsIt)
		{
			if (mBinnedColSpace->SkipTest(*skinInstIt, *secondSkinsIt)) continue;
			if (!(*skinInstIt)->mRigidBody->IsAwake() && !(*secondSkinsIt)->mRigidBody->IsAwake()) continue;

			XMMATRIX worldB = XMLoadFloat4x4(&(*secondSkinsIt)->mRigidBody->mTransformMatrix);
			XMMatrixDecompose(&scale, &rotQuat, &translation, worldB);
			Makina::OrientedBox transformedBox_SecondSkin;
			TransformOrientedBox(&transformedBox_SecondSkin, &(*secondSkinsIt)->mCollisionSkin->mOBB, XMVectorGetX(scale), rotQuat, translation);

			if (IntersectOrientedBoxOrientedBox(&transformedBox_Skin, &transformedBox_SecondSkin))
				colDet->DetectContacts(*skinInstIt, *secondSkinsIt);
		}
	}
}

BinnedCollisionSpace::BinnedCollisionSpace(const XMFLOAT3 &numberOfBins, const XMFLOAT3 &spaceCenter, const XMFLOAT3 &spaceDimensionsExtents)
	: mNumberOfBins(numberOfBins),
	mSpaceCenter(spaceCenter),
	mSpaceDimensionsExtents(spaceDimensionsExtents),
	mBinCount((UINT)(numberOfBins.x*numberOfBins.y*numberOfBins.z)),
	mUpdateCodes(NULL),
	mUpdateCodesLocks(NULL),
	mCurentUpdateCode(1),
	mCanCollide(NULL)
{
	mBins = new CollisionBin[mBinCount];
	mAsyncColDetPerformer = new AsyncColDetPerformer[mBinCount];
	mBinCache = new bool[mBinCount];

	mBinExtents = XMFLOAT3(	spaceDimensionsExtents.x / numberOfBins.x,
		spaceDimensionsExtents.y / numberOfBins.y,
		spaceDimensionsExtents.z / numberOfBins.z);

	XMFLOAT3 binSize = XMFLOAT3(	mBinExtents.x * 2.0f,
		mBinExtents.y * 2.0f,
		mBinExtents.z * 2.0f);

	for (UINT x = 0; x < numberOfBins.x; ++x)
		for (UINT y = 0; y < numberOfBins.y; ++y)
			for (UINT z = 0; z < numberOfBins.z; ++z)
			{
				XMFLOAT3 binCenter = XMFLOAT3(	spaceCenter.x - spaceDimensionsExtents.x + binSize.x*x + mBinExtents.x,
					spaceCenter.y - spaceDimensionsExtents.y + binSize.y*y + mBinExtents.y,
					spaceCenter.z - spaceDimensionsExtents.z + binSize.z*z + mBinExtents.z);

				mBins[GetIndexFrom3D(x, y, z)].mBinSpace.Center = binCenter;
				mBins[GetIndexFrom3D(x, y, z)].mBinSpace.Extents = mBinExtents;
			}

}

BinnedCollisionSpace::BinnedCollisionSpace(const BinnedCollisionSpace &obj)
: mNumberOfBins(obj.mNumberOfBins),
mSpaceCenter(obj.mSpaceCenter),
mSpaceDimensionsExtents(obj.mSpaceDimensionsExtents),
mBinCount((UINT)(obj.mNumberOfBins.x*obj.mNumberOfBins.y*obj.mNumberOfBins.z)),
mCurentUpdateCode(obj.mCurentUpdateCode)
{
	// Copy bins
	mBins = new CollisionBin[mBinCount];
	for (UINT i = 0; i < mBinCount; ++i) mBins[i] = obj.mBins[i];

	// Copy chache
	mBinCache = new bool[mBinCount];
	for (UINT i = 0; i < mBinCount; ++i) mBinCache[i] = obj.mBinCache[i];

	// Copy table
	int codesCount = mSkinInstances.size() * (mSkinInstances.size() - 1) / 2;
	mUpdateCodes = new UCHAR[codesCount];
	mUpdateCodesLocks = new mutex[codesCount];
	mCanCollide = new bool[codesCount];
	for (int i = 0; i < codesCount; ++i)
	{
		mUpdateCodes[i] = obj.mUpdateCodes[i];
		mCanCollide[i] = obj.mCanCollide[i];
	}
	// Copy skin instances. We are only interested in pointers.
	// Actual data is stored elsewhere.
	mSkinInstances = obj.mSkinInstances;
}

BinnedCollisionSpace::~BinnedCollisionSpace()
{
	delete[] mBins;
	delete[] mAsyncColDetPerformer;
	delete[] mBinCache;
	if (mUpdateCodes) delete[] mUpdateCodes;
	if (mUpdateCodesLocks) delete[] mUpdateCodesLocks;
	if (mCanCollide) delete[] mCanCollide;
}

void BinnedCollisionSpace::Update()
{
	// first clear movable skins from all bins
	for (UINT i = 0; i < mBinCount; ++i)
		mBins[i].mSkinInstances.clear();

	// now update
	for (auto &skinInstance : mSkinInstances)
	{
		if (skinInstance->mRigidBody->IsMovable())
		{ // test intersection

			XMMATRIX world = XMLoadFloat4x4(&skinInstance->mRigidBody->mTransformMatrix);
			XMVECTOR scale, rotQuat, translation;
			XMMatrixDecompose(&scale, &rotQuat, &translation, world);
			OrientedBox transformedBox_Skin;
			TransformOrientedBox(&transformedBox_Skin, &skinInstance->mCollisionSkin->mOBB, XMVectorGetX(scale), rotQuat, translation);

			UINT index[3];
			bool result = IsCenterInsideBinnedSpace(transformedBox_Skin.Center, index);

#if defined(DEBUG) || defined(_DEBUG)  
			if (!result)
			{
				wstring msg = wstring(L"WARNING: Skin is is being updated with binned collision space but it is outside of it!\n");
				OutputDebugString(&msg[0]);
			}
#endif

			for (UINT i = 0; i < mBinCount; ++i)
				mBinCache[i] = false;

			Add_Skin_Instance_To_Bin(skinInstance, &transformedBox_Skin, index, mBinCache);
		}
	}
}

void BinnedCollisionSpace::RegisterSkinInstance(CollisionSkin_RigidBody_Instance *skinInstance)
{
	XMMATRIX world = XMLoadFloat4x4(&skinInstance->mRigidBody->mTransformMatrix);
	XMVECTOR scale, rotQuat, translation;
	XMMatrixDecompose(&scale, &rotQuat, &translation, world);
	OrientedBox transformedBox_Skin;
	TransformOrientedBox(&transformedBox_Skin, &skinInstance->mCollisionSkin->mOBB, XMVectorGetX(scale), rotQuat, translation);

	UINT index[3];
	bool result = IsCenterInsideBinnedSpace(transformedBox_Skin.Center, index);

#if defined(DEBUG) || defined(_DEBUG)
	if (!result)
	{
		wstring msg = wstring(L"WARNING: Skin is is being registered with binned collision space but it is outside of it!");
		OutputDebugString(&msg[0]);
	}
#endif

	for (UINT i = 0; i < mBinCount; ++i)
		mBinCache[i] = false;

	mSkinInstances.push_back(skinInstance);

	// Update() will take care of this for movable skins.
	if (!skinInstance->mRigidBody->IsMovable()) Add_Skin_Instance_To_Bin(skinInstance, &transformedBox_Skin, index, mBinCache);

	AfterRegisterRemove();
}

void BinnedCollisionSpace::RemoveSkinInstance(CollisionSkin_RigidBody_Instance *skinInstance)
{
	// Erase from mCollisionIgnore
	for (auto it = mCollisionIgnore.begin(); it != mCollisionIgnore.end(); it++)
		if (it->first == skinInstance || it->second == skinInstance)
			mCollisionIgnore.erase(it);

	// Erase from mSkins
	list<CollisionSkin_RigidBody_Instance *>::iterator index = mSkinInstances.end();
	for (auto it = mSkinInstances.begin(); it != mSkinInstances.end(); it++)
	{
		if (*it == skinInstance)
		{
			index = it;
			break;
		}
	}

	if (index == mSkinInstances.end()) throw InvalidOperation(L"Skin is not on the list! (BinnedCollisionSpace::RemoveSkinInstance)");
	mSkinInstances.erase(index);

	// Now from all bins
	for (UINT i = 0; i < mBinCount; ++i)
	{
		list<CollisionSkin_RigidBody_Instance *> *skinsInstPt = (skinInstance->mRigidBody->IsMovable()) ? &mBins[i].mSkinInstances : &mBins[i].mImmovableSkinInstances;

		list<CollisionSkin_RigidBody_Instance *>::iterator index = skinsInstPt->end();
		for (auto it = skinsInstPt->begin(); it != skinsInstPt->end(); it++)
		{
			if (*it == skinInstance)
			{
				index = it;
				break;
			}
		}

		if (index == skinsInstPt->end()) // not registered with this bin
			continue;

		skinsInstPt->erase(index);
	}

	AfterRegisterRemove();
}

void BinnedCollisionSpace::AfterRegisterRemove()
{
	// Update indices
	int indexInCollSpace = 0;
	for (auto &skinInstance : mSkinInstances)
		skinInstance->mIndexInCollisionSpace = indexInCollSpace++;

	// Create new appropriate table
	if (mUpdateCodes) delete [] mUpdateCodes;
	if (mUpdateCodesLocks) delete[] mUpdateCodesLocks;
	if (mCanCollide) delete[] mCanCollide;
	int codesCount = mSkinInstances.size() * (mSkinInstances.size() - 1) / 2;
	mUpdateCodes = new UCHAR[codesCount];
	mUpdateCodesLocks = new mutex[codesCount];
	mCanCollide = new bool[codesCount];
	for (int i = 0; i < codesCount; ++i)
	{
		mUpdateCodes[i] = mCurentUpdateCode;
		mCanCollide[i] = true; // this is default value
	}

	for (auto it = mCollisionIgnore.begin(); it != mCollisionIgnore.end(); it++)
	{
		// generate index (this is using recursive relations)
		int index = it->first->mIndexInCollisionSpace - it->second->mIndexInCollisionSpace - 1 + mSkinInstances.size()*it->second->mIndexInCollisionSpace - it->second->mIndexInCollisionSpace*(it->second->mIndexInCollisionSpace + 1) / 2;
		mCanCollide[index] = false;
	}
}

bool BinnedCollisionSpace::SkipTest(CollisionSkin_RigidBody_Instance *a, CollisionSkin_RigidBody_Instance *b)
{
	if (b->mIndexInCollisionSpace == a->mIndexInCollisionSpace) // nonsense, this should never happen.
		throw UnexpectedError(wstring(L"Two identical instances tested. (BinnedCollisionSpace::PairAlreadyTested)"));

	if (b->mIndexInCollisionSpace > a->mIndexInCollisionSpace)
	{
		// swap
		CollisionSkin_RigidBody_Instance *temp = a;
		a = b;
		b = temp;
	}

	// generate index (this is using recursive relations)
	int index = a->mIndexInCollisionSpace - b->mIndexInCollisionSpace - 1 + mSkinInstances.size()*b->mIndexInCollisionSpace - b->mIndexInCollisionSpace*(b->mIndexInCollisionSpace + 1) / 2;
	lock_guard<mutex> lock(mUpdateCodesLocks[index]); // pairs that are both on a border between two bins will be tested on a separate threads concurrently
	if (mCanCollide[index] == false) return true;
	bool value = (mUpdateCodes[index] == mCurentUpdateCode);

	// update the table
	mUpdateCodes[index] = mCurentUpdateCode;

	return value;
}

void BinnedCollisionSpace::PerformCollisionDetection(CollisionDetector *colDet)
{
	if (++mCurentUpdateCode == 1)
	{
		int codesCount = mSkinInstances.size() * (mSkinInstances.size() - 1) / 2;
		for (int i = 0; i < codesCount; ++i)
			mUpdateCodes[i] = mCurentUpdateCode - 1;
	}

	AsyncColDetPerformerData data;
	data.colDet = colDet;
	data.mBinnedColSpace = this;

	// for each bin
	for (UINT i = 0; i < mBinCount; ++i)
	{
		/*
		// for each movable skin
		for (auto skinInstIt = mBins[i].mSkinInstances.begin(); skinInstIt != mBins[i].mSkinInstances.end(); ++skinInstIt)
		{	
			XMMATRIX worldA = XMLoadFloat4x4(&(*skinInstIt)->mRigidBody->mTransformMatrix);
			XMVECTOR scale, rotQuat, translation;
			XMMatrixDecompose(&scale, &rotQuat, &translation, worldA);
			OrientedBox transformedBox_Skin;
			TransformOrientedBox(&transformedBox_Skin, &(*skinInstIt)->mCollisionSkin->mOBB, XMVectorGetX(scale), rotQuat, translation);

			// first check collision with all immovable skins
			if ((*skinInstIt)->mRigidBody->IsAwake())
			{
				for (auto secondSkinsIt = mBins[i].mImmovableSkinInstances.begin(); secondSkinsIt != mBins[i].mImmovableSkinInstances.end(); ++secondSkinsIt)
				{
					if (PairAlreadyTested(*skinInstIt, *secondSkinsIt)) continue;

					XMMATRIX worldB = XMLoadFloat4x4(&(*secondSkinsIt)->mRigidBody->mTransformMatrix);
					XMMatrixDecompose(&scale, &rotQuat, &translation, worldB);
					OrientedBox transformedBox_SecondSkin;
					TransformOrientedBox(&transformedBox_SecondSkin, &(*secondSkinsIt)->mCollisionSkin->mOBB, XMVectorGetX(scale), rotQuat, translation);

					if (IntersectOrientedBoxOrientedBox(&transformedBox_Skin, &transformedBox_SecondSkin))
						colDet->DetectContacts(*skinInstIt, *secondSkinsIt);
				}
			}

			// then check collision with all other movable skins (that are not already tested!! meaning -->  secondSkinsIt = skinsItNext)
			list<CollisionSkin_RigidBody_Instance *>::iterator skinInsItNext = skinInstIt;
			skinInsItNext++;

			// if this one is last, we have nothing to compare it with
			if (skinInsItNext == mBins[i].mSkinInstances.end()) break;

			for (auto secondSkinsIt = skinInsItNext; secondSkinsIt != mBins[i].mSkinInstances.end(); ++secondSkinsIt)
			{
				if (PairAlreadyTested(*skinInstIt, *secondSkinsIt)) continue;
				if (!(*skinInstIt)->mRigidBody->IsAwake() && !(*secondSkinsIt)->mRigidBody->IsAwake()) continue;

				XMMATRIX worldB = XMLoadFloat4x4(&(*secondSkinsIt)->mRigidBody->mTransformMatrix);
				XMMatrixDecompose(&scale, &rotQuat, &translation, worldB);
				Makina::OrientedBox transformedBox_SecondSkin;
				TransformOrientedBox(&transformedBox_SecondSkin, &(*secondSkinsIt)->mCollisionSkin->mOBB, XMVectorGetX(scale), rotQuat, translation);

				if (IntersectOrientedBoxOrientedBox(&transformedBox_Skin, &transformedBox_SecondSkin))
					colDet->DetectContacts(*skinInstIt, *secondSkinsIt);
			}
		}
		*/

		data.mBin = &mBins[i];
		mAsyncColDetPerformer[i].Assign(&data, sizeof(data));
#ifndef UPDATE_PHYSICS_ASYNCHRONOUSLY
		mAsyncColDetPerformer[i].Join();
#endif
	}

#ifdef UPDATE_PHYSICS_ASYNCHRONOUSLY
	for (UINT i = 0; i < mBinCount; ++i) mAsyncColDetPerformer[i].Join();
#endif
}

void BinnedCollisionSpace::AddToCollisionIgnore(CollisionSkin_RigidBody_Instance *first, CollisionSkin_RigidBody_Instance *second)
{
	if (second->mIndexInCollisionSpace > first->mIndexInCollisionSpace)
	{
		// swap
		CollisionSkin_RigidBody_Instance *temp = first;
		first = second;
		second = temp;
	}
	pair<CollisionSkin_RigidBody_Instance *, CollisionSkin_RigidBody_Instance *> p;
	p.first = first;
	p.second = second;
	mCollisionIgnore.push_back(p);

	// update table
	int index = p.first->mIndexInCollisionSpace - p.second->mIndexInCollisionSpace - 1 + mSkinInstances.size()*p.second->mIndexInCollisionSpace - p.second->mIndexInCollisionSpace*(p.second->mIndexInCollisionSpace + 1) / 2;
	lock_guard<mutex> lock(mUpdateCodesLocks[index]);
	mCanCollide[index] = false;
}

int BinnedCollisionSpace::IntersectOrientedBoxAxisAlignedBox( const OrientedBox &obb, AxisAlignedBox &aabb, char &adjacentContainers)
{
	XMVECTOR aabbCenter = XMLoadFloat3(&aabb.Center);
	XMVECTOR aabbStart = aabbCenter - XMLoadFloat3(&aabb.Extents);
	XMVECTOR aabbEnd = aabbCenter + XMLoadFloat3(&aabb.Extents);
	XMVECTOR normal, plane;

	adjacentContainers = 0;
	bool isCompletelyInside = true;
	int result;

	//
	// Faces
	//
	// If result is 2 then obb is completely inside
	// if however result is 0, there is no way obb can be inside this aabb
	// As for return value of 1, obb is partially inside this aabb and partially inside adjacent one. adjacentContainers is updated accordingly.
	//
	//

	// front
	normal = XMVectorSet(0, 0, -1, 0);
	plane = XMPlaneFromPointNormal(aabbStart, normal);
	result = IntersectOrientedBoxPlane(&obb, plane);
	if (result == 0) return 0;
	else if (result == 1)
	{
		isCompletelyInside = false;
		adjacentContainers |= AABB_Front;
	}

	// back
	normal = XMVectorSet(0, 0, 1, 0);
	plane = XMPlaneFromPointNormal(aabbEnd, normal);
	result = IntersectOrientedBoxPlane(&obb, plane);
	if (result == 0) return 0;
	else if (result == 1)
	{
		isCompletelyInside = false;
		adjacentContainers |= AABB_Back;
	}

	// right
	normal = XMVectorSet(1, 0, 0, 0);
	plane = XMPlaneFromPointNormal(aabbEnd, normal);
	result = IntersectOrientedBoxPlane(&obb, plane);
	if (result == 0) return 0;
	else if (result == 1)
	{
		isCompletelyInside = false;
		adjacentContainers |= AABB_Right;
	}

	// left
	normal = XMVectorSet(-1, 0, 0, 0);
	plane = XMPlaneFromPointNormal(aabbStart, normal);
	result = IntersectOrientedBoxPlane(&obb, plane);
	if (result == 0) return 0;
	else if (result == 1)
	{
		isCompletelyInside = false;
		adjacentContainers |= AABB_Left;
	}

	// bottom
	normal = XMVectorSet(0, -1, 0, 0);
	plane = XMPlaneFromPointNormal(aabbStart, normal);
	result = IntersectOrientedBoxPlane(&obb, plane);
	if (result == 0) return 0;
	else if (result == 1)
	{
		isCompletelyInside = false;
		adjacentContainers |= AABB_Bottom;
	}

	// up
	normal = XMVectorSet(0, 1, 0, 0);
	plane = XMPlaneFromPointNormal(aabbEnd, normal);
	result = IntersectOrientedBoxPlane(&obb, plane);
	if (result == 0) return 0;
	else if (result == 1)
	{
		isCompletelyInside = false;
		adjacentContainers |= AABB_Up;
	}

	if (isCompletelyInside)
		return 2;
	else 
		return 1;
}

bool BinnedCollisionSpace::IsCenterInsideBinnedSpace(const XMFLOAT3 &center, UINT index[3])
{
	// Offset the centerPoint to the first octant (this octant now contains whole binned space) and then divide to get proper indices
	index[0] = UINT(	(center.x - mSpaceCenter.x + mSpaceDimensionsExtents.x) / (2*mBinExtents.x));
	index[1] = UINT(	(center.y - mSpaceCenter.y + mSpaceDimensionsExtents.y) / (2*mBinExtents.y));
	index[2] = UINT(	(center.z - mSpaceCenter.z + mSpaceDimensionsExtents.z) / (2*mBinExtents.z));

	// is it outside??
	if ((index[0] >= (UINT)(mNumberOfBins.x)) ||
		(index[1] >= (UINT)(mNumberOfBins.y)) ||
		(index[2] >= (UINT)(mNumberOfBins.z)) ||
		index[0] < 0 ||
		index[1] < 0 ||
		index[2] < 0)
		return false;

	// else
	return true;
}

void BinnedCollisionSpace::Add_Skin_Instance_To_Bin(CollisionSkin_RigidBody_Instance *skinInstance, Makina::OrientedBox *oob, UINT index[3], bool *isChecked)
{
	char adjacentContainers;
	int i = GetIndexFrom3D(index);
	if (i < 0) return; // This bin does not exist, obb is partially outside the binning space.

	if (isChecked[i]) // Already processed?
		return;

	// adjacentContainers is 6 bit flags inside char
	int result = IntersectOrientedBoxAxisAlignedBox(*oob, mBins[i].mBinSpace, adjacentContainers);

	if (result == 0) // skin is outside completely
		return;

	// Make sure we dont process this bin again
	isChecked[i] = true;

	// Add skin
	if (skinInstance->mRigidBody->IsMovable()) mBins[i].mSkinInstances.push_back(skinInstance);
	else mBins[i].mImmovableSkinInstances.push_back(skinInstance);

	if (result == 2) // skin is inside completely
		return;

	char mask;
	UINT newIndex[3];

	// front
	mask = AABB_Front;
	newIndex[0] = index[0];
	newIndex[1] = index[1];
	newIndex[2] = index[2] - 1;
	if (adjacentContainers & mask)
		Add_Skin_Instance_To_Bin(skinInstance, oob, newIndex, isChecked);

	// back
	mask = AABB_Back;
	newIndex[0] = index[0];
	newIndex[1] = index[1];
	newIndex[2] = index[2] + 1;
	if (adjacentContainers & mask)
		Add_Skin_Instance_To_Bin(skinInstance, oob, newIndex, isChecked);

	// right
	mask = AABB_Right;
	newIndex[0] = index[0] + 1;
	newIndex[1] = index[1];
	newIndex[2] = index[2];
	if (adjacentContainers & mask)
		Add_Skin_Instance_To_Bin(skinInstance, oob, newIndex, isChecked);

	// left
	mask = AABB_Left;
	newIndex[0] = index[0] - 1;
	newIndex[1] = index[1];
	newIndex[2] = index[2];
	if (adjacentContainers & mask)
		Add_Skin_Instance_To_Bin(skinInstance, oob, newIndex, isChecked);

	// bottom
	mask = AABB_Bottom;
	newIndex[0] = index[0];
	newIndex[1] = index[1] - 1;
	newIndex[2] = index[2];
	if (adjacentContainers & mask)
		Add_Skin_Instance_To_Bin(skinInstance, oob, newIndex, isChecked);

	// up
	mask = AABB_Up;
	newIndex[0] = index[0];
	newIndex[1] = index[1] + 1;
	newIndex[2] = index[2];
	if (adjacentContainers & mask)
		Add_Skin_Instance_To_Bin(skinInstance, oob, newIndex, isChecked);
}

UINT BinnedCollisionSpace::GetIndexFrom3D(UINT x, UINT y, UINT z)
{
	if (x < 0 || y < 0 || z < 0 ||
		x >= mNumberOfBins.x || y >= mNumberOfBins.y || z >= mNumberOfBins.z) return -1;

	UINT result = (UINT)(x + y*mNumberOfBins.y + z*mNumberOfBins.x*mNumberOfBins.y);
	return result;
}