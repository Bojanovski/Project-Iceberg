
#ifndef BSPLINESURFACES_H
#define BSPLINESURFACES_H
#define MAX_CONTROL_POINTS 100

#include "DirectX11Headers.h"
#include "D3DAppValues.h"
#include "Geometry.h"
#include "XnaCollision.h"

namespace Makina
{
	class BSplineSurface
	{
	public:
		__declspec(dllexport) BSplineSurface(D3DAppValues *values, UINT countU, UINT countV, float uSize, float vSize);
		__declspec(dllexport) virtual ~BSplineSurface();

		virtual void Draw() = 0;
		void SetDiffuseMap(ID3D11ShaderResourceView *diffuseMapSRV) { mDiffuseMapSRV = diffuseMapSRV; }
		//void SetNormalMap(ID3D11ShaderResourceView *normalMapSRV) { mNormalMapSRV = normalMapSRV; }

		// Dynamic allocation must be aligned to the 16, therefore custom 'new' and 'delete' operators are needed.
		__declspec(dllexport) void *operator new(size_t size);
		__declspec(dllexport) void operator delete(void *pt);	
		
	protected:
		__declspec(dllexport) void RebuildBoundingVolume();
		UINT GetIndexCount(){ return mIndexCount; }
		UINT GetVertexCount(){ return mVertexCount; }
		size_t GetStride();

		UINT mUCP_Count; // control point count in U direction
		UINT mVCP_Count; // control point count in V direction
		float mMaxTessDistance;
		float mMinTessDistance;
		float mMinTessFactor;
		float mMaxTessFactor;
		XMFLOAT4 mCP[MAX_CONTROL_POINTS];
		Material mMat;	
		D3DAppValues *mValues;
		ID3DX11EffectTechnique *mBSplineDraw_FinalComplete;
		ID3DX11EffectTechnique *mBSplineDraw_DepthOnly;
		ID3DX11EffectTechnique *mBSplineDraw_DepthOnlyAlphaClip;
		AxisAlignedBox mBoundingVolume;
		ID3DX11EffectShaderResourceVariable *mDiffuseMapVar;
		//ID3DX11EffectShaderResourceVariable *mNormalMapVar;
		ID3DX11EffectShaderResourceVariable *mShadowMapVar;
		ID3DX11EffectVariable *mDirLightning;
		ID3DX11EffectMatrixVariable *mViewProj;
		ID3DX11EffectMatrixVariable *mShadowTransform;
		ID3DX11EffectScalarVariable *mShadowMapSize;
		ID3DX11EffectVectorVariable *mEyePosW;
		ID3DX11EffectVariable *mMaterial;
		ID3DX11EffectVariable *mCPVar;
		ID3DX11EffectVectorVariable *mCenterVar;
		ID3DX11EffectScalarVariable *mNUVar;
		ID3DX11EffectScalarVariable *mNVVar;
		ID3DX11EffectScalarVariable *mMaxTessDistanceVar;
		ID3DX11EffectScalarVariable *mMinTessDistanceVar;
		ID3DX11EffectScalarVariable *mMinTessFactorVar;
		ID3DX11EffectScalarVariable *mMaxTessFactorVar;
		ID3D11ShaderResourceView *mDiffuseMapSRV;
		//ID3D11ShaderResourceView *mNormalMapSRV;
		ID3D11InputLayout *mInputLayout;
		ID3D11RasterizerState *mRastState;

		ID3D11Buffer *mVertexBuffer;
		ID3D11Buffer *mIndexBuffer;

	private:
		void BuildGeometryBuffers(float uSize, float vSize);
		void BuildVertexLayout();
		
		UINT mIndexCount;
		UINT mVertexCount;
	};
}

#endif