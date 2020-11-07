#ifndef UTILITIES_H
#define UTILITIES_H

#include "DirectX11Headers.h"

namespace Makina
{
	// 2D
	struct VertexText2D
	{
		XMFLOAT4 PosRec;
		XMFLOAT4 TexRec;
	};

	static D3D11_INPUT_ELEMENT_DESC VertexText2DDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	struct VertexT2D
	{
		XMFLOAT2 Tex;
	};

	static D3D11_INPUT_ELEMENT_DESC VertexT2DDesc[] =
	{
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// 3D
	const D3D11_INPUT_ELEMENT_DESC VertexPDesc[] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	struct VertexPC
	{
		XMFLOAT3 Pos;
		XMFLOAT4 Color;
	};

	static D3D11_INPUT_ELEMENT_DESC VertexPCDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	struct VertexPT
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Tex;
	};

	static D3D11_INPUT_ELEMENT_DESC VertexPTDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	struct VertexPN
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
	};

	static D3D11_INPUT_ELEMENT_DESC VertexPNDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	struct VertexPNT
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
	};

	static D3D11_INPUT_ELEMENT_DESC VertexPNTDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	struct VertexFull
	{
		VertexFull(){}
		VertexFull(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT3& t, const XMFLOAT2& uv)
			: Position(p), Normal(n), TangentU(t), TexC(uv){}
		VertexFull(
			float px, float py, float pz, 
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v)
			: Position(px,py,pz), Normal(nx,ny,nz),
			TangentU(tx, ty, tz), TexC(u,v){}

		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT3 TangentU;
		XMFLOAT2 TexC;
	};

	static D3D11_INPUT_ELEMENT_DESC VertexFullDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	struct VertexWeights
	{
		UINT bI0, bI1, bI2, bI3;	// indices
		float bW0, bW1, bW2, bW3;	// weights
	};

	struct SkinnedVertexFull
	{
		SkinnedVertexFull(){}
		SkinnedVertexFull(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT3& t, const XMFLOAT2& uv, const XMFLOAT3& weights, const BYTE bIndices[])
			: Position(p), Normal(n), TangentU(t), TexC(uv), Weights(weights) 
		{
			BoneIndices[0] = bIndices[0];
			BoneIndices[1] = bIndices[1];
			BoneIndices[2] = bIndices[2];
			BoneIndices[3] = bIndices[3];
		}

		SkinnedVertexFull(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v,
			float w0, float w1, float w2,
			BYTE bi0, BYTE bi1, BYTE bi2, BYTE bi3)
			: Position(px, py, pz), Normal(nx, ny, nz),
			TangentU(tx, ty, tz), TexC(u, v), Weights(w0, w1, w2)
		{
			BoneIndices[0] = bi0;
			BoneIndices[1] = bi1;
			BoneIndices[2] = bi2;
			BoneIndices[3] = bi3;
		}

		void SetWeights(const VertexWeights &vW)
		{
			BoneIndices[0] = vW.bI0;
			BoneIndices[1] = vW.bI1;
			BoneIndices[2] = vW.bI2;
			BoneIndices[3] = vW.bI3;
			Weights.x = vW.bW0;
			Weights.y = vW.bW1;
			Weights.z = vW.bW2;
		}

		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT3 TangentU;
		XMFLOAT2 TexC;
		XMFLOAT3 Weights;
		BYTE BoneIndices[4];
	};

	static D3D11_INPUT_ELEMENT_DESC SkinnedVertexFullDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	struct VertexParticle
	{
		XMFLOAT3 InitialPos;
		XMFLOAT3 InitialVel;
		XMFLOAT2 Size;
		float Age;
		unsigned int Type;
	};

	const D3D11_INPUT_ELEMENT_DESC ParticleDesc[5] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"SIZE",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"AGE",      0, DXGI_FORMAT_R32_FLOAT,       0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TYPE",     0, DXGI_FORMAT_R32_UINT,        0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	namespace Colors
	{
		XMGLOBALCONST XMVECTORF32 White     = {1.0f, 1.0f, 1.0f, 1.0f};
		XMGLOBALCONST XMVECTORF32 Black     = {0.0f, 0.0f, 0.0f, 1.0f};
		XMGLOBALCONST XMVECTORF32 Red       = {1.0f, 0.0f, 0.0f, 1.0f};
		XMGLOBALCONST XMVECTORF32 Green     = {0.0f, 1.0f, 0.0f, 1.0f};
		XMGLOBALCONST XMVECTORF32 Blue      = {0.0f, 0.0f, 1.0f, 1.0f};
		XMGLOBALCONST XMVECTORF32 Yellow    = {1.0f, 1.0f, 0.0f, 1.0f};
		XMGLOBALCONST XMVECTORF32 Cyan      = {0.0f, 1.0f, 1.0f, 1.0f};
		XMGLOBALCONST XMVECTORF32 Magenta   = {1.0f, 0.0f, 1.0f, 1.0f};

		XMGLOBALCONST XMVECTORF32 Silver    = {0.75f, 0.75f, 0.75f, 1.0f};
		XMGLOBALCONST XMVECTORF32 LightSteelBlue = {0.69f, 0.77f, 0.87f, 1.0f};
		XMGLOBALCONST XMVECTORF32 CornflowerBlue = {0.39f, 0.58f, 0.92f, 1.0f};
	}

	struct DirectionalLight
	{
		DirectionalLight() { ZeroMemory(this, sizeof(this)); }

		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
		XMFLOAT4 Specular;
		XMFLOAT3 Direction;
		float Pad; // Pad the last float so we can set an array of lights if we wanted.
	};

	struct PointLight
	{
		PointLight() { ZeroMemory(this, sizeof(this)); }

		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
		XMFLOAT4 Specular;

		// Packed into 4D vector: (Position, Range)
		XMFLOAT3 Position;
		float Range;

		// Packed into 4D vector: (A0, A1, A2, Pad)
		XMFLOAT3 Att;
		float Pad; // Pad the last float so we can set an array of lights if we wanted.
	};

	struct SpotLight
	{
		SpotLight() { ZeroMemory(this, sizeof(this)); }

		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
		XMFLOAT4 Specular;

		// Packed into 4D vector: (Position, Range)
		XMFLOAT3 Position;
		float Range;

		// Packed into 4D vector: (Direction, Spot)
		XMFLOAT3 Direction;
		float Spot;

		// Packed into 4D vector: (Att, Pad)
		XMFLOAT3 Att;
		float Pad; // Pad the last float so we can set an array of lights if we wanted.
	};

	struct Material
	{
		Material() { ZeroMemory(this, sizeof(this)); }

		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
		XMFLOAT4 Specular; // w = SpecPower
		XMFLOAT4 Reflect;
	};
}

//// Definitions to help write faster.
//#define XMVECTOR Vector4
//#define XMFLOAT2 Float2
//#define XMFLOAT3 Float3
//#define XMFLOAT4 Float4
//#define XMMATRIX Matrix

#endif