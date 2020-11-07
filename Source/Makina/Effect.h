
#ifndef EFFECT_H
#define EFFECT_H

#include "D3DAppValues.h"

namespace Makina
{
	class Effect
	{
	public:	
		__declspec(dllexport) Effect(D3DAppValues *values, const wchar_t *file);
		__declspec(dllexport) Effect(D3DAppValues *values, const wchar_t *library, UINT resourceId);
		__declspec(dllexport) virtual ~Effect();

		__declspec(dllexport) ID3DX11Effect *GetFX() { return mFX; }

	protected:
		D3DAppValues *mValues;
		ID3DX11Effect *mFX;

	private:
		__declspec(dllexport) void LoadEffect(const wchar_t *file);
		__declspec(dllexport) void LoadEffect(const wchar_t *library, UINT resourceId);


	};
}

#endif