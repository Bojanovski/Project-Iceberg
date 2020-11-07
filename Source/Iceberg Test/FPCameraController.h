#include <CameraController.h>
#include <vector>
#include "InitD3DApp.h"
#include "Object.h"

class FPCameraController : public Makina::CameraController
{
public:
	FPCameraController(InitD3DApp *appPt, std::vector<Object *> *objPt);
	~FPCameraController();
	
	void Update(float dt);
	void Draw(float dt);
	void OnResize();

private:
	float mSpeed;
	int mPickingIndex;
	int mInitialWheelValue;
	std::vector<Object *> *objPt;
};