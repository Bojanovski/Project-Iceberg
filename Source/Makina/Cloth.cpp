
#include "Cloth.h"
#include "ShadowMap.h"
#include "D3DAppValues.h"
#include "Camera.h"
#include "BasicEffect.h"

#define RESTITUTION_COEFFICIENT 0.0f

#define DIV_6 1.0f / 6.0f
#define DIV_3 1.0f / 3.0f
#define DIV_2 1.0f / 2.0f

using namespace Makina;

void Patch::SetInvNeigh(int u_count, int v_count)
{
	for (int n = 0; n < 4; ++n)
	{
		float c;
		if (i[n] == 0) c = DIV_2; // upper left
		else if (i[n] == u_count*v_count - 1) c = DIV_2; // lower right
		else if (i[n] == u_count - 1) c = 1.0f; // upper right
		else if (i[n] == u_count*(v_count - 1)) c = 1.0f; // lower left

		else if (i[n] % u_count == 0) c = DIV_3; // first column
		else if (i[n] % u_count == (u_count - 1)) c = DIV_3; // last column

		else if (i[n] / u_count == 0) c = DIV_3; // first row
		else if (i[n] / u_count == (v_count - 1)) c = DIV_3; // last row

		else c = DIV_6;

		invNeigh[n] = c;
	}
}

Cloth::Cloth(D3DAppValues *values, UINT countU, UINT countV, float uSize, float vSize, float cpMass, float elasticityC)
: BSplineSurface(values, countU, countV, uSize, vSize),
mCP_mass(cpMass),
mElasticityC(elasticityC)
{
	mAvgSpringLength = 0.0f;
	int springsCount = 0;
	int cpNum = mUCP_Count * mVCP_Count;
	for (int i = 0; i < cpNum; ++i)
	{
		mCP_fixed[i] = false;

		if ((i % mUCP_Count != (mUCP_Count - 1)) && (i / mUCP_Count != (mVCP_Count - 1)))
		{ // not last column and not last row

			/*		i----------i1
					| \        |
					|   \      |
					|     \    |
					|       \  |
					i2---------i3

					This is how upper left control point will be connected to its neighbours. */

			Spring tempS;
			tempS.index1 = i;
			int i1 = i + 1;
			int i2 = i + mUCP_Count;
			int i3 = i + mUCP_Count + 1;
			XMVECTOR pos0 = XMLoadFloat4(&mCP[i]);
			XMVECTOR pos1 = XMLoadFloat4(&mCP[i1]);
			XMVECTOR pos2 = XMLoadFloat4(&mCP[i2]);
			XMVECTOR pos3 = XMLoadFloat4(&mCP[i3]);

			// i1
			tempS.index2 = i1;
			tempS.restLength = XMVectorGetX(XMVector3Length(pos0 - pos1));
			mAvgSpringLength += tempS.restLength;
			++springsCount;
			mSprings.push_back(tempS);

			// i2
			tempS.index2 = i2;
			tempS.restLength = XMVectorGetX(XMVector3Length(pos0 - pos2));
			mAvgSpringLength += tempS.restLength;
			++springsCount;
			mSprings.push_back(tempS);

			// i3
			tempS.index2 = i3;
			tempS.restLength = XMVectorGetX(XMVector3Length(pos0 - pos3));
			mAvgSpringLength += tempS.restLength;
			++springsCount;
			mSprings.push_back(tempS);

			// also connect i2 and i1 (optional)
			tempS.index1 = i1;
			tempS.index2 = i2;
			tempS.restLength = XMVectorGetX(XMVector3Length(pos1 - pos2));
			mAvgSpringLength += tempS.restLength;
			++springsCount;
			mSprings.push_back(tempS);

			// patches for drag are added here
			Patch tempP;
			tempP.i[0] = i;
			tempP.i[1] = i1;
			tempP.i[2] = i2;
			tempP.i[3] = i3;
			// first triangle (i0, i2 and i3)
			XMVECTOR normal1 = XMVector3Cross(pos3 - pos0, pos2 - pos0);
			tempP.approxArea[0] = 0.5f*XMVectorGetX(XMVector3Length(normal1));
			// then the other one (i0, i1 and i3)
			XMVECTOR normal2 = XMVector3Cross(pos1 - pos0, pos3 - pos0);
			tempP.approxArea[1] = 0.5f*XMVectorGetX(XMVector3Length(normal2));
			tempP.SetInvNeigh(mUCP_Count, mVCP_Count);
			mPatches.push_back(tempP);
		}

		if ((i % mUCP_Count == (mUCP_Count - 1)) && (i / mUCP_Count != (mVCP_Count - 1)))
		{// last column but not last row

			/*		 
					------i
						  |
						  |
						  |
						  |
					------i1

						  We need to connect the last column. */

			Spring tempS;
			tempS.index1 = i;
			int i1 = i + mUCP_Count;

			// i1
			tempS.index2 = i1;
			tempS.restLength = XMVectorGetX(XMVector3Length(XMLoadFloat4(&mCP[i]) - XMLoadFloat4(&mCP[i1])));
			mAvgSpringLength += tempS.restLength;
			++springsCount;
			mSprings.push_back(tempS);
		}

		if ((i % mUCP_Count != (mUCP_Count - 1)) && (i / mUCP_Count == (mVCP_Count - 1)))
		{// last row but not last column

			/*
					|     \    |
					|       \  |
					i----------i1

					We need to connect the last row. */

			Spring tempS;
			tempS.index1 = i;
			int i1 = i + 1;

			// i1
			tempS.index2 = i1;
			tempS.restLength = XMVectorGetX(XMVector3Length(XMLoadFloat4(&mCP[i]) - XMLoadFloat4(&mCP[i1])));
			mAvgSpringLength += tempS.restLength;
			++springsCount;
			mSprings.push_back(tempS);
		}
	}

	mAvgSpringLength /= (float)springsCount;
	mCollisionEpsilon = mAvgSpringLength * 0.3333f;

	// initialize temporary cp array
	for (int i = 0; i < cpNum; ++i)
	{
		mCP_posTemp[i].x = mCP[i].x;
		mCP_posTemp[i].y = mCP[i].y;
		mCP_posTemp[i].z = mCP[i].z;
	}
}

Cloth::~Cloth()
{

}
#include "GameTimer.h"
void Cloth::Draw()
{
	// check subset bounding volume
	if (IntersectAxisAlignedBoxFrustum(&mBoundingVolume, &mValues->mCamera->GetFrustum()) == 0) 	return;

	mValues->md3dImmediateContext->IASetInputLayout(mInputLayout);
	mValues->md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	mValues->md3dImmediateContext->OMSetDepthStencilState(0, 0);
	mValues->md3dImmediateContext->RSSetState(mRastState);
	mValues->md3dImmediateContext->OMSetBlendState(0, 0, 0xffffffff);

	mMaterial->SetRawValue(&mMat, 0, sizeof(Material));
	mDirLightning->SetRawValue(&mValues->mBasicEffect->DirLight(), 0, sizeof(DirectionalLight));
	mCPVar->SetRawValue(mCP, 0, MAX_CONTROL_POINTS * sizeof(XMFLOAT4));
	mCenterVar->SetRawValue(&mBoundingVolume.Center, 0, sizeof(XMFLOAT3));
	mNUVar->SetInt(mUCP_Count);
	mNVVar->SetInt(mVCP_Count);
	mDiffuseMapVar->SetResource(mDiffuseMapSRV);
	//mNormalMap->SetResource(mNormalMapSRV);
	mShadowMapVar->SetResource(mShadowMap->GetSRV());
	mShadowMapSize->SetFloat((float)mShadowMap->GetSize());
	mShadowTransform->SetMatrix(reinterpret_cast<float*>(&mShadowMap->GetViewProjTransform()));

	mViewProj->SetMatrix(reinterpret_cast<float*>(&mValues->mCamera->ViewProj()));
	mEyePosW->SetRawValue(&mValues->mCamera->GetPosition(), 0, sizeof(XMFLOAT3));
	mMaxTessDistanceVar->SetFloat(mMaxTessDistance);
	mMinTessDistanceVar->SetFloat(mMinTessDistance);
	mMinTessFactorVar->SetFloat(mMinTessFactor);
	mMaxTessFactorVar->SetFloat(mMaxTessFactor);

	UINT stride = GetStride();
	UINT offset = 0;
	mValues->md3dImmediateContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	mValues->md3dImmediateContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	mBSplineDraw_FinalComplete->GetDesc(&techDesc);

	for (UINT i = 0; i < techDesc.Passes; i++)
	{
		mBSplineDraw_FinalComplete->GetPassByIndex(i)->Apply(0, mValues->md3dImmediateContext);
		mValues->md3dImmediateContext->DrawIndexed(GetIndexCount(), 0, 0);
	}

	// FX sets tessellation stages, but it does not disable them.  So do that here
	// to turn off tessellation.
	mValues->md3dImmediateContext->HSSetShader(0, 0, 0);
	mValues->md3dImmediateContext->DSSetShader(0, 0, 0);
}

void Cloth::DrawDepthOnly()
{
	// First check frustum or OBB culling
	if (mShadowMap->GetType() == Projective)
	{
		if (IntersectAxisAlignedBoxFrustum(&mBoundingVolume, &mShadowMap->GetFrustum()) == 0)	return;
	}
	else if (mShadowMap->GetType() == Orthographic)
	{
		if (IntersectAxisAlignedBoxOrientedBox(&mBoundingVolume, &mShadowMap->GetOrientedBox()) == 0) return;
	}

	// It's ok, let's countinue.
	mValues->md3dImmediateContext->IASetInputLayout(mInputLayout);
	mValues->md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	mValues->md3dImmediateContext->OMSetDepthStencilState(0, 0);
	mValues->md3dImmediateContext->RSSetState(mRastState);
	mValues->md3dImmediateContext->OMSetBlendState(0, 0, 0xffffffff);

	mCPVar->SetRawValue(mCP, 0, MAX_CONTROL_POINTS * sizeof(XMFLOAT4));
	mCenterVar->SetRawValue(&mBoundingVolume.Center, 0, sizeof(XMFLOAT3));
	mNUVar->SetInt(mUCP_Count);
	mNVVar->SetInt(mVCP_Count);
	mDiffuseMapVar->SetResource(mDiffuseMapSRV);

	mViewProj->SetMatrix(reinterpret_cast<float*>(&mShadowMap->GetViewProj()));
	mEyePosW->SetRawValue(&mValues->mCamera->GetPosition(), 0, sizeof(XMFLOAT3));
	mMaxTessDistanceVar->SetFloat(mMaxTessDistance);
	mMinTessDistanceVar->SetFloat(mMinTessDistance);
	mMinTessFactorVar->SetFloat(mMinTessFactor);
	mMaxTessFactorVar->SetFloat(mMaxTessFactor);

	UINT stride = GetStride();
	UINT offset = 0;
	mValues->md3dImmediateContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	mValues->md3dImmediateContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	mBSplineDraw_DepthOnlyAlphaClip->GetDesc(&techDesc);

	for (UINT i = 0; i < techDesc.Passes; i++)
	{
		mBSplineDraw_DepthOnlyAlphaClip->GetPassByIndex(i)->Apply(0, mValues->md3dImmediateContext);
		mValues->md3dImmediateContext->DrawIndexed(GetIndexCount(), 0, 0);
	}

	// FX sets tessellation stages, but it does not disable them.  So do that here
	// to turn off tessellation.
	mValues->md3dImmediateContext->HSSetShader(0, 0, 0);
	mValues->md3dImmediateContext->DSSetShader(0, 0, 0);
}

void Cloth::Update(float dt)
{
	//dt /= 10;

	Join();

	// Adjust positions
	int cpNum = mUCP_Count * mVCP_Count;
	for (int i = 0; i < cpNum; ++i)
	{
		XMVECTOR pos = XMLoadFloat3(&mCP_posTemp[i]);
		XMStoreFloat4(&mCP[i], pos);
	}

	Assign(&dt, sizeof(dt));
	//Join();
}

void Cloth::FixControlPoint(int index, XMFLOAT3 pos, bool movable)
{
	mCP_fixed[index] = !movable;
	mCP[index] = XMFLOAT4(pos.x, pos.y, pos.z, 1.0f);
	mCP_posTemp[index] = pos;
	mCP_velocities[index] = {0.0f, 0.0f, 0.0f};
}

void Cloth::Work()
{
	float dt = *(float *)mData;

	// Integrate (euler method)
	float CP_inverseMass = 1 / mCP_mass;
	int cpNum = mUCP_Count * mVCP_Count;

	// air drag
	XMVECTOR wind = XMLoadFloat3(&mWind);
	float windLen = XMVectorGetX(XMVector3Length(wind));
	XMVECTOR windDir = XMVector3Normalize(wind);
	float C = 0.9f; // Drag coefficient – a dimensionless number.
	for (auto &patch : mPatches)
	{
		/*		i0---------i1
				| \        |
				|   \      |
				|     \    |
				|       \  |
				i2---------i3

				Drag will be calculated for triangles separately.  */

		XMVECTOR vel0 = XMLoadFloat3(&mCP_velocities[patch.i[0]]);
		XMVECTOR vel1 = XMLoadFloat3(&mCP_velocities[patch.i[1]]);
		XMVECTOR vel2 = XMLoadFloat3(&mCP_velocities[patch.i[2]]);
		XMVECTOR vel3 = XMLoadFloat3(&mCP_velocities[patch.i[3]]);
		XMVECTOR pos0 = XMLoadFloat3(&mCP_posTemp[patch.i[0]]);
		XMVECTOR pos1 = XMLoadFloat3(&mCP_posTemp[patch.i[1]]);
		XMVECTOR pos2 = XMLoadFloat3(&mCP_posTemp[patch.i[2]]);
		XMVECTOR pos3 = XMLoadFloat3(&mCP_posTemp[patch.i[3]]);

		// first triangle (i0, i2 and i3)
		XMVECTOR normal1 = XMVector3Cross(pos3 - pos0, pos2 - pos0);
		normal1 = XMVector3Normalize(normal1);
		XMVECTOR avgVel = 0.3333f*(vel0 + vel2 + vel3);
		float windDirNormal = XMVectorGetX(XMVector3Dot(windDir, normal1));
		float A = abs(windDirNormal) * patch.approxArea[0];
		float relVel = XMVectorGetX(XMVector3Dot(wind, normal1)) - XMVectorGetX(XMVector3Dot(avgVel, normal1));
		float relVelSign = (float)((relVel > 0) - (relVel < 0));
		// 0.3333f is here because we have 3 control points with their masses.
		XMVECTOR windAcc1 = 0.3333f*CP_inverseMass*0.5f*relVel*C*A*normal1;

		// then the other one (i0, i1 and i3)
		XMVECTOR normal2 = XMVector3Cross(pos1 - pos0, pos3 - pos0);
		normal2 = XMVector3Normalize(normal2);
		avgVel = 0.3333f*(vel0 + vel1 + vel3);
		windDirNormal = XMVectorGetX(XMVector3Dot(windDir, normal2));
		A = abs(windDirNormal) * patch.approxArea[1];
		relVel = XMVectorGetX(XMVector3Dot(wind, normal2)) - XMVectorGetX(XMVector3Dot(avgVel, normal2));
		relVelSign = (float)((relVel > 0) - (relVel < 0));
		XMVECTOR windAcc2 = 0.3333f*CP_inverseMass*0.5f*relVel*C*A*normal2;

		// Apply and store
		if (!mCP_fixed[patch.i[0]])
		{
			vel0 += (windAcc1 + windAcc2)* dt * patch.invNeigh[0];
			XMStoreFloat3(&mCP_velocities[patch.i[0]], vel0);
		}

		if (!mCP_fixed[patch.i[1]])
		{
			vel1 += (windAcc2)* dt * patch.invNeigh[1];
			XMStoreFloat3(&mCP_velocities[patch.i[1]], vel1);
		}

		if (!mCP_fixed[patch.i[2]])
		{
			vel2 += (windAcc1)* dt * patch.invNeigh[2];
			XMStoreFloat3(&mCP_velocities[patch.i[2]], vel2);
		}

		if (!mCP_fixed[patch.i[3]])
		{
			vel3 += (windAcc1 + windAcc2)* dt * patch.invNeigh[3];
			XMStoreFloat3(&mCP_velocities[patch.i[3]], vel3);
		}
	}

	// cloth elasticity
	for (auto &spring : mSprings)
	{
		XMVECTOR dir = XMLoadFloat3(&mCP_posTemp[spring.index1]) - XMLoadFloat3(&mCP_posTemp[spring.index2]);
		float length = XMVectorGetX(XMVector3Length(dir));
		dir = XMVector3Normalize(dir);

		XMVECTOR acc = mElasticityC*CP_inverseMass*(length - spring.restLength)*dir;

		XMVECTOR vel1 = XMLoadFloat3(&mCP_velocities[spring.index1]);
		XMVECTOR vel2 = XMLoadFloat3(&mCP_velocities[spring.index2]);
		vel1 -= acc * dt;
		vel2 += acc * dt;

		if (!mCP_fixed[spring.index1]) XMStoreFloat3(&mCP_velocities[spring.index1], vel1);
		if (!mCP_fixed[spring.index2]) XMStoreFloat3(&mCP_velocities[spring.index2], vel2);
	}


	XMVECTOR gravAcc = XMLoadFloat3(&mGravityAcc);

	// apply gravity and adjust positions
	for (int i = 0; i < cpNum; ++i)
	{
		if (mCP_fixed[i]) continue;

		XMVECTOR vel = XMLoadFloat3(&mCP_velocities[i]);
		vel += gravAcc * dt;
		vel *= pow(0.4f, dt);
		float maxSpeed = mCollisionEpsilon / (dt * 3.01f);
		float speed = XMVectorGetX(XMVector3Length(vel));
		if (speed > maxSpeed) vel *= maxSpeed / speed;

		XMVECTOR posOld = XMLoadFloat4(&mCP[i]);
		XMVECTOR pos = XMLoadFloat3(&mCP_posTemp[i]);
		pos += vel * dt;

		XMStoreFloat3(&mCP_velocities[i], vel);
		XMStoreFloat3(&mCP_posTemp[i], pos);
	}

	ResolveSelfInterpenetration();
	RebuildBoundingVolume();
}

struct Edge
{
	XMVECTOR *p1, line;
	bool doTest;
};

void Cloth::ResolveSelfInterpenetration()
{
	//		i0----e0----i1
	//		| \         |
	//		|   \       |
	//		e3    e4    e1
	//		|       \   |
	//		|         \ |
	//		i2----e2----i3

	int cpNum = mUCP_Count * mVCP_Count;

	for (auto &patch : mPatches)
	{
		XMVECTOR pos0 = XMLoadFloat3(&mCP_posTemp[patch.i[0]]);
		XMVECTOR pos1 = XMLoadFloat3(&mCP_posTemp[patch.i[1]]);
		XMVECTOR pos2 = XMLoadFloat3(&mCP_posTemp[patch.i[2]]);
		XMVECTOR pos3 = XMLoadFloat3(&mCP_posTemp[patch.i[3]]);
		XMVECTOR *coordO, normal[2];
		coordO = &pos0;

		// Compute bounding sphere
		XMVECTOR patchCenter = (pos0 + pos1 + pos2 + pos3) * 0.25f;
		float rSq0 = XMVectorGetX(XMVector3LengthSq(pos0 - patchCenter));
		float rSq1 = XMVectorGetX(XMVector3LengthSq(pos1 - patchCenter));
		float rSq2 = XMVectorGetX(XMVector3LengthSq(pos2 - patchCenter));
		float rSq3 = XMVectorGetX(XMVector3LengthSq(pos3 - patchCenter));
		float patchR = rSq0; // find max
		if (patchR < rSq1) patchR = rSq1;
		if (patchR < rSq2) patchR = rSq2;
		if (patchR < rSq3) patchR = rSq3;
		patchR = sqrt(patchR);
		float boundingSphereRadius = patchR + mCollisionEpsilon;
		float boundingSphereRadiusSq = boundingSphereRadius*boundingSphereRadius;
		
		// Define edges
		Edge e[5];
		e[0].line = pos1 - pos0;
		e[0].p1 = &pos0;
		e[1].line = pos3 - pos1;
		e[1].p1 = &pos1;
		e[2].line = pos3 - pos2;
		e[2].p1 = &pos2;
		e[3].line = pos2 - pos0;
		e[3].p1 = &pos0;
		e[4].line = pos3 - pos0;
		e[4].p1 = &pos0;

		// first triangle
		normal[0] = XMVector3Normalize(XMVector3Cross(e[0].line, e[4].line));
		XMVECTOR e0_Plane = XMPlaneFromPointNormal(pos0, XMVector3Normalize(XMVector3Cross(normal[0], e[0].line)));
		XMVECTOR e1_Plane = XMPlaneFromPointNormal(pos1, XMVector3Normalize(XMVector3Cross(normal[0], e[1].line)));

		// second triangle
		normal[1] = XMVector3Normalize(XMVector3Cross(e[4].line, e[3].line));
		XMVECTOR e3_Plane = XMPlaneFromPointNormal(pos0, XMVector3Normalize(XMVector3Cross(e[3].line, normal[1])));
		XMVECTOR e2_Plane = XMPlaneFromPointNormal(pos2, XMVector3Normalize(XMVector3Cross(e[2].line, normal[1])));

		for (int i = 0; i < cpNum; ++i)
		{
			int i_cp = i;
			if (mCP_fixed[i_cp]) continue;

			if (i_cp == patch.i[0] || i_cp == patch.i[1] || i_cp == patch.i[2] || i_cp == patch.i[3]) continue;
			XMVECTOR cp_pos = XMLoadFloat3(&mCP_posTemp[i_cp]);
			if (XMVectorGetX(XMVector3LengthSq(patchCenter - cp_pos)) > boundingSphereRadiusSq) continue;
			XMVECTOR originToPos = cp_pos - *coordO;
			float normalProj;

			normalProj = XMVectorGetX(XMVector3Dot(normal[0], originToPos));
			if (abs(normalProj) < mCollisionEpsilon) // first triangle
			{
				XMVECTOR e4_Plane = XMPlaneFromPointNormal(pos0, XMVector3Normalize(XMVector3Cross(e[4].line, normal[0])));
				if (XMVectorGetX(XMPlaneDotCoord(e0_Plane, cp_pos)) < 0.0f)  e[0].doTest = true;
				else e[0].doTest = false;
				if (XMVectorGetX(XMPlaneDotCoord(e1_Plane, cp_pos)) < 0.0f)  e[1].doTest = true;
				else e[1].doTest = false;
				if (XMVectorGetX(XMPlaneDotCoord(e4_Plane, cp_pos)) < 0.0f)  e[4].doTest = true;
				else e[4].doTest = false;

				if (!e[0].doTest && !e[1].doTest && !e[4].doTest)
				{
					ResolveCP(i_cp, &cp_pos, normal[0], normalProj, mCollisionEpsilon);
					originToPos = cp_pos - *coordO;
				}
			}
			else e[0].doTest = e[1].doTest = e[4].doTest = false;

			normalProj = XMVectorGetX(XMVector3Dot(normal[1], originToPos));
			if (abs(normalProj) < mCollisionEpsilon) // second triangle
			{
				XMVECTOR e4_Plane = XMPlaneFromPointNormal(pos0, XMVector3Normalize(XMVector3Cross(normal[1], e[4].line)));
				if (XMVectorGetX(XMPlaneDotCoord(e3_Plane, cp_pos)) < 0.0f)  e[3].doTest = true;
				else e[3].doTest = false;
				if (XMVectorGetX(XMPlaneDotCoord(e4_Plane, cp_pos)) < 0.0f)  e[4].doTest = true;
				else e[4].doTest = false;
				if (XMVectorGetX(XMPlaneDotCoord(e2_Plane, cp_pos)) < 0.0f)  e[2].doTest = true;
				else e[2].doTest = false;

				if (!e[3].doTest && !e[4].doTest && !e[2].doTest)
				{
					ResolveCP(i_cp, &cp_pos, normal[1], normalProj, mCollisionEpsilon);
				}
			}
			else e[3].doTest = e[4].doTest = e[2].doTest = false;

			if (patch.i[1] % mUCP_Count != (mUCP_Count - 1)) e[1].doTest = false; // not last column
			if (patch.i[1] / mUCP_Count != (mVCP_Count - 1)) e[2].doTest = false; // not last row

			// edges
			for (int j = 0; j < 5; ++j)
			if (e[j].doTest)
			{
				XMVECTOR cp_plane = XMPlaneFromPointNormal(cp_pos, XMVector3Normalize(e[j].line));
				XMVECTOR ePt2 = *e[j].p1 + e[j].line;
				XMVECTOR intersectionP = XMPlaneIntersectLine(cp_plane, *e[j].p1, ePt2);
				float dot1 = XMVectorGetX(XMVector3Dot(e[j].line, intersectionP - *e[j].p1));
				float dot2 = XMVectorGetX(XMVector3Dot(-e[j].line, intersectionP - ePt2));
				if (XMVectorGetX(XMVectorIsNaN(intersectionP)) == 0 && dot1 >= 0.0f && dot2 >= 0.0f)
				{
					XMVECTOR n = XMVector3Normalize(cp_pos - intersectionP);
					normalProj = XMVectorGetX(XMVector3Dot(n, cp_pos - intersectionP));
					if (normalProj < mCollisionEpsilon) ResolveCP(i_cp, &cp_pos, n, normalProj, mCollisionEpsilon);
				}
			}

			// vertices
			if (e[0].doTest && e[3].doTest) // i0
			{
				XMVECTOR n = XMVector3Normalize(cp_pos - pos0);
				normalProj = XMVectorGetX(XMVector3Dot(n, cp_pos - pos0));
				if (normalProj < mCollisionEpsilon) ResolveCP(i_cp, &cp_pos, n, normalProj, mCollisionEpsilon);
			}
			if (e[0].doTest && e[1].doTest) // i1
			{
				XMVECTOR n = XMVector3Normalize(cp_pos - pos1);
				normalProj = XMVectorGetX(XMVector3Dot(n, cp_pos - pos1));
				if (normalProj < mCollisionEpsilon) ResolveCP(i_cp, &cp_pos, n, normalProj, mCollisionEpsilon);
			}
			if (e[2].doTest && e[3].doTest) // i2
			{
				XMVECTOR n = XMVector3Normalize(cp_pos - pos2);
				normalProj = XMVectorGetX(XMVector3Dot(n, cp_pos - pos2));
				if (normalProj < mCollisionEpsilon) ResolveCP(i_cp, &cp_pos, n, normalProj, mCollisionEpsilon);
			}
			if (e[1].doTest && e[2].doTest) // i3
			{
				XMVECTOR n = XMVector3Normalize(cp_pos - pos3);
				normalProj = XMVectorGetX(XMVector3Dot(n, cp_pos - pos3));
				if (normalProj < mCollisionEpsilon) ResolveCP(i_cp, &cp_pos, n, normalProj, mCollisionEpsilon);
			}

			//// Now some ray casting to spot the mistakes made in previous part of this algorithm.
			//// Those mistakes are very rare.
			//bool collision = false;
			//if (i_cp % mUCP_Count != 0) // not first column
			//{
			//	//collision = IntersectRayTriangle()
			//	XMVECTOR cpN_pos = XMLoadFloat3(&mCP_posTemp[i_cp - mUCP_Count]);
			//	XMVECTOR dir = cpN_pos - cp_pos;
			//	float length = XMVectorGetX(XMVector3Length(dir));
			//	if (length != 0.0f)
			//	{
			//		dir = XMVector3Normalize(dir);
			//		float d1, d2;
			//		if (IntersectRayTriangle(cp_pos, dir, pos0, pos2, pos3, &d1) || IntersectRayTriangle(cp_pos, dir, pos0, pos2, pos3, &d2))
			//		if ((d1 < length && d1 > 0.0f) || (d2 < length && d2 > 0.0f))
			//		{
			//			++cp_coll[i_cp];
			//		}
			//	}
			//	

			//	
			//}
			//if (i_cp % mUCP_Count != (mUCP_Count - 1)) // not last column
			//{

			//}
			//if (i_cp / mUCP_Count != 0) // not first row
			//{

			//}
			//if (i_cp / mUCP_Count != (mVCP_Count - 1)) // not last row
			//{

			//}
		}
	}
}

void Cloth::ResolveCP(short i_cp, XMVECTOR *cp_pos, FXMVECTOR n, float nProj, float minDist)
{
	*cp_pos += n * (minDist / abs(nProj) - 1.0f) * nProj;

	XMVECTOR vel = XMLoadFloat3(&mCP_velocities[i_cp]);
	float normalProj = XMVectorGetX(XMVector3Dot(vel, n));
	vel -= normalProj * n * (1.0f + RESTITUTION_COEFFICIENT);

	XMStoreFloat3(&mCP_posTemp[i_cp], *cp_pos);
	XMStoreFloat3(&mCP_velocities[i_cp], vel);
}