
sparksPt = nil
firePt = nil
pillarModelPt = nil
pillarObjectPt = nil
timer = 0

-- Called right after the script is loaded.
function Start()
	totalTime = 0
	
	mainFont = font.load("Arial")
	text1 = text.add(mainFont, "Press 'F' button.", 0.01, 0.93, 0.6, 0.9, 0.9, 0.9)
	
	sparksPt1 = particles.addSparksSource()
	sparksPt2 = particles.addSparksSource()
	sparksPt3 = particles.addSparksSource()
	sparksPt4 = particles.addSparksSource()

	firePt = particles.addFireSource()
	particles.updateSource(firePt, 0, 7.5, 0)

	pillarModelPt = LoadModel("Models\\Pillar.obj")
	planeModelPt = LoadModel("Models\\Plane2.obj")
	pillarObjectPt = object3D.add(pillarModelPt, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.05, 0.05, 0.05, false, false)

	planeObjectPt1 = object3D.add(planeModelPt, 10.0, -0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 10.0, 10.0, 10.0, false, false)
	planeObjectPt2 = object3D.add(planeModelPt, 0.0, -0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 10.0, 10.0, 10.0, false, false)
	planeObjectPt3 = object3D.add(planeModelPt, -10.0, -0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 10.0, 10.0, 10.0, false, false)

	planeObjectPt4 = object3D.add(planeModelPt, 10.0, -0.0, 10.0, 0.0, 0.0, 0.0, 1.0, 10.0, 10.0, 10.0, false, false)
	planeObjectPt5 = object3D.add(planeModelPt, 0.0, -0.0, 10.0, 0.0, 0.0, 0.0, 1.0, 10.0, 10.0, 10.0, false, false)
	planeObjectPt6 = object3D.add(planeModelPt, -10.0, -0.0, 10.0, 0.0, 0.0, 0.0, 1.0, 10.0, 10.0, 10.0, false, false)

	planeObjectPt7 = object3D.add(planeModelPt, 10.0, -0.0, -10.0, 0.0, 0.0, 0.0, 1.0, 10.0, 10.0, 10.0, false, false)
	planeObjectPt8 = object3D.add(planeModelPt, 0.0, -0.0, -10.0, 0.0, 0.0, 0.0, 1.0, 10.0, 10.0, 10.0, false, false)
	planeObjectPt9 = object3D.add(planeModelPt, -10.0, -0.0, -10.0, 0.0, 0.0, 0.0, 1.0, 10.0, 10.0, 10.0, false, false)
end

-- Called before every frame is rendered.
function Update(dt)

	local fDown = input.isKeyDown(0x46)
	if (fDown > 0.0) then
		totalTime = totalTime + dt
		local xPos = -10.0*math.sin(totalTime/5.0)

		local offset = 1.5
		local slide = 1.2*math.sin(totalTime * 100.0)
		particles.updateSource(sparksPt1, xPos + slide, 0.01, -offset)
		particles.updateSource(sparksPt2, xPos + slide, 0.01, offset)
		particles.updateSource(sparksPt3, xPos + offset, 0.01, slide)
		particles.updateSource(sparksPt4, xPos - offset, 0.01, slide)

		particles.updateSource(firePt, xPos, 7.5, 0)
		object3D.update(pillarObjectPt, xPos, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.05, 0.05, 0.05)
	else
		particles.updateSource(sparksPt1, 0, -100, 0)
		particles.updateSource(sparksPt2, 0, -100, 0)
		particles.updateSource(sparksPt3, 0, -100, 0)
		particles.updateSource(sparksPt4, 0, -100, 0)
	end
	
	timer = timer + dt

end

-- Called before the script is finished.
function Stop()

end