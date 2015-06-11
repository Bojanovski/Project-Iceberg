
vrijeme = 0

-- Called right after the script is loaded.
function Start()

	camera.lookAt(18.0, 8.0, 18.0, -3.0, -2.0, 0.0);
	mainFont = font.load("Bickham Script Pro Semibold")
	text1 = text.add(mainFont, "", 0.02, 0.02, 1.9, 0.9, 0.9, 0.9)

	--StartAnotherScript("Physics1.lua")
	--StartAnotherScript("Physics2.lua")
	--StartAnotherScript("PhysicsJoints1.lua")
	--StartAnotherScript("PhysicsJoints2.lua")
	--StartAnotherScript("JustSomeModels.lua")
	StartAnotherScript("Animations1.lua")
	--StartAnotherScript("ClothSimulation1.lua")
	--StartAnotherScript("ClothSimulation2.lua")

	StartAnotherScript("Stats.lua")

end

-- Called before every frame is rendered.
function Update(dt)

		vrijeme = vrijeme + dt

end

-- Called before the script is finished.
function Stop()

	font.remove(mainFont)
	text.remove(text1)


end