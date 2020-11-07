#ifndef PARTICLESUTILITIES_FX
#define PARTICLESUTILITIES_FX

#define PT_EMITTER 0
#define PT_FLARE 1

struct Particle
{
	float3 PosW : POSITION;
	float3 VelW : VELOCITY;
	float2 SizeW       : SIZE;
	float Age : AGE;
	uint Type          : TYPE;
};

struct VertexOut_PosDirType
{
	float3 PosW  : POSITION0;
	float3 DirW   : POSITION1;
	uint   Type  : TYPE;
};

struct VertexOut_PosDirTypeSizeOpacity
{
	float3 PosW		: POSITION0;
	float3 DirW		: POSITION1;
	uint   Type		: TYPE;
	float2 SizeW	: SIZE;
	float Opacity	: OPACITY;
};

struct GeoOut_PosColor
{
	float4 PosH  : SV_Position;
	float3 Color : COLOR;
};

struct GeoOut_PosTexOpacity
{
	float4 PosH		: SV_Position;
	float2 Tex		: TEXCOORD;
	float Opacity	: OPACITY;
};

cbuffer cbPerFrame
{
	// is system asleep?
	bool gEmitting;


	float3 gEyePosW;

	// for when the emit position/direction is varying
	float3 gEmitPosW;
	float3 gEmitDirW;
	
	float gGameTime;
	float gTimeStep;
	float4x4 gViewProj;

	// Acceleration used to accerlate the particles.
	float3 gGravity = {0.0f, -9.8f, 0.0f};

	// Wind speed
	float3 gWind = {0.0f, 0.0f, 0.0f};

	// Speed of a Falling Raindrop
	float3 gRainSpeed = float3(0.0f, -11.0f, 0.0f);
};

cbuffer cbFixed
{
	// Texture coordinates used to stretch texture over quad 
	// when we expand point particle into a quad.
	float2 gQuadTexC[4] =
	{
		float2(0.0f, 0.0f),
		float2(0.0f, 1.0f),
		float2(1.0f, 0.0f),
		float2(1.0f, 1.0f)
	};
};
 
// Array of textures for texturing the particles.
Texture2DArray gTexArray;

// Random texture used to generate random numbers in shaders.
Texture1D gRandomTex;
 
SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};

DepthStencilState NoDepthWrites
{
    DepthEnable = TRUE;
    DepthWriteMask = ZERO;
};

BlendState AdditiveBlending
{
	AlphaToCoverageEnable = FALSE;
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
	DestBlend = ONE;
	BlendOp = ADD;
	SrcBlendAlpha = ZERO;
	DestBlendAlpha = ZERO;
	BlendOpAlpha = ADD;
	RenderTargetWriteMask[0] = 0x0F;
};

//***********************************************
// HELPER FUNCTIONS                             *
//***********************************************
float3 RandUnitVec3(float offset)
{
	// Use game time plus offset to sample random texture.
	float u = (gGameTime + offset);
	
	// coordinates in [-1,1]
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;
	
	// project onto unit sphere
	return normalize(v);
}

float3 RandVec3(float offset)
{
	// Use game time plus offset to sample random texture.
	float u = (gGameTime + offset);
	
	// coordinates in [-1,1]
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;
	
	return v;
}

float2 RandVec2(float offset)
{
	// Use game time plus offset to sample random texture.
	float u = (gGameTime + offset);

	// coordinates in [-1,1]
	float2 v = gRandomTex.SampleLevel(samLinear, u, 0).xy;

	return v;
}

float Rand(float offset)
{
	// Use game time plus offset to sample random texture.
	float u = (gGameTime + offset);

	// coordinates in [-1,1]
	float v = gRandomTex.SampleLevel(samLinear, u, 0).x;

	return v;
}

#endif