#include "World.h"
#include "Exceptions.h"
#include "RigidBody.h"

using namespace Makina;
using namespace Physics_Makina;

World::World()
:mCheckForSleepTimer(0.0f)
{

}

World::~World()
{
	for (auto &body : mBodies)
		delete body;
}

void World::Prepare()
{
	for (auto &body : mBodies)
	{
		body->ClearAccumulators();
		body->UpdateTransformationMatrix();	
	}
}

void World::Update(float dt)
{
	// First apply the force generators
	mReg.UpdateForces(dt);

	// Then integrate the objects
	for (auto &body : mBodies)
		body->Integrate(dt);

	// Check if some rigid bodies should be put to sleep.
	mCheckForSleepTimer += dt;
	if (mCheckForSleepTimer >= 1.0f)
	{
		for (auto &body : mBodies) if (body->IsAwake()) body->CheckForSleep();
		mCheckForSleepTimer = 0.0f;
	}
}

void World::UpdateTransformationMatrices_Rend()
{
	for (auto &body : mBodies)
	{
		// Proper rigid body transformation matrix is saved for drawing.
		body->mTransformMatrix_Rend = body->mTransformMatrix;
	}
}

void World::RegisterBody(RigidBody *body)
{
	mBodies.push_back(body);
	body->mReg = true;
}

void World::RemoveBody(RigidBody *body)
{
	// erase from mBodies
	int index = -1;
	UINT size = mBodies.size();
	for (UINT i = 0; i < size; ++i)
	{
		if (mBodies[i] == body)
		{
			index = i;
			break;
		}
	}

	if (index == -1) throw InvalidOperation(L"Rigid body is not registered! (World::RemoveBody)");
	mBodies.erase(mBodies.begin() + index);
}