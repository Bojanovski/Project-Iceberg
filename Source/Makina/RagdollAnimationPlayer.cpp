
#include "RagdollAnimationPlayer.h"
#include "PhysicsSystem.h"
#include "CollisionSkin.h"
#include "PhysicsUtil.h"
#include "RigidBody.h"
#include "UniversalJoint.h"
#include "HingeJoint.h"
#include "ForceGenerators.h"
#include "BasicModel.h"

using namespace Makina;
using namespace Physics_Makina;
using namespace std;

#ifdef RAGDOLLANIMATIONPLAYER_DEBUG_MODE
RagdollAnimationPlayer::RagdollAnimationPlayer(D3DAppValues *val, MeshAnimationData *anmData, CXMMATRIX world, MeshSimulationData *simData, CXMVECTOR size)
#else
RagdollAnimationPlayer::RagdollAnimationPlayer(MeshAnimationData *anmData, CXMMATRIX world, MeshSimulationData *simData, CXMVECTOR size)
#endif
: AnimationPlayer(anmData, world),
mSimData(simData)
{
	BasicMeshData meshData;
	OrientedBox obb;

	for (auto &brb : simData->mBoneBodies)
	{
		XMFLOAT3 boxSize;
		XMStoreFloat3(&boxSize, size);
		boxSize.x = brb.mThickness.x * boxSize.x;
		boxSize.y = brb.mThickness.y * boxSize.y;
		boxSize.z = brb.mLength * boxSize.z;
		GeometryGenerator::CreateBox(boxSize.x, boxSize.y, boxSize.z, meshData);
		ComputeBoundingOrientedBoxFromPoints(&obb, meshData.Vertices.size(), &(meshData.Vertices[0].Position), meshData.SizeOfVertexElement());
		CollisionSkin *cs = new Mesh_CS(&obb, &meshData);
		mCs.push_back(cs);

#ifdef RAGDOLLANIMATIONPLAYER_DEBUG_MODE
		BasicModel *model = new BasicModel(val);
		model->AddSubset(meshData);
		model->UpdateChanges();
		mModel.push_back(model);
#endif
	}

	mBonePrevPos.resize(simData->mBoneBodies.size());
	mBoneVel.resize(simData->mBoneBodies.size());
}

RagdollAnimationPlayer::~RagdollAnimationPlayer()
{
	for (auto cs : mCs) delete cs;
#ifdef RAGDOLLANIMATIONPLAYER_DEBUG_MODE
	for (auto model : mModel) delete model;
#endif
}

#ifdef RAGDOLLANIMATIONPLAYER_DEBUG_MODE
void RagdollAnimationPlayer::StartSimulation(PhysicsSystem *phySys, vector<ForceGenerator *> &forceGen, XMFLOAT4X4 &world,
	void *pMethodOwner,
	void *(*AddObjectMethod)(BasicModel *model, void *pMethodOwner, CXMMATRIX world),
	void (*UpdateObjectMethod)(void *pObject, void *pMethodOwner, XMFLOAT4X4 &world))
#else
void RagdollAnimationPlayer::StartSimulation(PhysicsSystem *phySys, vector<ForceGenerator *> &forceGen, XMFLOAT4X4 &world)
#endif
{
#ifdef RAGDOLLANIMATIONPLAYER_DEBUG_MODE
	mPMethodOwner = pMethodOwner;
	mUpdateObjectMethod = UpdateObjectMethod;
#endif
	// bones
	XMMATRIX W = XMLoadFloat4x4(&world);
	XMVECTOR s, r, t;
	XMMatrixDecompose(&s, &r, &t, W);
	float mass;
	XMFLOAT3 cm;
	XMFLOAT3X3 inertia;
	bool isAwake = true;
	for (UINT i = 0; i < mSimData->mBoneBodies.size(); ++i)
	{
		Mesh_CS *cs = static_cast<Mesh_CS *>(mCs[i]);
		ComputeRigidBodyProperties(*cs->GetMesh(), true, mass, cm, inertia);

		XMVECTOR bonePosModelLocalSpace = XMLoadFloat3(&mSimData->mBoneBodies[i].mPos);
		XMVECTOR boneDirModelLocalSpace = XMLoadFloat3(&mSimData->mBoneBodies[i].mDir);
		XMVECTOR newPos = XMVector3TransformCoord(bonePosModelLocalSpace, W);
		XMFLOAT3 newPosF;
		XMStoreFloat3(&newPosF, newPos);

		XMVECTOR ori = MathHelper::CreateQuaternionFromDir(boneDirModelLocalSpace);
		ori = XMQuaternionMultiply(ori, r);
		XMFLOAT4 oriF;
		XMStoreFloat4(&oriF, ori);

		RigidBody *rgdBody = new RigidBody(true, true, isAwake, 0.2f, 0.4f, mass, cm, inertia, newPosF, oriF, mBoneVel[i]);
		rgdBody->AddCollisionSkin(cs);
		// Register rigid body
		phySys->RegisterRigidBody(rgdBody);
		// Add the force generators
		for (auto fg : forceGen) phySys->AddForceGeneratorToRigidBody(fg, rgdBody);
		// Add it to private list
		mRgdBodies.push_back(rgdBody);

#ifdef RAGDOLLANIMATIONPLAYER_DEBUG_MODE
		bool wireframe = false;
		bool refl = false;
		XMFLOAT4X4 Wf;
		rgdBody->GetTransformation(&Wf);
		XMMATRIX world = XMLoadFloat4x4(&Wf);
		void *objPt = AddObjectMethod(mModel[i], mPMethodOwner, world);
		mObjects.push_back(objPt);
#endif
	}

	for (int i = 0; i < (int)mRgdBodies.size(); ++i)
	{
		// add joint if possible
		CreateJointWithParent(phySys, W, i);

		// Update collision ignore table for sibling relations
		for (int j = i + 1; j < (int)mRgdBodies.size(); ++j)
		if (mSimData->AreSiblings(i, j))
		{
			RigidBody *rgdBody1 = mRgdBodies[i];
			RigidBody *rgdBody2 = mRgdBodies[j];
			for (int k = 0; k < rgdBody1->GetCollisionSkin_RigidBody_InstanceCount(); ++k)
			for (int l = 0; l < rgdBody2->GetCollisionSkin_RigidBody_InstanceCount(); ++l)
				phySys->AddToCollisionIgnore(&rgdBody1->GetCollisionSkin_RigidBody_Instance(k), &rgdBody2->GetCollisionSkin_RigidBody_Instance(l));
		}
	}

	for (int i = 0; i < (int)mRgdBodies.size(); ++i)
	{
		// apply change in position and orientation that will put rigid body in the same configuration that is the skinned model
		XMVECTOR bonePosModelLocalSpace = XMLoadFloat3(&mSimData->mBoneBodies[i].mPos);
		XMMATRIX boneFinal = XMLoadFloat4x4(&mBoneFinal[mSimData->mBoneBodies[i].mIndex]) * W;
		XMVECTOR newPosAfterAnimation = XMVector3TransformCoord(bonePosModelLocalSpace, boneFinal);
		mRgdBodies[i]->SetPosition(newPosAfterAnimation);
		XMVECTOR s, p, r;
		XMMatrixDecompose(&s, &r, &p, boneFinal);
		XMVECTOR ori = MathHelper::CreateQuaternionFromDir(XMLoadFloat3(&mSimData->mBoneBodies[i].mDir));
		XMVECTOR newOriAfterAnimation = XMQuaternionMultiply(ori, r);
		mRgdBodies[i]->SetOrientation(newOriAfterAnimation);
		mRgdBodies[i]->ForceUpdateOfTransformationMatrix();
	}

	mState = AnimationPlayerState::Other;
}

void RagdollAnimationPlayer::Update(float dt, XMFLOAT4X4 *world)
{
	XMMATRIX w = XMLoadFloat4x4(world);
	if (mState != AnimationPlayerState::Other)
	{
		AnimationPlayer::Update(dt, world);
		
		// velocity update
		for (UINT i = 0; i < mSimData->mBoneBodies.size(); ++i)
		{
			XMVECTOR bonePosModelLocalSpace = XMLoadFloat3(&mSimData->mBoneBodies[i].mPos);
			XMMATRIX boneFinal = XMLoadFloat4x4(&mBoneFinal[mSimData->mBoneBodies[i].mIndex]) * w;
			XMVECTOR newPos = XMVector3TransformCoord(bonePosModelLocalSpace, boneFinal);
			XMVECTOR oldPos = XMLoadFloat3(&mBonePrevPos[i]);
			XMVECTOR vel = (newPos - oldPos) / dt;
			XMStoreFloat3(&mBonePrevPos[i], newPos);
			XMStoreFloat3(&mBoneVel[i], vel);
		}
	}
	else
	{ // simulation

		// Translate world matrix to the position of the root bone rigid body. This is for the frustrum culling.
		XMVECTOR posWorld = XMVector3Transform(XMVectorZero(), w);
		XMFLOAT4X4 fRootBoneRigidBodyWorld;
		mRgdBodies[0]->GetTransformation(&fRootBoneRigidBodyWorld);
		XMMATRIX rootBoneRigidBodyWorld = XMLoadFloat4x4(&fRootBoneRigidBodyWorld);
		XMVECTOR posRootBoneRigidBody = XMVector3Transform(XMVectorZero(), rootBoneRigidBodyWorld);
		XMVECTOR translationVec = posRootBoneRigidBody - posWorld;
		w = w*XMMatrixTranslation(XMVectorGetX(translationVec), XMVectorGetY(translationVec), XMVectorGetZ(translationVec));
		XMStoreFloat4x4(world, w);

		// mSimData->mBoneBodies[i] is related to mRgdBodies[i]
		for (UINT i = 0; i < mRgdBodies.size(); ++i)
		{
			XMFLOAT4X4 W;
			mRgdBodies[i]->GetTransformation(&W);
			XMMATRIX rigidBodyWorld = XMLoadFloat4x4(&W);
			XMMATRIX boneInRigidBodyLocalSpace = XMLoadFloat4x4(&mSimData->mBoneBodies[i].mBoneSpace);	
			XMVECTOR s, r, t, d;
			XMMatrixDecompose(&s, &r, &t, w);
			XMMATRIX invWorld = XMMatrixInverse(&d, w);
			// multiply by inverse world because it will be multiplied by world later
			// and it is already in world coordinates
			XMMATRIX boneTransform = boneInRigidBodyLocalSpace * XMMatrixScalingFromVector(s) * rigidBodyWorld * invWorld;
			SetBoneTransform(boneTransform, mSimData->mBoneBodies[i].mIndex);

#ifdef RAGDOLLANIMATIONPLAYER_DEBUG_MODE
			mUpdateObjectMethod(mObjects[i], mPMethodOwner, W);
#endif
		}
	}
}

void RagdollAnimationPlayer::Play(const string &animName)
{
	AnimationPlayer::Play(animName);
}

bool RagdollAnimationPlayer::CreateJointWithParent(PhysicsSystem *phySys, CXMMATRIX world, UINT boneIndex)
{
	// joints
	int parentIndex = -1;
	BoneJoint *bj = NULL;
	for (UINT i = 0; i < mSimData->mBoneJoints.size(); ++i)
	{
		if (mSimData->mBoneJoints[i].mIChild == boneIndex)
		{
			parentIndex = mSimData->mBoneJoints[i].mIParent;
			bj = &mSimData->mBoneJoints[i];
			break;
		}
	}

	if (parentIndex == -1)
		return false; // it is ok, there is no joint for this pair

	// add the jont
	RigidBody *rgdBodyParent = mRgdBodies[parentIndex];
	RigidBody *rgdBodyChild = mRgdBodies[boneIndex];
	XMVECTOR anchorPos = XMLoadFloat3(&bj->mAnchorPos);
	anchorPos = XMVector3TransformCoord(anchorPos, world);
	XMFLOAT3 anchorPosF;
	XMStoreFloat3(&anchorPosF, anchorPos);

	// Update collision ignore table for parent-child relation
	for (int i = 0; i < rgdBodyParent->GetCollisionSkin_RigidBody_InstanceCount(); ++i)
	for (int j = 0; j < rgdBodyChild->GetCollisionSkin_RigidBody_InstanceCount(); ++j)
		phySys->AddToCollisionIgnore(&rgdBodyParent->GetCollisionSkin_RigidBody_Instance(i), &rgdBodyChild->GetCollisionSkin_RigidBody_Instance(j));

	// Create the actual joint
	Joint *joint;
	if (bj->mType == 'u')
		joint = new UniversalJoint(rgdBodyParent, rgdBodyChild, anchorPosF, bj->mP1, bj->mP2);
	else // must be 'h'
		joint = new HingeJoint(rgdBodyParent, rgdBodyChild, anchorPosF, bj->mP1, bj->mP2);
	phySys->AddJoint(joint);

	return true;
}