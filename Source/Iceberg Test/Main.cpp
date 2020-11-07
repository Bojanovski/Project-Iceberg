#include "InitD3DApp.h"
#include "FPCameraController.h"
#include "MainState.h"

using namespace Makina;
using namespace Physics_Makina;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	try
	{
		//DIRECT3D 11
		InitD3DApp theApp(hInstance, 600, 600, 0);
		//InitD3DApp theApp(hInstance, 1280, 720, 0); // this is 720p - https://support.google.com/youtube/answer/1722171?hl=en
		//InitD3DApp theApp(hInstance, 1920, 1080, 0); // this is 720p - https://support.google.com/youtube/answer/1722171?hl=en
		theApp.SetMainWndCaption(L"Title");

		if (!theApp.Init())
			throw UnexpectedError(L"Failed to initialize app!");

		// field for frustrum culling test
		std::vector<Object *> objects;

		//Camera Controller
		FPCameraController *cControler = new FPCameraController(&theApp, &objects);
		theApp.AddGameComponent(cControler);

		theApp.GetD3DAppValues()->mGameStatesManager->PushState(new MainState(&theApp));

		return theApp.Run();
	}
	catch (Exception &ex)
	{
		MessageBox(NULL, ex.Message(), ex.Type(), MB_ICONERROR | MB_OK);
		return 0;
	}
}