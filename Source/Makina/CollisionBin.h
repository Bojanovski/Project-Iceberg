
#ifndef COLLISION_BIN_H
#define COLLISION_BIN_H

#include "PhysicsHeaders.h"

namespace Physics_Makina
{
	struct CollisionBin
	{
		std::list<CollisionSkin_RigidBody_Instance *> mSkinInstances;
		std::list<CollisionSkin_RigidBody_Instance *> mImmovableSkinInstances;
		Makina::AxisAlignedBox mBinSpace;

		// Dynamic allocation must be aligned to the 16, therefore custom 'new' and 'delete' operators are needed.
		void *operator new(size_t size)
		{
			void *storage = _aligned_malloc(size, 16);
			if (NULL == storage)
			{
				throw Makina::AllocationError(L"No free memory. (CollisionBin.h)");
			}
			return storage;
		}

		void *operator new[](size_t size)
		{
			void *storage = _aligned_malloc(size, 16);
			if (NULL == storage)
			{
				throw Makina::AllocationError(L"No free memory. (CollisionBin.h)");
			}
			return storage;
		}

		void operator delete(void *pt)
		{
			_aligned_free(pt);
		}

		void operator delete[](void *pt)
		{
			_aligned_free(pt);
		}
	};
}

#endif
