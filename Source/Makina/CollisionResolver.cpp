#include "CollisionResolver.h"
#include "CollisionDetector.h"
#include "RigidBody.h"
#include "Joint.h"

using namespace Physics_Makina;
using namespace Makina;
using namespace Makina::MathHelper;
using namespace std;

struct PrepareContactsAsyncWorkerData
{
	CollisionResolver *colRes;
	Contact *contact;
	const CollisionSkin_RigidBody_Instance *A, *B;
	float dt;
};

void CollisionResolver::PrepareContactsAsyncWorker::Work()
{
	PrepareContactsAsyncWorkerData data = *(PrepareContactsAsyncWorkerData *)mData;
	data.colRes->CalculateContactInterpenetrationData(data.contact, data.A, data.B);
}

CollisionResolver::CollisionResolver(UINT positionIterations, UINT velocityIterations, float angularMoveLimitConstant, float velocityEpsilon, float interpenetrationRelaxation, float velocityRelaxation)
	: mPositionIterations(positionIterations),
	mVelocityIterations(velocityIterations),
	mAngularMoveLimitConstant(angularMoveLimitConstant),
	mVelocityEpsilon(velocityEpsilon),
	mInterpenetrationRelaxation(interpenetrationRelaxation),
	mVelocityRelaxation(velocityRelaxation)
{

}

CollisionResolver::~CollisionResolver()
{

}

void CollisionResolver::ResolveContacts(CollisionDetector *colDet, float dt)
{
	int numContacts = colDet->mColData.size();
	mVelocityIterations = mPositionIterations = 50 + numContacts * 4;

	// Make sure we have something to do.
	if (numContacts == 0 && mJoints.size() == 0) return;

	// Prepare the contacts for processing
	UINT currentAsyncWorker = 0;
	PrepareContactsAsyncWorkerData data;
	data.colRes = this;
	data.dt = dt;
	for (auto dataIt = colDet->mColData.begin(); dataIt != colDet->mColData.end();)
	{	
		data.A = (*dataIt)->mSkinInstance[0];
		data.B = (*dataIt)->mSkinInstance[1];
		int index = (*dataIt)->mContacts.GetFirstIndex();
		while (index != HostedList_End)
		{
			//data.contact = &(*contactIt);
			//mPrepareContactsAsyncWorkers[currentAsyncWorker].Assign(&data, sizeof(data));
			//if (++currentAsyncWorker >= PREPARE_CONTACTS_ASYNCWORKER_N) currentAsyncWorker = 0;
			CalculateContactInterpenetrationData(&(*dataIt)->mContacts[index], (*dataIt)->mSkinInstance[0], (*dataIt)->mSkinInstance[1]);
			index = (*dataIt)->mContacts.GetNextIndex(index);
		}
		++dataIt;
	}

	// Wait for all the workers
	//for (int i = 0; i < PREPARE_CONTACTS_ASYNCWORKER_N; ++i)
		//mPrepareContactsAsyncWorkers[i].Join();

	// Process joints for position error severity
	for (auto jointIt = mJoints.begin(); jointIt != mJoints.end(); ++jointIt)
		(*jointIt)->CalculatePositionErrorSeverity(dt);

	// Resolve the interpenetration problems with the contacts.
	AdjustPositions(colDet, dt);

	// Prepare the contacts for processing again (now with velocity data).
	for (auto dataIt = colDet->mColData.begin(); dataIt != colDet->mColData.end();)
	{
		int index = (*dataIt)->mContacts.GetFirstIndex();
		while (index != HostedList_End)
		{
			CalculateContactBasisMatrix(&(*dataIt)->mContacts[index], (*dataIt)->mSkinInstance[0], (*dataIt)->mSkinInstance[1]);
			CalculateContactVelocityData(&(*dataIt)->mContacts[index], (*dataIt)->mSkinInstance[0], (*dataIt)->mSkinInstance[1], dt);
			index = (*dataIt)->mContacts.GetNextIndex(index);
		}
		++dataIt;
	}

	// Process joints for velocity error severity
	for (auto jointIt = mJoints.begin(); jointIt != mJoints.end(); ++jointIt)
		(*jointIt)->CalculateVeclocityErrorSeverity(dt);

	// Resolve the velocity problems with the contacts.
	AdjustVelocities(colDet, dt);
}

void CollisionResolver::RemoveJoint(Joint *joint)
{
	for (auto it = mJoints.begin(); it != mJoints.end();)
	if ((*it) == joint) it = mJoints.erase(it);
	else ++it;
}

void CollisionResolver::CalculateContactInterpenetrationData(Contact *contact, const CollisionSkin_RigidBody_Instance *A, const CollisionSkin_RigidBody_Instance *B)
{
	const CollisionSkin_RigidBody_Instance *i[2];
	i[0] = A;
	i[1] = B;

	XMVECTOR localNormal = XMLoadFloat3(&contact->mContactNormal);
	XMVECTOR localPosition = XMLoadFloat3(&contact->mContactPoint);
	XMMATRIX fromLocalToWorld = XMLoadFloat4x4(&i[contact->mContactPointHolderSkinI]->mRigidBody->mTransformMatrix);
	XMVECTOR worldNormal = XMVector3TransformNormal(localNormal, fromLocalToWorld);
	XMVECTOR worldPosition = XMVector3Transform(localPosition, fromLocalToWorld);

	XMVECTOR A_bodyPosition = XMLoadFloat3(&A->mRigidBody->mPosition);
	XMVECTOR B_bodyPosition = XMLoadFloat3(&B->mRigidBody->mPosition);

	//*****************************************************************
	//	First calculate relative positions in world coordinates.
	//*****************************************************************
	XMVECTOR A_bodyPointRelativePos = worldPosition - A_bodyPosition;
	XMVECTOR B_bodyPointRelativePos = worldPosition - B_bodyPosition;

	XMStoreFloat3(&contact->mRelativeContactPosition[0], A_bodyPointRelativePos);
	XMStoreFloat3(&contact->mRelativeContactPosition[1], B_bodyPointRelativePos);
}

void CollisionResolver::CalculateContactBasisMatrix(Contact *contact, const CollisionSkin_RigidBody_Instance *A, const CollisionSkin_RigidBody_Instance *B)
{
	const CollisionSkin_RigidBody_Instance *i[2];
	i[0] = A;
	i[1] = B;

	XMVECTOR localNormal = XMLoadFloat3(&contact->mContactNormal);
	XMMATRIX fromLocalToWorld = XMLoadFloat4x4(&i[contact->mContactPointHolderSkinI]->mRigidBody->mTransformMatrix);
	XMVECTOR worldNormal = XMVector3TransformNormal(localNormal, fromLocalToWorld);
	XMVECTOR basisY = worldNormal;
	XMVECTOR basisZ, basisX;
	if (abs(XMVectorGetX(worldNormal)) > abs(XMVectorGetZ(worldNormal)))
	{
		// We’re nearer the X axis, so use the Z axis.
		basisX = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	}
	else
	{
		// We’re nearer the Z axis, so use the X axis.
		basisX = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	}

	basisZ = XMVector3Cross(basisX, basisY);
	basisX = XMVector3Cross(basisY, basisZ);

	basisX = XMVector3Normalize(basisX);
	basisY = XMVector3Normalize(basisY);
	basisZ = XMVector3Normalize(basisZ);

	XMMATRIX contactToWorld = XMMatrixSet(XMVectorGetX(basisX), XMVectorGetY(basisX), XMVectorGetZ(basisX), 0.0f,
		XMVectorGetX(basisY), XMVectorGetY(basisY), XMVectorGetZ(basisY), 0.0f,
		XMVectorGetX(basisZ), XMVectorGetY(basisZ), XMVectorGetZ(basisZ), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	XMStoreFloat4x4(&contact->mContactToWorld, contactToWorld);
}

void CollisionResolver::CalculateContactVelocityData(Contact *contact, const CollisionSkin_RigidBody_Instance *A, const CollisionSkin_RigidBody_Instance *B, float dt)
{
	XMVECTOR bodyPointRelPos[2];
	bodyPointRelPos[0] = XMLoadFloat3(&contact->mRelativeContactPosition[0]);
	bodyPointRelPos[1] = XMLoadFloat3(&contact->mRelativeContactPosition[1]);
	XMMATRIX worldToContact = XMMatrixTranspose(XMLoadFloat4x4(&contact->mContactToWorld));

	//*****************************************************************
	//	Now calculate closing velocity.
	//*****************************************************************
	// acceleration based velocity
	XMVECTOR lastUpdateAcc[2];
	lastUpdateAcc[0] = XMLoadFloat3(&A->mRigidBody->mLastUpdateAcceleration);
	lastUpdateAcc[1] = XMLoadFloat3(&B->mRigidBody->mLastUpdateAcceleration);

	XMVECTOR closingAcc = lastUpdateAcc[contact->mContactPointHolderSkinI] - lastUpdateAcc[1 - contact->mContactPointHolderSkinI];
	closingAcc = XMVector3TransformNormal(closingAcc, worldToContact);
	XMVECTOR velocityFromAcc = closingAcc*dt;

	// current velocity
	XMVECTOR bodyPointVelocity[2];
	bodyPointVelocity[0] = XMVectorZero(); // collision point relative to A
	if (A->mRigidBody->mIsMovable)
	{
		XMVECTOR A_bodyAngVelocity = XMLoadFloat3(&A->mRigidBody->mAngVelocity);
		bodyPointVelocity[0] = XMVector3Cross(A_bodyAngVelocity, bodyPointRelPos[0]);
		bodyPointVelocity[0] += XMLoadFloat3(&A->mRigidBody->mLinVelocity);
	}

	bodyPointVelocity[1] = XMVectorZero(); // collision point relative to B
	if (B->mRigidBody->mIsMovable)
	{
		XMVECTOR B_bodyAngVelocity = XMLoadFloat3(&B->mRigidBody->mAngVelocity);
		bodyPointVelocity[1] = XMVector3Cross(B_bodyAngVelocity, bodyPointRelPos[1]);
		bodyPointVelocity[1] += XMLoadFloat3(&B->mRigidBody->mLinVelocity);
	}

	XMVECTOR closingVelocityWorldSpace = bodyPointVelocity[contact->mContactPointHolderSkinI] - bodyPointVelocity[1 - contact->mContactPointHolderSkinI];
	XMVECTOR closingVelocityContactSpace = XMVector3Transform(closingVelocityWorldSpace, worldToContact);
	XMStoreFloat3(&contact->mContactVelocity, closingVelocityContactSpace + velocityFromAcc);

	//*****************************************************************
	//	Calculate desired delta velocity and we are done.
	//*****************************************************************
	// If the velocity is very slow, limit the restitution.
	float restitution = sqrt(A->mRigidBody->mRestitutionCoefficient * B->mRigidBody->mRestitutionCoefficient);
	if (abs(XMVectorGetY(closingVelocityContactSpace)) < mVelocityEpsilon) restitution = 0.0f;

	contact->mDesiredDeltaVelocity = -XMVectorGetY(closingVelocityContactSpace) - restitution * (XMVectorGetY(closingVelocityContactSpace)) - XMVectorGetY(velocityFromAcc);
}

inline void CollisionResolver::UpdateContactsAndJoints_pos(CollisionDetector *colDet, const RigidBody *rgdB1, const RigidBody *rgdB2, float dt)
{
	// For each collision skin
	for (auto dataIt = colDet->mColData.begin(); dataIt != colDet->mColData.end(); ++dataIt)
	{
		if ((rgdB1 == (*dataIt)->mSkinInstance[0]->mRigidBody && rgdB1->IsMovable()) ||
			(rgdB2 == (*dataIt)->mSkinInstance[0]->mRigidBody && rgdB2->IsMovable()) ||
			(rgdB1 == (*dataIt)->mSkinInstance[1]->mRigidBody && rgdB1->IsMovable()) ||
			(rgdB2 == (*dataIt)->mSkinInstance[1]->mRigidBody && rgdB2->IsMovable()))
		{
			CollisionData_MeshMesh *colDat_mm = dynamic_cast<CollisionData_MeshMesh *>(*dataIt);
			UpdateContact_MeshMesh(colDat_mm);
		}
	}
	// For each joint
	for (auto jointIt = mJoints.begin(); jointIt != mJoints.end(); ++jointIt)
	{
		if ((rgdB1 == (*jointIt)->mParent && rgdB1->IsMovable()) ||
			(rgdB2 == (*jointIt)->mParent && rgdB2->IsMovable()) ||
			(rgdB1 == (*jointIt)->mChild && rgdB1->IsMovable()) ||
			(rgdB2 == (*jointIt)->mChild && rgdB2->IsMovable()))
		{
			(*jointIt)->CalculatePositionErrorSeverity(dt);
		}
	}
}

void CollisionResolver::AdjustPositions(CollisionDetector *colDet, float dt)
{
	float max;
	UINT numContacts = colDet->mColData.size();
	Contact *cPt;
	Joint *jPt;
	XMVECTOR linearChange[2], angularChange[2];
	//const CollisionSkin_RigidBody_Instance *A, *B;
	const CollisionSkin_RigidBody_Instance *collRgd[2];

	// Iteratively resolve interpenetration in order of severity.
	UINT positionIterationsUsed = 0;
	while (positionIterationsUsed < mPositionIterations)
	{
		//********************************************************************
		//	Find biggest penetration
		//********************************************************************
		max = 0;
		cPt = 0;
		for (auto dataIt = colDet->mColData.begin(); dataIt != colDet->mColData.end();)
		{
			int index = (*dataIt)->mContacts.GetFirstIndex();
			while (index != HostedList_End)
			{
				if ((*dataIt)->mContacts[index].mInterpenetration < max && (*dataIt)->mContacts[index].mInterpenetration < 0.0f)
				{
					max = (*dataIt)->mContacts[index].mInterpenetration;
					cPt = &(*dataIt)->mContacts[index];
					collRgd[0] = (*dataIt)->mSkinInstance[0];
					collRgd[1] = (*dataIt)->mSkinInstance[1];
				}
				index = (*dataIt)->mContacts.GetNextIndex(index);
			}
			++dataIt;
		}
		if (cPt != 0)
		{
			// Set awake state
			if (!collRgd[0]->mRigidBody->IsAwake() && collRgd[0]->mRigidBody->IsMovable()) collRgd[0]->mRigidBody->SetAwake(true);
			if (!collRgd[1]->mRigidBody->IsAwake() && collRgd[1]->mRigidBody->IsMovable()) collRgd[1]->mRigidBody->SetAwake(true);

			// Resolve the penetration.
			ApplyPositionChange(cPt, collRgd[0], collRgd[1], linearChange, angularChange, dt);

			//	Again this action may have changed the penetration of other
			//	bodies, so we update contacts and joints.
			UpdateContactsAndJoints_pos(colDet, collRgd[0]->mRigidBody, collRgd[1]->mRigidBody, dt);
		}

		//********************************************************************
		//	Find joint with the biggest severity
		//********************************************************************
		max = 0;
		jPt = 0;
		for (auto jointIt = mJoints.begin(); jointIt != mJoints.end(); ++jointIt)
		{
			if ((*jointIt)->GetPosErrorSeverity() > max && (*jointIt)->AtLeastOneAwake())
			{
				max = (*jointIt)->GetPosErrorSeverity();
				jPt = (*jointIt);
			}
		}
		if (cPt == 0 && jPt == 0)
			break;

		if (jPt != 0)
		{
			// Set awake state
			if (!jPt->mParent->IsAwake() && jPt->mParent->IsMovable()) jPt->mParent->SetAwake(true);
			if (!jPt->mChild->IsAwake() && jPt->mChild->IsMovable()) jPt->mChild->SetAwake(true);

			// Resolve the joint error
			jPt->ApplyPositionChange(mInterpenetrationRelaxation, dt);

			//	Again this action may have changed the penetration of other
			//	bodies, so we update contacts and joints.
			UpdateContactsAndJoints_pos(colDet, jPt->mParent, jPt->mChild, dt);
		}
		positionIterationsUsed++;
	}
}

void CollisionResolver::ApplyPositionChange(Contact *contact, const CollisionSkin_RigidBody_Instance *A, const CollisionSkin_RigidBody_Instance *B, XMVECTOR linearChange[2], XMVECTOR angularChange[2], float dt)
{
	XMMATRIX inverseInertiaTensor_A = XMLoadFloat3x3(&A->mRigidBody->mInverseInertiaTensorWorld);
	XMMATRIX inverseInertiaTensor_B = XMLoadFloat3x3(&B->mRigidBody->mInverseInertiaTensorWorld);

	XMVECTOR relativeContactPos_A = XMLoadFloat3(&contact->mRelativeContactPosition[0]);
	XMVECTOR relativeContactPos_B = XMLoadFloat3(&contact->mRelativeContactPosition[1]);

	XMMATRIX world[2];
	world[0] = XMLoadFloat4x4(&A->mRigidBody->mTransformMatrix);
	world[1] = XMLoadFloat4x4(&B->mRigidBody->mTransformMatrix);

	XMVECTOR normalW = XMVector3TransformNormal(XMLoadFloat3(&contact->mContactNormal), world[contact->mContactPointHolderSkinI]);

	//********************************************************************
	//	Use the same procedure as for calculating frictionless
	//	velocity change to work out the angular inertia.
	//********************************************************************
	float angularInertia_A = 0;
	if (A->mRigidBody->mIsMovable)
	{
		XMVECTOR angularInertiaVec_A = XMVector3Cross(relativeContactPos_A, normalW);
		angularInertiaVec_A = XMVector3Transform(angularInertiaVec_A, inverseInertiaTensor_A);
		angularInertiaVec_A = XMVector3Cross(angularInertiaVec_A, relativeContactPos_A);
		angularInertiaVec_A = XMVector3Dot(angularInertiaVec_A, normalW);
		angularInertia_A = XMVectorGetX(angularInertiaVec_A);
	}
	float angularInertia_B = 0;
	if (B->mRigidBody->mIsMovable)
	{
		XMVECTOR angularInertiaVec_B = XMVector3Cross(relativeContactPos_B, normalW);
		angularInertiaVec_B = XMVector3Transform(angularInertiaVec_B, inverseInertiaTensor_B);
		angularInertiaVec_B = XMVector3Cross(angularInertiaVec_B, relativeContactPos_B);
		angularInertiaVec_B = XMVector3Dot(angularInertiaVec_B, normalW);
		angularInertia_B = XMVectorGetX(angularInertiaVec_B);
	}
	//********************************************************************
	//	The linear component is simply the inverse mass.
	//********************************************************************
	float linearInertia_A = (A->mRigidBody->mIsMovable) ? A->mRigidBody->mInverseMass : 0;
	float linearInertia_B = (B->mRigidBody->mIsMovable) ? B->mRigidBody->mInverseMass : 0;

	//********************************************************************
	//	Use these values to calculate changes in position.
	//********************************************************************
	float totalInertia = angularInertia_A + angularInertia_B + linearInertia_A + linearInertia_B;
	float inverseInertia = 1.0f / totalInertia;
	float sign = (contact->mContactPointHolderSkinI == 0) ? 1.0f : -1.0f;
	// contact->mInterpenetration is negative value
	float linearMove_A = -contact->mInterpenetration * linearInertia_A * inverseInertia * sign;
	float linearMove_B = contact->mInterpenetration * linearInertia_B * inverseInertia * sign;
	float angularMove_A = -contact->mInterpenetration * angularInertia_A * inverseInertia * sign;
	float angularMove_B = contact->mInterpenetration * angularInertia_B * inverseInertia * sign;


	//********************************************************************
	//	To avoid angular projections that are too great (when mass is large
	//	but inertia tensor is small) limit the angular move.
	//********************************************************************
	// for A
	float limit = mAngularMoveLimitConstant * XMVectorGetX(XMVector3Length(relativeContactPos_A));
	// Check the angular move is within limits.
	if (abs(angularMove_A) > limit)
	{
		float totalMove = linearMove_A + angularMove_A;
		// Set the new angular move, with the same sign as before.
		if (angularMove_A >= 0)
			angularMove_A = limit;
		else
			angularMove_A = -limit;

		// Make the linear move take the extra slack.
		linearMove_A = totalMove - angularMove_A;
	}
	// for B
	limit = mAngularMoveLimitConstant * XMVectorGetX(XMVector3Length(relativeContactPos_B));
	// Check the angular move is within limits.
	if (abs(angularMove_B) > limit)
	{
		float totalMove = linearMove_B + angularMove_B;
		// Set the new angular move, with the same sign as before.
		if (angularMove_B >= 0)
			angularMove_B = limit;
		else
			angularMove_B = -limit;

		// Make the linear move take the extra slack.
		linearMove_B = totalMove - angularMove_B;
	}

	//********************************************************************
	//	We have the linear amount of movement required by turning
	//	the rigid body (in angularMove[i]). We now need to
	//	calculate the desired rotation (vector) to achieve that.
	//********************************************************************
	if (angularMove_A == 0)
		angularChange[0] = XMVectorZero();
	else
	{
		XMVECTOR impulsiveTorque = XMVector3Cross(relativeContactPos_A, normalW);
		angularChange[0] = XMVector3Transform(impulsiveTorque, inverseInertiaTensor_A) * (angularMove_A / angularInertia_A) * mInterpenetrationRelaxation;
	}
	if (angularMove_B == 0)
		angularChange[1] = XMVectorZero();
	else
	{
		XMVECTOR impulsiveTorque = XMVector3Cross(relativeContactPos_B, normalW);
		angularChange[1] = XMVector3Transform(impulsiveTorque, inverseInertiaTensor_B) * (angularMove_B / angularInertia_B) * mInterpenetrationRelaxation;
	}
	// Velocity change is easier - it is just the linear movement
	// along the contact normal.
	linearChange[0] = normalW * linearMove_A * mInterpenetrationRelaxation;
	linearChange[1] = normalW * linearMove_B * mInterpenetrationRelaxation;

	//********************************************************************
	//	Now we can start to apply the values we've calculated.
	//********************************************************************

	// Apply the linear movement
	XMVECTOR A_pos = XMLoadFloat3(&A->mRigidBody->mPosition);
	A_pos += linearChange[0];
	XMStoreFloat3(&A->mRigidBody->mPosition, A_pos);
	XMVECTOR B_pos = XMLoadFloat3(&B->mRigidBody->mPosition);
	B_pos += linearChange[1];
	XMStoreFloat3(&B->mRigidBody->mPosition, B_pos);

	// And the change in orientation
	if (angularMove_A != 0)
	{
		XMVECTOR A_orientation = XMLoadFloat4(&A->mRigidBody->mOrientation);


		XMVECTOR angularChangeQ = XMVectorSet(XMVectorGetX(angularChange[0]), XMVectorGetY(angularChange[0]), XMVectorGetZ(angularChange[0]), 0);
		A_orientation += XMQuaternionMultiply(A_orientation, angularChangeQ*0.5f);
		A_orientation = XMQuaternionNormalize(A_orientation);


		//A_orientation = XMQuaternionMultiply(A_orientation, XMQuaternionRotationNormal(XMVector3Normalize(angularChange[0]), XMVectorGetX(XMVector3Length(angularChange[0]))));
		//A_orientation = XMQuaternionNormalize(A_orientation);
		XMStoreFloat4(&A->mRigidBody->mOrientation, A_orientation);
	}
	if (angularMove_B != 0)
	{
		XMVECTOR B_orientation = XMLoadFloat4(&B->mRigidBody->mOrientation);


		XMVECTOR angularChangeQ = XMVectorSet(XMVectorGetX(angularChange[1]), XMVectorGetY(angularChange[1]), XMVectorGetZ(angularChange[1]), 0);
		B_orientation += XMQuaternionMultiply(B_orientation, angularChangeQ*0.5f);
		B_orientation = XMQuaternionNormalize(B_orientation);

		//B_orientation = XMQuaternionMultiply(B_orientation, XMQuaternionRotationNormal(XMVector3Normalize(angularChange[1]), XMVectorGetX(XMVector3Length(angularChange[1]))));
		//B_orientation = XMQuaternionNormalize(B_orientation);
		XMStoreFloat4(&B->mRigidBody->mOrientation, B_orientation);
	}

	// And update transform matrix as final step.
	A->mRigidBody->UpdateTransformationMatrix();
	B->mRigidBody->UpdateTransformationMatrix();
}

inline void CollisionResolver::UpdateContact_MeshMesh(CollisionData_MeshMesh *colDat)
{
	const Mesh_CS *skin[2];
	skin[0] = dynamic_cast<const Mesh_CS *>(colDat->mSkinInstance[0]->mCollisionSkin);
	skin[1] = dynamic_cast<const Mesh_CS *>(colDat->mSkinInstance[1]->mCollisionSkin);

	XMMATRIX world[2];
	world[0] = XMLoadFloat4x4(&colDat->mSkinInstance[0]->mRigidBody->mTransformMatrix);
	XMVECTOR detA;
	XMMATRIX toLocalA = XMMatrixInverse(&detA, world[0]);
	world[1] = XMLoadFloat4x4(&colDat->mSkinInstance[1]->mRigidBody->mTransformMatrix);
	XMVECTOR detB;
	XMMATRIX toLocalB = XMMatrixInverse(&detB, world[1]);
	XMMATRIX fromLToL[2];
	fromLToL[0] = world[0] * toLocalB; // from A to B
	fromLToL[1] = world[1] * toLocalA; // from B to A
	XMVECTOR centerOfMeshA = XMLoadFloat3(&skin[0]->mCenterOfMesh);
	XMVECTOR centerOfMeshB = XMLoadFloat3(&skin[1]->mCenterOfMesh);

	//*******************************************************
	//	First points.
	//*******************************************************
	list<char> *cPnt = &colDat->mContactPointsI;
	for (auto pointIt = cPnt->begin(); pointIt != cPnt->end();)
	{
		Contact *contactPt = &colDat->mContacts[(*pointIt)];
		// get vertice responsible for Contact point
		XMVECTOR vertPos = XMLoadFloat3(&contactPt->mContactPoint);
		XMVECTOR vertPosW = XMVector3TransformCoord(vertPos, world[contactPt->mContactPointHolderSkinI]);
		// transform it to appropriate local space
		XMVECTOR vertPosLocal = XMVector3TransformCoord(vertPos, fromLToL[contactPt->mContactPointHolderSkinI]);

		// get fresh depth of interpenetration
		UINT faceIndex;
		float newInterpenetration;
		bool passed = CollisionDetector::GetInterpenetration(vertPosLocal, 0.0f, &newInterpenetration, &faceIndex, skin[1 - contactPt->mContactPointHolderSkinI]);

		// now the critical part for contact point
		if (passed)
		{// keep the point
			contactPt->mInterpenetration = newInterpenetration; // new interpenetration value

			XMVECTOR axis = XMVector3TransformNormal(XMLoadFloat3(&skin[1 - contactPt->mContactPointHolderSkinI]->mFaces[faceIndex].mNormal),
				fromLToL[1 - contactPt->mContactPointHolderSkinI]);

			contactPt->mIndexData[1] = faceIndex; // new face
			XMStoreFloat3(&contactPt->mContactNormal, axis); // new normal

			XMVECTOR A_bodyPosition = XMLoadFloat3(&colDat->mSkinInstance[0]->mRigidBody->mPosition);
			XMVECTOR A_bodyPointRelativePos = vertPosW - A_bodyPosition;
			XMStoreFloat3(&contactPt->mRelativeContactPosition[0], A_bodyPointRelativePos);
			XMVECTOR B_bodyPosition = XMLoadFloat3(&colDat->mSkinInstance[1]->mRigidBody->mPosition);
			XMVECTOR B_bodyPointRelativePos = vertPosW - B_bodyPosition;
			XMStoreFloat3(&contactPt->mRelativeContactPosition[1], B_bodyPointRelativePos);
		}
		else
			contactPt->mInterpenetration = D3D11_FLOAT32_MAX; // no interpenetration
		++pointIt;
	}

	//*******************************************************
	//	Now edges.
	//*******************************************************
	cPnt = &colDat->mContactEdgesI;
	for (auto pointIt = cPnt->begin(); pointIt != cPnt->end(); ++pointIt)
	{
		Contact *contactPt = &colDat->mContacts[(*pointIt)];
		XMVECTOR edgeA_p0 = XMLoadFloat3(&skin[0]->mMesh.Vertices[skin[0]->mEdges[contactPt->mIndexData[0]].mP0].Position);
		XMVECTOR edgeA_p1 = XMLoadFloat3(&skin[0]->mMesh.Vertices[skin[0]->mEdges[contactPt->mIndexData[0]].mP1].Position);
		edgeA_p0 = XMVector3TransformCoord(edgeA_p0, fromLToL[0]); // Now go to B's local space
		edgeA_p1 = XMVector3TransformCoord(edgeA_p1, fromLToL[0]);
		XMVECTOR AC_in_BLS = XMVector3TransformCoord(centerOfMeshA, fromLToL[0]); // A center in B local space
		float edgeALength = XMVectorGetX(XMVector3Length(edgeA_p1 - edgeA_p0));

		XMVECTOR edgeB_p0 = XMLoadFloat3(&skin[1]->mMesh.Vertices[skin[1]->mEdges[contactPt->mIndexData[1]].mP0].Position);
		XMVECTOR edgeB_p1 = XMLoadFloat3(&skin[1]->mMesh.Vertices[skin[1]->mEdges[contactPt->mIndexData[1]].mP1].Position);
		float edgeBLength = XMVectorGetX(XMVector3Length(edgeB_p1 - edgeB_p0));

		// run four tests
		bool passed = true;

		float edgeA_s, edgeB_s;
		XMVECTOR cA, cB, normal;
		// 1. test
		if (!CollisionDetector::GetClosestPoints_Lines(edgeA_p0, edgeA_p1, edgeB_p0, edgeB_p1, &edgeA_s, &edgeB_s, &normal, &cA, &cB))
			passed = false; // edges are parallel

		// 2. test
		if ((edgeA_s < 0.0f) || (edgeA_s > edgeALength) || (edgeB_s < 0.0f) || (edgeB_s > edgeBLength))
			passed = false; // closest points are not on edges

		XMVECTOR lineA_r = edgeA_p0 + XMVectorScale(cA, edgeA_s);
		XMVECTOR lineB_r = edgeB_p0 + XMVectorScale(cB, edgeB_s);
		XMVECTOR r = lineB_r - lineA_r;
		normal = XMVector3Normalize(r); // this normal is pointing in right direction
		XMVECTOR contactPos = lineA_r;

		// project A center, B center and B collision point (lineB_r) onto normal, with center lineA_r.
		float AC_projection = XMVectorGetX(XMVector3Dot(normal, AC_in_BLS - lineA_r));
		float BC_projection = XMVectorGetX(XMVector3Dot(normal, centerOfMeshB - lineA_r));
		float lineB_r_projection = XMVectorGetX(XMVector3Dot(normal, lineB_r - lineA_r));
		float penetrationDepth_AinB = XMVectorGetX(XMVector3Length(r));

		// 3. test -> three very important conditions to check, think about them (drawing helps).
		bool penetrationIsPositive = false;
		if ((AC_projection < 0.0f) || (BC_projection > 0.0f) || (lineB_r_projection > AC_projection))
		{// Here is extra tolerance if point is outside of mesh but closer than mInterpenetrationEpsilon.
			penetrationIsPositive = true;
			if (penetrationDepth_AinB >= 0)
				passed = false;
		}

		if (passed)
		{// keep the point
			XMVECTOR newPosition = XMVector3TransformCoord(contactPos, fromLToL[1]); // contactPos was in B's local space!
			XMVECTOR newNormal = XMVector3TransformNormal(normal, fromLToL[1]); // normal was in B's local space!

			contactPt->mInterpenetration = penetrationDepth_AinB * ((penetrationIsPositive) ? 1.0f : -1.0f); // new interpenetration value
			XMStoreFloat3(&contactPt->mContactPoint, newPosition); // new position
			XMStoreFloat3(&contactPt->mContactNormal, newNormal * ((penetrationIsPositive) ? -1.0f : 1.0f)); // new normal

			XMVECTOR newPositionW = XMVector3TransformCoord(contactPos, world[1]);
			XMVECTOR A_bodyPosition = XMLoadFloat3(&colDat->mSkinInstance[0]->mRigidBody->mPosition);
			XMVECTOR A_bodyPointRelativePos = newPositionW - A_bodyPosition;
			XMStoreFloat3(&contactPt->mRelativeContactPosition[0], A_bodyPointRelativePos);
			XMVECTOR B_bodyPosition = XMLoadFloat3(&colDat->mSkinInstance[1]->mRigidBody->mPosition);
			XMVECTOR B_bodyPointRelativePos = newPositionW - B_bodyPosition;
			XMStoreFloat3(&contactPt->mRelativeContactPosition[1], B_bodyPointRelativePos);
		}
		else
			contactPt->mInterpenetration = D3D11_FLOAT32_MAX; // no interpenetration
	}
}

inline void CollisionResolver::UpdateContactsAndJoints_vel(CollisionDetector *colDet, const RigidBody *rgdB1, const RigidBody *rgdB2, float dt)
{
	// For each collision skin
	for (auto dataIt = colDet->mColData.begin(); dataIt != colDet->mColData.end();)
	{
		if (((*dataIt)->mSkinInstance[0]->mRigidBody == rgdB1) || ((*dataIt)->mSkinInstance[1]->mRigidBody == rgdB1) ||
			((*dataIt)->mSkinInstance[0]->mRigidBody == rgdB2) || ((*dataIt)->mSkinInstance[1]->mRigidBody == rgdB2))
		{
			int index = (*dataIt)->mContacts.GetFirstIndex();
			while (index != HostedList_End)
			{
				const CollisionSkin_RigidBody_Instance *skin_A = (*dataIt)->mSkinInstance[0];
				const CollisionSkin_RigidBody_Instance *skin_B = (*dataIt)->mSkinInstance[1];
				CalculateContactVelocityData(&(*dataIt)->mContacts[index], skin_A, skin_B, dt);
				index = (*dataIt)->mContacts.GetNextIndex(index);
			}
		}
		++dataIt;
	}
	// For each joint
	for (auto jointIt = mJoints.begin(); jointIt != mJoints.end(); ++jointIt)
	{
		if ((rgdB1 == (*jointIt)->mParent && rgdB1->IsMovable()) ||
			(rgdB2 == (*jointIt)->mParent && rgdB2->IsMovable()) ||
			(rgdB1 == (*jointIt)->mChild && rgdB1->IsMovable()) ||
			(rgdB2 == (*jointIt)->mChild && rgdB2->IsMovable()))
		{
			(*jointIt)->CalculateVeclocityErrorSeverity(dt);
		}
	}
}

void CollisionResolver::AdjustVelocities(CollisionDetector *colDet, float dt)
{
	float max;
	UINT numContacts = colDet->mColData.size();
	Contact *cPt;
	Joint *jPt;
	XMVECTOR velocityChange[2], rotationChange[2];
	const CollisionSkin_RigidBody_Instance *A, *B;

	// Iteratively resolve closing velocities in order of severity.
	UINT velocityIterationsUsed = 0;
	while (velocityIterationsUsed < mVelocityIterations)
	{
		//********************************************************************
		//	Find biggest closing velocity
		//********************************************************************
		max = 0;
		cPt = 0;
		for (auto dataIt = colDet->mColData.begin(); dataIt != colDet->mColData.end();)
		{
			int index = (*dataIt)->mContacts.GetFirstIndex();
			while (index != HostedList_End)
			{
				if ((*dataIt)->mContacts[index].mDesiredDeltaVelocity > max)
				{
					max = (*dataIt)->mContacts[index].mDesiredDeltaVelocity;
					cPt = &(*dataIt)->mContacts[index];
					A = (*dataIt)->mSkinInstance[0];
					B = (*dataIt)->mSkinInstance[1];
				}
				index = (*dataIt)->mContacts.GetNextIndex(index);
			}
			++dataIt;
		}
		if (cPt != 0)
		{
			// Resolve the closing velocities.
			ApplyVelocityChange(cPt, A, B, velocityChange, rotationChange, dt);

			//	We update contacts and joints.
			UpdateContactsAndJoints_vel(colDet, A->mRigidBody, B->mRigidBody, dt);
		}
		//********************************************************************
		//	Find joint with the biggest severity
		//********************************************************************
		max = 0;
		jPt = 0;
		for (auto jointIt = mJoints.begin(); jointIt != mJoints.end(); ++jointIt)
		{
			if ((*jointIt)->GetVelErrorSeverity() > max)
			{
				max = (*jointIt)->GetVelErrorSeverity();
				jPt = (*jointIt);
			}
		}
		if (cPt == 0 && jPt == 0)
			break;

		if (jPt != 0)
		{
			// Resolve the joint error
			jPt->ApplyVelocityChange(mInterpenetrationRelaxation, dt);

			//	We update contacts and joints.
			UpdateContactsAndJoints_vel(colDet, jPt->mParent, jPt->mChild, dt);
		}

		velocityIterationsUsed++;
	}
}

void CollisionResolver::ApplyVelocityChange(Contact *contact, const CollisionSkin_RigidBody_Instance *A, const CollisionSkin_RigidBody_Instance *B, XMVECTOR velocityChange[2], XMVECTOR rotationChange[2], float dt)
{
	XMMATRIX inverseInertiaTensor_A = XMLoadFloat3x3(&A->mRigidBody->mInverseInertiaTensorWorld);
	XMMATRIX inverseInertiaTensor_B = XMLoadFloat3x3(&B->mRigidBody->mInverseInertiaTensorWorld);
	XMMATRIX contactToWorld = XMLoadFloat4x4(&contact->mContactToWorld);
	XMMATRIX worldToContact = XMMatrixTranspose(contactToWorld);
	XMMATRIX worldA = XMLoadFloat4x4(&A->mRigidBody->mTransformMatrix);
	XMMATRIX worldB = XMLoadFloat4x4(&B->mRigidBody->mTransformMatrix);

	// in world coordinates
	XMVECTOR relativeContactPos_A = XMLoadFloat3(&contact->mRelativeContactPosition[0]);
	XMVECTOR relativeContactPos_B = XMLoadFloat3(&contact->mRelativeContactPosition[1]);

	//********************************************************************
	// Build the matrix to convert contact impulse to change in velocity
	// in world coordinates.
	//********************************************************************
	XMMATRIX deltaVelWorld = MathHelper::ZeroMatrix();

	// Velocity change for A
	if (A->mRigidBody->mIsMovable)
	{
		// This is in world space.
		XMMATRIX skew_A = MathHelper::SkewSymmetric(relativeContactPos_A);
		XMMATRIX deltaVelWorld_A = skew_A * XMMatrixScaling(-1.0f, -1.0f, -1.0f); // this is inverse cross product (reason for XMMatrixScaling(-1)).
		deltaVelWorld_A *= inverseInertiaTensor_A;
		deltaVelWorld_A *= skew_A; // normal cross product
		deltaVelWorld += deltaVelWorld_A;

		// Combine the linear motion with the angular motion we already have.
		deltaVelWorld._11 += A->mRigidBody->mInverseMass;
		deltaVelWorld._22 += A->mRigidBody->mInverseMass;
		deltaVelWorld._33 += A->mRigidBody->mInverseMass;
	}

	// Velocity change for B
	if (B->mRigidBody->mIsMovable)
	{
		// This is in world space.
		XMMATRIX skew_B = MathHelper::SkewSymmetric(relativeContactPos_B);
		XMMATRIX deltaVelWorld_B = skew_B * XMMatrixScaling(-1.0f, -1.0f, -1.0f); // this is inverse cross product (reason for XMMatrixScaling(-1)).
		deltaVelWorld_B *= inverseInertiaTensor_B;
		deltaVelWorld_B *= skew_B; // normal cross product
		deltaVelWorld += deltaVelWorld_B;

		// Combine the linear motion with the angular motion we already have.
		deltaVelWorld._11 += B->mRigidBody->mInverseMass;
		deltaVelWorld._22 += B->mRigidBody->mInverseMass;
		deltaVelWorld._33 += B->mRigidBody->mInverseMass;
	}
	deltaVelWorld._44 = 1.0f;

	// Do a change of basis to convert into contact coordinates.
	XMMATRIX deltaVel = contactToWorld * deltaVelWorld * worldToContact;
	XMVECTOR det; // Invert to get the impulse needed per unit velocity.
	XMMATRIX impulseMatrix = XMMatrixInverse(&det, deltaVel);
	// Find the target velocities to kill.
	XMVECTOR velKill = XMVectorSet(-contact->mContactVelocity.x, contact->mDesiredDeltaVelocity, -contact->mContactVelocity.z, 0.0f);
	// Find the impulse to kill target velocities.
	XMVECTOR impulseContact = XMVector3TransformNormal(velKill, impulseMatrix);

	// Impulse that is created from forces in delta time. Impulse from gravity for example.
	XMVECTOR lastUpdateAcc[2];
	lastUpdateAcc[0] = XMLoadFloat3(&A->mRigidBody->mLastUpdateAcceleration);
	lastUpdateAcc[1] = XMLoadFloat3(&B->mRigidBody->mLastUpdateAcceleration);
	XMVECTOR closingAcc = lastUpdateAcc[contact->mContactPointHolderSkinI] - lastUpdateAcc[1 - contact->mContactPointHolderSkinI];
	closingAcc = XMVector3TransformNormal(closingAcc, worldToContact);
	XMVECTOR velocityFromAcc = closingAcc*dt;
	XMVECTOR impulseFromForces = XMVector3TransformNormal(velocityFromAcc, impulseMatrix);

	// Check for exceeding friction.
	float friction = sqrt(A->mRigidBody->mFrictionCoefficient * B->mRigidBody->mFrictionCoefficient);
	XMFLOAT3 impulseC;
	XMStoreFloat3(&impulseC, impulseContact);
	float planarImpulse = sqrt(impulseC.x*impulseC.x + impulseC.z*impulseC.z);
	if ((planarImpulse > impulseC.y * friction) && (planarImpulse > -XMVectorGetY(impulseFromForces) * friction))
	{
		// We need to use dynamic friction.
		impulseC.x /= planarImpulse;
		impulseC.z /= planarImpulse;


		impulseC.y =
			deltaVel._21 * friction * impulseC.x +
			deltaVel._22 +
			deltaVel._23 * friction * impulseC.z;
		impulseC.y = contact->mDesiredDeltaVelocity / impulseC.y;
		//impulseC.y = contact->mDesiredDeltaVelocity / deltaVel._22;


		impulseC.x *= friction * impulseC.y;
		impulseC.z *= friction * impulseC.y;
		impulseContact = XMLoadFloat3(&impulseC);
	}

	// Transform impulse to world space.
	XMVECTOR impulseContactWorld = XMVector3TransformNormal(impulseContact*mVelocityRelaxation, contactToWorld);

	//********************************************************************
	// Applying the impulse
	//********************************************************************
	velocityChange[0] = velocityChange[1] = rotationChange[0] = rotationChange[1] = XMVectorZero();

	// Calculate velocity and rotation change for A.
	if (A->mRigidBody->mIsMovable)
	{
		XMVECTOR impulse = impulseContactWorld * ((contact->mContactPointHolderSkinI == 0) ? 1.0f : -1.0f);
		velocityChange[0] = impulse * A->mRigidBody->mInverseMass;
		XMVECTOR impulsiveTorque = XMVector3Cross(relativeContactPos_A, impulse);
		rotationChange[0] = XMVector3TransformNormal(impulsiveTorque, inverseInertiaTensor_A);

		XMVECTOR linVelocity = XMLoadFloat3(&A->mRigidBody->mLinVelocity) + velocityChange[0];
		XMVECTOR angVelocity = XMLoadFloat3(&A->mRigidBody->mAngVelocity) + rotationChange[0];

		XMStoreFloat3(&A->mRigidBody->mLinVelocity, linVelocity);
		XMStoreFloat3(&A->mRigidBody->mAngVelocity, angVelocity);
	}

	// Calculate velocity and rotation change for B.
	if (B->mRigidBody->mIsMovable)
	{
		XMVECTOR impulse = impulseContactWorld * ((contact->mContactPointHolderSkinI == 0) ? -1.0f : 1.0f);
		velocityChange[1] = impulse * B->mRigidBody->mInverseMass;
		XMVECTOR impulsiveTorque = XMVector3Cross(relativeContactPos_B, impulse);
		rotationChange[1] = XMVector3TransformNormal(impulsiveTorque, inverseInertiaTensor_B);

		XMVECTOR linVelocity = XMLoadFloat3(&B->mRigidBody->mLinVelocity) + velocityChange[1];
		XMVECTOR angVelocity = XMLoadFloat3(&B->mRigidBody->mAngVelocity) + rotationChange[1];

		XMStoreFloat3(&B->mRigidBody->mLinVelocity, linVelocity);
		XMStoreFloat3(&B->mRigidBody->mAngVelocity, angVelocity);
	}
}
