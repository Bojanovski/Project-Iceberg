
time = 0
planetPt = 0
seed = 12345

-- Called right after the script is loaded.
function Start()
	camera.lookAt(14.0, 8.0, 14.0, 0.0, 3.0, 0.0);
	mainFont = font.load("Bickham Script Pro Semibold")
	text1 = text.add(mainFont, "Planet: " .. seed, 0.02, 0.02, 1.9, 0.9, 0.9, 0.9)
	time = 0

	planetPt = pcg.generatePlanet(seed)

	pillarModelPt = LoadModel("Models\\Earth.obj")
	dt = 0.21
	t = 0.0
	for j=0, 0 do
		pillarObjectPt = object3D.add(pillarModelPt, t, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.05, 0.05, 0.05, false, false)
		t = t + dt
	end
end
 
-- Called before every frame is rendered.
function Update(dt)
		time = time + dt
end

-- Called before the script is finished.
function Stop()
	font.unload(mainFont)
	text.remove(text1)
end