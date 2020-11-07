
vrijeme = 0
boxMeshPt = nil
boxMeshCSPt = nil

boxRgdBodyPt1 = nil
boxMeshObjPt1 = nil

boxRgdBodyPt2 = nil
boxMeshObjPt2 = nil

boxRgdBodyPt3 = nil
boxMeshObjPt3 = nil

boxRgdBodyPt4 = nil
boxMeshObjPt4 = nil

-- Ground
groundMeshPt = nil
groundRgdBodyPt = nil
groundMeshCSPt = nil
groundMeshObjPt = nil

-- Called right after the script is loaded.
function Start()

	boxMeshPt = GenerateMesh_Box(2.0, 1.0, 1.0)
	boxMeshCSPt = physics.addMeshCollisionSkin(boxMeshPt)
	boxMeshPt2 = GenerateMesh_Box(2.0, 2.0, 2.0)
	boxMeshCSPt2 = physics.addMeshCollisionSkin(boxMeshPt2)

	boxRgdBodyPt1 = physics.addRigidBody(boxMeshCSPt, false, true, 0.0, 0.0, 0.0)
	boxMeshObjPt1 = object3D.add(boxMeshPt, 0, 5, 0, 0, 0, 0, 1, 1.0, 1.0, 1.0, false, false)
	--forceGenerator = physics.addForceGenerator(boxRgdBodyPt1)

	boxRgdBodyPt2 = physics.addRigidBody(boxMeshCSPt, true, true, -2.0, 0.0, 0.0)
	boxMeshObjPt2 = object3D.add(boxMeshPt, 0, 5, 0, 0, 0, 0, 1, 1.0, 1.0, 1.0, false, false)
	forceGenerator = physics.addForceGenerator(boxRgdBodyPt2)

	--boxRgdBodyPt3 = physics.addRigidBody(boxMeshCSPt, true, true, -4.0, 0.0, 0.0)
	--boxMeshObjPt3 = object3D.add(boxMeshPt, 0, 5, 0, 0, 0, 0, 1, 1.0, 1.0, 1.0, false, false)
	--forceGenerator = physics.addForceGenerator(boxRgdBodyPt3)

	--boxRgdBodyPt4 = physics.addRigidBody(boxMeshCSPt, true, true, -6.0, 0.0, 0.0)
	--boxMeshObjPt4 = object3D.add(boxMeshPt, 0, 5, 0, 0, 0, 0, 1, 1.0, 1.0, 1.0, false, false)
	--forceGenerator = physics.addForceGenerator(boxRgdBodyPt4)

	physics.addUniversalJoint(boxRgdBodyPt1, boxRgdBodyPt2, -1.0, 0.0, 0.0, 1.5, 1.5)
	--physics.addUniversalJoint(boxRgdBodyPt2, boxRgdBodyPt3, -3.0, 0.0, 0.0, 1.5, 1.5)
	--physics.addUniversalJoint(boxRgdBodyPt3, boxRgdBodyPt4, -5.0, 0.0, 0.0, 1.5, 1.5)

	-- Ground
	groundMeshPt = GenerateMesh_Box(2.0, 2.0, 20.0)
	groundMeshCSPt = physics.addMeshCollisionSkin(groundMeshPt)
	groundRgdBodyPt = physics.addRigidBody(groundMeshCSPt, false, true, -2.0, -4.0, 0.0)
	pX, pY, pZ, rX, rY, rZ, rW = physics.getRigidBodyCoords(groundRgdBodyPt)
	groundMeshObjPt = object3D.add(groundMeshPt, pX, pY, pZ, rX, rY, rZ, rW, 1.0, 1.0, 1.0, false, false)
end

-- Called before every frame is rendered.
function Update(dt)

	vrijeme = vrijeme + dt

	pX, pY, pZ, rX, rY, rZ, rW = physics.getRigidBodyCoords(boxRgdBodyPt1)
	object3D.update(boxMeshObjPt1, pX, pY, pZ, rX, rY, rZ, rW, 1.0, 1.0, 1.0)

	pX, pY, pZ, rX, rY, rZ, rW = physics.getRigidBodyCoords(boxRgdBodyPt2)
	object3D.update(boxMeshObjPt2, pX, pY, pZ, rX, rY, rZ, rW, 1.0, 1.0, 1.0)

	--pX, pY, pZ, rX, rY, rZ, rW = physics.getRigidBodyCoords(boxRgdBodyPt3)
	--object3D.update(boxMeshObjPt3, pX, pY, pZ, rX, rY, rZ, rW, 1.0, 1.0, 1.0)
	
	--pX, pY, pZ, rX, rY, rZ, rW = physics.getRigidBodyCoords(boxRgdBodyPt4)
	--object3D.update(boxMeshObjPt4, pX, pY, pZ, rX, rY, rZ, rW, 1.0, 1.0, 1.0)


	torque = 10
	local fDown = input.isKeyDown(0x46)
	local gDown = input.isKeyDown(0x47)
	local iDown = input.isKeyDown(0x49)
	local kDown = input.isKeyDown(0x4B)
	local hDown = input.isKeyDown(0x48)
	local jDown = input.isKeyDown(0x4A)
	if (fDown > 0.0) then physics.addTorque(forceGenerator, 0.0, torque, 0.0) end
	if (gDown > 0.0) then physics.addTorque(forceGenerator, 0.0, -torque, 0.0) end
	if (iDown > 0.0) then physics.addTorque(forceGenerator, 0.0, 0.0, torque) end
	if (kDown > 0.0) then physics.addTorque(forceGenerator, 0.0, 0.0, -torque) end
	if (hDown > 0.0) then physics.addTorque(forceGenerator, torque, 0.0, 0.0) end
	if (jDown > 0.0) then physics.addTorque(forceGenerator, -torque, 0.0, 0.0) end

end

-- Called before the script is finished.
function Stop()
	
end