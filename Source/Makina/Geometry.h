#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <Windows.h>
#include <xnamath.h>
#include <vector>
#include "D3DUtilities.h"

namespace Makina
{
	struct MeshData
	{
		virtual size_t SizeOfVertexElement() = 0;
	};

	struct BasicMeshData : public MeshData
	{
		std::vector<VertexFull> Vertices;
		std::vector<UINT> Indices;
		size_t SizeOfVertexElement() { return sizeof(VertexFull); }
	};

	struct SkinnedMeshData : public MeshData
	{
		std::vector<SkinnedVertexFull> Vertices;
		std::vector<UINT> Indices;
		size_t SizeOfVertexElement() { return sizeof(SkinnedVertexFull); }
	};

	class GeometryGenerator
	{
	public:
		__declspec(dllexport) static void CreateBox(float width, float height, float depth, BasicMeshData& meshData);
		__declspec(dllexport) static void CreateGrid(float width, float depth, UINT m, UINT n, BasicMeshData& meshData);
		__declspec(dllexport) static void CreateSphere(float radius, UINT sliceCount, UINT stackCount, BasicMeshData& meshData);
		__declspec(dllexport) static void Subdivide(BasicMeshData& meshData);
		__declspec(dllexport) static void CreateGeosphere(float radius, UINT numSubdivisions, BasicMeshData& meshData);
		__declspec(dllexport) static void CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, BasicMeshData& meshData);
		__declspec(dllexport) static void CreateCylinderSmooth(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, BasicMeshData& meshData);
		__declspec(dllexport) static void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, BasicMeshData& meshData);
		__declspec(dllexport) static void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, BasicMeshData& meshData);
		__declspec(dllexport) static void CreateCapsule(float radius, float height, UINT sliceCount, UINT stackCount, UINT halfSphereStackCount, BasicMeshData& meshData);
		__declspec(dllexport) static void BuildCapsuleTopCap(float radius, float height, UINT sliceCount, UINT stackCount, BasicMeshData& meshData);
		__declspec(dllexport) static void BuildCapsuleBottomCap(float radius, float height, UINT sliceCount, UINT stackCount, BasicMeshData& meshData);
		__declspec(dllexport) static void GeometryGenerator::CreateFullscreenQuad(BasicMeshData& meshData);
	};
}

#endif