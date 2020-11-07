#ifndef PHYSICS_HEADERS_H
#define PHYSICS_HEADERS_H

#define COLLISION_SKINS_PER_RIGID_BODY 10

#define UPDATE_PHYSICS_ASYNCHRONOUSLY

#define MAXIMUM_DELTA_TIME_FOR_PHYSICS_UPDATE 0.02f

#include <Windows.h>
#include <xnamath.h>
#include <vector>
#include <list>
#include <map>
#include "XnaCollision.h"
#include "MathHelper.h"
#include "Geometry.h"
#include "Exceptions.h"
#include "AsyncWorker.h"
#include "HostedList.h"

namespace Physics_Makina
{
	class CollisionSkin;
	class RigidBody;
	struct CollisionSkin_RigidBody_Instance
	{
		friend class BinnedCollisionSpace;

	public:
		RigidBody *mRigidBody;
		CollisionSkin *mCollisionSkin;

	private:
		// Two of this indexes (from a pair of CollisionSkin_RigidBody_Instance objects) 
		// are used to calculate the third index that is used to check (from a table)
		// whether this pair is already tested in this loop.
		UINT mIndexInCollisionSpace;


		//std::mutex mLock;
	};
}

#endif
