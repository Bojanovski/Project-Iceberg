#include "ForceGenerators.h"
#include "RigidBody.h"

using namespace std;
using namespace Physics_Makina;

ForceGenerator::ForceGenerator()
{

}

ForceGenerator::~ForceGenerator()
{

}

void ForceGenerator::AddForce(RigidBody *body, const XMFLOAT3 &force)
{ 
	body->AddForce(force); 
}

void ForceGenerator::AddTorque(RigidBody *body, const XMFLOAT3 &torque)
{ 
	body->AddTorque(torque); 
}

void ForceGenerator::AddForceAtBodyPoint(RigidBody *body, const XMFLOAT3 &force, const XMFLOAT3 &point)
{ 
	body->AddForceAtBodyPoint(force, point); 
}

void ForceGenerator::AddForceAtPoint(RigidBody *body, const XMFLOAT3 &force, const XMFLOAT3 &point)
{ 
	body->AddForceAtPoint(force, point);
}

Gravity::Gravity(const XMFLOAT3 &gravity)
	: ForceGenerator(),
	mGravity(gravity)
{

}

Gravity::~Gravity()
{

}

XMFLOAT3 operator* (const XMFLOAT3 &left, const float &right)
{
	return XMFLOAT3(left.x * right, left.y * right, left.z * right);
}

void Gravity::ApplyForce(RigidBody *body, float dt)
{
	// Check that we do not have infinite mass
	if (!body->HasFiniteMass()) return;

	// Apply the mass-scaled force to the body
	AddForce(body, mGravity * body->GetMass());

	// for testing purposes
	//AddForceAtBodyPoint(body, mGravity * -body->GetMass() * 1.0f, XMFLOAT3(1.7f, -1.5f, 0.0f));
	//AddTorque(body, XMFLOAT3(1, 0, 0));
}

ForceQueue::ForceQueue()
: ForceGenerator()
{

}

ForceQueue::~ForceQueue()
{

}

void ForceQueue::AddForceToQueue(const XMFLOAT3 &f)
{
	lock_guard<mutex> lock(mLock);
	mForces.push_back(f); 
}

void ForceQueue::AddTorqueToQueue(const XMFLOAT3 &t)
{
	lock_guard<mutex> lock(mLock);
	mTorques.push_back(t);
}

void ForceQueue::ApplyForce(RigidBody *body, float dt)
{
	// Check that we do not have infinite mass
	if (!body->HasFiniteMass() || (mForces.empty() && mTorques.empty())) return;

	lock_guard<mutex> lock(mLock);

	// wakie-wakie
	if (!body->IsAwake()) body->SetAwake(true);

	// Apply all the forces
	for (auto &f : mForces) AddForce(body, f);

	// Apply all the torques
	for (auto &t : mTorques) AddTorque(body, t);

	// Clear all the queues
	mForces.clear();
	mTorques.clear();
}