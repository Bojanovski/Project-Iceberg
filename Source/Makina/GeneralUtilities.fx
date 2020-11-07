#ifndef GENERALUTILITIES_FX
#define GENERALUTILITIES_FX

#include "LightsUtilities.fx"

// Structs

struct VertexInPT
{
	float3 PosL     : POSITION;
	float2 TexCoord : TEXCOORD;
};

struct VertexOutPT
{
	float4 PosH     : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

struct VertexOutP
{
	float4 PosH     : SV_POSITION;
};

struct VertexIn
{
	float3 PosL     : POSITION;
	float3 NormalL  : NORMAL;
	float3 TangentL : TANGENT;
	float2 TexCoord : TEXCOORD;
};

struct SkinnedVertexIn
{
	float3 PosL			: POSITION;
	float3 NormalL		: NORMAL;
	float3 TangentL		: TANGENT;
	float2 TexCoord		: TEXCOORD;
	float3 Weights		: WEIGHTS;
	uint4 BoneIndices	: BONEINDICES;
};

struct VertexOut
{
	float4 PosH     : SV_POSITION;
	float3 PosW     : POSTION;
	float3 NormalW  : NORMAL;
	float3 TangentW : TANGENT;
	float2 TexCoord : TEXCOORD;
};

struct VertexOutGouraud
{
	float4 PosH			 : SV_POSITION;
	float2 TexCoord		 : TEXCOORD;
	float4 Color		 : COLOR;
};

struct VertexOutNTB
{
	float4 PosH           : SV_POSITION;
	float4 NormalPosH     : POSTION0;
	float4 TangentPosH    : POSTION1;
	float4 BitangentPosH  : POSTION2;
};

struct VertexOutShadowAndSSAO
{
	float4 PosH       : SV_POSITION;
	float3 PosW       : POSITION;
	float3 NormalW    : NORMAL;
	float3 TangentW   : TANGENT;
	float2 TexCoord   : TEXCOORD0;
	float4 ShadowPosH : TEXCOORD1;
	float4 ScreenPosH : TEXCOORD2;
};

struct VertexOutTess
{
	float3 PosW			: POSTION;
	float3 NormalW		: NORMAL;
	float3 TangentW		: TANGENT;
	float2 TexCoord		: TEXCOORD;
	float TessFactor	: TESS;
};

struct VertexOutNormalAndDepth
{
	float4 PosH			: SV_POSITION;
	float4 PosV			: POSITION;
	float3 NormalV		: NORMAL;
	float3 TangentV		: TANGENT;
	float2 TexCoord		: TEXCOORD;
};

struct GeoOut
{
	float4 PosH     : SV_POSITION;
	float4 Color    : COLOR;
};

// Buffers

cbuffer cbPerFrame
{
	// Lights
	DirectionalLight gDirLight;
	PointLight gPointLight;
	SpotLight gSpotLight;

	// Camera
	float3 gEyePosW;
	float4x4 gViewProj;
	float gZNear;
	float gZFar;

	// Tesselation
	float gHeightScale;
	float gMaxTessDistance;
	float gMinTessDistance;
	float gMinTessFactor;
	float gMaxTessFactor;
};

cbuffer cbPerObjectBasic
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	Material gMaterial;
	float4x4 gTexTransform;
};

cbuffer cbPerObjectAdvanced
{
	float4x4 gWorldViewProjTex;
	float4x4 gWorldView;
	float4x4 gWorldInvTransposeView;
	float4x4 gShadowTransform;
	float gShadowMapSize; 
	float gReflection;
};

cbuffer cbSkinned
{
	// Max support of 96 bones per model.
	float4x4 gBoneTransforms[96];
};

// Textures and Sampler States

Texture2D gDiffuseMap;

Texture2D gNormalMap;

TextureCube gCubeMap;

Texture2D gShadowMap;

Texture2D gSsaoMap;

SamplerState fastSampler
{
	Filter = MIN_MAG_MIP_POINT;

	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerState linearSampler
{
	Filter = MIN_MAG_MIP_LINEAR;

	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerComparisonState samShadow
{
	Filter   = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;
	AddressW = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    ComparisonFunc = LESS;
};

// Raster states

RasterizerState BackfaceCull
{
	Cullmode = back;
};

#endif