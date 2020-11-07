#include "RigidBodyForceRegistry.h"
#include "ForceGenerators.h"
#include "Exceptions.h"

using namespace Physics_Makina;
using namespace Makina;

void RigidBodyForceRegistry::Add(RigidBody *body, ForceGenerator *forceGen)
{
	Registration reg = {body, forceGen};
	mReg.push_back(reg);
}

void RigidBodyForceRegistry::Remove(RigidBody *body, ForceGenerator *forceGen)
{
	Registration reg = {body, forceGen};

	// erase from mBodies
	int index = -1;
	UINT size = mReg.size();
	for (UINT i = 0; i < size; ++i)
	{
		if (mReg[i] == reg)
		{
			index = i;
			break;
		}
	}

	if (index == -1) throw InvalidOperation(L"Registry is not on the list! (RigidBodyForceRegistry::Remove)");
	mReg.erase(mReg.begin() + index);
}

void RigidBodyForceRegistry::UpdateForces(float dt)
{
	std::vector<Registration>::iterator i = mReg.begin();
	for (; i != mReg.end(); i++)
	{
		i->forceGen->ApplyForce(i->body, dt);
	}
}