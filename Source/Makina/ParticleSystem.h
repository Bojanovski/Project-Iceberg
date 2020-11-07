#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include "D3DAppValues.h"
#include "Camera.h"
#include "Effect.h"

namespace Makina
{
	class ParticleSystem
	{
	public:

		__declspec(dllexport) ParticleSystem(D3DAppValues *values, ID3D11ShaderResourceView* randomTexSRV, UINT maxParticles);
		__declspec(dllexport) virtual ~ParticleSystem();


		//
		// Get
		//

		// Time elapsed since the system was reset.
		float GetAge()const	{ return mAge; }
		const bool &IsEmitting() { return mEmitting; }
		const XMFLOAT3 &GetEmitPos() { return mEmitPosW; }
		const XMFLOAT3 &GetEmitDir() { return mEmitDirW; }
		const XMFLOAT3 &GetWind() { return mWind; }
		const XMFLOAT3 &GetGravity() { return mGravity; }

		//
		// Set
		//

		void SetEmitting(bool emit) { mEmitting = emit; }
		void SetEmitPos(const XMFLOAT3 &emitPosW) { mEmitPosW = emitPosW; }
		void SetEmitDir(const XMFLOAT3 &emitDirW) { mEmitDirW = emitDirW; }
		void SetWind(const XMFLOAT3 &wind) { mWind = wind; }
		void SetGravity(const XMFLOAT3 &gravity) { mGravity = gravity; }

		__declspec(dllexport) void Reset();
		virtual void Draw() = 0;

	protected:		
		__declspec(dllexport) void UpdateParameters();

		__declspec(dllexport) void BuildGeometryBuffers();
		virtual void BuildVertexLayout() = 0;


		Camera *mCam;
		D3DAppValues *mValues;

		UINT mMaxParticles;
		bool mFirstRun;
		bool mEmitting;

		float mGameTime;
		float mTimeStep;
		float mAge;

		XMFLOAT3 mEmitPosW;
		XMFLOAT3 mEmitDirW;
		XMFLOAT3 mWind;
		XMFLOAT3 mGravity;

		ID3D11Buffer* mInitVB;	
		ID3D11Buffer* mDrawVB;
		ID3D11Buffer* mStreamOutVB;

		ID3D11ShaderResourceView* mRandomTexSRV;

		//
		// Shader variables
		//
		ID3DX11EffectTechnique* mParticleUpdate;
		ID3DX11EffectTechnique* mParticleDraw;
		ID3D11InputLayout *mInputLayout;

		ID3DX11EffectScalarVariable *mEmit;
		ID3DX11EffectMatrixVariable* mViewProjVar;
		ID3DX11EffectScalarVariable* mGameTimeVar;
		ID3DX11EffectScalarVariable* mTimeStepVar;
		ID3DX11EffectVectorVariable* mEyePosWVar;
		ID3DX11EffectVectorVariable* mEmitPosWVar;
		ID3DX11EffectVectorVariable* mEmitDirWVar;
		ID3DX11EffectVectorVariable* mWindVar;
		ID3DX11EffectVectorVariable* mGravityVar;
		ID3DX11EffectShaderResourceVariable* mTexArrayVar;
		ID3DX11EffectShaderResourceVariable* mRandomTexVar;
	};
}

#endif