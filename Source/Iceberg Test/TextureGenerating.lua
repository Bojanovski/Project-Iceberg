
time = 0
texx = nil
bitt = nil

function GenerateTexture()
	
	--texx = texture.generatePerlinNoise(10000 * math.random(), 1024, 512)
	--texx = texture.generateRidgedNoise(10000 * math.random(), 1024, 512)
	--texx = texture.generateRidgedPerlinMixed(10000 * math.random(), 1024, 512)
	texx = texture.generateSphericalRidgedPerlinMixed(10000 * math.random(), 1024, 512)

	points = {}
	for i=1, 5000 do
		table.insert(points, math.random()) -- Red
		table.insert(points, math.random()) -- Green
		table.insert(points, math.random()) -- Blue
		table.insert(points, math.random() * 512) -- xPos
		table.insert(points, math.random() * 512) -- yPos
	end
	--texx = texture.generate2DVoronoiTexture(512, 512, points)
end

-- Called right after the script is loaded.
function Start()
	
	GenerateTexture()
	bitt = bitmap.add(texx, 0, 0, 1.0, 1.0);
	vrijeme = 0

end
 
-- Called before every frame is rendered.
function Update(dt)

		time = time + dt
		if (time > 1.0) then
			texture.destroy(texx)
			GenerateTexture()
			bitmap.update(bitt, texx, 0, 0, 1.0, 1.0)
			time = 0
		end

end

-- Called before the script is finished.
function Stop()

end