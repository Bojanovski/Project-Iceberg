
#ifndef CLOTH_H
#define CLOTH_H

#include "BSplineSurface.h"
#include "AsyncWorker.h"

namespace Makina
{
	class ShadowMap;

	struct Spring
	{
		short index1, index2;
		float restLength;
	};

	struct Patch
	{
		short i[4];
		float approxArea[2];
		// inverse number of neighbours
		float invNeigh[4];

		void SetInvNeigh(int u_count, int v_count);
	};

	class Cloth : public BSplineSurface, private AsyncWorker
	{
	public:
		__declspec(dllexport) Cloth(D3DAppValues *values, UINT countU, UINT countV, float uSize, float vSize, float cpMass, float elasticityC);
		__declspec(dllexport) ~Cloth();

		__declspec(dllexport) void Draw();
		__declspec(dllexport) void DrawDepthOnly();
		__declspec(dllexport) void Update(float dt);
		void SetShadowMap(ShadowMap *shadowMap) { mShadowMap = shadowMap; }
		void SetWind(const XMFLOAT3 &wind){ mWind = wind; }
		void SetGravityAcc(const XMFLOAT3 &gravityAcc){ mGravityAcc = gravityAcc; }
		__declspec(dllexport) void FixControlPoint(int index, XMFLOAT3 pos, bool movable);

	private:
		void Work();
		void ResolveSelfInterpenetration();
		void ResolveCP(short i_cp, XMVECTOR *cp_pos, FXMVECTOR n, float nProj, float minDist);

		XMFLOAT3 mCP_velocities[MAX_CONTROL_POINTS];
		XMFLOAT3 mCP_posTemp[MAX_CONTROL_POINTS];
		bool mCP_fixed[MAX_CONTROL_POINTS];
		ShadowMap *mShadowMap;

		/*Implementation of Hooke's law used to simulate elasticity of cloth.*/
		std::vector<Spring> mSprings;
		float mAvgSpringLength;

		/* Patches consist of two triangles and they are used to easily control the wind influence
		and for collision detection. */
		std::vector<Patch> mPatches;

		float mCollisionEpsilon;
		XMFLOAT3 mWind;
		XMFLOAT3 mGravityAcc;
		float mCP_mass;
		float mElasticityC;
	};
}

#endif