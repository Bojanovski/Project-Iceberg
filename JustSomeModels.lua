
blur = false
vrijeme1 = 0
vrijeme2 = 0
model1Pt = nil
model2Pt = nil
tankObjectPt = nil
setObjPt = nil

gx = 0
gy = -9.81
gz = 0
m = 1
c = 1

v0x = 25
v0y = 0
v0z = 0

p0x = 0
p0y = 3
p0z = 0

c2x = -m*v0x/c
c2y = -m*v0y/c
c2z = -m*v0z/c

c1x = p0x - c2x
c1y = p0y - c2y
c1z = p0z - c2z

-- Called right after the script is loaded.
function Start()
	model1Pt = LoadModel("Models\\ScorpionTank.obj")
	model2Pt = LoadModel("Models\\plane.obj")
	tankObjectPt = object3D.add(model1Pt, -10.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.5, 0.5, 0.5, false, false)
	setObjPt = object3D.add(model2Pt, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.5, 0.5, 0.5, false, true)
end

-- Called before every frame is rendered.
function Update(dt)

	vrijeme2 = vrijeme2 + dt

	local fDown = input.isKeyDown(0x46)
	if (fDown > 0.0) then
		vrijeme2 = 0
		SetNextFrameProperties(fDown + 1, true)
	else
		SetNextFrameProperties(0, true)
	end

	
	object3D.update(tankObjectPt, -10.0*math.cos(vrijeme2/5.0), 0.0, -10.0*math.sin(vrijeme2/5.0), 0.0, 0.0, 0.0, 1.0, 0.5, 0.5, 0.5)
	

	--object3D.update(tankObjectPt,
	-- c1x + c2x*math.exp(-c*vrijeme2/m) + vrijeme2*gx*m/c,
	-- c1y + c2y*math.exp(-c*vrijeme2/m) + vrijeme2*gy*m/c,
	-- c1z + c2z*math.exp(-c*vrijeme2/m) + vrijeme2*gz*m/c,
	-- 0.0, 0.0, 0.0, 1.0, 0.5, 0.5, 0.5)


	local gDown = input.isKeyDown(0x47) 
	object3D.setWireframe(tankObjectPt, (gDown > 0.0))

end

-- Called before the script is finished.
function Stop()

end