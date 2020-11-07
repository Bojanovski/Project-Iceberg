#ifndef PHYSICS_UTIL_H
#define PHYSICS_UTIL_H

#include "PhysicsHeaders.h"

namespace Physics_Makina
{
	const float oneDiv3 = (float)(1.0 / 3.0);
	const float oneDiv6 = (float)(1.0 / 6.0);
	const float oneDiv8 = (float)(1.0 / 8.0);
	const float oneDiv12 = (float)(1.0 / 12.0);
	const float oneDiv24 = (float)(1.0 / 24.0);
	const float oneDiv60 = (float)(1.0 / 60.0);
	const float oneDiv120 = (float)(1.0 / 120.0);

	__declspec(dllexport) void ComputeRigidBodyProperties(const Makina::BasicMeshData &mesh, bool bodyCoords, float &mass, XMFLOAT3 &cm, XMFLOAT3X3 &inertia);
	__declspec(dllexport) void ComputeRigidBodyProperties(std::vector<Makina::BasicMeshData> &meshes, float &mass, XMFLOAT3 &cm, XMFLOAT3X3 &inertia);
	__declspec(dllexport) void ComputeRigidBodyProperties(float capsuleHeight, float capsuleRadius, float &mass, XMFLOAT3 &cm, XMFLOAT3X3 &inertia);
}

#endif
