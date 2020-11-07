
#ifndef BSPLINESURFACES_UTILITIES_FX
#define BSPLINESURFACES_UTILITIES_FX

#define MAX_N 10				// maximum number of control points in either direction (U or V)
#define D 2						// degree of the curve
#define EPSILON 0.00002f		// used for normal and tangent calculation

cbuffer cbPerObject
{
	// Camera
	float4x4 gViewProj;
	float3 gEyePosW;

	// Tessellation
	float gMaxTessDistance;
	float gMinTessDistance;
	float gMinTessFactor;
	float gMaxTessFactor;

	// B-Spline
	int gNU;
	int gNV;

	float4 gCP[MAX_N * MAX_N];	// control points
	float3 gCenter;				// arithmetic mean of control points
};

cbuffer cbPerObjectAdditional
{
	// Lights
	DirectionalLight gDirLight;

	// Cloth object
	Material gMaterial;

	// Shadows
	float4x4 gShadowTransform;
	float gShadowMapSize;
}

Texture2D gDiffuseMap;
//Texture2D gNormalMap;

Texture2D gShadowMap;

SamplerState fastSampler
{
	Filter = MIN_MAG_MIP_POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
};

SamplerState linearSampler
{
	Filter = MIN_MAG_MIP_LINEAR;

	AddressU = CLAMP;
	AddressV = CLAMP;
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = CLAMP;
	AddressV = CLAMP;
};

SamplerComparisonState samShadow
{
	Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;
	AddressW = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	ComparisonFunc = LESS;
};

float CalcTessFactor(float3 p)
{
	float d = distance(p, gEyePosW);
	float s = saturate((d - gMinTessDistance) / (gMaxTessDistance - gMinTessDistance));
	return lerp(gMinTessFactor, gMaxTessFactor, pow(s, 1.5f));
}

struct V_TexCoord
{
	float2 TexCoord		: TEXCOORD;
};

struct V_PosW_NormalW_TexCoord
{
	float3 PosW			: POSTION;
	float3 NormalW		: NORMAL;
	float2 TexCoord		: TEXCOORD;
};

struct V_PosW
{
	float3 PosW			: POSTION;
};

struct V_PosW_TexCoord
{
	float3 PosW			: POSTION;
	float2 TexCoord		: TEXCOORD;
};

struct V_PosH
{
	float4 PosH			: SV_POSITION;
};

struct V_PosH_TexCoord
{
	float4 PosH			: SV_POSITION;
	float2 TexCoord		: TEXCOORD;
};

struct V_PosH_NormalW_TexCoord_ShadowPosH
{
	float4 PosH			: SV_POSITION;
	float3 NormalW		: NORMAL;
	float2 TexCoord		: TEXCOORD;
	float4 ShadowPosH	: TEXCOORD1;
};

// Calcuate a knot form an open uniform knot vector
float GetKnot(int i, int n)
{
	return saturate((float)(i - D) / (float)(n - D));
}

// For an open uniform B-Spline
int GetKey(float u, int n)
{
	// This implementation of B-Spline is not defined at u = 1.0f
	// therefore whole surface/curve is slightly squeezed.
	return D + (int)floor((n - D) * u * 0.9999f);
}

void ComputePosNormalTangent(in float2 texCoord, out float3 pos, out float3 normal, out float3 tan)
{
	float u = texCoord.x;
	float v = texCoord.y;
	float u_pdu = texCoord.x + EPSILON;
	float v_pdv = texCoord.y + EPSILON;
	int iU = GetKey(u, gNU);
	int iV = GetKey(v, gNV);
	// make sure we stay in the same interval between knots
	//if (GetKnot(iU, gNU) > u_pdu) u_pdu += EPSILON * 2.0f;
	//if (GetKnot(iV, gNV) > v_pdv) v_pdv += EPSILON * 2.0f;

	// set basis
	float basisU[D + 1][MAX_N + D];
	float basisV[D + 1][MAX_N + D];
	float basisU_pdu[D + 1][MAX_N + D];
	float basisV_pdv[D + 1][MAX_N + D];
	basisU[0][iU] = basisV[0][iV] = basisU_pdu[0][iU] = basisV_pdv[0][iV] = 1.0f;

	// evaluate left-diagonal and right-vertical edges
	[unroll]
	for (int j = 1; j <= D; ++j)
	{
		float gKI;
		float gKI1;
		float gKIJ;
		float gKIJ1;

		// U
		gKI = GetKnot(iU, gNU);
		gKI1 = GetKnot(iU + 1, gNU);
		gKIJ = GetKnot(iU + j, gNU);
		gKIJ1 = GetKnot(iU - j + 1, gNU);
		float c0U = (u - gKI) / (gKIJ - gKI);
		float c1U = (gKI1 - u) / (gKI1 - gKIJ1);
		basisU[j][iU] = c0U * basisU[j - 1][iU];
		basisU[j][iU - j] = c1U * basisU[j - 1][iU - j + 1];
		float c0U_pdu = (u_pdu - gKI) / (gKIJ - gKI);
		float c1U_pdu = (gKI1 - u_pdu) / (gKI1 - gKIJ1);
		basisU_pdu[j][iU] = c0U_pdu * basisU_pdu[j - 1][iU];
		basisU_pdu[j][iU - j] = c1U_pdu * basisU_pdu[j - 1][iU - j + 1];

		// V
		gKI = GetKnot(iV, gNV);
		gKI1 = GetKnot(iV + 1, gNV);
		gKIJ = GetKnot(iV + j, gNV);
		gKIJ1 = GetKnot(iV - j + 1, gNV);
		float c0V = (v - gKI) / (gKIJ - gKI);
		float c1V = (gKI1 - v) / (gKI1 - gKIJ1);
		basisV[j][iV] = c0V * basisV[j - 1][iV];
		basisV[j][iV - j] = c1V * basisV[j - 1][iV - j + 1];
		float c0V_pdv = (v_pdv - gKI) / (gKIJ - gKI);
		float c1V_pdv = (gKI1 - v_pdv) / (gKI1 - gKIJ1);
		basisV_pdv[j][iV] = c0V_pdv * basisV_pdv[j - 1][iV];
		basisV_pdv[j][iV - j] = c1V_pdv * basisV_pdv[j - 1][iV - j + 1];
	}

	// evaluate interior
	[unroll]
	for (j = 2; j <= D; ++j)
	{
		// U
		[unroll(j - 1)]
		for (int k = iU - j + 1; k < iU; ++k)
		{
			float gKK = GetKnot(k, gNU);
			float gKK1 = GetKnot(k + 1, gNU);
			float gKKJ = GetKnot(k + j, gNU);
			float gKKJ1 = GetKnot(k + j + 1, gNU);
			float c0U = (u - gKK) / (gKKJ - gKK);
			float c1U = (gKKJ1 - u) / (gKKJ1 - gKK1);
			basisU[j][k] = c0U * basisU[j - 1][k] + c1U * basisU[j - 1][k + 1];
			float c0U_pdu = (u_pdu - gKK) / (gKKJ - gKK);
			float c1U_pdu = (gKKJ1 - u_pdu) / (gKKJ1 - gKK1);
			basisU_pdu[j][k] = c0U_pdu * basisU_pdu[j - 1][k] + c1U_pdu * basisU_pdu[j - 1][k + 1];
		}

		// V
		[unroll(j - 1)]
		for (k = iV - j + 1; k < iV; ++k)
		{
			float gKK = GetKnot(k, gNV);
			float gKK1 = GetKnot(k + 1, gNV);
			float gKKJ = GetKnot(k + j, gNV);
			float gKKJ1 = GetKnot(k + j + 1, gNV);
			float c0V = (v - gKK) / (gKKJ - gKK);
			float c1V = (gKKJ1 - v) / (gKKJ1 - gKK1);
			basisV[j][k] = c0V * basisV[j - 1][k] + c1V * basisV[j - 1][k + 1];
			float c0V_pdv = (v_pdv - gKK) / (gKKJ - gKK);
			float c1V_pdv = (gKKJ1 - v_pdv) / (gKKJ1 - gKK1);
			basisV_pdv[j][k] = c0V_pdv * basisV_pdv[j - 1][k] + c1V_pdv * basisV_pdv[j - 1][k + 1];
		}
	}

	float3 pos_pdu, pos_pdv;
	pos.x = pos_pdu.x = pos_pdv.x = 0.0f;
	pos.y = pos_pdu.y = pos_pdv.y = 0.0f;
	pos.z = pos_pdu.z = pos_pdv.z = 0.0f;

	[unroll(D + 1)]
	for (int jU = iU - D; jU <= iU; ++jU)
	{
		[unroll(D + 1)]
		for (int jV = iV - D; jV <= iV; ++jV)
		{
			pos += basisU[D][jU] * basisV[D][jV] * (gCP[jU + jV * gNU].xyz - gCenter);
			pos_pdu += basisU_pdu[D][jU] * basisV[D][jV] * (gCP[jU + jV * gNU].xyz - gCenter);
			pos_pdv += basisU[D][jU] * basisV_pdv[D][jV] * (gCP[jU + jV * gNU].xyz - gCenter);
		}
	}
	
	tan = normalize(pos_pdu - pos);
	float3 bTan = normalize(pos_pdv - pos);
	normal = normalize(cross(tan, bTan));
	pos += gCenter;
}

void ComputePos(in float2 texCoord, out float3 pos)
{
	// Initialize outputs.
	pos = float3(0.0f, 0.0f, 0.0f);

	float u = texCoord.x;
	float v = texCoord.y;
	int iU = GetKey(u, gNU);
	int iV = GetKey(v, gNV);
	// make sure we stay in the same interval between knots
	//if (GetKnot(iU, gNU) > u_pdu) u_pdu += EPSILON * 2.0f;
	//if (GetKnot(iV, gNV) > v_pdv) v_pdv += EPSILON * 2.0f;

	// set basis
	float basisU[D + 1][MAX_N + D];
	float basisV[D + 1][MAX_N + D];
	basisU[0][iU] = basisV[0][iV] = 1.0f;

	// evaluate left-diagonal and right-vertical edges
	[unroll]
	for (int j = 1; j <= D; ++j)
	{
		float gKI;
		float gKI1;
		float gKIJ;
		float gKIJ1;

		// U
		gKI = GetKnot(iU, gNU);
		gKI1 = GetKnot(iU + 1, gNU);
		gKIJ = GetKnot(iU + j, gNU);
		gKIJ1 = GetKnot(iU - j + 1, gNU);
		float c0U = (u - gKI) / (gKIJ - gKI);
		float c1U = (gKI1 - u) / (gKI1 - gKIJ1);
		basisU[j][iU] = c0U * basisU[j - 1][iU];
		basisU[j][iU - j] = c1U * basisU[j - 1][iU - j + 1];

		// V
		gKI = GetKnot(iV, gNV);
		gKI1 = GetKnot(iV + 1, gNV);
		gKIJ = GetKnot(iV + j, gNV);
		gKIJ1 = GetKnot(iV - j + 1, gNV);
		float c0V = (v - gKI) / (gKIJ - gKI);
		float c1V = (gKI1 - v) / (gKI1 - gKIJ1);
		basisV[j][iV] = c0V * basisV[j - 1][iV];
		basisV[j][iV - j] = c1V * basisV[j - 1][iV - j + 1];
	}

	// evaluate interior
	[unroll]
	for (j = 2; j <= D; ++j)
	{
		// U
		[unroll(j - 1)]
		for (int k = iU - j + 1; k < iU; ++k)
		{
			float gKK = GetKnot(k, gNU);
			float gKK1 = GetKnot(k + 1, gNU);
			float gKKJ = GetKnot(k + j, gNU);
			float gKKJ1 = GetKnot(k + j + 1, gNU);
			float c0U = (u - gKK) / (gKKJ - gKK);
			float c1U = (gKKJ1 - u) / (gKKJ1 - gKK1);
			basisU[j][k] = c0U * basisU[j - 1][k] + c1U * basisU[j - 1][k + 1];
		}

		// V
		[unroll(j - 1)]
		for (k = iV - j + 1; k < iV; ++k)
		{
			float gKK = GetKnot(k, gNV);
			float gKK1 = GetKnot(k + 1, gNV);
			float gKKJ = GetKnot(k + j, gNV);
			float gKKJ1 = GetKnot(k + j + 1, gNV);
			float c0V = (v - gKK) / (gKKJ - gKK);
			float c1V = (gKKJ1 - v) / (gKKJ1 - gKK1);
			basisV[j][k] = c0V * basisV[j - 1][k] + c1V * basisV[j - 1][k + 1];
		}
	}

	[unroll(D + 1)]
	for (int jU = iU - D; jU <= iU; ++jU)
	{
		[unroll(D + 1)]
		for (int jV = iV - D; jV <= iV; ++jV)
		{
			pos += basisU[D][jU] * basisV[D][jV] * (gCP[jU + jV * gNU].xyz);
		}
	}
}

#endif