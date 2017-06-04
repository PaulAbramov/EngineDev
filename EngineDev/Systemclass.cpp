#include "Systemclass.h"
#include <minwinbase.h>

SystemClass::SystemClass()
{
	
}

SystemClass::~SystemClass()
{
	
}

/*
	Initialize the window, which will display everything

	Declare some variables we are going to need later on

	Get the instance of this application so we can use it later on
	Give the application a name

	Fill the windowClass like we want it to use and register it so we can use it
	Get the screenHeight and screenWidth so we can use the fullscreen mode

	Initialize the DEVMODE object with the size we will need
	Fill the object with its needed values

	Create a new window with the size we defined/calculated and pass the instanceHandle
	Afterwards show the window, set it to the foreground and set the focus on this window
	We do not want to show a cursor so hide it
*/
void SystemClass::Initialize()
{
	WNDCLASSEX windowClass;
	DEVMODE devModeScreenSettings;
	int xPosition;
	int yPosition;
	int screenWidth;
	int screenHeight;

	m_instanceHandle = GetModuleHandle(nullptr);	// Get the instance of this application

	m_applicationName = L"EngineDev";				// Give the application a name

	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;				// allow redraw of the window
	windowClass.lpfnWndProc = WndProc;									// function to handle the windows messages
	windowClass.cbClsExtra = 0;											
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = m_instanceHandle;							// pass the handle of this instance to the window
	windowClass.hIcon = LoadIcon(nullptr, IDI_WINLOGO);					// set the icon of the application IDI = IDICON_...
	windowClass.hIconSm = windowClass.hIcon;
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);				// set the cursor for the application IDC = IDCURSOR_...
	windowClass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);	// set a black background
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = m_applicationName;						// set the applicationname
	windowClass.cbSize = sizeof(WNDCLASSEX);							// the whole size if this object shall be the size of it's class

	RegisterClassEx(&windowClass);

	screenHeight = GetSystemMetrics(SM_CYSCREEN);
	screenWidth = GetSystemMetrics(SM_CXSCREEN);

	memset(&devModeScreenSettings, 0, sizeof(devModeScreenSettings));				// Allocate the memory we need
	devModeScreenSettings.dmSize = sizeof(devModeScreenSettings);					// set its size
	devModeScreenSettings.dmPelsHeight = (unsigned long)screenHeight;				// set the height of the window
	devModeScreenSettings.dmPelsWidth = (unsigned long)screenWidth;					// set the width of the window
	devModeScreenSettings.dmBitsPerPel = 32;										// Bits per Pixel, this is the color resolution
	devModeScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;	// Sets the bit inside the struct of the fields we have initialized

	ChangeDisplaySettings(&devModeScreenSettings, CDS_FULLSCREEN);					// Set the window to fullscreen and pass the devmode

	xPosition = 0;
	yPosition = 0;

	m_windowHandle = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, 
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP, 
		xPosition, yPosition, screenWidth, screenHeight, 
		nullptr, nullptr, m_instanceHandle, nullptr);

	ShowWindow(m_windowHandle, SW_SHOW);
	SetForegroundWindow(m_windowHandle);
	SetFocus(m_windowHandle);

	ShowCursor(false);
}

/*
	Loop the program until we decide to quit it
	If we leave this loop the programm will be shutdown inside the main function

	Check for every input we get (messages in windows) and handle them inside of WndProc
*/
void SystemClass::Run()
{
	MSG message;

	ZeroMemory(&message, sizeof(MSG));

	while (true)
	{
		if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		if (message.message == WM_QUIT)
		{
			break;
		}
	}
}

/*
	Activate and show the cursor again
	Coming back from fullscreen set the display settings to default
	Remove the window and set its pointer to null
	Remove the application instance and set its pointer to null
	Release the pointer to this class
*/
void SystemClass::Shutdown()
{
	ShowCursor(true);

	ChangeDisplaySettings(nullptr, 0);

	DestroyWindow(m_windowHandle);
	m_windowHandle = nullptr;

	UnregisterClass(m_applicationName, m_instanceHandle);
	m_instanceHandle = nullptr;

	ApplicationHandle = NULL;
}

/*
	Handle all incoming messages from the method WndProc which are not handled yet
	In the future we want to handle keyinput here
	For now pass the messages to the standard windows handler for messages
*/
LRESULT CALLBACK SystemClass::MessageHandler(HWND _windowHandle, UINT _message, WPARAM _wParam, LPARAM _lParam)
{
	switch (_message)
	{
		case WM_KEYDOWN:
			// TODO handle input
			return 0;

		case WM_KEYUP:
			// TODO handle input
			return 0;

		default:
			return DefWindowProc(_windowHandle, _message, _wParam, _lParam);
	}
}

/*
	Handle all incoming messages from the method "Run"
	If the message equals to WM_DESTROY or WM_CLOSE quit the program and close the window
	Every other language pass it to the MessageHandler of systemclass

	Windows sends its messages to this function because of the line in the initialization
	windowClass.lpfnWndProc = WndProc;
*/
LRESULT CALLBACK WndProc(HWND _windowHandle, UINT _message, WPARAM _wParam, LPARAM _lParam)
{
	switch (_message)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;

		default:
			return ApplicationHandle->MessageHandler(_windowHandle, _message, _wParam, _lParam);
	}
}