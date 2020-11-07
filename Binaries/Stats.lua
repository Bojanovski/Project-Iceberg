
timer1 = nil
timer2 = nil

infoBmp = nil
grayColorTex = nil
mainFont = nil
text1 = nil
text2 = nil
text3 = nil
text4 = nil

-- Called right after the script is loaded.
function Start()

	grayColorTex = texture.load("Textures\\grayGUI.dds");
	infoBmp = bitmap.add(grayColorTex, 0.0, 0.8, 0.3, 0.2)
	mainFont = font.load("Arial")
	text1 = text.add(mainFont, "", 0.01, 0.83, 0.6, 0.9, 0.9, 0.9)
	text2 = text.add(mainFont, "", 0.01, 0.87, 0.6, 0.9, 0.9, 0.9)
	text3 = text.add(mainFont, "", 0.01, 0.91, 0.6, 0.9, 0.9, 0.9)
	text4 = text.add(mainFont, "", 0.01, 0.95, 0.6, 0.9, 0.9, 0.9)

end

-- Called before every frame is rendered.
function Update(dt)

	width, height = getClientWidthHeight()
	fps, mmspf, cpuUsage, ramUsage = getProcessAndSystemData()
	bitmap.update(infoBmp, grayColorTex, 0.0, 0.8, 255/width, 0.2)
	text.update(text1, "- FPS: " .. fps)
	text.update(text2, "- Frame time: " .. mmspf .. " (ms)")
	text.update(text3, "- CPU usage: " .. cpuUsage .. "%")
	text.update(text4, "- RAM usage: " .. ramUsage .. " MB")

end

-- Called before the script is finished.
function Stop()

	texture.unload(grayColorTex)
	font.unload(mainFont)
	text.remove(text1)
	text.remove(text2)
	text.remove(text3)

end