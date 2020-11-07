
#include "OBJFileLoader.h"
#include <sstream>
#include "Exceptions.h"
#include "MathHelper.h"

using namespace Makina;
using namespace std;
using namespace MathHelper;

int StringToWString(std::wstring &ws, const std::string &s)
{
	std::wstring wsTmp(s.begin(), s.end());

	ws = wsTmp;

	return 0;
}

OBJFile::OBJFile(const wchar_t *path)
: mPath(path)
{
	mDirectory = mPath.substr(0, mPath.find_last_of(L"\\"));

	ifstream input(path, ios_base::in);
	if (!input)
	{
		input.close();
		throw FileNotFound(path);
	}

	wstring fileName;

	while (true)
	{
		string line;
		getline(input, line);


		if (line.find("mtllib ") != string::npos)
		{
			string inS;
			istringstream iss(line);
			iss >> inS >> inS;
			StringToWString(fileName, inS);
			break;
		}

		if (input.peek() == EOF)
		{
			mHasMaterials = false;
			return;
		}
	}

	input.close();

	mMatPath = mDirectory + wstring(L"\\") + fileName;
	mHasMaterials = true;
}

OBJFile::~OBJFile()
{

}

OBJFileLoader::OBJFileLoader(D3DAppValues *values, const wchar_t *texDir)
: ModelFileLoader(values),
mTexDir(texDir)
{

}

OBJFileLoader::~OBJFileLoader()
{
	for (auto modelIt = mLoaded.begin(); modelIt != mLoaded.end(); ++modelIt)
	{ // Delete all loaded models.
		delete modelIt->second;
	}
}

void OBJFileLoader::LoadModel(const wchar_t *path)
{
	if (mLoaded.find(path) == mLoaded.end()) 
	{
		// not found
		OBJFile objFile(path);
		
		BasicModel *bMod = new BasicModel(mValues);
		Model *mdl = bMod;

		vector<BasicMeshData> subsets;
		vector<string> subMatNames;
		vector<Material> materials;
		vector<string> matNames;
		vector<wstring> dMaps, bMaps;

		LoadSubsets(objFile, &subsets, &subMatNames);
		LoadMaterials(objFile, &materials, &matNames, &dMaps, &bMaps);

		int subsetCount = subsets.size();

		for (int i = 0; i < subsetCount; ++i)
		{
			if (find(matNames.begin(), matNames.end(), subMatNames[i]) == matNames.end())
				throw UnexpectedError(L"Error during connection of mesh data to materials!");

			int index = find(matNames.begin(), matNames.end(), subMatNames[i]) - matNames.begin();
			// SRVs loaded here will be released by subsed destructor.
			ID3D11ShaderResourceView *dMap = (dMaps[index].compare(L"NONE")) ? LoadSRVFromFile(&dMaps[index][0]) : 0;
			ID3D11ShaderResourceView *bMap = (bMaps[index].compare(L"NONE")) ? LoadSRVFromFile(&bMaps[index][0]) : 0;

			bMod->AddSubset(subsets[i], materials[index], dMap, bMap);
		}
		bMod->UpdateChanges();

		// Finally add the model
		mLoaded[path] = mdl;
	}
}

UINT AddVerticeAndGetIndex(vector<VertexFull> *vertices, XMFLOAT3 &v, XMFLOAT3 &vn, XMFLOAT3 &vt)
{
	VertexFull vert(v, vn, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2(vt.x, vt.y));

	UINT index = -1;
	for(UINT i = 0; i < vertices->size(); ++i)
		if (((*vertices)[i].Position == vert.Position) &&
			((*vertices)[i].Normal == vert.Normal) && 
			((*vertices)[i].TexC == vert.TexC))
			return i;

	// there is no such vertice
	vertices->push_back(vert);
	return (UINT)(vertices->size() - 1);
}

void OBJFileLoader::LoadSubsets(OBJFile &OBJFile, vector<BasicMeshData> *subsets, vector<string> *matNames)
{
	ifstream input(OBJFile.mPath, ios_base::in);
	if (!input)
	{
		input.close();
		throw FileNotFound(OBJFile.mPath);
	}

	string currentObjName;
	vector<XMFLOAT3> v;		// vertices
	vector<XMFLOAT3> vn;	// normals
	vector<XMFLOAT3> vt;	// texture coords
	BasicMeshData currMesh;
	currMesh.Indices.clear();
	currMesh.Vertices.clear();

	while(input.peek() != EOF)
	{
		string line;
		getline(input, line);


		if (line.find("# object") != string::npos)
		{
			string ignore;
			istringstream iss(line);
			iss >> ignore >> ignore >> currentObjName;

			// first save and then clear currMesh for new subset
			if (!currMesh.Indices.empty())
			{
				for (auto &sub : currMesh.Vertices)
					OrthonormalizeTangents(sub);

				subsets->push_back(currMesh);
			}

			currMesh.Indices.clear();
			currMesh.Vertices.clear();

			continue;
		}

		if (line.find("v ") != string::npos)
		{
			string ignore;
			float x, y, z;
			istringstream iss(line);
			iss >> ignore >> x >> y >> z;
			v.push_back(XMFLOAT3(x, y, z));
			continue;
		}

		if (line.find("vn ") != string::npos)
		{
			string ignore;
			float x, y, z;
			istringstream iss(line);
			iss >> ignore >> x >> y >> z;
			vn.push_back(XMFLOAT3(x, y, z));
			continue;
		}

		if (line.find("vt ") != string::npos)
		{
			string ignore;
			float x, y, z;
			istringstream iss(line);
			iss >> ignore >> x >> y >> z;
			vt.push_back(XMFLOAT3(x, y, z));
			continue;
		}

		if (line.find("usemtl ") != string::npos)
		{
			string matName;
			istringstream iss(line);
			iss >> matName >> matName;
			matNames->push_back(matName);
			continue;
		}

		if (line.find("f ") != string::npos)
		{
			string ignore;
			char ignoreC;
			UINT vertex[4], texCoord[4], normal[4];
			vertex[3] = texCoord[3] = normal[3] = 0;
			istringstream iss(line);
			iss >> ignore >>
				vertex[0] >> ignoreC >> texCoord[0] >> ignoreC >> normal[0] >>
				vertex[1] >> ignoreC >> texCoord[1] >> ignoreC >> normal[1] >>
				vertex[2] >> ignoreC >> texCoord[2] >> ignoreC >> normal[2] >>
				vertex[3] >> ignoreC >> texCoord[3] >> ignoreC >> normal[3];
			
			int i0 = AddVerticeAndGetIndex(&currMesh.Vertices, v[vertex[0]-1], vn[normal[0]-1], vt[texCoord[0]-1]);
			int i1 = AddVerticeAndGetIndex(&currMesh.Vertices, v[vertex[1]-1], vn[normal[1]-1], vt[texCoord[1]-1]);
			int i2 = AddVerticeAndGetIndex(&currMesh.Vertices, v[vertex[2]-1], vn[normal[2]-1], vt[texCoord[2]-1]);

			currMesh.Indices.push_back(i0);
			currMesh.Indices.push_back(i1);
			currMesh.Indices.push_back(i2);

			CalculateTangents(currMesh.Vertices[i0], currMesh.Vertices[i1], currMesh.Vertices[i2]);

			if (vertex[3] != 0 && texCoord[3] != 0 && normal[3] != 0)
			{ // proces one more vertex because this face is a quad
				int i3 = AddVerticeAndGetIndex(&currMesh.Vertices, v[vertex[3] - 1], vn[normal[3] - 1], vt[texCoord[3] - 1]);

				currMesh.Indices.push_back(i0);
				currMesh.Indices.push_back(i2);
				currMesh.Indices.push_back(i3);

				CalculateTangents(currMesh.Vertices[i0], currMesh.Vertices[i2], currMesh.Vertices[i3]);
			}

			continue;
		}
	}

	// save the last one
	if (!currMesh.Indices.empty())
	{
		for (auto &vert : currMesh.Vertices)
			OrthonormalizeTangents(vert);

		subsets->push_back(currMesh);
	}

	input.close();

#if defined(DEBUG) || defined(_DEBUG)  
	if (subsets->size() != matNames->size())
	{
		wstring msg = wstring(L"WARNING: The number of subsets and material names is not equal, something is wrong with ") + OBJFile.mPath;
		OutputDebugString(&msg[0]);
	}
#endif
}

void OBJFileLoader::LoadMaterials(OBJFile &OBJFile, vector<Material> *materials, vector<string> *matNames, std::vector<std::wstring> *dMaps, std::vector<std::wstring> *bMaps)
{	
	if (!OBJFile.mHasMaterials)
		throw UnexpectedError(OBJFile.mPath + L" does no have .mtl file.");


	ifstream input(OBJFile.mMatPath, ios_base::in);
	if (!input)
	{
		input.close();
		throw FileNotFound(OBJFile.mMatPath);
	}

	string currentMatName;
	wstring currDMap, currBMap;
	Material *currMat = 0;

	while(input.peek() != EOF)
	{
		string line;
		getline(input, line);


		if (line.find("newmtl ") != string::npos)
		{
			string ignore;
			istringstream iss(line);
			iss >> ignore >> currentMatName;
			matNames->push_back(currentMatName);

			// first save and then clear currMat for new Material
			if (currMat != 0)
			{
				currMat->Reflect = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
				materials->push_back(*currMat);
				dMaps->push_back(currDMap);
				bMaps->push_back(currBMap);
				delete currMat;
			}

			currMat = new Material;
			currBMap = currDMap = wstring(L"NONE");

			continue;
		}

		if (line.find("\tKa ") != string::npos)
		{
			string ignore;
			istringstream iss(line);
			iss >> ignore >> currMat->Ambient.x >> currMat->Ambient.y >> currMat->Ambient.z;
			currMat->Ambient.w = 1.0f;
			continue;
		}

		if (line.find("\tKd ") != string::npos)
		{
			string ignore;
			istringstream iss(line);
			iss >> ignore >> currMat->Diffuse.x >> currMat->Diffuse.y >> currMat->Diffuse.z;
			currMat->Diffuse.w = 1.0f;
			continue;
		}

		if (line.find("\tKs ") != string::npos)
		{
			string ignore;
			istringstream iss(line);
			iss >> ignore >> currMat->Specular.x >> currMat->Specular.y >> currMat->Specular.z;
			continue;
		}

		if (line.find("\tNs ") != string::npos)
		{
			string ignore;
			istringstream iss(line);
			iss >> ignore >> currMat->Specular.w;
			continue;
		}

		// Now maps
		if ((line.find("\tmap_Ka ") != string::npos) || (line.find("map_Kd ") != string::npos))
		{
			string mapFile;
			istringstream iss(line);
			iss >> mapFile >> mapFile;
			StringToWString(currDMap, mapFile);
			currDMap = wstring(L"\\") + currDMap;
			continue;
		}

		if ((line.find("\tmap_bump ") != string::npos) || (line.find("bump ") != string::npos))
		{
			string mapFile;
			istringstream iss(line);
			iss >> mapFile >> mapFile;
			StringToWString(currBMap, mapFile);
			currBMap = wstring(L"\\") + currBMap;
			continue;
		}
	}

	// save the last one
	if (currMat != 0)
	{
		currMat->Reflect = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		materials->push_back(*currMat);
		dMaps->push_back(currDMap);
		bMaps->push_back(currBMap);
		delete currMat;
	}

	input.close();

	if (materials->size() != matNames->size())
	{
		wstring msg = wstring(L"WARNING: The number of materials and material names is not equal, something is wrong with ") + OBJFile.mMatPath;
		OutputDebugString(&msg[0]);
	}
}

ID3D11ShaderResourceView *OBJFileLoader::LoadSRVFromFile(const wchar_t *texPath)
{
	wstring fullPath = mTexDir + wstring(L"\\") + wstring(texPath);
	ID3D11ShaderResourceView *srv;
	HRESULT hr;
	hr = D3DX11CreateShaderResourceViewFromFile(mValues->md3dDevice, &fullPath[0], 0, 0, &srv, 0);
	if (FAILED(hr))
		throw UnexpectedError(wstring(L"Failed to load texture ") + fullPath + L"! (OBJFileLoader::LoadSRVFromFile)");

	return srv;
}