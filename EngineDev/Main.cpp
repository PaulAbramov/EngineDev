#include "Systemclass.h"

/*
	Mainfunction
	Create a new instance of the systemclass
	Initialize the instance and run the program
	Should the program quit for any reason so shut it down and release the memory
*/
int WINAPI WinMain(HINSTANCE _instanceHandle, HINSTANCE _previous, PSTR _pScmdline, int _cmdShow)
{
	SystemClass *system = new SystemClass;
	if (!system)
	{
		return 0;
	}

	bool intializedWindow = system->Initialize();
	if(intializedWindow)
	{
		system->Run();
	}

	system->Shutdown();

	delete system;
	system = nullptr;

	return 0;
}