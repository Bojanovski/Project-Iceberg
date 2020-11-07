
#ifndef WORLD_H
#define WORLD_H

#include "PhysicsHeaders.h"
#include "RigidBodyForceRegistry.h"

namespace Physics_Makina
{
	class RigidBody;

	class World
	{
	public:
		__declspec(dllexport) World();
		__declspec(dllexport) ~World();

		/**
		* Initializes the world for a simulation frame. This clears
		* the force and torque accumulators for bodies in the
		* world. After calling this, the bodies can have their forces
		* and torques for this frame added.
		*/
		__declspec(dllexport) void Prepare();

		/**
		* Processes all the physics for the world.
		*/
		__declspec(dllexport) void Update(float dt);

		/**
		* Updates transformation matrices used for rendering. These represent
		* the scene in which collision resolution is finished.
		*/
		__declspec(dllexport) void UpdateTransformationMatrices_Rend();

		// Registrations
		__declspec(dllexport) void RegisterBody(RigidBody *body);
		__declspec(dllexport) void RemoveBody(RigidBody *body);
		RigidBodyForceRegistry &GetRigidBodyForceRegistry() { return mReg; }

	private:
		std::vector<RigidBody *> mBodies;
		float mCheckForSleepTimer;
		RigidBodyForceRegistry mReg;
	};
}

#endif
