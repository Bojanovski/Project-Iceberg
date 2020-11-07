
#include "ModelFileLoader.h"
#include "Exceptions.h"
#include "MathHelper.h"

using namespace Makina;
using namespace std;
using namespace MathHelper;

ModelFileLoader::ModelFileLoader(D3DAppValues *values)
: mValues(values),
mLoaded()
{

}

ModelFileLoader::~ModelFileLoader()
{

}

Model *ModelFileLoader::GetModel(const wchar_t *path)
{
	if (mLoaded.find(path) == mLoaded.end())
	{
		// not found
		throw UnexpectedError(wstring(L"Failed to get model '") + path + L"'! (ModelFileLoader::GetModel)");
	}
	else
	{
		return mLoaded[path];
	}
}

void ModelFileLoader::RemoveModel(Model *mdl)
{
	bool removed = false;

	for (auto texIt = mLoaded.begin(); texIt != mLoaded.end();)
	{
		if (texIt->second == mdl)
		{
			delete texIt->second;
			texIt = mLoaded.erase(texIt);
			removed = true;
		}
		else ++texIt;
	}

	if (!removed) throw UnexpectedError(L"Model does not exist. (ModelFileLoader::RemoveModel)");
}

void ModelFileLoader::CalculateTangents(VertexFull &v0, VertexFull &v1, VertexFull &v2)
{
	XMFLOAT3 e1(v1.Position.x - v0.Position.x, v1.Position.y - v0.Position.y, v1.Position.z - v0.Position.z);
	XMFLOAT3 e2(v2.Position.x - v0.Position.x, v2.Position.y - v0.Position.y, v2.Position.z - v0.Position.z);

	float deltaU0 = v1.TexC.x - v0.TexC.x;
	float deltaV0 = v1.TexC.y - v0.TexC.y;

	float deltaU1 = v2.TexC.x - v0.TexC.x;
	float deltaV1 = v2.TexC.y - v0.TexC.y;

	float c = 1.0f / (deltaU0*deltaV1 - deltaV0*deltaU1);

	XMFLOAT3 tangent(c*(deltaV1*e1.x - deltaV0*e2.x),
		c*(deltaV1*e1.y - deltaV0*e2.y),
		c*(deltaV1*e1.z - deltaV0*e2.z));

	v0.TangentU += tangent;
	v1.TangentU += tangent;
	v2.TangentU += tangent;
}

void ModelFileLoader::OrthonormalizeTangents(VertexFull &v)
{
	XMVECTOR n = XMLoadFloat3(&v.Normal);
	XMVECTOR t = XMLoadFloat3(&v.TangentU);
	t = XMVector3Normalize(t - n * XMVector3Dot(n, t));
	XMStoreFloat3(&v.TangentU, t);
}

void ModelFileLoader::CalculateTangents(SkinnedVertexFull &v0, SkinnedVertexFull &v1, SkinnedVertexFull &v2)
{
	XMFLOAT3 e1(v1.Position.x - v0.Position.x, v1.Position.y - v0.Position.y, v1.Position.z - v0.Position.z);
	XMFLOAT3 e2(v2.Position.x - v0.Position.x, v2.Position.y - v0.Position.y, v2.Position.z - v0.Position.z);

	float deltaU0 = v1.TexC.x - v0.TexC.x;
	float deltaV0 = v1.TexC.y - v0.TexC.y;

	float deltaU1 = v2.TexC.x - v0.TexC.x;
	float deltaV1 = v2.TexC.y - v0.TexC.y;

	float c = 1.0f / (deltaU0*deltaV1 - deltaV0*deltaU1);

	XMFLOAT3 tangent(c*(deltaV1*e1.x - deltaV0*e2.x),
		c*(deltaV1*e1.y - deltaV0*e2.y),
		c*(deltaV1*e1.z - deltaV0*e2.z));

	v0.TangentU += tangent;
	v1.TangentU += tangent;
	v2.TangentU += tangent;
}

void ModelFileLoader::OrthonormalizeTangents(SkinnedVertexFull &v)
{
	XMVECTOR n = XMLoadFloat3(&v.Normal);
	XMVECTOR t = XMLoadFloat3(&v.TangentU);
	t = XMVector3Normalize(t - n * XMVector3Dot(n, t));
	XMStoreFloat3(&v.TangentU, t);
}