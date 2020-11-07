#ifndef PARTICLEFIRE_FX
#define PARTICLEFIRE_FX

#include "Particles-Utilities.fx"

//***********************************************
// STREAM-OUT TECH                              *
//***********************************************

#define FIRE_EMIT_AT_ONCE 20
#define FIRE_EMIT_TIME 0.002f
#define FIRE_LIFE_SPAN 0.9f

float3 flameVecField(float3 i, float age)
{
	float t = saturate(2.0f*age / FIRE_LIFE_SPAN);
	float x = sin(i.x * 10.0f + i.y + gGameTime*3.0f) + sin(i.z * 10.0f + i.y + gGameTime*2.0f);
	float y = 0.0f;
	float z = sin(i.z * 10.0f + i.y + gGameTime*3.0f) + sin(i.z * 10.0f + i.y + gGameTime*2.0f);

	return float3(x, y, z);
}

Particle StreamOutFireVS(Particle vin)
{
	vin.Age += gTimeStep;

	// Acceleration from wind
	float c = 0.3f;
	float3 windAcc = c*(gWind - vin.VelW);
	float3 flameAcc = float3(0.0f, 18.0f, 0.0f);

	vin.VelW += (windAcc + gGravity + flameAcc + flameVecField(vin.PosW, vin.Age))*gTimeStep;
	vin.PosW += vin.VelW*gTimeStep;
	return vin;
}

// The stream-out GS is just responsible for emitting 
// new particles and destroying old particles.  The logic
// programed here will generally vary from particle system
// to particle system, as the destroy/spawn rules will be 
// different.

[maxvertexcount(FIRE_EMIT_AT_ONCE + 1)]
void StreamOutFireGS(point Particle gin[1],
	inout PointStream<Particle> ptStream)
{
	if (gin[0].Type == PT_EMITTER)
	{
		// time to emit a new particle?
		if ((gin[0].Age > FIRE_EMIT_TIME) && gEmitting)
		{
			for (int i = 0; i < FIRE_EMIT_AT_ONCE; ++i)
			{
				// Spread rain drops out above the camera.
				float3 vRandom3 = RandVec3((float)i / FIRE_EMIT_AT_ONCE);
				float2 vRandom2 = RandVec2(((float)i + 1) / FIRE_EMIT_AT_ONCE);

				Particle p;
				p.PosW = gEmitPosW.xyz + float3(vRandom3.x*0.4f, 0.0f, vRandom3.y*0.4f);
				p.VelW = gEmitDirW - vRandom3*0.4f;
				p.SizeW = vRandom2*0.5f + float2(0.5f, 0.5f);
				p.Age = 0.0f;
				p.Type = PT_FLARE;

				ptStream.Append(p);
			}

			// reset the time to emit
			gin[0].Age = 0.0f;
		}

		// always keep emitters
		ptStream.Append(gin[0]);
	}
	else
	{
		// Specify conditions to keep particle; this may vary from system to system.
		if (gin[0].Age <= FIRE_LIFE_SPAN)
			ptStream.Append(gin[0]);
	}
}

//***********************************************
// DRAW TECH                                    *
//***********************************************

float OpacityF(float x)
{
	float t = saturate(x / FIRE_LIFE_SPAN);
	return 2.0f * pow(t, 6.0f) - 3.0f * pow(t, 4.0f) + 1.0f;
}

VertexOut_PosDirTypeSizeOpacity DrawFireVS(Particle vIn)
{
	VertexOut_PosDirTypeSizeOpacity vOut;
	vOut.PosW = vIn.PosW;
	vOut.DirW = normalize(vIn.VelW);
	vOut.Type = vIn.Type;
	vOut.SizeW = vIn.SizeW;
	vOut.Opacity = OpacityF(vIn.Age);

	return vOut;
}

// The draw GS expands points into camera facing quads.
[maxvertexcount(4)]
void DrawFireGS(point VertexOut_PosDirTypeSizeOpacity gin[1], inout TriangleStream<GeoOut_PosTexOpacity> triStream)
{	
	// do not draw emitter particles.
	if( gin[0].Type != PT_EMITTER )
	{
		//
		// Compute world matrix so that billboard faces the camera.
		//
		float3 look = normalize(gEyePosW.xyz - gin[0].PosW);
		float3 right = normalize(cross(float3(0, 1, 0), look));
		float3 up = cross(look, right);

		//
		// Compute triangle strip vertices (quad) in world space.
		//
		float halfWidth = 0.5f*gin[0].SizeW.x;
		float halfHeight = 0.5f*gin[0].SizeW.y;

		float4 v[4];
		v[0] = float4(gin[0].PosW + halfWidth*right - halfHeight*up, 1.0f);
		v[1] = float4(gin[0].PosW + halfWidth*right + halfHeight*up, 1.0f);
		v[2] = float4(gin[0].PosW - halfWidth*right - halfHeight*up, 1.0f);
		v[3] = float4(gin[0].PosW - halfWidth*right + halfHeight*up, 1.0f);

		//
		// Transform quad vertices to world space and output 
		// them as a triangle strip.
		//
		GeoOut_PosTexOpacity gout;
		[unroll]
		for (int i = 0; i < 4; ++i)
		{
			gout.PosH = mul(v[i], gViewProj);
			gout.Tex = gQuadTexC[i];
			gout.Opacity = gin[0].Opacity;
			triStream.Append(gout);
		}
	}
}

float4 DrawFirePS(GeoOut_PosTexOpacity pin) : SV_TARGET
{
	return gTexArray.Sample(samLinear, float3(pin.Tex, 1)) * pin.Opacity;
}

#endif