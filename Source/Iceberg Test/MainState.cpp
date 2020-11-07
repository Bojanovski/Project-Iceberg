
#include "MainState.h"
#include <sstream>
#include <algorithm>
#include <Sparks.h>
#include <RigidBody.h>
#include <PhysicsUtil.h>
#include <ForceGenerators.h>
#include <PhysicsSystem.h>
#include <BinnedCollisionSpace.h>
#include <UniversalJoint.h>
#include <HingeJoint.h>
#include <InputHandler.h>
#include <RagdollAnimationPlayer.h>

using namespace std;
using namespace Makina;
using namespace Physics_Makina;

//
// Lua functions
//

int MainState::StartAnotherScript(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	string scriptName;
	scriptName = Lua::ToString(luaState, 1);
	vector<string> parameters;

	// loop through each argument
	for (int i = 2; i <= n; ++i)
	{
		// add to string
		parameters.push_back(Lua::ToString(luaState, i));
	}

	LuaScript *tempPt = 0;
	for (auto &luaScript : ownerPt->mLuaScripts)
	if (luaScript->GetPath().compare(scriptName) == 0) tempPt = luaScript;

	if (tempPt == 0)
	{
		// It has not been loaded yet!
		LuaScript *scriptPt = new LuaScript(&scriptName[0], ownerPt);
		RegisterFunctions(scriptPt);
		ownerPt->mLuaScripts.push_back(scriptPt);
		scriptPt->Start(parameters);
	}
	else // It was loaded already and it might be ready to start.
		tempPt->Start(parameters);

	// return the number of results
	return 0;
}

int MainState::GetProcessAndSystemData(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 0) throw ScriptError(L"Invalid call to getProcessAndSystemData()!");

	int fps = (int)ownerPt->mD3DAppValues->mProcessAndSystemData->mFPS;
	float mmspf = (float)ownerPt->mD3DAppValues->mProcessAndSystemData->m_mSPF;
	float cpuUsage =  (float)ownerPt->mD3DAppValues->mProcessAndSystemData->mCPU_usageByMe;
	float ramUsage = (float)ownerPt->mD3DAppValues->mProcessAndSystemData->mPhysMemUsedByMe / (1024.0f*1024.0f);

	string fpsS = to_string(fps);
	string mmspfS = to_string(mmspf);
	string cpuUsageS = to_string(cpuUsage);
	string ramUsageS = to_string(ramUsage);

	Lua::PushString(luaState, &fpsS[0]);
	Lua::PushString(luaState, &mmspfS[0]);
	Lua::PushString(luaState, &cpuUsageS[0]);
	Lua::PushString(luaState, &ramUsageS[0]);

	return 4;
}

int MainState::GetClientWidthHeight(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 0) throw ScriptError(L"Invalid call to getClientWidthHeight()!");

	int width = (int)ownerPt->mD3DAppValues->mClientWidth;
	int height = (int)ownerPt->mD3DAppValues->mClientHeight;

	Lua::PushNumber(luaState, width);
	Lua::PushNumber(luaState, height);

	return 2;
}

int MainState::LoadModel(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 1) throw ScriptError(L"Invalid call to LoadModel()!");

	// get arguments
	string path = Lua::ToString(luaState, 1);
	wstring wpath(path.begin(), path.end());

	// get type
	wstring type = wpath.substr(wpath.find_last_of('.') + 1, wpath.length());
	transform(type.begin(), type.end(), type.begin(), ::tolower);

	// load model
	Model *model;
	if (type.compare(L"obj") == 0)
	{
		ownerPt->mContentLoader->GetOBJFileLoader()->LoadModel(&wpath[0]);
		model = ownerPt->mContentLoader->GetOBJFileLoader()->GetModel(&wpath[0]);
	}
	else if (type.compare(L"x") == 0)
	{
		ownerPt->mContentLoader->GetXFileLoader()->LoadModel(&wpath[0]);
		model = ownerPt->mContentLoader->GetXFileLoader()->GetModel(&wpath[0]);
	}
	else throw ScriptError(L"Invalid file passed to LoadModel()!");

	//Model *model = new Model(ownerPt->mD3DAppValues, ownerPt->mContentLoader, &wpath[0]);
	//obj->UpdateChanges();

	// return pointer to this object
	Lua::PushLightUserData(luaState, model); // pushes the value on the stack

	return 1;
}

int MainState::GenerateMesh_Box(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 3) throw ScriptError(L"Invalid call to GenerateMesh_Box()!");

	// get arguments
	float scaleX = (float)Lua::ToNumber(luaState, 1);
	float scaleY = (float)Lua::ToNumber(luaState, 2);
	float scaleZ = (float)Lua::ToNumber(luaState, 3);

	BasicMeshData meshData;
	GeometryGenerator::CreateBox(scaleX, scaleY, scaleZ, meshData);
	BasicModel *model = new BasicModel(ownerPt->mD3DAppValues);
	model->AddSubset(meshData);
	model->UpdateChanges();

	// return pointer to this object
	Lua::PushLightUserData(luaState, model); // pushes the value on the stack

	return 1;
}

int MainState::GenerateMesh_Cylinder(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 5) throw ScriptError(L"Invalid call to GenerateMesh_Cylinder()!");

	// get arguments
	float bRadius = (float)Lua::ToNumber(luaState, 1);
	float tRadius = (float)Lua::ToNumber(luaState, 2);
	float height = (float)Lua::ToNumber(luaState, 3);
	int sliceCount = Lua::ToInteger(luaState, 4);
	int stackCount = Lua::ToInteger(luaState, 5);

	BasicMeshData meshData;
	GeometryGenerator::CreateCylinder(bRadius, tRadius, height, sliceCount, stackCount, meshData);
	BasicModel *model = new BasicModel(ownerPt->mD3DAppValues);
	model->AddSubset(meshData);
	model->UpdateChanges();

	// return pointer to this object
	Lua::PushLightUserData(luaState, model); // pushes the value on the stack

	return 1;
}

int MainState::GenerateMesh_Capsule(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 4) throw ScriptError(L"Invalid call to GenerateMesh_Capsule()!");

	// get arguments
	float radius = (float)Lua::ToNumber(luaState, 1);
	float height = (float)Lua::ToNumber(luaState, 2);
	int sliceCount = Lua::ToInteger(luaState, 3);
	int stackCount = Lua::ToInteger(luaState, 4);

	//ownerPt->mContentLoader->GetTextureLoader()->LoadSRV(L"Textures\\bricks.dds");
	//ID3D11ShaderResourceView *tex = ownerPt->mContentLoader->GetTextureLoader()->GetSRV(L"Textures\\bricks.dds");
	//ownerPt->mContentLoader->GetTextureLoader()->LoadSRV(L"Textures\\bricks_nmap.dds");
	//ID3D11ShaderResourceView *texN = ownerPt->mContentLoader->GetTextureLoader()->GetSRV(L"Textures\\bricks_nmap.dds");

	//Material mMat;
	//mMat.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//mMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//mMat.Reflect = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//mMat.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 5.0f);



	BasicMeshData meshData;
	GeometryGenerator::CreateCapsule(radius, height, sliceCount, stackCount, sliceCount - 2, meshData);
	BasicModel *model = new BasicModel(ownerPt->mD3DAppValues);
	model->AddSubset(meshData);
	model->UpdateChanges();

	// return pointer to this object
	Lua::PushLightUserData(luaState, model); // pushes the value on the stack

	return 1;
}

int MainState::Add3DObject(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 13) throw ScriptError(L"Invalid call to object3D.add()!");

	// get arguments
	const void *pt = Lua::ToLightUserData(luaState, 1);
	const BasicModel *cModel = static_cast<const BasicModel *>(pt);
	BasicModel *model = const_cast<BasicModel *>(cModel);
	float posX = (float)Lua::ToNumber(luaState, 2);
	float posY = (float)Lua::ToNumber(luaState, 3);
	float posZ = (float)Lua::ToNumber(luaState, 4);
	float qX = (float)Lua::ToNumber(luaState, 5);
	float qY = (float)Lua::ToNumber(luaState, 6);
	float qZ = (float)Lua::ToNumber(luaState, 7);
	float qW = (float)Lua::ToNumber(luaState, 8);
	float scaleX = (float)Lua::ToNumber(luaState, 9);
	float scaleY = (float)Lua::ToNumber(luaState, 10);
	float scaleZ = (float)Lua::ToNumber(luaState, 11);
	bool wireframe = Lua::ToBoolean(luaState, 12);
	bool refl = Lua::ToBoolean(luaState, 13);

	XMMATRIX world = XMMatrixScaling(scaleX, scaleY, scaleZ) * XMMatrixRotationQuaternion(XMVectorSet(qX, qY, qZ, qW)) * XMMatrixTranslation(posX, posY, posZ);
	Object *obj = new Object(ownerPt->mD3DAppValues, model, world);
	obj->SetCubeMap(ownerPt->mDyCubeMaps);
	obj->SetShadowMap(ownerPt->mShadow);
	obj->SetSsaoMap(ownerPt->mSsao);
	obj->mRefl = refl;
	obj->mWireframe = wireframe;

	ownerPt->m3DObjects.push_back(obj);

	// return pointer to this object
	Lua::PushLightUserData(luaState, obj); // pushes the value on the stack

	return 1;
}

int MainState::PlayAnimation3DObject(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 2) throw ScriptError(L"Invalid call to object3D.playAnimation()!");

	// get arguments
	const void *pt = Lua::ToLightUserData(luaState, 1);
	const Object *cObj = static_cast<const Object *>(pt);
	Object *obj = const_cast<Object *>(cObj);
	string animName = Lua::ToString(luaState, 2);

	if (obj->GetAnimationPlayer())
		obj->GetAnimationPlayer()->Play(animName);
	else
		throw ScriptError(L"Invalid call to object3D.playAnimation()! The object has no skinning data.");

	return 0;
}

void *MainState::Add3DObject(BasicModel *model, void *ownerPt, CXMMATRIX world)
{
	MainState *ownerPtMS = (MainState *)ownerPt;
	bool wireframe = false;
	bool refl = false;
	
	Object *obj = new Object(ownerPtMS->mD3DAppValues, model, world);
	obj->SetCubeMap(ownerPtMS->mDyCubeMaps);
	obj->SetShadowMap(ownerPtMS->mShadow);
	obj->SetSsaoMap(ownerPtMS->mSsao);
	obj->mRefl = refl;
	obj->mWireframe = wireframe;

	ownerPtMS->m3DObjects.push_back(obj);
	return obj;
}

void MainState::Update3DObject(void *objectPt, void *ownerPt, XMFLOAT4X4 &world)
{
	Object *obj = (Object *)objectPt;
	obj->SetWorld(world);
}

int MainState::StartSimulation3DObject(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 1) throw ScriptError(L"Invalid call to object3D.startSimulation()!");

	// get arguments
	const void *pt = Lua::ToLightUserData(luaState, 1);
	const Object *cObj = static_cast<const Object *>(pt);
	Object *obj = const_cast<Object *>(cObj);

	vector<ForceGenerator *> fg;
	fg.push_back(ownerPt->mGrv);

	if (obj->GetAnimationPlayer() && dynamic_cast<RagdollAnimationPlayer *>(obj->GetAnimationPlayer()))
#ifdef RAGDOLLANIMATIONPLAYER_DEBUG_MODE
		((RagdollAnimationPlayer *)(obj->GetAnimationPlayer()))->StartSimulation(ownerPt->mPhySys, fg, obj->GetWorld(),
		ownerPt, &Add3DObject, &Update3DObject);
#else
		((RagdollAnimationPlayer *)(obj->GetAnimationPlayer()))->StartSimulation(ownerPt->mPhySys, fg, obj->GetWorld());
#endif
	else
		throw ScriptError(L"Invalid call to object3D.startSimulation()! The object has no skinning data.");

	return 0;
}

int MainState::Update3DObject(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 11) throw ScriptError(L"Invalid call to object3D.update()!");

	// get arguments
	const void *pt = Lua::ToLightUserData(luaState, 1);
	const Object *cObj = static_cast<const Object *>(pt);
	Object *obj = const_cast<Object *>(cObj);
	float posX = (float)Lua::ToNumber(luaState, 2);
	float posY = (float)Lua::ToNumber(luaState, 3);
	float posZ = (float)Lua::ToNumber(luaState, 4);
	float qX = (float)Lua::ToNumber(luaState, 5);
	float qY = (float)Lua::ToNumber(luaState, 6);
	float qZ = (float)Lua::ToNumber(luaState, 7);
	float qW = (float)Lua::ToNumber(luaState, 8);
	float scaleX = (float)Lua::ToNumber(luaState, 9);
	float scaleY = (float)Lua::ToNumber(luaState, 10);
	float scaleZ = (float)Lua::ToNumber(luaState, 11);

	XMMATRIX world = XMMatrixScaling(scaleX, scaleY, scaleZ) * XMMatrixRotationQuaternion(XMVectorSet(qX, qY, qZ, qW)) * XMMatrixTranslation(posX, posY, posZ);
	XMFLOAT4X4 fWorld;
	XMStoreFloat4x4(&fWorld, world);
	obj->SetWorld(fWorld);

	return 0;
}

int MainState::SetWireframe_3DObject(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 2) throw ScriptError(L"Invalid call to object3D.setWireframe()!");

	// get arguments
	const void *pt = Lua::ToLightUserData(luaState, 1);
	const Object *cObj = static_cast<const Object *>(pt);
	Object *obj = const_cast<Object *>(cObj);
	bool wirframe = Lua::ToBoolean(luaState, 2);

	//// Go back if it's not ready.
	//if (!obj->->IsReady())
	//	return;

	obj->mWireframe = wirframe;
	return 0;
}

int MainState::SetNextFrameProperties(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 2) throw ScriptError(L"Invalid call to SetNextFrameProperties()!");

	ownerPt->mBlurAmount = (float)Lua::ToNumber(luaState, 1);
	ownerPt->mDyCubeMap = Lua::ToBoolean(luaState, 2);

	return 0;
}

int MainState::AddBitmap(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 5) throw ScriptError(L"Invalid call to bitmap.add()!");

	ID3D11ShaderResourceView *tex = (ID3D11ShaderResourceView *)Lua::ToLightUserData(luaState, 1);
	float posX = (float)Lua::ToNumber(luaState, 2);
	float posY = (float)Lua::ToNumber(luaState, 3);
	float sizeX = (float)Lua::ToNumber(luaState, 4);
	float sizeY = (float)Lua::ToNumber(luaState, 5);

	Bitmap *newBitmap = new Bitmap(ownerPt->mD3DAppValues, tex, posX, posY, sizeX, sizeY, Colors::White);
	ownerPt->mBitMaps.push_back(newBitmap);

	// return pointer to this bitmap
	Lua::PushLightUserData(luaState, newBitmap); // pushes the value on the stack

	return 1;
}

int MainState::UpdateBitmap(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 6) throw ScriptError(L"Invalid call to bitmap.update()!");

	Bitmap *bmp = (Bitmap *)Lua::ToLightUserData(luaState, 1);
	ID3D11ShaderResourceView *tex = (ID3D11ShaderResourceView *)Lua::ToLightUserData(luaState, 2);
	float posX = (float)Lua::ToNumber(luaState, 3);
	float posY = (float)Lua::ToNumber(luaState, 4);
	float sizeX = (float)Lua::ToNumber(luaState, 5);
	float sizeY = (float)Lua::ToNumber(luaState, 6);

	bmp->SetSRV(tex);
	bmp->ChangeProp(posX, posY, sizeX, sizeY, Colors::White);

	return 0;
}

int MainState::RemoveBitmap(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 1) throw ScriptError(L"Invalid call to bitmap.remove()!");

	Bitmap *bmp = (Bitmap *)Lua::ToLightUserData(luaState, 1);

	bool found = false;
	for (auto bmpIte = ownerPt->mBitMaps.begin(); bmpIte != ownerPt->mBitMaps.end();)
	{
		if ((*bmpIte) == bmp)
		{
			found = true;
			delete (*bmpIte);
			bmpIte = ownerPt->mBitMaps.erase(bmpIte);
		}
		else
			++bmpIte;
	}

	if (!found) throw ScriptError(L"Trying to remove non-existing bitmap in bitmap.remove()!");

	return 0;
}

int MainState::AddCloth(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 7) throw ScriptError(L"Invalid call to cloth.add()!");

	ID3D11ShaderResourceView *tex = (ID3D11ShaderResourceView *)Lua::ToLightUserData(luaState, 1);
	//ID3D11ShaderResourceView *texN = (ID3D11ShaderResourceView *)Lua::ToLightUserData(luaState, 2);
	int u_count = Lua::ToInteger(luaState, 2);
	int v_count = Lua::ToInteger(luaState, 3);
	float u_size = (float)Lua::ToNumber(luaState, 4);
	float v_size = (float)Lua::ToNumber(luaState, 5);
	float cp_mass = (float)Lua::ToNumber(luaState, 6);
	float elasticity = (float)Lua::ToNumber(luaState, 7);

	if ((u_count < 3) || (v_count < 3) || (u_count > 10) || (v_count > 10)) throw ScriptError(L"Error in cloth.add()! U and V control point count must be in [3, 10].");

	Cloth *newCloth = new Cloth(ownerPt->mD3DAppValues, u_count, v_count, u_size, v_size, cp_mass, elasticity);
	newCloth->SetDiffuseMap(tex);
	newCloth->SetShadowMap(ownerPt->mShadow);
	//newCloth->SetSsaoMap(ownerPt->mSsao);
	//newCloth->SetNormalMap(texN);
	ownerPt->mCloths.push_back(newCloth);

	// return pointer to this cloth object
	Lua::PushLightUserData(luaState, newCloth); // pushes the value on the stack
	return 1;
}

int MainState::FixControlPointCloth(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 6) throw ScriptError(L"Invalid call to cloth.fixControlPoint()!");

	Cloth *cloth = (Cloth *)Lua::ToLightUserData(luaState, 1);
	int index = Lua::ToInteger(luaState, 2);
	float posX = (float)Lua::ToNumber(luaState, 3);
	float posY = (float)Lua::ToNumber(luaState, 4);
	float posZ = (float)Lua::ToNumber(luaState, 5);
	bool movable = Lua::ToBoolean(luaState, 6);

	cloth->FixControlPoint(index, XMFLOAT3(posX, posY, posZ), movable);
	return 0;
}

int MainState::SetWindGravityCloth(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 7) throw ScriptError(L"Invalid call to cloth.setWindGravity()!");

	Cloth *cloth = (Cloth *)Lua::ToLightUserData(luaState, 1);
	float windX = (float)Lua::ToNumber(luaState, 2);
	float windY = (float)Lua::ToNumber(luaState, 3);
	float windZ = (float)Lua::ToNumber(luaState, 4);
	float gravityX = (float)Lua::ToNumber(luaState, 5);
	float gravityY = (float)Lua::ToNumber(luaState, 6);
	float gravityZ = (float)Lua::ToNumber(luaState, 7);

	cloth->SetGravityAcc(XMFLOAT3(gravityX, gravityY, gravityZ));
	cloth->SetWind(XMFLOAT3(windX, windY, windZ));
	return 0;
}

int MainState::AddSparksSource(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 0) throw ScriptError(L"Invalid call to particles.addSparksSource()!");
	Sparks *sparksPt = ownerPt->mPSM->AddSparks();

	// return pointer to this particle object
	Lua::PushLightUserData(luaState, sparksPt); // pushes the value on the stack
	return 1;
}

int MainState::AddFireSource(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 0) throw ScriptError(L"Invalid call to particles.addFireSource()!");
	vector<wstring> paths;
	paths.push_back(L"Textures//flare0.dds");
	//paths.push_back(L"Textures//floor.dds");
	Fire *sparksPt = ownerPt->mPSM->AddFire(paths);

	// return pointer to this particle object
	Lua::PushLightUserData(luaState, sparksPt); // pushes the value on the stack
	return 1;
}

int MainState::UpdateParticleSource(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n == 4)
	{
		ParticleSystem *psPt = (ParticleSystem *)Lua::ToLightUserData(luaState, 1);
		float posX = (float)Lua::ToNumber(luaState, 2);
		float posY = (float)Lua::ToNumber(luaState, 3);
		float posZ = (float)Lua::ToNumber(luaState, 4);
		psPt->SetEmitPos(XMFLOAT3(posX, posY, posZ));
	}
	else if (n == 7)
	{
		ParticleSystem *psPt = (ParticleSystem *)Lua::ToLightUserData(luaState, 1);
		float posX = (float)Lua::ToNumber(luaState, 2);
		float posY = (float)Lua::ToNumber(luaState, 3);
		float posZ = (float)Lua::ToNumber(luaState, 4);
		float dirX = (float)Lua::ToNumber(luaState, 5);
		float dirY = (float)Lua::ToNumber(luaState, 6);
		float dirZ = (float)Lua::ToNumber(luaState, 7);
		psPt->SetEmitPos(XMFLOAT3(posX, posY, posZ));
		psPt->SetEmitDir(XMFLOAT3(dirX, dirY, dirZ));
	}
	else
		throw ScriptError(L"Invalid call to particles.updateSource()!");

	return 0;
}

int MainState::LoadTexture(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 1) throw ScriptError(L"Invalid call to texture.load()!");

	wstring path = Lua::ToWString(luaState, 1);

	ID3D11ShaderResourceView *tex = 0;

	ownerPt->mContentLoader->GetTextureLoader()->LoadSRV(&path[0]);
	tex = ownerPt->mContentLoader->GetTextureLoader()->GetSRV(&path[0]);

	// return pointer to this texture
	Lua::PushLightUserData(luaState, tex); // pushes the value on the stack

	return 1;
}

int MainState::Generate2DVoronoiTexture(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 3) throw ScriptError(L"Invalid call to texture.generate2DVoronoiTexture()!");

	int width = Lua::ToInteger(luaState, 1);
	int height = Lua::ToInteger(luaState, 2);
	vector<float> vec = Lua::ToFloatVector(luaState, 3);

	if (vec.size() % 5 != 0) throw ScriptError(L"Invalid call to texture.generate2DVoronoiTexture()!");

	ID3D11ShaderResourceView *tex = 0;
	vector<Voronoi2DPoint> points;
	int pLen = vec.size() / 5;
	for (int i = 0; i < pLen; ++i)
	{
		Voronoi2DPoint p;
		p.mColor = XMFLOAT3(vec[i * 5], vec[i * 5 + 1], vec[i * 5 + 2]);
		p.mPos = XMFLOAT2(vec[i * 5 + 3], vec[i * 5 + 4]);
		points.push_back(p);
	}
	ownerPt->mTexGen->Generate2DVoronoiMap(width, height, &tex, points);

	// return pointer to this texture
	Lua::PushLightUserData(luaState, tex); // pushes the value on the stack

	return 1;
}

int MainState::GeneratePerlinNoiseTexture(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 3) throw ScriptError(L"Invalid call to texture.generatePerlinNoise()!");

	int seed = Lua::ToInteger(luaState, 1);
	int width = Lua::ToInteger(luaState, 2);
	int height = Lua::ToInteger(luaState, 3);

	ID3D11ShaderResourceView *tex = 0;
	ownerPt->mTexGen->GeneratePerlinNoise(seed, width, height, &tex);

	// return pointer to this texture
	Lua::PushLightUserData(luaState, tex); // pushes the value on the stack

	return 1;
}

int MainState::GenerateRidgedNoiseTexture(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 3) throw ScriptError(L"Invalid call to texture.generateRidgedNoise()!");

	int seed = Lua::ToInteger(luaState, 1);
	int width = Lua::ToInteger(luaState, 2);
	int height = Lua::ToInteger(luaState, 3);

	ID3D11ShaderResourceView *tex = 0;
	ownerPt->mTexGen->GenerateRidgedNoise(seed, width, height, &tex);

	// return pointer to this texture
	Lua::PushLightUserData(luaState, tex); // pushes the value on the stack

	return 1;
}

int MainState::GenerateRidgedPerlinMixedTexture(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 3) throw ScriptError(L"Invalid call to texture.generateRidgedPerlinMixedTexture()!");

	int seed = Lua::ToInteger(luaState, 1);
	int width = Lua::ToInteger(luaState, 2);
	int height = Lua::ToInteger(luaState, 3);

	ID3D11ShaderResourceView *tex = 0;
	ownerPt->mTexGen->GenerateRidgedPerlinMix1(seed, width, height, &tex);

	// return pointer to this texture
	Lua::PushLightUserData(luaState, tex); // pushes the value on the stack

	return 1;
}

int MainState::GenerateSphericalRidgedPerlinMixedTexture(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 3) throw ScriptError(L"Invalid call to texture.generateSphericalRidgedPerlinMixedTexture()!");

	int seed = Lua::ToInteger(luaState, 1);
	int width = Lua::ToInteger(luaState, 2);
	int height = Lua::ToInteger(luaState, 3);

	ID3D11ShaderResourceView *tex = 0;
	ownerPt->mTexGen->GenerateSphericalRidgedPerlinMix1(seed, width, height, &tex);

	// return pointer to this texture
	Lua::PushLightUserData(luaState, tex); // pushes the value on the stack

	return 1;
}

int MainState::GeneratePlanet(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 1) throw ScriptError(L"Invalid call to pcg.generatePlanet()!");

	int seed = Lua::ToInteger(luaState, 1);
	Planet *pPt = new Planet(ownerPt->mD3DAppValues, seed, ownerPt->mNDM, ownerPt->mTexGen);
	ownerPt->mPlanets.push_back(pPt);

	// return pointer to this planet
	Lua::PushLightUserData(luaState, pPt); // pushes the value on the stack

	return 1;
}

int MainState::UnloadTexture(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 1) throw ScriptError(L"Invalid call to texture.unload()!");

	ID3D11ShaderResourceView *tex = (ID3D11ShaderResourceView *)Lua::ToLightUserData(luaState, 1);

	ownerPt->mContentLoader->GetTextureLoader()->RemoveTexture(tex);

	return 0;
}

int MainState::DestroyTexture(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 1) throw ScriptError(L"Invalid call to texture.destroy()!");

	ID3D11ShaderResourceView *tex = (ID3D11ShaderResourceView *)Lua::ToLightUserData(luaState, 1);
	tex->Release();

	return 0;
}

int MainState::AddRigidBody(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 6 && n != 9) throw ScriptError(L"Invalid call to addRigidBody()!");

	// get arguments
	const void *pt = Lua::ToLightUserData(luaState, 1);
	const CollisionSkin *csC = static_cast<const CollisionSkin *>(pt);
	CollisionSkin *cs = const_cast<CollisionSkin *>(csC);
	bool movable = Lua::ToBoolean(luaState, 2);
	bool isAwake = Lua::ToBoolean(luaState, 3);
	float posX = (float)Lua::ToNumber(luaState, 4);
	float posY = (float)Lua::ToNumber(luaState, 5);
	float posZ = (float)Lua::ToNumber(luaState, 6);

	XMFLOAT3 linVel = {0.0f, 0.0f, 0.0f};
	if (n == 9)
	{
		linVel.x = (float)Lua::ToNumber(luaState, 7);
		linVel.y = (float)Lua::ToNumber(luaState, 8);
		linVel.z = (float)Lua::ToNumber(luaState, 9);
	}

	// Only first subset (mesh) is added. Here is some room for future improvements
	float mass;
	XMFLOAT3 cm;
	XMFLOAT3X3 inertia;

	if (typeid(*cs) == typeid(Mesh_CS))
	{
		Mesh_CS *mcs = dynamic_cast<Mesh_CS *>(cs);
		ComputeRigidBodyProperties(*mcs->GetMesh(), true, mass, cm, inertia);
	}
	else if (typeid(*cs) == typeid(Capsule_CS))
	{
		Capsule_CS *ccs = dynamic_cast<Capsule_CS *>(cs);
		ComputeRigidBodyProperties(ccs->GetHeight(), ccs->GetRadius(), mass, cm, inertia);
	}
	else throw UnexpectedError(L"Appearance of nonexistant collision skin type. (MainState::AddRigidBody)");

	RigidBody *rgdBody = new RigidBody(movable, true, isAwake, 0.2f, 0.4f, mass, cm, inertia, XMFLOAT3(posX, posY, posZ), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), linVel);
	rgdBody->AddCollisionSkin(cs);

	// Register everything
	ownerPt->mPhySys->RegisterRigidBody(rgdBody);
	ownerPt->mPhySys->AddForceGeneratorToRigidBody(ownerPt->mGrv, rgdBody);

	// return pointer to this object
	Lua::PushLightUserData(luaState, rgdBody); // pushes the value on the stack
	return 1;
}

int MainState::AddForceGenerator(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 1) throw ScriptError(L"Invalid call to addForceGenerator()!");

	// get arguments
	const void *pt = Lua::ToLightUserData(luaState, 1);
	const RigidBody *cRgdBody = static_cast<const RigidBody *>(pt);
	RigidBody *rgdBody = const_cast<RigidBody *>(cRgdBody);

	ForceGenerator *fq = new ForceQueue();

	// Register rigid body - force generator instance
	ownerPt->mPhySys->AddForceGeneratorToRigidBody(fq, rgdBody);

	// return pointer to this object
	Lua::PushLightUserData(luaState, fq); // pushes the value on the stack
	return 1;
}

int MainState::AddTorque(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 4) throw ScriptError(L"Invalid call to addTorque()!");

	// get arguments
	const void *pt = Lua::ToLightUserData(luaState, 1);
	const ForceQueue *cfq = static_cast<const ForceQueue *>(pt);
	ForceQueue *fq = const_cast<ForceQueue *>(cfq);
	float tX = (float)Lua::ToNumber(luaState, 2);
	float tY = (float)Lua::ToNumber(luaState, 3);
	float tZ = (float)Lua::ToNumber(luaState, 4);

	fq->AddTorqueToQueue({tX, tY, tZ});
	return 0;
}

int MainState::AddForce(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 4) throw ScriptError(L"Invalid call to addForce()!");

	// get arguments
	const void *pt = Lua::ToLightUserData(luaState, 1);
	const ForceQueue *cfq = static_cast<const ForceQueue *>(pt);
	ForceQueue *fq = const_cast<ForceQueue *>(cfq);
	float tX = (float)Lua::ToNumber(luaState, 2);
	float tY = (float)Lua::ToNumber(luaState, 3);
	float tZ = (float)Lua::ToNumber(luaState, 4);

	fq->AddForceToQueue({ tX, tY, tZ });
	return 0;
}

int MainState::GetRigidBodyCoords(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 1) throw ScriptError(L"Invalid call to getRigidBodyCoords()!");

	// get arguments
	const void *pt = Lua::ToLightUserData(luaState, 1);
	const RigidBody *cRgdBody = static_cast<const RigidBody *>(pt);
	RigidBody *rgdBody = const_cast<RigidBody *>(cRgdBody);

	XMVECTOR s, r, t;
	XMFLOAT4X4 world;
	rgdBody->GetTransformation(&world);
	XMMatrixDecompose(&s, &r, &t, XMLoadFloat4x4(&world));

	XMFLOAT4 pos, rot;
	XMStoreFloat4(&pos, t);
	XMStoreFloat4(&rot, r);

	// return coords
	Lua::PushNumber(luaState, pos.x);
	Lua::PushNumber(luaState, pos.y);
	Lua::PushNumber(luaState, pos.z);

	Lua::PushNumber(luaState, rot.x);
	Lua::PushNumber(luaState, rot.y);
	Lua::PushNumber(luaState, rot.z);
	Lua::PushNumber(luaState, rot.w);

	return 7;
}

int MainState::AddCollisionIgnorePair(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 2) throw ScriptError(L"Invalid call to addCollisionIgnorePair()!");

	// get arguments
	const void *pt = Lua::ToLightUserData(luaState, 1);
	const RigidBody *cRgdBody = static_cast<const RigidBody *>(pt);
	RigidBody *rgdBody1 = const_cast<RigidBody *>(cRgdBody);
	pt = Lua::ToLightUserData(luaState, 2);
	cRgdBody = static_cast<const RigidBody *>(pt);
	RigidBody *rgdBody2 = const_cast<RigidBody *>(cRgdBody);

	for (int i = 0; i < rgdBody1->GetCollisionSkin_RigidBody_InstanceCount(); ++i)
	for (int j = 0; j < rgdBody2->GetCollisionSkin_RigidBody_InstanceCount(); ++j)
		ownerPt->mPhySys->AddToCollisionIgnore(&rgdBody1->GetCollisionSkin_RigidBody_Instance(i), &rgdBody2->GetCollisionSkin_RigidBody_Instance(j));

	return 0;
}

int MainState::AddUniversalJoint(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 7) throw ScriptError(L"Invalid call to addUniversalJoint()!");

	// get arguments
	const void *pt = Lua::ToLightUserData(luaState, 1);
	const RigidBody *cRgdBody = static_cast<const RigidBody *>(pt);
	RigidBody *rgdBody1 = const_cast<RigidBody *>(cRgdBody);
	pt = Lua::ToLightUserData(luaState, 2);
	cRgdBody = static_cast<const RigidBody *>(pt);
	RigidBody *rgdBody2 = const_cast<RigidBody *>(cRgdBody);

	float posX = (float)Lua::ToNumber(luaState, 3);
	float posY = (float)Lua::ToNumber(luaState, 4);
	float posZ = (float)Lua::ToNumber(luaState, 5);
	float rotationalFreedom = (float)Lua::ToNumber(luaState, 6);
	float bendFreedom = (float)Lua::ToNumber(luaState, 7);

	// Update collision ignore table
	for (int i = 0; i < rgdBody1->GetCollisionSkin_RigidBody_InstanceCount(); ++i)
	for (int j = 0; j < rgdBody2->GetCollisionSkin_RigidBody_InstanceCount(); ++j)
		ownerPt->mPhySys->AddToCollisionIgnore(&rgdBody1->GetCollisionSkin_RigidBody_Instance(i), &rgdBody2->GetCollisionSkin_RigidBody_Instance(j));

	// Create the actual joint
	Joint *joint = new UniversalJoint(rgdBody1, rgdBody2, { posX, posY, posZ }, rotationalFreedom, bendFreedom);
	ownerPt->mPhySys->AddJoint(joint);

	// return pointer to this joint
	Lua::PushLightUserData(luaState, joint); // pushes the value on the stack
	return 1;
}

int MainState::AddHingeJoint(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 7) throw ScriptError(L"Invalid call to addHingeJoint()!");

	// get arguments
	const void *pt = Lua::ToLightUserData(luaState, 1);
	const RigidBody *cRgdBody = static_cast<const RigidBody *>(pt);
	RigidBody *rgdBody1 = const_cast<RigidBody *>(cRgdBody);
	pt = Lua::ToLightUserData(luaState, 2);
	cRgdBody = static_cast<const RigidBody *>(pt);
	RigidBody *rgdBody2 = const_cast<RigidBody *>(cRgdBody);

	float posX = (float)Lua::ToNumber(luaState, 3);
	float posY = (float)Lua::ToNumber(luaState, 4);
	float posZ = (float)Lua::ToNumber(luaState, 5);

	float fwdAngle = (float)Lua::ToNumber(luaState, 6);
	float bckAngle = (float)Lua::ToNumber(luaState, 7);

	// Update collision ignore table
	for (int i = 0; i < rgdBody1->GetCollisionSkin_RigidBody_InstanceCount(); ++i)
	for (int j = 0; j < rgdBody2->GetCollisionSkin_RigidBody_InstanceCount(); ++j)
		ownerPt->mPhySys->AddToCollisionIgnore(&rgdBody1->GetCollisionSkin_RigidBody_Instance(i), &rgdBody2->GetCollisionSkin_RigidBody_Instance(j));

	// Create the actual joint
	Joint *joint = new HingeJoint(rgdBody1, rgdBody2, { posX, posY, posZ }, fwdAngle, bckAngle);
	ownerPt->mPhySys->AddJoint(joint);

	// return pointer to this joint
	Lua::PushLightUserData(luaState, joint); // pushes the value on the stack
	return 1;
}

int MainState::AddMeshCollisionSkin(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 1) throw ScriptError(L"Invalid call to addMeshCollisionSkin()!");

	// get arguments
	const void *pt = Lua::ToLightUserData(luaState, 1);
	const BasicModel *cModel = static_cast<const BasicModel *>(pt);
	BasicModel *model = const_cast<BasicModel *>(cModel);

	// Only first subset (mesh) is added. Here is some room for future improvements
	CollisionSkin *collisionSkin = new Mesh_CS(model->GetSubset(0)->GetBoundingVolume(), model->GetSubset(0)->GetMesh());

	// return pointer to this object
	Lua::PushLightUserData(luaState, collisionSkin); // pushes the value on the stack
	return 1;
}

int MainState::AddCapsuleCollisionSkin(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 2) throw ScriptError(L"Invalid call to addCapsuleCollisionSkin()!");

	// get arguments
	float h = (float)Lua::ToNumber(luaState, 1); // height
	float r = (float)Lua::ToNumber(luaState, 2); // radius

	OrientedBox obb;
	obb.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	obb.Extents = XMFLOAT3(r, r + h, r);
	XMStoreFloat4(&obb.Orientation, XMQuaternionIdentity());
	CollisionSkin *collisionSkin = new Capsule_CS(&obb, h, r);

	// return pointer to this object
	Lua::PushLightUserData(luaState, collisionSkin); // pushes the value on the stack
	return 1;
}

int MainState::LoadFont(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 1) throw ScriptError(L"Invalid call to font.load()!");

	wstring folderPath = Lua::ToWString(luaState, 1);

	Font *font = new Font(ownerPt->mD3DAppValues, &folderPath[0]);

	// return pointer to this font
	Lua::PushLightUserData(luaState, font); // pushes the value on the stack

	return 1;
}

int MainState::UnloadFont(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 1) throw ScriptError(L"Invalid call to font.unload()!");

	Font *font = (Font *)Lua::ToLightUserData(luaState, 1);

	delete font;

	return 0;
}

int MainState::AddText(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 8) throw ScriptError(L"Invalid call to text.add()!");

	Font *font = (Font *)Lua::ToLightUserData(luaState, 1);
	wstring text = Lua::ToWString(luaState, 2);
	float posX = (float)Lua::ToNumber(luaState, 3);
	float posY = (float)Lua::ToNumber(luaState, 4);
	float size = (float)Lua::ToNumber(luaState, 5);
	float R = (float)Lua::ToNumber(luaState, 6);
	float G = (float)Lua::ToNumber(luaState, 7);
	float B = (float)Lua::ToNumber(luaState, 8);

	Text *newText = new Text(ownerPt->mD3DAppValues, font, &text[0], posX, posY, size, XMVectorSet(R, G, B, 1.0f));
	ownerPt->mTexts.push_back(newText);

	// return pointer to this text
	Lua::PushLightUserData(luaState, newText); // pushes the value on the stack

	return 1;
}

int MainState::UpdateText(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 2) throw ScriptError(L"Invalid call to text.update()!");

	Text *txtPt = (Text *)Lua::ToLightUserData(luaState, 1);
	wstring text = Lua::ToWString(luaState, 2);

	txtPt->ChangeText(&text[0]);
	return 0;
}

int MainState::RemoveText(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 1) throw ScriptError(L"Invalid call to text.remove()!");

	Text *txt = (Text *)Lua::ToLightUserData(luaState, 1);

	bool found = false;
	for (auto txtIte = ownerPt->mTexts.begin(); txtIte != ownerPt->mTexts.end();)
	{
		if ((*txtIte) == txt)
		{
			found = true;
			delete (*txtIte);
			txtIte = ownerPt->mTexts.erase(txtIte);
		}
		else
			++txtIte;
	}

	if (!found) throw ScriptError(L"Trying to remove non-existing text in text.remove()!");

	return 0;
}

int MainState::IsKeyDown(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 1) throw ScriptError(L"Invalid call to input.isKeyDown()!");

	UINT keyCode = (UINT)Lua::ToInteger(luaState, 1);
	float pressedTime = 0;

	ownerPt->mD3DAppValues->mInputHandler->IsPressed(keyCode, &pressedTime);

	// return pressed time
	Lua::PushNumber(luaState, pressedTime); // pushes the value on the stack

	return 1;
}

int MainState::CameraLookAt(lua_State *luaState)
{
	LuaScript *callerScript = LuaScript::GetCallerScript(luaState);
	MainState *ownerPt = static_cast<MainState *>(callerScript->GetOwnerObjectPt());

	// get number of arguments
	int n = Lua::GetTop(luaState);
	if (n != 6) throw ScriptError(L"Invalid call to camera.lookAt()!");

	XMFLOAT3 pos, target, up;

	pos.x = (float)Lua::ToNumber(luaState, 1);
	pos.y = (float)Lua::ToNumber(luaState, 2);
	pos.z = (float)Lua::ToNumber(luaState, 3);

	target.x = (float)Lua::ToNumber(luaState, 4);
	target.y = (float)Lua::ToNumber(luaState, 5);
	target.z = (float)Lua::ToNumber(luaState, 6);
	
	up = {0.0f, 1.0f, 0.0f};

	ownerPt->mD3DAppValues->mCamera->LookAt(pos, target, up);

	return 0;
}

//
// Class implementation
//

MainState::MainState(InitD3DApp *d3DAppPt)
: GameState(),
mD3DAppPt(d3DAppPt),
mContentLoader(NULL),
mSceneSRV(NULL),
mSceneUAV(NULL),
mStaticCubeMapSRV(NULL),
mDyCubeMaps(NULL),
mTexGen(NULL),
mBlurFX(NULL),
mFullScreenQuad(NULL),
mShadow(NULL),
mNDM(NULL),
mSsao(NULL),
mSV(NULL),
mSkyBox(NULL),
mPhySys(NULL),
mGrv(NULL),
mPSM(NULL)
{
	mDyCubeMap = false;
	mBlurAmount = 0.0f;
}

MainState::~MainState()
{	
	// It is very important we let PhysicsSystem to finish it's work
	// before we destroy objects it is depending on.
	mPhySys->Join();

	// Delete and stop (if necessary) all Lua functions!
	vector<string> params; // empty
	for (auto &luaScript : mLuaScripts)
	{
		if (luaScript->GetState() == LuaScriptState::Running) luaScript->Stop(params);
		delete luaScript;
	}

	if (mGrv) delete mGrv;
	// if (mPhySys) delete mPhySys; --> is game component, does not need cleaning

	// Delete bitmaps
	for (auto &bitmap : mBitMaps)
		delete bitmap;

	// Delete cloths
	for (auto &cloth : mCloths)
		delete cloth;

	// Delete texts
	for (auto &text : mTexts)
		delete text;

	// Delete planets
	for (auto &planet : mPlanets)
		delete planet;

	// Delete 3D objects
	for (auto &obj : m3DObjects)
		delete obj;

	if (mSceneSRV) mSceneSRV->Release();
	if (mSceneUAV) mSceneUAV->Release();

	if (mSkyBox) delete mSkyBox;
	if (mTexGen) delete mTexGen;
	if (mBlurFX) delete mBlurFX;
	if (mFullScreenQuad) delete mFullScreenQuad;
	if (mDyCubeMaps) delete mDyCubeMaps;
	if (mShadow) delete mShadow;
	if (mNDM) delete mNDM;
	if (mSsao) delete mSsao;
	if (mSV) delete mSV;
	//mD3DAppValues->mTextureManager->Remove(mStaticCubeMapSRV);
	if (mPSM) delete mPSM;

	// Loader
	if (mContentLoader) delete mContentLoader;
}

void MainState::Initialize()
{	
	mContentLoader = new ContentLoader(mD3DAppValues);

	mTexGen = new TextureGenerator(mD3DAppValues);
	mBlurFX = new BlurEffect(mD3DAppValues, DXGI_FORMAT_R8G8B8A8_UNORM);
	mBlurFX->SetGaussianWeights(1.8f);
	mFullScreenQuad = new FullScreenQuad(mD3DAppValues);
	mFullScreenQuad->SetSRV(mSceneSRV);
	mDyCubeMaps = new DynamicCubeMap(mD3DAppValues, true);
	mSkyBox = new SkyBox(mD3DAppValues);
	mContentLoader->GetTextureLoader()->LoadSRV(L"Textures\\grayCubeMap.dds");
	mStaticCubeMapSRV = mContentLoader->GetTextureLoader()->GetSRV(L"Textures\\grayCubeMap.dds");
	mSkyBox->SetCubeMapSRV(mStaticCubeMapSRV);
	mShadow = new ShadowMap(mD3DAppValues, 2048);
	mNDM = new SceneMapper(mD3DAppValues);
	mNDM->SetProperties(100.0f);
	mSsao = new SSAO(mD3DAppValues, mNDM);
	mSsao->SetProperties(0.3f, 0.05f, 0.5f, 0.05f);
	mSV = new SceneVoronoizator(mD3DAppValues);

	mPhySys = new PhysicsSystem(mD3DAppValues);
	mD3DAppPt->AddGameComponent(mPhySys);
	mGrv = new Gravity(XMFLOAT3(0.0f, -9.81f, 0.0f));
	mPSM = new ParticleSystemManager(mD3DAppValues, mContentLoader);

	LuaScript *scriptPt = new LuaScript("Entry.lua", this);
	RegisterFunctions(scriptPt);
	mLuaScripts.push_back(scriptPt);
	vector<string> params;
	scriptPt->Start(params);

	//mD3DAppValues->mGameTimer->SetMinFrameTime(1.0f/60.0f);
}

void MainState::Update(float dt)
{
	UINT i;
	UINT size = mLuaScripts.size();
	for (i = 0; i < size; ++i)
	if (mLuaScripts[i]->GetState() == LuaScriptState::Running) mLuaScripts[i]->Update(dt);

	for (UINT i = 0; i < mPlanets.size(); i++)
		mPlanets[i]->Update(dt);

	for (UINT i = 0; i < m3DObjects.size(); i++)
		m3DObjects[i]->Update(dt);

	for (UINT i = 0; i < mCloths.size(); i++)
		mCloths[i]->Update(dt);
}

void MainState::Draw(float dt)
{
	// Generate shadow map
	mShadow->PrepareForMappingOrthographic(40, {0.0f, 0.0f, 0.0f}, XMLoadFloat3(&mD3DAppValues->mBasicEffect->DirLight().Direction));
	//mShadow->PrepareForMappingOrthographic(70, XMLoadFloat3(&mD3DAppValues->mCamera->GetPosition()), XMLoadFloat3(&mD3DAppValues->mBasicEffect->DirLight().Direction));
	//mShadow->PrepareForMappingProjective(0.25f * XM_PI, 2.0f, 1000.0f, XMVectorSet(-20, 15, -20, 1) + XMLoadFloat3(&mD3DAppValues.mCamera->GetPosition()), XMLoadFloat3(&mBasicFX->DirLight().Direction));

	for (UINT i = 0; i < m3DObjects.size(); i++)
		m3DObjects[i]->DrawDepthOnly();

	for (UINT i = 0; i < mCloths.size(); i++)
		mCloths[i]->DrawDepthOnly();

	mShadow->FinishMapping();

	// Generate NormalDepth map
	mNDM->PrepareForMapping();

	//update effects
	mD3DAppValues->mBasicEffect->UpdateVariables();

	for (UINT i = 0; i < m3DObjects.size(); i++)
		m3DObjects[i]->DrawNormalAndDepth();

	mNDM->FinishMapping();

	mSsao->ComputeSsao();
	mSsao->BlurAmbientMap(2);

	//Generate cube map
	if (mDyCubeMap)
	{
		XMFLOAT3 pos = mD3DAppValues->mCamera->GetPosition();
		mDyCubeMaps->PrepareForMapping(XMFLOAT3(0.0f, 0.0f, 0.0f));
		for (int j = 0; j < 6; ++j)
		{
			mDyCubeMaps->MapFaceAt(j);

			// draw sky
			mSkyBox->Draw();

			for (UINT i = 0; i < m3DObjects.size(); ++i)
			if (i != 4)
			{
				if (m3DObjects[i]->mRefl == true) continue;

				m3DObjects[i]->Draw(dt);
			}
		}
		mDyCubeMaps->FinishMapping();
	}

	// Draw scene normally
	mD3DAppPt->GetImmediateContext()->RSSetViewports(1, &mD3DAppValues->mScreenViewport);
	mD3DAppPt->GetImmediateContext()->OMSetRenderTargets(1, &mD3DAppValues->mRenderTargetView, mD3DAppValues->mDepthStencilView);
	mD3DAppPt->GetImmediateContext()->ClearRenderTargetView(mD3DAppValues->mRenderTargetView, reinterpret_cast<const float *>(&Colors::Black));
	mD3DAppPt->GetImmediateContext()->ClearDepthStencilView(mD3DAppValues->mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	// Draw sky
	mSkyBox->Draw();

	for (UINT i = 0; i < m3DObjects.size(); i++)
		m3DObjects[i]->Draw(dt);

	for (UINT i = 0; i < mCloths.size(); i++)
		mCloths[i]->Draw();

	for (UINT i = 0; i < mPlanets.size(); i++)
		mPlanets[i]->Draw(dt);

	// Draw particles
	mPSM->Draw();
	{
		// Then use ResolveSubresource() method to copy textels from back buffer to off screen texture.
		// http://www.gamasutra.com/view/feature/131995/resolve_your_resolves.php?print=1
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff476474%28v=vs.85%29.aspx
		ID3D11Resource *dest;
		mSceneSRV->GetResource(&dest);
		ID3D11Resource *src;
		mD3DAppValues->mRenderTargetView->GetResource(&src);
		mD3DAppPt->GetImmediateContext()->ResolveSubresource(dest, 0, src, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
		dest->Release();
		src->Release();

		// Clear buffers to draw full screen quad.
		mD3DAppPt->GetImmediateContext()->ClearRenderTargetView(mD3DAppValues->mRenderTargetView, reinterpret_cast<const float *>(&Colors::Black));
		mD3DAppPt->GetImmediateContext()->ClearDepthStencilView(mD3DAppValues->mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Do some post processing with the compute shader.
		mBlurFX->Blur(mSceneSRV, mSceneUAV, (int)mBlurAmount);

		// Now draw full screen quad after postprocessing.
		mFullScreenQuad->Draw();
	}

	mDyCubeMap = false;
	mBlurAmount = 0.0f;

	// Now UI
	for (auto &bitmap : mBitMaps) bitmap->Draw();

	for (auto &text : mTexts) text->Draw();
}

void MainState::OnResize()
{
	BuildOffScreenViews();
	if (mFullScreenQuad) mFullScreenQuad->SetSRV(mSceneSRV);

	// effects
	if (mBlurFX) mBlurFX->OnResize();
	if (mNDM) mNDM->OnResize();
	if (mSsao) mSsao->OnResize();
	if (mSV) mSV->OnResize();

	for (UINT i = 0; i < mPlanets.size(); i++) 
		mPlanets[i]->OnResize();

	// text
	for (auto &text : mTexts) text->OnResize();
}

void MainState::BuildOffScreenViews()
{
	// Start fresh.
	if (mSceneSRV) mSceneSRV->Release();
	if (mSceneUAV) mSceneUAV->Release();

	D3D11_TEXTURE2D_DESC texDesc;

	texDesc.Width = mD3DAppValues->mClientWidth;
	texDesc.Height = mD3DAppValues->mClientHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if (false)
	{
		texDesc.SampleDesc.Count = 4;
		texDesc.SampleDesc.Quality = mD3DAppPt->Get4xMsaaQuality() - 1;
	}
	// No MSAA
	else
	{
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
	}

	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D* offscreenTex = 0;
	HRESULT hr = mD3DAppValues->md3dDevice->CreateTexture2D(&texDesc, 0, &offscreenTex);
	if (FAILED(hr)) throw UnexpectedError(wstring(L"Failed to create texture! (MainState::BuildOffScreenViews)"));

	// Null description means to create a view to all mipmap levels using 
	// the format the texture was created with.
	hr = mD3DAppValues->md3dDevice->CreateShaderResourceView(offscreenTex, 0, &mSceneSRV);
	if (FAILED(hr)) throw UnexpectedError(wstring(L"Failed to create SRV! (MainState::BuildOffScreenViews)"));

	hr = mD3DAppValues->md3dDevice->CreateUnorderedAccessView(offscreenTex, 0, &mSceneUAV);
	if (FAILED(hr)) throw UnexpectedError(wstring(L"Failed to create UAV! (MainState::BuildOffScreenViews)"));

	// View saves a reference to the texture so we can release our reference.
	offscreenTex->Release();
}

void MainState::RegisterFunctions(LuaScript *scriptPt)
{
	scriptPt->RegisterFunction("StartAnotherScript", StartAnotherScript);
	scriptPt->RegisterFunction("getProcessAndSystemData", GetProcessAndSystemData);
	scriptPt->RegisterFunction("getClientWidthHeight", GetClientWidthHeight);
	scriptPt->RegisterFunction("LoadModel", LoadModel);
	scriptPt->RegisterFunction("GenerateMesh_Box", GenerateMesh_Box);
	scriptPt->RegisterFunction("GenerateMesh_Cylinder", GenerateMesh_Cylinder);
	scriptPt->RegisterFunction("GenerateMesh_Capsule", GenerateMesh_Capsule);
	scriptPt->RegisterFunction("SetNextFrameProperties", SetNextFrameProperties);

	// pcg
	static const luaL_Reg pcgLib[] = {
		{ "generatePlanet", GeneratePlanet },
		{ NULL, NULL }
	};
	scriptPt->RegisterGlobalLib("pcg", pcgLib);

	// physics
	static const luaL_Reg physicsLib[] = {
		{ "addRigidBody", AddRigidBody },
		{ "addTorque", AddTorque },
		{ "addForce", AddForce },
		{ "addForceGenerator", AddForceGenerator },
		{ "getRigidBodyCoords", GetRigidBodyCoords },
		{ "addCollisionIgnorePair", AddCollisionIgnorePair },
		{ "addUniversalJoint", AddUniversalJoint },
		{ "addHingeJoint", AddHingeJoint},
		{ "addMeshCollisionSkin", AddMeshCollisionSkin },
		{ "addCapsuleCollisionSkin", AddCapsuleCollisionSkin },
		{ NULL, NULL }
	};
	scriptPt->RegisterGlobalLib("physics", physicsLib);

	// 3DObject
	static const luaL_Reg _3dObjectLib[] = {
		{ "add", Add3DObject },
		{ "update", Update3DObject },
		{ "playAnimation", PlayAnimation3DObject },
		{ "startSimulation", StartSimulation3DObject },
		{ "setWireframe", SetWireframe_3DObject },
		{ NULL, NULL }
	};
	scriptPt->RegisterGlobalLib("object3D", _3dObjectLib);

	// textures
	static const luaL_Reg textureLib[] = {
		{ "load", LoadTexture },
		{ "generate2DVoronoiTexture", Generate2DVoronoiTexture },
		{ "generatePerlinNoise", GeneratePerlinNoiseTexture },
		{ "generateRidgedNoise", GenerateRidgedNoiseTexture },
		{ "generateRidgedPerlinMixed", GenerateRidgedPerlinMixedTexture },
		{ "generateSphericalRidgedPerlinMixed", GenerateSphericalRidgedPerlinMixedTexture },
		{ "destroy", DestroyTexture },
		{ "unload", UnloadTexture },
		{ NULL, NULL }
	};
	scriptPt->RegisterGlobalLib("texture", textureLib);

	// bitmap
	static const luaL_Reg bitmapLib[] = {
		{ "add", AddBitmap },
		{ "update", UpdateBitmap },
		{ "remove", RemoveBitmap },
		{ NULL, NULL }
	};
	scriptPt->RegisterGlobalLib("bitmap", bitmapLib);

	// cloth
	static const luaL_Reg clothLib[] = {
		{ "add", AddCloth },
		{ "fixControlPoint", FixControlPointCloth },
		{ "setWindGravity", SetWindGravityCloth },
		{ NULL, NULL }
	};
	scriptPt->RegisterGlobalLib("cloth", clothLib);

	// particles
	static const luaL_Reg particlesLib[] = {
		{ "addSparksSource", AddSparksSource },
		{ "addFireSource", AddFireSource },
		{ "updateSource", UpdateParticleSource },
		{ NULL, NULL }
	};
	scriptPt->RegisterGlobalLib("particles", particlesLib);

	// font
	static const luaL_Reg fontLib[] = {
		{ "load", LoadFont },
		{ "unload", UnloadFont },
		{ NULL, NULL }
	};
	scriptPt->RegisterGlobalLib("font", fontLib);

	// text
	static const luaL_Reg textLib[] = {
		{ "add", AddText },
		{ "update", UpdateText },
		{ "remove", RemoveText },
		{ NULL, NULL }
	};
	scriptPt->RegisterGlobalLib("text", textLib);

	// input
	static const luaL_Reg inputLib[] = {
		{ "isKeyDown", IsKeyDown },
		{ NULL, NULL }
	};
	scriptPt->RegisterGlobalLib("input", inputLib);

	// camera
	static const luaL_Reg cameraLib[] = {
		{ "lookAt", CameraLookAt },
		{ NULL, NULL }
	};
	scriptPt->RegisterGlobalLib("camera", cameraLib);
}
