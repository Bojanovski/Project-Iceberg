#include "PhysicsUtil.h"
#include "Geometry.h"

using namespace Makina;
using namespace Physics_Makina;
using namespace std;

inline void Subexpressions(float w0, float w1, float w2, float &f1, float &f2, float &f3, float &g0, float &g1, float &g2)
{
	float temp0 = w0 + w1;
	f1 = temp0 + w2;
	float temp1 = w0*w0;
	float temp2 = temp1 + w1*temp0;
	f2 = temp2 + w2*f1;
	f3 = w0*temp1 + w1*temp2 + w2*f2;
	g0 = f2 + w0*(f1 + w0);
	g1 = f2 + w1*(f1 + w1);
	g2 = f2 + w2*(f1 + w2);
}

void Physics_Makina::ComputeRigidBodyProperties(const BasicMeshData &mesh, bool bodyCoords, float &mass, XMFLOAT3 &cm, XMFLOAT3X3 &inertia)
{
	// order 1, x, y, z, x^2, y^2, z^2, xy, yz, zx
	float integral[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	UINT faceCount = mesh.Indices.size() / 3;

	for (UINT t = 0; t < faceCount; ++t)
	{
		// get vertices of triange t 
		UINT i0 = mesh.Indices[3*t];
		UINT i1 = mesh.Indices[3*t + 1];
		UINT i2 = mesh.Indices[3*t + 2];
		XMVECTOR v0 = XMLoadFloat3(&mesh.Vertices[i0].Position);
		XMVECTOR v1 = XMLoadFloat3(&mesh.Vertices[i1].Position);
		XMVECTOR v2 = XMLoadFloat3(&mesh.Vertices[i2].Position);

		// get edges and cross product of edges
		XMVECTOR e1 = v1 - v0;
		XMVECTOR e2 = v2 - v0;
		XMVECTOR dVec = XMVector3Cross(e1, e2); // e2xe1 for clocwise ordered triangle

		// compute integral terms
		XMFLOAT3 d;
		XMStoreFloat3(&d, dVec);
		float f1x, f2x, f3x, g0x, g1x, g2x;
		Subexpressions(mesh.Vertices[i0].Position.x, mesh.Vertices[i1].Position.x, mesh.Vertices[i2].Position.x, f1x, f2x, f3x, g0x, g1x, g2x);
		float f1y, f2y, f3y, g0y, g1y, g2y;
		Subexpressions(mesh.Vertices[i0].Position.y, mesh.Vertices[i1].Position.y, mesh.Vertices[i2].Position.y, f1y, f2y, f3y, g0y, g1y, g2y);
		float f1z, f2z, f3z, g0z, g1z, g2z;
		Subexpressions(mesh.Vertices[i0].Position.z, mesh.Vertices[i1].Position.z, mesh.Vertices[i2].Position.z, f1z, f2z, f3z, g0z, g1z, g2z);

		// Update integrals.
		integral[0] += d.x*f1x;
		integral[1] += d.x*f2x;
		integral[2] += d.y*f2y;
		integral[3] += d.z*f2z;
		integral[4] += d.x*f3x;
		integral[5] += d.y*f3y;
		integral[6] += d.z*f3z;
		integral[7] += d.x*(mesh.Vertices[i0].Position.y*g0x + mesh.Vertices[i1].Position.y*g1x + mesh.Vertices[i2].Position.y*g2x);
		integral[8] += d.y*(mesh.Vertices[i0].Position.z*g0y + mesh.Vertices[i1].Position.z*g1y + mesh.Vertices[i2].Position.z*g2y);
		integral[9] += d.z*(mesh.Vertices[i0].Position.x*g0z + mesh.Vertices[i1].Position.x*g1z + mesh.Vertices[i2].Position.x*g2z);
	}

	integral[0] *= oneDiv6;
	integral[1] *= oneDiv24;
	integral[2] *= oneDiv24;
	integral[3] *= oneDiv24;
	integral[4] *= oneDiv60;
	integral[5] *= oneDiv60;
	integral[6] *= oneDiv60;
	integral[7] *= oneDiv120;
	integral[8] *= oneDiv120;
	integral[9] *= oneDiv120;

	// mass
	mass = integral[0];

	// center of mass
	cm.x = integral[1] / mass;
	cm.y = integral[2] / mass;
	cm.z = integral[3] / mass;

	// inertia relative to world origin
	inertia._11 = integral[5] + integral[6];
	inertia._12 = -integral[7];
	inertia._13 = -integral[9];
	inertia._21 = inertia._12;
	inertia._22 = integral[4] + integral[6];
	inertia._23 = -integral[8];
	inertia._31 = inertia._13;
	inertia._32 = inertia._23;
	inertia._33 = integral[4] + integral[5];


	// inertia relative to center of mass
	if (bodyCoords)
	{
		inertia._11 -= mass*(cm.y*cm.y + cm.z*cm.z);
		inertia._12 += mass*cm.x*cm.y;
		inertia._13 += mass*cm.z*cm.x;
		inertia._21 = inertia._12;
		inertia._22 -= mass*(cm.z*cm.z + cm.x*cm.x);
		inertia._23 += mass*cm.y*cm.z;
		inertia._31 = inertia._13;
		inertia._32 = inertia._23;
		inertia._33 -= mass*(cm.x*cm.x + cm.y*cm.y);
	}
}

void Physics_Makina::ComputeRigidBodyProperties(vector<BasicMeshData> &meshes, float &mass, XMFLOAT3 &cm, XMFLOAT3X3 &inertia)
{
	// order 1, x, y, z, x^2, y^2, z^2, xy, yz, zx
	float integral[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	for (auto mesh : meshes)
	{
		UINT faceCount = mesh.Indices.size() / 3;

		for (UINT t = 0; t < faceCount; ++t)
		{
			// get vertices of triange t 
			UINT i0 = mesh.Indices[3*t];
			UINT i1 = mesh.Indices[3*t + 1];
			UINT i2 = mesh.Indices[3*t + 2];
			XMVECTOR v0 = XMLoadFloat3(&mesh.Vertices[i0].Position);
			XMVECTOR v1 = XMLoadFloat3(&mesh.Vertices[i1].Position);
			XMVECTOR v2 = XMLoadFloat3(&mesh.Vertices[i2].Position);

			// get edges and cross product of edges
			XMVECTOR e1 = v1 - v0;
			XMVECTOR e2 = v2 - v0;
			XMVECTOR dVec = XMVector3Cross(e2, e1); // e2xe1 for clocwise ordered triangle

			// compute integral terms
			XMFLOAT3 d;
			XMStoreFloat3(&d, dVec);
			float f1x, f2x, f3x, g0x, g1x, g2x;
			Subexpressions(mesh.Vertices[i0].Position.x, mesh.Vertices[i1].Position.x, mesh.Vertices[i2].Position.x, f1x, f2x, f3x, g0x, g1x, g2x);
			float f1y, f2y, f3y, g0y, g1y, g2y;
			Subexpressions(mesh.Vertices[i0].Position.y, mesh.Vertices[i1].Position.y, mesh.Vertices[i2].Position.y, f1y, f2y, f3y, g0y, g1y, g2y);
			float f1z, f2z, f3z, g0z, g1z, g2z;
			Subexpressions(mesh.Vertices[i0].Position.z, mesh.Vertices[i1].Position.z, mesh.Vertices[i2].Position.z, f1z, f2z, f3z, g0z, g1z, g2z);

			// Update integrals.
			integral[0] += d.x*f1x;
			integral[1] += d.x*f2x;
			integral[2] += d.y*f2y;
			integral[3] += d.z*f2z;
			integral[4] += d.x*f3x;
			integral[5] += d.y*f3y;
			integral[6] += d.z*f3z;
			integral[7] += d.x*(mesh.Vertices[i0].Position.y*g0x + mesh.Vertices[i1].Position.y*g1x + mesh.Vertices[i2].Position.y*g2x);
			integral[8] += d.y*(mesh.Vertices[i0].Position.z*g0y + mesh.Vertices[i1].Position.z*g1y + mesh.Vertices[i2].Position.z*g2y);
			integral[9] += d.z*(mesh.Vertices[i0].Position.x*g0z + mesh.Vertices[i1].Position.x*g1z + mesh.Vertices[i2].Position.x*g2z);
		}
	}

	integral[0] *= oneDiv6;
	integral[1] *= oneDiv24;
	integral[2] *= oneDiv24;
	integral[3] *= oneDiv24;
	integral[4] *= oneDiv60;
	integral[5] *= oneDiv60;
	integral[6] *= oneDiv60;
	integral[7] *= oneDiv120;
	integral[8] *= oneDiv120;
	integral[9] *= oneDiv120;

	// mass
	mass = integral[0];

	// center of mass
	cm.x = integral[1] / mass;
	cm.y = integral[2] / mass;
	cm.z = integral[3] / mass;

	// inertia relative to world origin
	inertia._11 = integral[5] + integral[6];
	inertia._12 = -integral[7];
	inertia._13 = -integral[9];
	inertia._21 = inertia._12;
	inertia._22 = integral[4] + integral[6];
	inertia._23 = -integral[8];
	inertia._31 = inertia._13;
	inertia._32 = inertia._23;
	inertia._33 = integral[4] + integral[5];


	// inertia relative to center of mass
	inertia._11 -= mass*(cm.y*cm.y + cm.z*cm.z);
	inertia._12 += mass*cm.x*cm.y;
	inertia._13 += mass*cm.z*cm.x;
	inertia._21 = inertia._12;
	inertia._22 -= mass*(cm.z*cm.z + cm.x*cm.x);
	inertia._23 += mass*cm.y*cm.z;
	inertia._31 = inertia._13;
	inertia._32 = inertia._23;
	inertia._33 -= mass*(cm.x*cm.x + cm.y*cm.y);
}

void Physics_Makina::ComputeRigidBodyProperties(float capsuleHeight, float capsuleRadius, float &mass, XMFLOAT3 &cm, XMFLOAT3X3 &inertia)
{
	// We first calculate only a part of a capsule that is a cylinder and then the hemisphere caps.
	float cM; // cylinder mass
	float hsM; // mass of half sphere
	float rSq = capsuleRadius*capsuleRadius;
	cM = XM_PI*capsuleHeight*rSq;
	hsM = XM_2PI*oneDiv3*rSq*capsuleRadius;

	// Ixy = Ixz = Iyz = 0, so we only need to calculate Ixx, Iyy and Izz.
	// from cylinder
	inertia._22 = rSq*cM*0.5f;
	inertia._11 = inertia._33 = inertia._22*0.5f + cM*capsuleHeight*capsuleHeight*oneDiv12;
	// from hemispheres
	float temp0 = hsM*2.0f*rSq / 5.0f;
	inertia._22 += temp0 * 2.0f; // *2.0f because we have two caps on both side of the cylinder. Same goes for Ixx and Izz.
	float temp1 = capsuleHeight*0.5f;
	float temp2 = temp0 + hsM*(temp1*temp1 + 3.0f*oneDiv8*capsuleHeight*capsuleRadius);
	inertia._11 += temp2 * 2.0f;
	inertia._33 += temp2 * 2.0f;
	inertia._12 = inertia._13 = inertia._21 = inertia._23 = inertia._31 = inertia._32 = 0.0f;

	mass = cM + hsM*2.0f;
	cm = {0.0f, 0.0f, 0.0f};
}