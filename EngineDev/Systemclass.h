#pragma once

#pragma region pre-processing directives
#define WIN32_LEAN_AND_MEAN		// Exclude some APIs we do not need to speed up the build process
#pragma endregion 

#pragma region includes
#include <windows.h>			// Create a window and use further win32 functions
#pragma endregion

class SystemClass
{
public:
	SystemClass();
	~SystemClass();
	void Initialize();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND _windowHandle, UINT _message, WPARAM _wParam, LPARAM _lParam);

private:
	LPCWSTR m_applicationName;
	HINSTANCE m_instanceHandle;	// Handles our instance of the application
	HWND m_windowHandle;		// Handles the window of our application
};

#pragma region Function prototypes
static LRESULT CALLBACK WndProc(HWND _windowHandle, UINT _message, WPARAM _wParam, LPARAM _lParam);
#pragma endregion

#pragma region Globals
static SystemClass* ApplicationHandle = nullptr;
#pragma endregion