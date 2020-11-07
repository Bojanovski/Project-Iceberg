
diffuseTex = nil
cloth1 = nil
timer = nil

-- Called right after the script is loaded.
function Start()
	
	diffuseTex = texture.load("Textures\\cloth_texture.dds")

	local uc = 6
	local vc = 6

	for i=0, 3 do

		cloth1 = cloth.add(diffuseTex, uc, vc, 7.0, 5.0, 0.02, 5.0)
		cloth.setWindGravity(cloth1, 40, 0, 0, 0, -10, 0)

		local scaleY = 0.8
		local x = -3
		local y = -1.7
		local z = -7

		for j=0, uc-1 do
			for k=0, vc-1 do
				local movable
				if (j % uc == 0) then movable = false else movable = true end
				cloth.fixControlPoint(cloth1, k*uc + j, j + x, y + k * scaleY - j*0.25, i*5 + z + math.sin(j) * math.random(5,10)/10, movable)
			end
		end

		

		-- scenery
		scMesh = GenerateMesh_Box(1.0, 9.0, 1.0)
		scMeshObj = object3D.add(scMesh, -3.5, -2.0, i*5 + z, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, false, false)

	end

	-- ground
	groundMeshPt = GenerateMesh_Box(30.0, 2.0, 30.0)
	groundMeshObjPt = object3D.add(groundMeshPt, 0.0, -7.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, false, false)

	timer = 0

end

-- Called before every frame is rendered.
function Update(dt)
	
	timer = timer + dt
	camera.lookAt(27.0*math.sin(timer*0.2), 4.0, -17.0*math.cos(timer*0.2), 0.0, -3.0, 0.0);

end

-- Called before the script is finished.
function Stop()

	texture.unload(diffuseTex)

end