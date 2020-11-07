
#ifndef UTILITIES_FX
#define UTILITIES_FX

#define THREADS_NUMBER 256
#define CELLS_DIMENSION 8
#define GRADIENTS_DIMENSION (CELLS_DIMENSION + 1)
#define CUBES_DIMENSION 4
#define GRADIENTS_3D_DIMENSION (CUBES_DIMENSION + 1)
#define PI 3.14159265359f

cbuffer cbPerOperationTexGenerator
{
	float gTexDimensions[3];
	uint gSeed;
	float gLacunarity;
	uint gOctaves;
	
	// 2D
	uint gGradientsToUse[2];
	float gGradients[GRADIENTS_DIMENSION][GRADIENTS_DIMENSION][2];

	//3D
	uint g3DGradientsToUse[3];
	float g3DGradients[GRADIENTS_3D_DIMENSION][GRADIENTS_3D_DIMENSION][GRADIENTS_3D_DIMENSION][3];
}

RWTexture2D<float4> gOutput;
RWTexture3D<float4> gOutput3D;

uint wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

uint rand_xorshift(uint seed)
{
    // Xorshift algorithm from George Marsaglia's paper
    seed ^= (seed << 13);
    seed ^= (seed >> 17);
    seed ^= (seed << 5);
    return seed;
}

// Scale from [-0.5f,0.5f] to [x1,x2].
float ScalePerlin(float tex, float x1, float x2)
{
	// first scale to [0,1]
	tex = (tex + 0.5f);

	// then to [x1,x2]
	float diff = x2 - x1;
	tex = tex*diff + x1;
	
	return tex;
}

float FilterRidged(float tex)
{
	if (tex > 0.0f) tex = -tex;
	return tex;
}

// Scale from [-0.5f,0] to [x1,x2].
float ScaleRidged(float tex, float x1, float x2)
{
	// first scale to [0,1]
	tex = (tex + 0.5f)*2;

	// then to [x1,x2]
	float diff = x2 - x1;
	tex = tex*diff + x1;

	return tex;
}

//			---------		2D		-----------

float SCurve12(float x, float y){	return (3*pow(x, 2) - 2*pow(x, 3)) * (3*pow(y, 2) - 2*pow(y, 3));	}

float SCurve11(float x, float y){	return (3*pow(1.0f-x, 2) - 2*pow(1.0f-x, 3)) * (3*pow(y, 2) - 2*pow(y, 3));	}

float SCurve21(float x, float y){	return (3*pow(1.0f-x, 2) - 2*pow(1.0f-x, 3)) * (3*pow(1.0f-y, 2) - 2*pow(1.0f-y, 3));	}

float SCurve22(float x, float y){	return (3*pow(x, 2) - 2*pow(x, 3)) * (3*pow(1.0f-y, 2) - 2*pow(1.0f-y, 3));	}

// Based on this: http://www.youtube.com/watch?v=Or19ilef4wE
float SampleGradient(float2 coords)
{
	coords = float2(coords.x % gTexDimensions[0], coords.y % gTexDimensions[1]);

	float2 grid = float2(gGradientsToUse[0] - 1, gGradientsToUse[1] - 1);
	float2 textelsPerCell = float2(gTexDimensions[0], gTexDimensions[1]) / grid;
	float2 gridCoords = coords / textelsPerCell;


	int2 upperLeft = (int2)(gridCoords);

	// If textel lies on the cell edge, consider it part of the cell closer to origin
	if ((gridCoords.x == upperLeft.x) && coords.x) upperLeft.x -= 1;
	if ((gridCoords.y == upperLeft.y) && coords.y) upperLeft.y -= 1;

	float2 frac = gridCoords - upperLeft;



	/*	CELL LAYOUT

		21            22
		  ____________ 
		 |            |
		 |            |
		 |            |
		 |            |
		 |____________|
		11            12

	*/

	float2 G11 = float2(gGradients[upperLeft.x][upperLeft.y + 1][0], gGradients[upperLeft.x][upperLeft.y + 1][1]);
	float2 G12 = float2(gGradients[upperLeft.x + 1][upperLeft.y + 1][0], gGradients[upperLeft.x + 1][upperLeft.y + 1][1]);
	float2 G21 = float2(gGradients[upperLeft.x][upperLeft.y][0], gGradients[upperLeft.x][upperLeft.y][1]);
	float2 G22 = float2(gGradients[upperLeft.x + 1][upperLeft.y][0], gGradients[upperLeft.x + 1][upperLeft.y][1]);

	float2 S11 = float2(-frac.x, 1.0f -frac.y);
	float2 S12 = float2(1.0f -frac.x, 1.0f -frac.y);
	float2 S21 = float2(-frac.x, -frac.y);
	float2 S22 = float2(1.0f -frac.x, -frac.y);

	float Q11 = dot(G11, S11);
	float Q12 = dot(G12, S12);
	float Q21 = dot(G21, S21);
	float Q22 = dot(G22, S22);

	float Q = Q11*SCurve11(frac.x, frac.y) + Q12*SCurve12(frac.x, frac.y) + Q21*SCurve21(frac.x, frac.y) + Q22*SCurve22(frac.x, frac.y);
	return Q;
}

//			---------		3D		-----------

// front face
float SCurve112(float3 pt){	return (3*pow(pt.x, 2) - 2*pow(pt.x, 3)) * (3*pow(pt.y, 2) - 2*pow(pt.y, 3)) * (3*pow(pt.z, 2) - 2*pow(pt.z, 3));	}
float SCurve111(float3 pt){	return (3*pow(1-pt.x, 2) - 2*pow(1-pt.x, 3)) * (3*pow(pt.y, 2) - 2*pow(pt.y, 3)) * (3*pow(pt.z, 2) - 2*pow(pt.z, 3));	}
float SCurve122(float3 pt){	return (3*pow(pt.x, 2) - 2*pow(pt.x, 3)) * (3*pow(1-pt.y, 2) - 2*pow(1-pt.y, 3)) * (3*pow(pt.z, 2) - 2*pow(pt.z, 3));	}
float SCurve121(float3 pt){	return (3*pow(1-pt.x, 2) - 2*pow(1-pt.x, 3)) * (3*pow(1-pt.y, 2) - 2*pow(1-pt.y, 3)) * (3*pow(pt.z, 2) - 2*pow(pt.z, 3));	}

// back face
float SCurve212(float3 pt){	return (3*pow(pt.x, 2) - 2*pow(pt.x, 3)) * (3*pow(pt.y, 2) - 2*pow(pt.y, 3)) * (3*pow(1-pt.z, 2) - 2*pow(1-pt.z, 3));	}
float SCurve211(float3 pt){	return (3*pow(1-pt.x, 2) - 2*pow(1-pt.x, 3)) * (3*pow(pt.y, 2) - 2*pow(pt.y, 3)) * (3*pow(1-pt.z, 2) - 2*pow(1-pt.z, 3));	}
float SCurve222(float3 pt){	return (3*pow(pt.x, 2) - 2*pow(pt.x, 3)) * (3*pow(1-pt.y, 2) - 2*pow(1-pt.y, 3)) * (3*pow(1-pt.z, 2) - 2*pow(1-pt.z, 3));	}
float SCurve221(float3 pt){	return (3*pow(1-pt.x, 2) - 2*pow(1-pt.x, 3)) * (3*pow(1-pt.y, 2) - 2*pow(1-pt.y, 3)) * (3*pow(1-pt.z, 2) - 2*pow(1-pt.z, 3));	}

float3 Get3DGradients(float x, float y, float z){return float3(g3DGradients[x][y][z][0], g3DGradients[x][y][z][1], g3DGradients[x][y][z][2]);}

// Based on previous 2D function, coords should be in range [0, 1].
float Sample3DGradient(float3 coords)
{
	coords = float3(coords.x % 1.0f, coords.y % 1.0f, coords.z % 1.0f);

	float3 space = float3(g3DGradientsToUse[0] - 1, g3DGradientsToUse[1] - 1, g3DGradientsToUse[2] - 1);
	float3 boxDimensions = float3(1, 1, 1) / space;
	float3 gridCoords = coords / boxDimensions;


	int3 upperLeftBack = (int3)(gridCoords); // or point 221 on scheme

	// If point lies on the box edge, consider it part of the box closer to origin
	if ((gridCoords.x == upperLeftBack.x) && coords.x) upperLeftBack.x -= 1;
	if ((gridCoords.y == upperLeftBack.y) && coords.y) upperLeftBack.y -= 1;
	if ((gridCoords.z == upperLeftBack.z) && coords.z) upperLeftBack.z -= 1;

	int3 p221 = upperLeftBack;
	float3 frac = gridCoords - upperLeftBack;



	/*				BOX LAYOUT
		FRONT FACE				BACK FACE

		121          122		221         222
		  ____________			 ____________	
		 |            |			|            |	
		 |            |			|            |	
		 |            |			|            |	
		 |            |			|            |	
		 |____________|			|____________|		
		111          112		211         212

	*/

	// back face
	float3 G221 = Get3DGradients(p221.x, p221.y, p221.z);
	float3 G222 = Get3DGradients(p221.x+1, p221.y, p221.z);
	float3 G211 = Get3DGradients(p221.x, p221.y+1, p221.z);
	float3 G212 = Get3DGradients(p221.x+1, p221.y+1, p221.z);

	// front face
	float3 G121 = Get3DGradients(p221.x, p221.y, p221.z+1);
	float3 G122 = Get3DGradients(p221.x+1, p221.y, p221.z+1);
	float3 G111 = Get3DGradients(p221.x, p221.y+1, p221.z+1);
	float3 G112 = Get3DGradients(p221.x+1, p221.y+1, p221.z+1);

	// back face
	float3 S221 = float3(-frac.x, -frac.y, -frac.z);
	float3 S222 = float3(1-frac.x, -frac.y, -frac.z);
	float3 S211 = float3(-frac.x, 1-frac.y, -frac.z);
	float3 S212 = float3(1-frac.x, 1-frac.y, -frac.z);

	// front face
	float3 S121 = float3(-frac.x, -frac.y, 1-frac.z);
	float3 S122 = float3(1-frac.x, -frac.y, 1-frac.z);
	float3 S111 = float3(-frac.x, 1-frac.y, 1-frac.z);
	float3 S112 = float3(1-frac.x, 1-frac.y, 1-frac.z);


	// back face
	float Q221 = dot(G221, S221);
	float Q222 = dot(G222, S222);
	float Q211 = dot(G211, S211);
	float Q212 = dot(G212, S212);

	// front face
	float Q121 = dot(G121, S121);
	float Q122 = dot(G122, S122);
	float Q111 = dot(G111, S111);
	float Q112 = dot(G112, S112);


	float Q = SCurve221(frac)*Q221 + SCurve222(frac)*Q222 + SCurve211(frac)*Q211 + SCurve212(frac)*Q212 +
		SCurve121(frac)*Q121 + SCurve122(frac)*Q122 + SCurve111(frac)*Q111 + SCurve112(frac)*Q112;
	return Q;
}

#endif
