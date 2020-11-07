#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <string>
#include <map>
#include <fstream>
#include "Geometry.h"
#include "BasicModel.h"
#include "SkinnedModel.h"
#include "MeshAnimationData.h"

namespace Makina
{
	class ModelFileLoader
	{
	public:
		__declspec(dllexport) ModelFileLoader(D3DAppValues *values);
		__declspec(dllexport) virtual ~ModelFileLoader();

		virtual void LoadModel(const wchar_t *path) = 0;
		__declspec(dllexport) Model *GetModel(const wchar_t *path);
		__declspec(dllexport) void RemoveModel(Model *mdl);
		
	protected:
		void CalculateTangents(VertexFull &v0, VertexFull &v1, VertexFull &v2);
		void OrthonormalizeTangents(VertexFull &v);
		void CalculateTangents(SkinnedVertexFull &v0, SkinnedVertexFull &v1, SkinnedVertexFull &v2);
		void OrthonormalizeTangents(SkinnedVertexFull &v);

		D3DAppValues *mValues;
		std::map<std::wstring, Model *> mLoaded;
	};
}

#endif