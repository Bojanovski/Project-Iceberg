
vrijeme = 0
boxMeshPt = nil
cylinderMeshPt = nil

-- Movable rigid bodies
numberOfRigidBodies = 5 --    <<<<<<< OVU VARIJABLU ZELITE MJENJATI!! :-) :-) >>>>>>>
rgdBodyPt = {}    -- new arrays
meshObjPt = {}
for i=1, numberOfRigidBodies do
	rgdBodyPt[i] = nil
	meshObjPt[i] = nil
end

-- Ground
groundMeshPt = nil
groundRgdBodyPt = nil
groundMeshObjPt = nil

-- Called right after the script is loaded.
function Start()
	
	math.randomseed(1234)
	boxMeshPt = GenerateMesh_Box(1.0, 2.0, 1.0)
	cylinderMeshPt = GenerateMesh_Cylinder(2.0, 0.8, 0.8, 3, 1)
	for i=1, numberOfRigidBodies do
		local tempPt = nil
		if (math.random() > 0.5) then
			tempPt = boxMeshPt
		else
			tempPt = cylinderMeshPt
		end
		rgdBodyPt[i] = AddRigidBody(tempPt, true, -2.0 + math.random()*5.0, 10.5 + i*2.0, -2.0 + math.random()*5.0)
		pX, pY, pZ, rX, rY, rZ, rW = GetRigidBodyCoords(rgdBodyPt[i])
		meshObjPt[i] = object3D.add(tempPt, pX, pY, pZ, rX, rY, rZ, rW, 1.0, 1.0, 1.0, false, false)
	end

	-- Ground
	groundMeshPt = GenerateMesh_Box(20.0, 2.0, 20.0)
	groundRgdBodyPt = AddRigidBody(groundMeshPt, false, 0.0, -1.0, 0.0)
	pX, pY, pZ, rX, rY, rZ, rW = GetRigidBodyCoords(groundRgdBodyPt)
	groundMeshObjPt = object3D.add(groundMeshPt, pX, pY, pZ, rX, rY, rZ, rW, 1.0, 1.0, 1.0, false, false)
end

-- Called before every frame is rendered.
function Update(dt)

	vrijeme = vrijeme + dt


	SetNextFrameProperties(0, true)


	for i=1, numberOfRigidBodies do
		pX, pY, pZ, rX, rY, rZ, rW = GetRigidBodyCoords(rgdBodyPt[i])
		object3D.update(meshObjPt[i], pX, pY, pZ, rX, rY, rZ, rW, 1.0, 1.0, 1.0)
	end

end

-- Called before the script is finished.
function Stop()
	
end