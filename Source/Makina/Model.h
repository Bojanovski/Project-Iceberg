
#ifndef MODEL_H
#define MODEL_H

#include "D3DAppValues.h"
#include "Subset.h"

namespace Makina
{
	class Model
	{
	public:
		Model(D3DAppValues *values)
			: mValues(values) {}

		virtual ~Model() = 0 {}

		virtual void Draw(ID3DX11EffectPass *pass, Frustum &localSpaceCamFrustum, bool distortingVertices = false) = 0;
		virtual void Draw(ID3DX11EffectPass *pass, OrientedBox &localSpaceCamOBB, bool distortingVertices = false) = 0;

		virtual void UpdateChanges() = 0;
		virtual bool ObjectSpaceRayIntersects(FXMVECTOR origin, FXMVECTOR dir, float *iDist) = 0;
		virtual OrientedBox const *GetBoundingVolume(bool distortingVertices = false) const = 0;

	protected:
		D3DAppValues *mValues;
	};
}

#endif