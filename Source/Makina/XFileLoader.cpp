
#include "XFileLoader.h"
#include <sstream>
#include "Exceptions.h"
#include "MathHelper.h"

using namespace Makina;
using namespace std;
using namespace MathHelper;

struct XFileLoader::MaterialWithMaps
{
	Material mMat;
	string mCMapName, mNMapName;
};

struct XFileLoader::ProcessMeshResult
{
	string mMeshName;
	vector<XMFLOAT3> mVertices;
	vector<UINT> mVIndices;
	vector<XMFLOAT3> mNormals;
	vector<UINT> mNIndices;
	vector<XMFLOAT2> mUVCoords;
	MaterialWithMaps mMat;

	// frame transform matrix
	XMFLOAT4X4 mFtm;
	XMFLOAT4X4 mFtmInv;

	// skinning part
	UINT mNumBones;
	vector<string> mBoneNames;
	vector<VertexWeights> mVW;
};

XFileLoader::XFileLoader(D3DAppValues *values, const wchar_t *texDir)
: ModelFileLoader(values),
mTexDir(texDir)
{

}

XFileLoader::~XFileLoader()
{

}

inline bool FileExists(const wstring& path) {
	ifstream f(path.c_str());
	if (f.good()) {
		f.close();
		return true;
	}
	else {
		f.close();
		return false;
	}
}

void XFileLoader::LoadModel(const wchar_t *path)
{
	ifstream input(path, ios_base::in);
	if (!input)
	{
		input.close();
		throw FileNotFound(path);
	}

	string ignore, versionS, accuracyS;
	input >> ignore >> versionS >> accuracyS;
	if (versionS.find("txt") == string::npos)
	{
		input.close();
		throw FileCorrupt(L"File must be in textual format.");
	}
	int version = stoi(versionS);
	int accuracy = stoi(accuracyS);

	SkinnedModel *sMdl = new SkinnedModel(mValues);
	vector<ProcessMeshResult *> meshes;
	MeshAnimationData *anmData = sMdl->GetMeshAnimationData();
	XMFLOAT4X4 ftm;
	XMStoreFloat4x4(&ftm, XMMatrixIdentity());
	
	while (input.peek() != EOF)
	{
		string line, firstWord;
		getline(input, line);
		stringstream lineS(line);
		lineS >> firstWord;
		if (firstWord.compare("Material") == 0)
		{
			MaterialWithMaps temp;
			string matName;
			lineS >> matName;
			Process_Material(line, input, &temp);
			mMaterials[matName] = temp;
		}
		if (firstWord.compare("Frame") == 0)
			Process_Frame(line, input, &meshes, anmData, &ftm, "");
		if (firstWord.compare("AnimTicksPerSecond") == 0)
			Process_AnimTicksPerSecond(line, input, anmData);
		if (firstWord.compare("AnimationSet") == 0)
			Process_AnimationSet(line, input, anmData);
	}
	input.close();

	// animation data stuff
	anmData->CalculateBoneHierarchy();

	// update weights and add all the subsets
	for (UINT i = 0; i < meshes.size(); ++i)
	{
		if (meshes[i]->mVertices.size() > 0) // ignore all bones
		{
			UpdateSubsetBoneIndices(meshes[i], anmData);
			AddSubsetToModel(sMdl, meshes[i]);
		}
		delete meshes[i];
	}
	sMdl->UpdateChanges();

	// load or create simulation data
	MeshSimulationData *simData = sMdl->GetMeshSimulationData();
	wstring wpath(path);
	wpath = wpath.substr(0, wpath.find_last_of('.'));
	wstring rgdDataPath = wpath + L".rgd";
	if (FileExists(rgdDataPath))
	{
		LoadMeshSimulationData(rgdDataPath, simData, anmData);
		simData->CalculateAllBoneSpaceMatrices();
	} 
	else // there is no file, generate one
	{
		simData->GenerateData();
		SaveMeshSimulationData(rgdDataPath, simData, anmData);
	}

	mLoaded[path] = sMdl; // Finally add the model
}

void XFileLoader::Process_Frame(string &fLine, ifstream &input, vector<ProcessMeshResult *> *meshes, MeshAnimationData *animData, XMFLOAT4X4 *frameParentWorld, const string &parentName)
{
	stringstream fLineS(fLine);
	string ignore, frameName;
	fLineS >> ignore >> frameName >> ignore;

	meshes->push_back(new ProcessMeshResult());
	ProcessMeshResult *processMeshResult = (*meshes)[meshes->size() - 1];
	processMeshResult->mFtm = *frameParentWorld;

	while (input.peek() != EOF)
	{
		string line, firstWord;
		getline(input, line);
		stringstream lineS(line);
		lineS >> firstWord;
		if (firstWord.compare("FrameTransformMatrix") == 0)
			Process_FrameTransformMatrix(line, input, &processMeshResult->mFtm, &processMeshResult->mFtmInv);
		if (firstWord.compare("Mesh") == 0)
			Process_Mesh(line, input, processMeshResult);
		if (firstWord.compare("Frame") == 0)
			Process_Frame(line, input, meshes, animData, &processMeshResult->mFtm, frameName);
		if (firstWord.compare("}") == 0)
		{
			if (processMeshResult->mVIndices.size() == 0) // it must be a bone
				animData->AddBone(frameName, parentName, processMeshResult->mFtmInv);
			return;
		}
	}

	// if the program reaches this point, something is wrong with the file
	input.close();
	throw FileCorrupt(L"File is invalid.");
}

void XFileLoader::Process_FrameTransformMatrix(string &fLine, ifstream &input, XMFLOAT4X4 *ftm, XMFLOAT4X4 *ftmInverse)
{
	XMFLOAT4X4 newFrame;
	string line;
	getline(input, line);
	stringstream lineS(line);
	char comma;
	lineS >> newFrame._11 >> comma >> newFrame._12 >> comma >> newFrame._13 >> comma >> newFrame._14 >> comma >>
		newFrame._21 >> comma >> newFrame._22 >> comma >> newFrame._23 >> comma >> newFrame._24 >> comma >>
		newFrame._31 >> comma >> newFrame._32 >> comma >> newFrame._33 >> comma >> newFrame._34 >> comma >>
		newFrame._41 >> comma >> newFrame._42 >> comma >> newFrame._43 >> comma >> newFrame._44;

	XMMATRIX ftmM = XMLoadFloat4x4(&newFrame) * XMLoadFloat4x4(ftm);
	XMVECTOR det;
	XMMATRIX ftmInverseM = XMMatrixInverse(&det, ftmM);

	XMStoreFloat4x4(ftm, ftmM);
	XMStoreFloat4x4(ftmInverse, ftmInverseM);
	
	// read until the '}' character
	string firstWord;
	do
	{
		string line;
		getline(input, line);
		stringstream lineS(line);
		lineS >> firstWord;
	} while (firstWord.compare("}") != 0);
}

void XFileLoader::Process_Mesh(string &fLine, ifstream &input, ProcessMeshResult *processMeshResult)
{
	string line, ignore;
	stringstream lineS(fLine);
	lineS >> ignore >> processMeshResult->mMeshName; // get mesh name

	// Get vertices
	getline(input, line);
	lineS = stringstream(line);
	int verticesNum;
	lineS >> verticesNum; // get number of vertices
	processMeshResult->mVertices.resize(verticesNum);
	for (int i = 0; i < verticesNum; ++i)
	{
		getline(input, line);
		lineS = stringstream(line);
		char semicolon;
		lineS >> processMeshResult->mVertices[i].x >> semicolon >>
			processMeshResult->mVertices[i].y >> semicolon >>
			processMeshResult->mVertices[i].z;
	}

	// Get indices
	getline(input, line);
	lineS = stringstream(line);
	int facesNum;
	lineS >> facesNum; // get number of vertices
	processMeshResult->mVIndices.resize(facesNum * 3);
	for (int i = 0; i < facesNum; ++i)
	{
		getline(input, line);
		lineS = stringstream(line);
		char separationChar, faceVertexNumber;
		lineS >> faceVertexNumber >> separationChar >>
			processMeshResult->mVIndices[i * 3] >> separationChar >>
			processMeshResult->mVIndices[i * 3 + 1] >> separationChar >>
			processMeshResult->mVIndices[i * 3 + 2];
		if (faceVertexNumber != '3')
		{
			input.close();
			throw FileCorrupt(L"Only triangular faces are supported.");
		}
	}

	// Get the rest
	while (input.peek() != EOF)
	{
		string line, firstWord;
		getline(input, line);
		stringstream lineS(line);
		lineS >> firstWord;
		if (firstWord.compare("MeshNormals") == 0)
			Process_MeshNormals(line, input, processMeshResult);
		if (firstWord.compare("MeshMaterialList") == 0)
			Process_MeshMaterialList(line, input, processMeshResult);
		if (firstWord.compare("MeshTextureCoords") == 0)
			Process_MeshTextureCoords(line, input, processMeshResult);
		if (firstWord.compare("XSkinMeshHeader") == 0)
			Process_XSkinMeshHeader(line, input, processMeshResult);
		if (firstWord.compare("SkinWeights") == 0)
			Process_SkinWeights(line, input, processMeshResult);
		if (firstWord.compare("}") == 0)
			return;
	}

	// if the program reaches this point, something is wrong with the file
	input.close();
	throw FileCorrupt(L"File is invalid.");
}

void XFileLoader::Process_MeshNormals(string &fLine, ifstream &input, ProcessMeshResult *processMeshResult)
{
	string line, ignore;
	stringstream lineS;

	// Get normals
	getline(input, line);
	lineS = stringstream(line);
	int normalsNum;
	lineS >> normalsNum; // get number of normals
	processMeshResult->mNormals.resize(normalsNum);
	for (int i = 0; i < normalsNum; ++i)
	{
		getline(input, line);
		lineS = stringstream(line);
		char semicolon;
		lineS >> processMeshResult->mNormals[i].x >> semicolon >>
			processMeshResult->mNormals[i].y >> semicolon >>
			processMeshResult->mNormals[i].z;
	}

	// Get normal indices
	getline(input, line);
	lineS = stringstream(line);
	int facesNum;
	lineS >> facesNum; // get number of vertices
	processMeshResult->mNIndices.resize(facesNum * 3);
	for (int i = 0; i < facesNum; ++i)
	{
		getline(input, line);
		lineS = stringstream(line);
		char separationChar, faceVertexNumber;
		lineS >> faceVertexNumber >> separationChar >>
			processMeshResult->mNIndices[i * 3] >> separationChar >>
			processMeshResult->mNIndices[i * 3 + 1] >> separationChar >>
			processMeshResult->mNIndices[i * 3 + 2];
		if (faceVertexNumber != '3')
		{
			input.close();
			throw FileCorrupt(L"Only triangular faces are supported.");
		}
	}

	getline(input, line); // } character
}

void XFileLoader::Process_MeshMaterialList(string &fLine, ifstream &input, ProcessMeshResult *processMeshResult)
{
	string line;
	getline(input, line);
	stringstream lineS(line);
	int matNum;
	lineS >> matNum; // get number of materials
	if (matNum != 1)
	{
		input.close();
		throw FileCorrupt(L"XFileLoader currently supports only one material per mesh.");
	}
	getline(input, line);
	lineS = stringstream(line);
	int facesNum;
	lineS >> facesNum; // get number of faces

	// skip all data for now
	for (int i = 0; i < facesNum; ++i)
		getline(input, line);

	// Get the rest
	while (input.peek() != EOF)
	{
		string line, firstWord;
		getline(input, line);
		stringstream lineS(line);
		lineS >> firstWord;
		if (firstWord.compare("{") == 0)
		{
			string matName;
			lineS >> matName;
			processMeshResult->mMat = mMaterials[matName];
		}
		if (firstWord.compare("Material") == 0)
			Process_Material(line, input, &processMeshResult->mMat);
		if (firstWord.compare("}") == 0)
			return;
	}

	// if the program reaches this point, something is wrong with the file
	input.close();
	throw FileCorrupt(L"File is invalid.");
}

void XFileLoader::Process_Material(string &fLine, ifstream &input, MaterialWithMaps *processMaterialResult)
{
	string line;
	getline(input, line);
	stringstream lineS(line);
	char semicolon;
	// load "face color"
	lineS	 >> processMaterialResult->mMat.Diffuse.x >> semicolon >>
				processMaterialResult->mMat.Diffuse.y >> semicolon >>
				processMaterialResult->mMat.Diffuse.z >> semicolon >>
				processMaterialResult->mMat.Diffuse.w;
	// copy it to ambient color
	processMaterialResult->mMat.Ambient = processMaterialResult->mMat.Diffuse;

	getline(input, line);
	lineS = stringstream(line);
	// load specular power
	lineS >> processMaterialResult->mMat.Specular.w;
	if (processMaterialResult->mMat.Specular.w == 0.0f)
		processMaterialResult->mMat.Specular.w = 0.00001f; // it can't be zero because of pow() function in shader

	getline(input, line);
	lineS = stringstream(line);
	// load specular color
	lineS >>	processMaterialResult->mMat.Specular.x >> semicolon >>
				processMaterialResult->mMat.Specular.y >> semicolon >>
				processMaterialResult->mMat.Specular.z;

	getline(input, line);
	lineS = stringstream(line);
	// load "emissive" color, save it in Reflect
	lineS >>	processMaterialResult->mMat.Reflect.x >> semicolon >>
				processMaterialResult->mMat.Reflect.y >> semicolon >>
				processMaterialResult->mMat.Reflect.z;
				processMaterialResult->mMat.Reflect.w = 1.0f;

	// Get the '}' character and TextureFilename if it exists.
	while (input.peek() != EOF)
	{
		string firstWord;
		getline(input, line);
		stringstream lineS(line);
		lineS >> firstWord;
		if (firstWord.compare("TextureFilename") == 0)
		{
			if (processMaterialResult->mCMapName.size() == 0)
				Process_TextureFilename(line, input, &processMaterialResult->mCMapName);
			else
				Process_TextureFilename(line, input, &processMaterialResult->mNMapName);
		}
		if (firstWord.compare("}") == 0)
			return;
	}
}

void XFileLoader::Process_TextureFilename(string &fLine, ifstream &input, string *fileName)
{
	string line;
	getline(input, line);
	stringstream lineS(line);
	// load file name
	lineS >> *fileName;
	// get rid of the " characters and the semicolon in the name
	*fileName = fileName->substr(1, fileName->size() - 3);

	// until the '}' character is read
	string firstWord;
	do
	{
		string line;
		getline(input, line);
		stringstream lineS(line);
		lineS >> firstWord;
	} while (firstWord.compare("}") != 0);
}

void XFileLoader::Process_MeshTextureCoords(string &fLine, ifstream &input, ProcessMeshResult *processMeshResult)
{
	string line;
	getline(input, line);
	stringstream lineS(line);
	int numTC; // Number of texture coordinates.
	lineS >> numTC;
	if (numTC != processMeshResult->mVertices.size())
	{
		input.close();
		throw FileCorrupt(L"File is invalid.");
	}
	processMeshResult->mUVCoords.resize(numTC);
	for (int i = 0; i < numTC; ++i)
	{
		getline(input, line);
		lineS = stringstream(line);
		char semicolon;
		lineS >> processMeshResult->mUVCoords[i].x >> semicolon >> processMeshResult->mUVCoords[i].y;
	}

	// until the '}' character is read
	string firstWord;
	do
	{
		string line;
		getline(input, line);
		stringstream lineS(line);
		lineS >> firstWord;
	} while (firstWord.compare("}") != 0);
}

void XFileLoader::Process_XSkinMeshHeader(std::string &fLine, std::ifstream &input, ProcessMeshResult *processMeshResult)
{	
	string line;
	getline(input, line);
	stringstream lineS(line);
	int numMaxTV; // Maximum number of transforms that affect a vertex in the mesh.
	lineS >> numMaxTV;
	if (numMaxTV > 4)
	{
		input.close();
		throw FileCorrupt(L"Maximum number of transforms that affect a vertex in the mesh is more than four.");
	}

	getline(input, line);
	lineS =stringstream(line);
	int numMaxTF; // Maximum number of unique transforms that affect the three vertices of any face. (not important)
	lineS >> numMaxTF;

	getline(input, line);
	lineS = stringstream(line);
	int numBones; // Number of bones that affect vertices in this mesh.
	lineS >> numBones;
	processMeshResult->mNumBones = numBones;
	processMeshResult->mVW.resize(processMeshResult->mVertices.size());

	getline(input, line); // get } character
}

void XFileLoader::Process_SkinWeights(std::string &fLine, std::ifstream &input, ProcessMeshResult *processMeshResult)
{
	string line;
	getline(input, line);
	stringstream lineS(line);
	string boneName;			// The name of the bone whose influence is being defined is transformNodeName,
	lineS >> boneName;			// and nWeights is the number of vertices affected by this bone.
	// get rid of the " characters and the semicolon in the bone name
	boneName = boneName.substr(1, boneName.size() - 3);
	processMeshResult->mBoneNames.push_back(boneName);
	int boneIndex = processMeshResult->mBoneNames.size() - 1;

	getline(input, line);
	lineS = stringstream(line);
	int numVertices;			// The vertices influenced by this bone are contained in vertexIndices,
	lineS >> numVertices;		// and the weights for each of the vertices influenced by this bone are contained in weights.

	// get the order in which vertices are updated
	vector<UINT> vi(numVertices);
	for (int i = 0; i < numVertices; ++i)
	{
		getline(input, line);
		lineS = stringstream(line);
		lineS >> vi[i];
	}

	// update vertices
	for (int i = 0; i < numVertices; ++i)
	{
		getline(input, line);
		lineS = stringstream(line);
		float boneWeight;
		lineS >> boneWeight;
		if (processMeshResult->mVW[vi[i]].bW0 == 0.0f)
		{
			processMeshResult->mVW[vi[i]].bW0 = boneWeight;
			processMeshResult->mVW[vi[i]].bI0 = boneIndex;
		}
		else if (processMeshResult->mVW[vi[i]].bW1 == 0.0f)
		{
			processMeshResult->mVW[vi[i]].bW1 = boneWeight;
			processMeshResult->mVW[vi[i]].bI1 = boneIndex;
		}
		else if (processMeshResult->mVW[vi[i]].bW2 == 0.0f)
		{
			processMeshResult->mVW[vi[i]].bW2 = boneWeight;
			processMeshResult->mVW[vi[i]].bI2 = boneIndex;
		}
		else if (processMeshResult->mVW[vi[i]].bW3 == 0.0f)
		{
			processMeshResult->mVW[vi[i]].bW3 = boneWeight;
			processMeshResult->mVW[vi[i]].bI3 = boneIndex;
		}
		else throw;
	}

	// until '}'
	string firstWord;
	do
	{
		string line;
		getline(input, line);
		stringstream lineS(line);
		lineS >> firstWord;
	} while (firstWord.compare("}") != 0);
}

void XFileLoader::Process_AnimTicksPerSecond(string &fLine, ifstream &input, MeshAnimationData *animData)
{
	string line;
	getline(input, line);
	stringstream lineS(line);
	lineS >> animData->mAnimTicksPerSecond;

	// until '}'
	string firstWord;
	do
	{
		string line;
		getline(input, line);
		stringstream lineS(line);
		lineS >> firstWord;
	} while (firstWord.compare("}") != 0);
}

void XFileLoader::Process_AnimationSet(string &fLine, ifstream &input, MeshAnimationData *animData)
{
	stringstream fLineS(fLine);
	string ignore, animSetName;
	fLineS >> ignore >> animSetName >> ignore;

	animData->mAnimationSets.push_back(AnimationSet());
	AnimationSet *animSet = &animData->mAnimationSets[animData->mAnimationSets.size() - 1];
	animSet->mName = animSetName;
	animSet->mTotalTime = 0;
	while (input.peek() != EOF)
	{
		string line, firstWord;
		getline(input, line);
		stringstream lineS(line);
		lineS >> firstWord;
		if (firstWord.compare("Animation") == 0)
			Process_Animation(line, input, animData, animSet);
		if (firstWord.compare("}") == 0)
			return;
	}

	// if the program reaches this point, something is wrong with the file
	input.close();
	throw FileCorrupt(L"File is invalid.");
}

void XFileLoader::Process_Animation(string &fLine, ifstream &input, MeshAnimationData *animData, AnimationSet *animSet)
{
	stringstream fLineS(fLine);

	animSet->mAnim.push_back(Animation());
	Animation *anim = &animSet->mAnim[animSet->mAnim.size() - 1];

	// get bone name
	string boneName;
	while (input.peek() != EOF)
	{
		string line, openCuBrack, closeCuBrack;
		getline(input, line);
		stringstream lineS(line);
		lineS >> openCuBrack;
		if (openCuBrack.compare("{") == 0)
		{
			lineS >> boneName >> closeCuBrack;
			break;
		}
	}
	anim->mBone = animData->GetBoneIndex(boneName);

	while (input.peek() != EOF)
	{
		string line, firstWord;
		getline(input, line);
		stringstream lineS(line);
		lineS >> firstWord;
		if (firstWord.compare("AnimationKey") == 0)
			Process_AnimationKey(line, input, animSet, anim);
		if (firstWord.compare("}") == 0)
			return;
	}

	// if the program reaches this point, something is wrong with the file
	input.close();
	throw FileCorrupt(L"File is invalid.");
}

void XFileLoader::Process_AnimationKey(string &fLine, ifstream &input, AnimationSet *animSet, Animation *anim)
{
	string line, ignore;
	getline(input, line);
	stringstream lineS(line);
	// Specifies whether the keys are rotation, scale, position, or matrix keys (using the integers 0, 1, 2, or 3, respectively).
	int keyType;
	lineS >> keyType;
	if (keyType != 4)
	{
		input.close();
		throw FileCorrupt(L"Invalid type of animation key.");
	}

	// Get keys
	getline(input, line);
	lineS = stringstream(line);
	int keysNum;
	lineS >> keysNum; // get number of keys
	anim->mAnimKeys.resize(keysNum);
	for (int i = 0; i < keysNum; ++i)
	{
		getline(input, line);
		lineS = stringstream(line);
		char semicolon, comma;
		int ignore;
		lineS >> anim->mAnimKeys[i].mT >> semicolon >> ignore >> semicolon >>
			anim->mAnimKeys[i].mW._11 >> comma >> anim->mAnimKeys[i].mW._12 >> comma >> anim->mAnimKeys[i].mW._13 >> comma >> anim->mAnimKeys[i].mW._14 >> comma >>
			anim->mAnimKeys[i].mW._21 >> comma >> anim->mAnimKeys[i].mW._22 >> comma >> anim->mAnimKeys[i].mW._23 >> comma >> anim->mAnimKeys[i].mW._24 >> comma >>
			anim->mAnimKeys[i].mW._31 >> comma >> anim->mAnimKeys[i].mW._32 >> comma >> anim->mAnimKeys[i].mW._33 >> comma >> anim->mAnimKeys[i].mW._34 >> comma >>
			anim->mAnimKeys[i].mW._41 >> comma >> anim->mAnimKeys[i].mW._42 >> comma >> anim->mAnimKeys[i].mW._43 >> comma >> anim->mAnimKeys[i].mW._44;
		
		if (anim->mAnimKeys[i].mT > animSet->mTotalTime)
			animSet->mTotalTime = anim->mAnimKeys[i].mT;
	}

	// until the '}' character is read
	string firstWord;
	do
	{
		string line;
		getline(input, line);
		stringstream lineS(line);
		lineS >> firstWord;
	} while (firstWord.compare("}") != 0);
}

void XFileLoader::LoadMeshSimulationData(wstring &path, MeshSimulationData *simData, MeshAnimationData *anmData)
{
	ifstream input(path, ios_base::in);

	// bones
	string line, boneName;
	XMFLOAT3 pos, dir;
	getline(input, line);
	stringstream lineS(line);
	int bodiesCount;
	lineS >> bodiesCount;
	simData->mBoneBodies.reserve(bodiesCount);
	getline(input, line); // comment
	while (input.peek() != EOF && bodiesCount > 0)
	{
		string line;
		getline(input, line);
		stringstream lineS(line);
		float length, thicknessX, thicknessY;
		lineS >> boneName >> pos.x >> pos.y >> pos.z >> length >> thicknessX >> thicknessY >> dir.x >> dir.y >> dir.z;
		int index = anmData->GetBoneIndex(boneName);
		BoneRigidBody brb;
		brb.mIndex = index;
		brb.mPos = pos;
		brb.mLength = length;
		brb.mThickness.x = thicknessX;
		brb.mThickness.y = thicknessY;
		brb.mDir = dir;
		simData->mBoneBodies.push_back(brb);
		--bodiesCount;
	}

	// joints
	getline(input, line);
	lineS = stringstream(line);
	int jointsCount;
	lineS >> jointsCount;
	simData->mBoneJoints.reserve(jointsCount);
	getline(input, line); // comment
	while (input.peek() != EOF && jointsCount > 0)
	{
		string line;
		getline(input, line);
		stringstream lineS(line);
		BoneJoint bj;
		lineS >> bj.mIParent >> bj.mIChild >> bj.mAnchorPos.x >> bj.mAnchorPos.y >> bj.mAnchorPos.z >> bj.mType >> bj.mP1 >> bj.mP2;
		simData->mBoneJoints.push_back(bj);
		--jointsCount;
	}

	input.close();
}

void XFileLoader::SaveMeshSimulationData(wstring &path, MeshSimulationData *simData, MeshAnimationData *anmData)
{
	ofstream output(path, ios_base::out);

	// bones	
	string line = to_string(simData->mBoneBodies.size()) + "\n";
	output.write(&line[0], line.size());
	line = "-- BONES: pos(x, y, z), length, thickness(x, y), dir(x, y, z)\n";
	output.write(&line[0], line.size());
	for (UINT i = 0; i < simData->mBoneBodies.size(); ++i)
	{
		line = anmData->mBones[simData->mBoneBodies[i].mIndex].mName + " ";
		line += to_string(simData->mBoneBodies[i].mPos.x) + " ";
		line += to_string(simData->mBoneBodies[i].mPos.y) + " ";
		line += to_string(simData->mBoneBodies[i].mPos.z) + " ";
		line += to_string(simData->mBoneBodies[i].mLength) + " ";
		line += to_string(simData->mBoneBodies[i].mThickness.x) + " ";
		line += to_string(simData->mBoneBodies[i].mThickness.y) + " ";
		line += to_string(simData->mBoneBodies[i].mDir.x) + " ";
		line += to_string(simData->mBoneBodies[i].mDir.y) + " ";
		line += to_string(simData->mBoneBodies[i].mDir.z);
		line += "\n";
		output.write(&line[0], line.size());
	}

	// joints
	line = to_string(simData->mBoneJoints.size()) + "\n";
	output.write(&line[0], line.size());
	line = "-- JOINTS: parentIndex, childIndex, anchorPos(x, y, z), type, for universal: rotationalFreedom, bendFreedom; for hinge: forwardAngle, backwardAngle\n";
	output.write(&line[0], line.size());
	for (UINT i = 0; i < simData->mBoneJoints.size(); ++i)
	{
		line = to_string(simData->mBoneJoints[i].mIParent) + " ";
		line += to_string(simData->mBoneJoints[i].mIChild) + " ";
		line += to_string(simData->mBoneJoints[i].mAnchorPos.x) + " ";
		line += to_string(simData->mBoneJoints[i].mAnchorPos.y) + " ";
		line += to_string(simData->mBoneJoints[i].mAnchorPos.z) + " ";
		line += string(1, simData->mBoneJoints[i].mType) + " ";
		line += to_string(simData->mBoneJoints[i].mP1) + " ";
		line += to_string(simData->mBoneJoints[i].mP2);
		line += "\n";
		output.write(&line[0], line.size());
	}

	output.close();
}

inline int ExistsInVector_PosNormalUV_test(vector<VertexFull> &vec, VertexFull &v)
{
	for (UINT i = 0; i < vec.size(); ++i)
	{
		if (vec[i].Position == v.Position && vec[i].Normal == v.Normal && vec[i].TexC == v.TexC)
			return i;
	}
	return -1;
}

inline int ExistsInVector_PosNormalUVBoneWeight_test(vector<SkinnedVertexFull> &vec, SkinnedVertexFull &v)
{
	for (UINT i = 0; i < vec.size(); ++i)
	{
		if (vec[i].Position == v.Position && vec[i].Normal == v.Normal && vec[i].TexC == v.TexC &&
			vec[i].BoneIndices[0] == v.BoneIndices[0] &&
			vec[i].BoneIndices[1] == v.BoneIndices[1] && 
			vec[i].BoneIndices[2] == v.BoneIndices[2] && 
			vec[i].BoneIndices[3] == v.BoneIndices[3] &&
			vec[i].Weights.x == v.Weights.x &&
			vec[i].Weights.y == v.Weights.y &&
			vec[i].Weights.z == v.Weights.z)
			return i;
	}
	return -1;
}

void XFileLoader::AddSubsetToModel(SkinnedModel *model, ProcessMeshResult *processMeshResult)
{
	int facesNum = processMeshResult->mVIndices.size() / 3;
	if (facesNum != (processMeshResult->mNIndices.size() / 3))
		throw FileCorrupt(L"Normal faces number does not match the vertex faces number. (XFileLoader::AddSubsetToModel)");

	if (processMeshResult->mNumBones == 0)
		throw FileCorrupt(L"Loading the file without skinning data. (XFileLoader::AddSubsetToModel)");

	SkinnedMeshData meshData;
	// for each face
	for (int f = 0; f < facesNum; ++f)
	{
		// vertex indices (these also work for UV coords)
		int vI0 = processMeshResult->mVIndices[f * 3];
		int vI1 = processMeshResult->mVIndices[f * 3 + 1];
		int vI2 = processMeshResult->mVIndices[f * 3 + 2];

		// normal indices
		int nI0 = processMeshResult->mNIndices[f * 3];
		int nI1 = processMeshResult->mNIndices[f * 3 + 1];
		int nI2 = processMeshResult->mNIndices[f * 3 + 2];

		SkinnedVertexFull v0, v1, v2;
		v0.Position = processMeshResult->mVertices[vI0];
		v1.Position = processMeshResult->mVertices[vI1];
		v2.Position = processMeshResult->mVertices[vI2];
		v0.TexC = processMeshResult->mUVCoords[vI0];
		v1.TexC = processMeshResult->mUVCoords[vI1];
		v2.TexC = processMeshResult->mUVCoords[vI2];
		v0.Normal = processMeshResult->mNormals[nI0];
		v1.Normal = processMeshResult->mNormals[nI1];
		v2.Normal = processMeshResult->mNormals[nI2];
		v0.TangentU = XMFLOAT3(0.0f, 0.0f, 0.0f);
		v1.TangentU = XMFLOAT3(0.0f, 0.0f, 0.0f);
		v2.TangentU = XMFLOAT3(0.0f, 0.0f, 0.0f);
		v0.SetWeights(processMeshResult->mVW[vI0]);
		v1.SetWeights(processMeshResult->mVW[vI1]);
		v2.SetWeights(processMeshResult->mVW[vI2]);

		// first vertex
		int mdIndex0;
		if ((mdIndex0 = ExistsInVector_PosNormalUVBoneWeight_test(meshData.Vertices, v0)) == -1)
		{
			meshData.Vertices.push_back(v0);
			mdIndex0 = meshData.Vertices.size() - 1;
		}
		meshData.Indices.push_back(mdIndex0);

		// second vertex
		int mdIndex1;
		if ((mdIndex1 = ExistsInVector_PosNormalUVBoneWeight_test(meshData.Vertices, v1)) == -1)
		{
			meshData.Vertices.push_back(v1);
			mdIndex1 = meshData.Vertices.size() - 1;
		}
		meshData.Indices.push_back(mdIndex1);

		// third vertex
		int mdIndex2;
		if ((mdIndex2 = ExistsInVector_PosNormalUVBoneWeight_test(meshData.Vertices, v2)) == -1)
		{
			meshData.Vertices.push_back(v2);
			mdIndex2 = meshData.Vertices.size() - 1;
		}
		meshData.Indices.push_back(mdIndex2);

		CalculateTangents(meshData.Vertices[mdIndex0], meshData.Vertices[mdIndex1], meshData.Vertices[mdIndex2]);
	}
	// transform to world space
	XMMATRIX frameToWorld = XMLoadFloat4x4(&processMeshResult->mFtm);
	XMMATRIX frameToWorldInvTranspose = MathHelper::InverseTranspose(frameToWorld);
	for (auto &vert : meshData.Vertices)
	{
		XMVECTOR p = XMVector3TransformCoord(XMLoadFloat3(&vert.Position), frameToWorld);
		XMVECTOR n = XMVector3TransformNormal(XMLoadFloat3(&vert.Normal), frameToWorldInvTranspose);
		XMVECTOR t = XMVector3TransformNormal(XMLoadFloat3(&vert.TangentU), frameToWorldInvTranspose);
		XMStoreFloat3(&vert.Position, p);
		XMStoreFloat3(&vert.Normal, n);
		XMStoreFloat3(&vert.TangentU, t);
	}
	// orthonormalize
	for (auto &vert : meshData.Vertices) OrthonormalizeTangents(vert);
	// add
	wstring cMap(processMeshResult->mMat.mCMapName.begin(), processMeshResult->mMat.mCMapName.end());
	wstring nMap(processMeshResult->mMat.mNMapName.begin(), processMeshResult->mMat.mNMapName.end());
	ID3D11ShaderResourceView *dMap = (cMap.size() > 0) ? LoadSRVFromFile(&cMap[0]) : 0;
	ID3D11ShaderResourceView *bMap = (nMap.size() > 0) ? LoadSRVFromFile(&nMap[0]) : 0;
	dynamic_cast<SkinnedModel *>(model)->AddSubset(meshData, processMeshResult->mMat.mMat, dMap, bMap);
}

void XFileLoader::UpdateSubsetBoneIndices(ProcessMeshResult *processMeshResult, MeshAnimationData *animData)
{
	// for each vertex weight
	for (UINT j = 0; j < processMeshResult->mVW.size(); ++j)
	{
		processMeshResult->mVW[j].bI0 =
			animData->GetBoneIndex(processMeshResult->mBoneNames[processMeshResult->mVW[j].bI0]);

		processMeshResult->mVW[j].bI1 =
			animData->GetBoneIndex(processMeshResult->mBoneNames[processMeshResult->mVW[j].bI1]);

		processMeshResult->mVW[j].bI2 =
			animData->GetBoneIndex(processMeshResult->mBoneNames[processMeshResult->mVW[j].bI2]);

		processMeshResult->mVW[j].bI3 =
			animData->GetBoneIndex(processMeshResult->mBoneNames[processMeshResult->mVW[j].bI3]);
	}
}

ID3D11ShaderResourceView *XFileLoader::LoadSRVFromFile(const wchar_t *texPath)
{
	wstring fullPath = mTexDir + wstring(L"\\") + wstring(texPath);
	ID3D11ShaderResourceView *srv;
	HRESULT hr;
	hr = D3DX11CreateShaderResourceViewFromFile(mValues->md3dDevice, &fullPath[0], 0, 0, &srv, 0);
	if (FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to load texture ") + fullPath + L"! (XFileLoader::LoadSRVFromFile)");

	return srv;
}
