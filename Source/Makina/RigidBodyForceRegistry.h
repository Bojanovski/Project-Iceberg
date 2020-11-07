
#ifndef RIGID_BODY_FORCE_REGISTRY_H
#define RIGID_BODY_FORCE_REGISTRY_H

#include "PhysicsHeaders.h"

namespace Physics_Makina
{
	class RigidBody;
	class ForceGenerator;

	class RigidBodyForceRegistry
	{
		friend class World;

	public:
		__declspec(dllexport) void Add(RigidBody *body, ForceGenerator *forceGen);
		__declspec(dllexport) void Remove(RigidBody *body, ForceGenerator *forceGen);

	private:
		__declspec(dllexport) void UpdateForces(float dt);

		struct Registration
		{
			RigidBody *body;
			ForceGenerator *forceGen;

			bool operator== (const Registration &val) { return ((body == val.body) && (forceGen == val.forceGen)); }
		};

		std::vector<Registration> mReg;
	};
}

#endif
