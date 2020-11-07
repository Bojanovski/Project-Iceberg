#ifndef PARTICLESSPARKS_FX
#define PARTICLESSPARKS_FX

#include "Particles-Utilities.fx"

//***********************************************
// STREAM-OUT TECH                              *
//***********************************************

Particle StreamOutSparksVS(Particle vin)
{
	vin.Age += gTimeStep;
	
	// Acceleration from wind
	float c = 0.3f;
	float3 windAcc = c*(gWind - vin.VelW);

	vin.VelW += (windAcc + gGravity)*gTimeStep;
	vin.PosW += vin.VelW*gTimeStep;
	return vin;
}

// The stream-out GS is just responsible for emitting 
// new particles and destroying old particles.  The logic
// programed here will generally vary from particle system
// to particle system, as the destroy/spawn rules will be 
// different.
#define SPARKS_EMIT_AT_ONCE 5
#define SPARKS_EMIT_TIME 0.002f
[maxvertexcount(SPARKS_EMIT_AT_ONCE + 1)]
void StreamOutSparksGS(point Particle gin[1],
	inout PointStream<Particle> ptStream)
{
	if (gin[0].Type == PT_EMITTER)
	{
		// time to emit a new particle?
		if ((gin[0].Age > SPARKS_EMIT_TIME) && gEmitting)
		{
			for (int i = 0; i < SPARKS_EMIT_AT_ONCE; ++i)
			{
				// Spread rain drops out above the camera.
				float3 vRandom = RandVec3((float)i / SPARKS_EMIT_AT_ONCE);

					Particle p;
				p.PosW = gEmitPosW.xyz;
				p.VelW = gEmitDirW + vRandom*length(gEmitDirW) * 0.8;
				p.SizeW = float2(1.0f, 1.0f);
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
		if (gin[0].Age <= (gin[0].VelW.x / length(gEmitDirW) * 0.5f) + 0.5f) // InitialVelW.x instead some fixed value for some randomness.
			ptStream.Append(gin[0]);
	}
}

//***********************************************
// DRAW TECH                                    *
//***********************************************

VertexOut_PosDirType DrawSparksVS(Particle vIn)
{
	VertexOut_PosDirType vOut;
	vOut.PosW = vIn.PosW;
	vOut.DirW = normalize(vIn.VelW);
	vOut.Type  = vIn.Type;
	
	return vOut;
}

// The draw GS just expands points into lines.
[maxvertexcount(2)]
void DrawSparksGS(point VertexOut_PosDirType gin[1], inout LineStream<GeoOut_PosColor> lineStream)
{	
	// do not draw emitter particles.
	if( gin[0].Type != PT_EMITTER )
	{
		// Slant the line in velocity direction.
		float3 p0 = gin[0].PosW;
		float3 p1 = gin[0].PosW + 0.05f*gin[0].DirW;
		
		GeoOut_PosColor v0;
		v0.PosH = mul(float4(p0, 1.0f), gViewProj);
		v0.Color = float3(1.0f, 0.74f, 0.18f);
		lineStream.Append(v0);
		
		GeoOut_PosColor v1;
		v1.PosH = mul(float4(p1, 1.0f), gViewProj);
		v1.Color = float3(1.0f, 1.0f, 1.0f);
		lineStream.Append(v1);
	}
}

float4 DrawSparksPS(GeoOut_PosColor pin) : SV_TARGET
{
	return float4(pin.Color, 1.0f);
}

#endif