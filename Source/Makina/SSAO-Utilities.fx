
#ifndef SSAO_UTILITIES_FX
#define SSAO_UTILITIES_FX

cbuffer cbPerFrameSSAO
{
	float4x4 gViewToTexSpace; // Proj*Texture
	float4   gOffsetVectors[14];
	float4   gFrustumCorners[4];

	// Coordinates given in view space.
	float    gOcclusionRadius    = 0.1f;
	float    gOcclusionFadeStart = 0.2f;
	float    gOcclusionFadeEnd   = 2.0f;
	float    gSurfaceEpsilon     = 0.05f;
};

cbuffer cbPerFrameCamPropSSAO
{
	float gZNear;
	float gZFar;
}

cbuffer cbPerFrameBlurSSAO
{
	float gTexelWidth;
	float gTexelHeight;
};

cbuffer cbSettingsSSAO
{
	float gWeights[11] = 
	{
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f
	};
};

cbuffer cbFixedSSAO
{
	static const int gBlurRadius = 5;
};
 
// Nonnumeric values cannot be added to a cbuffer.
Texture2D gNormalDepthMap;
Texture2D gInputImage;
Texture2D gRandomVecMap;

SamplerState samNormalDepthBlur
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
};

SamplerState samNormalDepth16Bit
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	// Set a very far depth value if sampling outside of the NormalDepth map
	// so we do not get false occlusions.
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 1e5f);
};

SamplerState samNormalDepth8Bit
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	// Set a very far depth value if sampling outside of the NormalDepth map
	// so we do not get false occlusions.
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(0.0f, 0.0f, 1e5f, 1e5f);
};

SamplerState samInputImage
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	AddressU  = CLAMP;
    AddressV  = CLAMP;
};

SamplerState samRandomVec
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU  = WRAP;
    AddressV  = WRAP;
};

void VectorsFrom8Bit(float4 input, out float3 normal, out float depth)
{
	// scale from [0, 1] to [-1, 1]
	normal.x = input.x*2.0f -1.0f;
	normal.y = input.y*2.0f -1.0f;

	// get z so that normal vector is unit length
	normal.z = -sqrt(abs(1 - normal.x*normal.x - normal.y*normal.y));	// abs() is important because
																		// we can have negative value under sqrt() due to small approximation errors

	normalize(normal);

	// now get depth
	depth = input.z + input.w/255.0f;

	// scale from [0, 1] to [zNear, zFar]
	depth = depth*(gZFar - gZNear) + gZNear;
}

void VectorsFrom8Bit(float4 input, out float4 output)
{
	VectorsFrom8Bit(input, output.xyz, output.w);
}

void DepthFrom8Bit(float4 input, out float depth)
{
	depth = input.z + input.w/255.0f;

	// scale from [0, 1] to [zNear, zFar]
	depth = depth*(gZFar - gZNear) + gZNear;
}

#endif
