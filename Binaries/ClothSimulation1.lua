
diffuseTex = nil
cloth1 = nil
timer1 = nil
timer2 = nil
windFactor = nil
grayColorTex = nil

-- Called right after the script is loaded.
function Start()

	grayColorTex = texture.load("Textures\\grayGUI.dds");

	local uc = 10
	local vc = 10

	cloth1 = cloth.add(grayColorTex, uc, vc, 6.0, 6.5, 0.02, 5.0)
	cloth.setWindGravity(cloth1, 0, 0, 0, 0, -10, 0)

	local scaleY = 0.65
	local scaleZ = 0.5
	local x = -3.5
	local y = 0.8
	local z = 0.0

	for j=0, uc-1 do
		for k=0, vc-1 do
			local movable
			if (j % uc == 0) then movable = false else movable = true end
			cloth.fixControlPoint(cloth1, k*uc + j, x + 0.01*math.sin(j % uc), y + -j * scaleY, z + k*scaleZ, movable)
		end
	end

		

	-- scenery
	scMesh1 = GenerateMesh_Box(1.0, 7.5, 1.0)
	scMeshObj1 = object3D.add(scMesh1, -3.5, -2.3, z -1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, false, false)
	scMeshObj2 = object3D.add(scMesh1, -3.5, -2.3, z + 5.5, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, false, false)

	scMesh2 = GenerateMesh_Box(0.8, 0.8, 7.0)
	scMeshObj3 = object3D.add(scMesh2, x, y + 0.4, z + 2.3, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, false, false)

	-- ground
	groundMeshPt = GenerateMesh_Box(30.0, 2.0, 30.0)
	groundMeshObjPt = object3D.add(groundMeshPt, 0.0, -7.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, false, false)

	timer1 = 0
	timer2 = 0
	windFactor = 0

end

-- Called before every frame is rendered.
function Update(dt)
	
	timer1 = timer1 + dt
	timer2 = timer2 + dt
	--camera.lookAt(17.0*math.sin(timer*0.2), 4.0, -17.0*math.cos(timer*0.2), 0.0, -3.0, 0.0);
	
	windFactor = math.pow(timer2/7, 4)
	if (windFactor > 1.0) then windFactor = 1.0 end

	cloth.setWindGravity(cloth1, 60*windFactor, 0, 0, 0, -10, 0)
	if (timer2 > 10.0) then 
		timer2 = 0.0
	end

end

-- Called before the script is finished.
function Stop()

	--texture.remove(grayColorTex)

end