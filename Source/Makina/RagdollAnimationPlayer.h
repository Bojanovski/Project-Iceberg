
#ifndef RAGDOLLANIMATIONPLAYER_H
#define RAGDOLLANIMATIONPLAYER_H
//#define RAGDOLLANIMATIONPLAYER_DEBUG_MODE

#include "AnimationPlayer.h"
#include "MeshSimulationData.h"

#ifdef RAGDOLLANIMATIONPLAYER_DEBUG_MODE
#include "D3DAppValues.h"

	class Object;
#endif

namespace Physics_Makina
{
	class CollisionSkin;
	class RigidBody;
	class ForceGenerator;
}

namespace Makina
{
	class PhysicsSystem;
	class BasicModel;

	/*********************************************************************
	*	Like AnimationPlayer, but with added support for ragdoll physics
	*	simulation.
	*********************************************************************/
	class RagdollAnimationPlayer : public AnimationPlayer
	{
	public:
#ifdef RAGDOLLANIMATIONPLAYER_DEBUG_MODE
		__declspec(dllexport) RagdollAnimationPlayer(D3DAppValues *val, MeshAnimationData *anmData, CXMMATRIX world, MeshSimulationData *simData, CXMVECTOR size);
#else
		__declspec(dllexport) RagdollAnimationPlayer(MeshAnimationData *anmData, CXMMATRIX world, MeshSimulationData *simData, CXMVECTOR size);
#endif
		__declspec(dllexport) ~RagdollAnimationPlayer();

#ifdef RAGDOLLANIMATIONPLAYER_DEBUG_MODE
		__declspec(dllexport) void StartSimulation(PhysicsSystem *phySys, std::vector<Physics_Makina::ForceGenerator *> &forceGen, XMFLOAT4X4 &world,
			void *pMethodOwner,
			void *(*AddObjectMethod)(BasicModel *model, void *pMethodOwner, CXMMATRIX world),
			void (*UpdateObjectMethod)(void *pObject, void *pMethodOwner, XMFLOAT4X4 &world));
#else
		__declspec(dllexport) void StartSimulation(PhysicsSystem *phySys, std::vector<Physics_Makina::ForceGenerator *> &forceGen, XMFLOAT4X4 &world);
#endif
		__declspec(dllexport) virtual void Update(float dt, XMFLOAT4X4 *world);
		__declspec(dllexport) virtual void Play(const std::string &animName);

	private:
		// Returns whether joint was created.
		bool CreateJointWithParent(PhysicsSystem *phySys, CXMMATRIX world, UINT boneIndex);

#ifdef RAGDOLLANIMATIONPLAYER_DEBUG_MODE
		std::vector<BasicModel *> mModel;
		std::vector<void *> mObjects;
		void(*mUpdateObjectMethod)(void *pObject, void *pMethodOwner, XMFLOAT4X4 &world);
		void *mPMethodOwner;
#endif
		MeshSimulationData *mSimData;
		std::vector<Physics_Makina::CollisionSkin *> mCs;
		std::vector<Physics_Makina::RigidBody *> mRgdBodies;

		// bone velocity data in world space
		std::vector<XMFLOAT3> mBoneVel;
		// previous frame position of a bone in world space
		std::vector<XMFLOAT3> mBonePrevPos;
	};
}

#endif
