#ifndef XFILELOADER_H
#define XFILELOADER_H

#include "ModelFileLoader.h"

namespace Makina
{
	class XFileLoader : public ModelFileLoader
	{
	public:
		__declspec(dllexport) XFileLoader(D3DAppValues *values, const wchar_t *texDir);
		__declspec(dllexport) ~XFileLoader();

		__declspec(dllexport) void LoadModel(const wchar_t *path);

	private:	
		struct MaterialWithMaps;
		struct ProcessMeshResult;

		void Process_Frame(std::string &fLine, std::ifstream &input, std::vector<ProcessMeshResult *> *meshes, MeshAnimationData *animData, XMFLOAT4X4 *frameParentWorld, const std::string &parentName);
		void Process_FrameTransformMatrix(std::string &fLine, std::ifstream &input, XMFLOAT4X4 *ftm, XMFLOAT4X4 *ftmInverse);
		void Process_Mesh(std::string &fLine, std::ifstream &input, ProcessMeshResult *processMeshResult);
		void Process_MeshNormals(std::string &fLine, std::ifstream &input, ProcessMeshResult *processMeshResult);
		void Process_MeshMaterialList(std::string &fLine, std::ifstream &input, ProcessMeshResult *processMeshResult);
		void Process_Material(std::string &fLine, std::ifstream &input, MaterialWithMaps *processMaterialResult);
		void Process_TextureFilename(std::string &fLine, std::ifstream &input, std::string *fileName);
		void Process_MeshTextureCoords(std::string &fLine, std::ifstream &input, ProcessMeshResult *processMeshResult);
		void Process_XSkinMeshHeader(std::string &fLine, std::ifstream &input, ProcessMeshResult *processMeshResult);
		void Process_SkinWeights(std::string &fLine, std::ifstream &input, ProcessMeshResult *processMeshResult);

		void Process_AnimTicksPerSecond(std::string &fLine, std::ifstream &input, MeshAnimationData *animData);
		void Process_AnimationSet(std::string &fLine, std::ifstream &input, MeshAnimationData *animData);
		void Process_Animation(std::string &fLine, std::ifstream &input, MeshAnimationData *animData, AnimationSet *animSet);
		void Process_AnimationKey(std::string &fLine, std::ifstream &input, AnimationSet *animSet, Animation *anim);

		void LoadMeshSimulationData(std::wstring &path, MeshSimulationData *simData, MeshAnimationData *anmData);
		void SaveMeshSimulationData(std::wstring &path, MeshSimulationData *simData, MeshAnimationData *anmData);

		void AddSubsetToModel(SkinnedModel *model, ProcessMeshResult *processMeshResult);
		void UpdateSubsetBoneIndices(ProcessMeshResult *processMeshResult, MeshAnimationData *animData);

		ID3D11ShaderResourceView *LoadSRVFromFile(const wchar_t *texPath);

		// Table of all the materials from the mesh that is currently being loaded.
		std::map<std::string, MaterialWithMaps> mMaterials;

		// Where to look for textures.
		std::wstring mTexDir;
	};
}

#endif