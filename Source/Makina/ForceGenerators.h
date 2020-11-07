
#ifndef FORCES_H
#define FORCES_H

#include "PhysicsHeaders.h"

namespace Physics_Makina
{
	class RigidBody;

	class ForceGenerator
	{
	public:
		__declspec(dllexport) ForceGenerator();
		__declspec(dllexport) virtual ~ForceGenerator();

		virtual void ApplyForce(RigidBody *body, float dt) = 0;

	protected:
		// For accessing private members of RigidBody (friendships do not inherit).
		__declspec(dllexport) void AddForce(RigidBody *body, const XMFLOAT3 &force);
		__declspec(dllexport) void AddTorque(RigidBody *body, const XMFLOAT3 &torque);
		__declspec(dllexport) void AddForceAtBodyPoint(RigidBody *body, const XMFLOAT3 &force, const XMFLOAT3 &point);
		__declspec(dllexport) void AddForceAtPoint(RigidBody *body, const XMFLOAT3 &force, const XMFLOAT3 &point);
	};


	class Gravity : public ForceGenerator
	{
	public:
		__declspec(dllexport) Gravity(const XMFLOAT3 &gravity);
		__declspec(dllexport) virtual ~Gravity();

	private:
		void ApplyForce(RigidBody *body, float dt);

		XMFLOAT3 mGravity;
	};

	class ForceQueue : public ForceGenerator
	{
	public:
		__declspec(dllexport) ForceQueue();
		__declspec(dllexport) virtual ~ForceQueue();

		__declspec(dllexport) void AddForceToQueue(const XMFLOAT3 &f);
		__declspec(dllexport) void AddTorqueToQueue(const XMFLOAT3 &t);

	private:
		void ApplyForce(RigidBody *body, float dt);

		std::mutex mLock;
		std::list<XMFLOAT3> mForces;
		std::list<XMFLOAT3> mTorques;
	};
}

#endif
