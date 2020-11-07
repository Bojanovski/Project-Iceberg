
#include <GameStatesManager.h>
#include <LuaScript.h>
#include <ContentLoader.h>
#include <Bitmap.h>
#include <Cloth.h>
#include <Text.h>
#include <World.h>
#include <PhysicsSystem.h>
#include <ForceGenerators.h>
#include <ParticleSystemManager.h>
#include <SkyBox.h>
#include <Planet.h>
#include <FullScreenQuad.h>
#include <BasicEffect.h>
#include <BasicModel.h>
#include <TextureGenerator.h>
#include <BlurEffect.h>
#include <DynamicCubeMap.h>
#include <ShadowMap.h>
#include <SceneMapper.h>
#include <SSAO.h>
#include "SceneVoronoizator.h"
#include "InitD3DApp.h"
#include "Object.h"

class MainState : public Makina::GameState
{
public:
	MainState(InitD3DApp *d3DAppPt);
	~MainState();

protected:
	void Initialize();

private:
	void Update(float dt);
	void Draw(float dt);
	void OnResize();
	void BuildOffScreenViews();

	static void RegisterFunctions(Makina::LuaScript *scriptPt);

	InitD3DApp *mD3DAppPt;
	std::vector<Makina::LuaScript *> mLuaScripts;

	// Loading
	Makina::ContentLoader *mContentLoader;

	// Graphics
	std::vector<Makina::Bitmap *> mBitMaps;
	std::vector<Makina::Cloth *> mCloths;
	std::vector<Makina::Text *> mTexts;
	std::vector<Object *> m3DObjects;
	std::vector<Makina::Planet *> mPlanets;
	Makina::TextureGenerator *mTexGen;
	Makina::BlurEffect *mBlurFX;
	Makina::FullScreenQuad *mFullScreenQuad;
	Makina::DynamicCubeMap *mDyCubeMaps;
	Makina::ShadowMap *mShadow;
	Makina::SceneMapper *mNDM;
	Makina::SSAO *mSsao;
	Makina::SkyBox *mSkyBox;
	SceneVoronoizator *mSV;
	// Particle system manager
	Makina::ParticleSystemManager *mPSM;

	float mBlurAmount;
	bool mDyCubeMap;

	// Views
	ID3D11ShaderResourceView *mSceneSRV;
	ID3D11UnorderedAccessView *mSceneUAV;
	ID3D11ShaderResourceView *mStaticCubeMapSRV;

	// Physics
	Makina::PhysicsSystem *mPhySys;
	Physics_Makina::Gravity *mGrv;

	// lua functions
	static int StartAnotherScript(lua_State *luaState);
	static int GetProcessAndSystemData(lua_State *luaState);
	static int GetClientWidthHeight(lua_State *luaState);
	static int LoadModel(lua_State *luaState);
	static int GenerateMesh_Box(lua_State *luaState);
	static int GenerateMesh_Cylinder(lua_State *luaState);
	static int GenerateMesh_Capsule(lua_State *luaState);
	static int Add3DObject(lua_State *luaState);
	static void *Add3DObject(Makina::BasicModel *model, void *ownerPt, CXMMATRIX world);
	static void Update3DObject(void *objectPt, void *ownerPt, XMFLOAT4X4 &world);
	static int Update3DObject(lua_State *luaState);
	static int PlayAnimation3DObject(lua_State *luaState);
	static int StartSimulation3DObject(lua_State *luaState);
	static int SetWireframe_3DObject(lua_State *luaState);
	static int SetNextFrameProperties(lua_State *luaState);
	static int LoadTexture(lua_State *luaState);
	static int Generate2DVoronoiTexture(lua_State *luaState);
	static int GeneratePerlinNoiseTexture(lua_State *luaState);
	static int GenerateRidgedNoiseTexture(lua_State *luaState);
	static int GenerateRidgedPerlinMixedTexture(lua_State *luaState);
	static int GenerateSphericalRidgedPerlinMixedTexture(lua_State *luaState);
	static int GeneratePlanet(lua_State *luaState);
	static int UnloadTexture(lua_State *luaState);
	static int DestroyTexture(lua_State *luaState);
	static int AddBitmap(lua_State *luaState);
	static int UpdateBitmap(lua_State *luaState);
	static int RemoveBitmap(lua_State *luaState);

	static int AddCloth(lua_State *luaState);
	static int FixControlPointCloth(lua_State *luaState);
	static int SetWindGravityCloth(lua_State *luaState);

	static int AddSparksSource(lua_State *luaState);
	static int AddFireSource(lua_State *luaState);
	static int UpdateParticleSource(lua_State *luaState);

	static int AddRigidBody(lua_State *luaState);
	static int AddForceGenerator(lua_State *luaState);
	static int AddTorque(lua_State *luaState);
	static int AddForce(lua_State *luaState);
	static int GetRigidBodyCoords(lua_State *luaState);
	static int AddCollisionIgnorePair(lua_State *luaState);
	static int AddUniversalJoint(lua_State *luaState);
	static int AddHingeJoint(lua_State *luaState);
	static int AddMeshCollisionSkin(lua_State *luaState);
	static int AddCapsuleCollisionSkin(lua_State *luaState);

	static int LoadFont(lua_State *luaState);
	static int UnloadFont(lua_State *luaState);
	static int AddText(lua_State *luaState);
	static int UpdateText(lua_State *luaState);
	static int RemoveText(lua_State *luaState);

	static int IsKeyDown(lua_State *luaState);
	static int CameraLookAt(lua_State *luaState);
};