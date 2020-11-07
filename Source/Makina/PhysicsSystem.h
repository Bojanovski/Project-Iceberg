
#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "GameComponent.h"
#include "AsyncWorker.h"
#include <vector>
#include <mutex>

namespace Physics_Makina
{
	class World;
	class RigidBody;
	class ForceGenerator;
	class CollisionDetector;
	class CollisionResolver;
	class BinnedCollisionSpace;
	class CollisionSkin;
	struct CollisionSkin_RigidBody_Instance;
	class Joint;
}

namespace Makina
{
	class PhysicsSystem : public GameComponent, private AsyncWorker
	{
	public:
		__declspec(dllexport) PhysicsSystem(D3DAppValues *value);
		__declspec(dllexport) ~PhysicsSystem();

		__declspec(dllexport) void Update(float dt);
		__declspec(dllexport) void Draw(float dt);
		__declspec(dllexport) void OnResize();

		// Rigid body registrations and rigid body related stuff
		__declspec(dllexport) void RegisterRigidBody(Physics_Makina::RigidBody *rigidBody);
		__declspec(dllexport) void AddForceGeneratorToRigidBody(Physics_Makina::ForceGenerator *forceGen, Physics_Makina::RigidBody *rigidBody);
		__declspec(dllexport) void AddToCollisionIgnore(Physics_Makina::CollisionSkin_RigidBody_Instance *first, Physics_Makina::CollisionSkin_RigidBody_Instance *second);
		__declspec(dllexport) void AddJoint(Physics_Makina::Joint *joint);

		// AsyncWorker's Join method method (private inheritance is used to hide everything else).
		void Join() { AsyncWorker::Join(); }

	private:
		// AsyncWorker's Work method.
		__declspec(dllexport) void Work();

		Physics_Makina::World *mWorld;
		Physics_Makina::CollisionDetector *mColDet;
		Physics_Makina::CollisionResolver *mColRes;
		Physics_Makina::BinnedCollisionSpace *mColSpace;
	};
}

#endif
