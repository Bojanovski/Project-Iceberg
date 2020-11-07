#include "PhysicsSystem.h"
#include "World.h"
#include "CollisionDetector.h"
#include "CollisionResolver.h"
#include "BinnedCollisionSpace.h"
#include "RigidBody.h"
#include "Geometry.h"
#include "CollisionSkin.h"

using namespace std;
using namespace Makina;
using namespace Physics_Makina;

PhysicsSystem::PhysicsSystem(D3DAppValues *value)
	: GameComponent(value)
{
	mWorld = new World();
	mColDet = new CollisionDetector(0.01f, 0.02f);
	mColRes = new CollisionResolver(100, 100, 0.1f, 0.5f, 0.8f, 0.8f);
	mColSpace = new BinnedCollisionSpace(XMFLOAT3(2, 2, 2), XMFLOAT3(0, 0, 0), XMFLOAT3(200, 200, 200));
}

PhysicsSystem::~PhysicsSystem()
{
	Join();

	delete mWorld;
	delete mColDet;
	delete mColRes;
	delete mColSpace;
}

void PhysicsSystem::Update(float dt)
{
	if (dt > MAXIMUM_DELTA_TIME_FOR_PHYSICS_UPDATE) dt = MAXIMUM_DELTA_TIME_FOR_PHYSICS_UPDATE;

	//dt = 1.0f / 100.f;

#ifdef UPDATE_PHYSICS_ASYNCHRONOUSLY
	Join();
	mWorld->UpdateTransformationMatrices_Rend();
	Assign(&dt, sizeof(dt));
#else
	mWorld->UpdateTransformationMatrices_Rend();

	// Integration (Euler method)
	mWorld->Prepare();
	mWorld->Update(dt*1.0f);

	// Update collision space (broad phase)
	mColSpace->Update();

	// Perform revalidation of contacts from previous frame
	mColDet->RevalidatePreviousContacts();

	// Actual collision detection
	mColSpace->PerformCollisionDetection(mColDet);

	// Resolution
	mColRes->ResolveContacts(mColDet, dt*1.0f);
#endif
}

void PhysicsSystem::Draw(float dt)
{

}

void PhysicsSystem::OnResize()
{

}

void PhysicsSystem::RegisterRigidBody(Physics_Makina::RigidBody *rigidBody)
{
	Join();

	// Register with physics world.
	mWorld->RegisterBody(rigidBody);

	// Register body with collision space
	for (int i = 0; i < rigidBody->GetCollisionSkin_RigidBody_InstanceCount(); ++i)
		mColSpace->RegisterSkinInstance(&rigidBody->GetCollisionSkin_RigidBody_Instance(i)); 
	
	// collision ignore (in case of multiple skins per body)
	for (int i = 0; i < rigidBody->GetCollisionSkin_RigidBody_InstanceCount(); ++i)
	for (int j = i + 1; j < rigidBody->GetCollisionSkin_RigidBody_InstanceCount(); ++j)
		mColSpace->AddToCollisionIgnore(&rigidBody->GetCollisionSkin_RigidBody_Instance(i), &rigidBody->GetCollisionSkin_RigidBody_Instance(j));
}

void PhysicsSystem::AddForceGeneratorToRigidBody(Physics_Makina::ForceGenerator *forceGen, Physics_Makina::RigidBody *rigidBody)
{
	Join();

	mWorld->GetRigidBodyForceRegistry().Add(rigidBody, forceGen);
}

void PhysicsSystem::AddToCollisionIgnore(CollisionSkin_RigidBody_Instance *first, CollisionSkin_RigidBody_Instance *second)
{
	mColSpace->AddToCollisionIgnore(first, second);
}

void PhysicsSystem::AddJoint(Joint *joint)
{
	mColRes->AddJoint(joint); 
}

void PhysicsSystem::Work()
{
	float dt = *(float *)mData;

	// Integration (Euler method)
	mWorld->Prepare();
	mWorld->Update(dt*1.0f);

	// Update collision space (broad phase)
	mColSpace->Update();

	// Perform revalidation of contacts from previous frame
	mColDet->RevalidatePreviousContacts();

	// Actual collision detection
	mColSpace->PerformCollisionDetection(mColDet);

	// Resolution
	mColRes->ResolveContacts(mColDet, dt*1.0f);
}