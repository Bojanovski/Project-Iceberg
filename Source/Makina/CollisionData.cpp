#include "CollisionData.h"
#include "RigidBody.h"

using namespace Physics_Makina;

bool CollisionData::BothAsleep()
{
	return (!mSkinInstance[0]->mRigidBody->IsAwake() && !mSkinInstance[1]->mRigidBody->IsAwake());
}

bool CollisionData_MeshMesh::HasNoContacts() 
{
	return (mContactPointsI.size() == 0 && mContactEdgesI.size() == 0); 
}

CollisionData_CapsuleMesh::CollisionData_CapsuleMesh(char capsuleIndex)
: mCapsuleIndex(capsuleIndex)
{

}

bool CollisionData_CapsuleMesh::HasNoContacts()
{
	throw;
	return true;
}