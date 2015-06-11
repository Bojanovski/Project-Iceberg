
vrijeme = 0

model1Pt = nil
groundMeshPt = nil
objectPt = nil
groundMeshObjPt = nil

isSimulating = false

-- Called right after the script is loaded.
function Start()
	mainFont = font.load("Arial")
	text1 = text.add(mainFont, "PRESS 3 FOR RAGDOLL XD", 0.02, 0.02, 1.8, 0.9, 0.9, 0.9)

	model1Pt = LoadModel("Models\\Khnum.X")
	objectPt = object3D.add(model1Pt, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.2, 0.2, 0.2, false, false)
	object3D.playAnimation(objectPt, "Walk")

	-- Ground
	groundMeshPt = GenerateMesh_Box(20.0, 5.9, 20.0)
	groundMeshCSPt = physics.addMeshCollisionSkin(groundMeshPt)
	groundRgdBodyPt = physics.addRigidBody(groundMeshCSPt, false, false, 0.0, -3.0, 0.0)
	pX, pY, pZ, rX, rY, rZ, rW = physics.getRigidBodyCoords(groundRgdBodyPt)
	groundMeshObjPt = object3D.add(groundMeshPt, pX, pY, pZ, rX, rY, rZ, rW, 1.0, 1.0, 1.0, false, false)
end

-- Called before every frame is rendered.
function Update(dt)

	vrijeme = vrijeme + dt/3

	local gDown = input.isKeyDown(0x47) 
	object3D.setWireframe(objectPt, (gDown > 0.0))

	local n1Down = input.isKeyDown(0x31) 
	local n2Down = input.isKeyDown(0x32) 
	local n3Down = input.isKeyDown(0x33) 

	if (n1Down > 0.0) then
		object3D.playAnimation(objectPt, "Anim1")
		isSimulating = false
	end
	if (n2Down > 0.0) then
		object3D.playAnimation(objectPt, "Anim2")
		isSimulating = false
	end
	if (n3Down > 0.0 and isSimulating == false) then
		object3D.startSimulation(objectPt)
		isSimulating = true
	end

	object3D.update(objectPt, -10.0*math.cos(vrijeme), 0.0, -10.0*math.sin(vrijeme),
	 0.0, math.cos(vrijeme/2.0), 0.0, math.sin(vrijeme/2.0),
	 0.2, 0.2, 0.2)

end

-- Called before the script is finished.
function Stop()

end