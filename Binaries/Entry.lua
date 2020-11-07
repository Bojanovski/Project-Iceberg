
-- Called right after the script is loaded.
function Start()

	camera.lookAt(14.0, 8.0, 14.0, 0.0, 3.0, 0.0);
	mainFont = font.load("Bickham Script Pro Semibold")
	text1 = text.add(mainFont, "", 0.02, 0.02, 1.9, 0.9, 0.9, 0.9)

	--StartAnotherScript("Physics1.lua")
	--StartAnotherScript("JustSomeModels.lua")
	--StartAnotherScript("Animations1.lua")
	--StartAnotherScript("ClothSimulation1.lua")
	StartAnotherScript("ClothSimulation2.lua")
	--StartAnotherScript("Particles.lua")
	--StartAnotherScript("TextureGenerating.lua")

	--StartAnotherScript("Stats.lua")

end
 
-- Called before every frame is rendered.
function Update(dt)

end

-- Called before the script is finished.
function Stop()

	font.unload(mainFont)
	text.remove(text1)

end
