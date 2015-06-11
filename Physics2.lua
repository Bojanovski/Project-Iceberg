
vrijeme = 0
capsuleMeshPt = nil
capsuleRgdBodyPt = nil
capsuleMeshCSPt = nil
capsuleMeshObjPt = nil

-- Ground
groundMeshPt = nil
groundRgdBodyPt = nil
groundMeshCSPt = nil
groundMeshObjPt = nil

-- Called right after the script is loaded.
function Start()
	
	r = 0.8
	h = 2.0
	capsuleMeshPt = GenerateMesh_Capsule(r, h, 6, 1)
	znj = physics.addMeshCollisionSkin(capsuleMeshPt)
	capsuleMeshCSPt = physics.addCapsuleCollisionSkin(h, r)
	capsuleRgdBodyPt = physics.addRigidBody(capsuleMeshCSPt, true, true, 0.0, -1.0, 0.0)
	capsuleMeshObjPt = object3D.add(capsuleMeshPt, 0, 5, 0, 0, 0, 0, 1, 1.0, 1.0, 1.0, false, false)
	forceGenerator = physics.addForceGenerator(capsuleRgdBodyPt)

	-- Ground
	groundMeshPt = GenerateMesh_Box(20.0, 2.0, 20.0)
	groundMeshCSPt = physics.addMeshCollisionSkin(groundMeshPt)
	groundRgdBodyPt = physics.addRigidBody(groundMeshCSPt, false, false, 0.0, -6.0, 0.0)
	pX, pY, pZ, rX, rY, rZ, rW = physics.getRigidBodyCoords(groundRgdBodyPt)
	groundMeshObjPt = object3D.add(groundMeshPt, pX, pY, pZ, rX, rY, rZ, rW, 1.0, 1.0, 1.0, false, false)
end

-- Called before every frame is rendered.
function Update(dt)

	vrijeme = vrijeme + dt

	pX, pY, pZ, rX, rY, rZ, rW = physics.getRigidBodyCoords(capsuleRgdBodyPt)
	object3D.update(capsuleMeshObjPt, pX, pY, pZ, rX, rY, rZ, rW, 1.0, 1.0, 1.0)

	torque = 10
	local fDown = input.isKeyDown(0x46)
	local gDown = input.isKeyDown(0x47)
	local iDown = input.isKeyDown(0x49)
	local kDown = input.isKeyDown(0x4B)
	if (fDown > 0.0) then physics.addTorque(forceGenerator, 0.0, torque, 0.0) end
	if (gDown > 0.0) then physics.addTorque(forceGenerator, 0.0, -torque, 0.0) end
	if (iDown > 0.0) then physics.addTorque(forceGenerator, 0.0, 0.0, torque) end
	if (kDown > 0.0) then physics.addTorque(forceGenerator, 0.0, 0.0, -torque) end

end

-- Called before the script is finished.
function Stop()
	
end