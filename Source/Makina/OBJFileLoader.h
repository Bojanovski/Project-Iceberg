
#ifndef OBJFILELOADER_H
#define OBJFILELOADER_H

#include "ModelFileLoader.h"

namespace Makina
{
	struct OBJFile
	{
		std::wstring mPath;
		std::wstring mMatPath;
		std::wstring mDirectory;
		bool mHasMaterials;

		__declspec(dllexport) OBJFile(const wchar_t *path);
		__declspec(dllexport) ~OBJFile();
	};

	class OBJFileLoader : public ModelFileLoader
	{
	public:
		__declspec(dllexport) OBJFileLoader(D3DAppValues *values, const wchar_t *texDir);
		__declspec(dllexport) ~OBJFileLoader();

		__declspec(dllexport) void LoadModel(const wchar_t *path);

	private:
		__declspec(dllexport) void LoadSubsets(OBJFile &OBJFile, std::vector<Makina::BasicMeshData> *subsets, std::vector<std::string> *matNames);
		__declspec(dllexport) void LoadMaterials(OBJFile &OBJFile, std::vector<Makina::Material> *materials, std::vector<std::string> *matNames, std::vector<std::wstring> *dMaps, std::vector<std::wstring> *bMaps);
		__declspec(dllexport) ID3D11ShaderResourceView *LoadSRVFromFile(const wchar_t *texPath);

		// Where to look for textures.
		std::wstring mTexDir;
	};
}

#endif
