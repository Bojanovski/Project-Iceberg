#ifndef UTILITIES_FX
#define UTILITIES_FX

Texture2D gTexture;

cbuffer cbPerObject
{
	float4 gOffsetAndSize;
	float4 gColor;
}

SamplerState pointSampler
{
	filter = MIN_MAG_MIP_POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

SamplerState linearSampler
{
	filter = MIN_MAG_MIP_LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

#endif