#pragma once

#include <windows.h>

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class GraphicsClass
{
public:
	GraphicsClass();
	~GraphicsClass();

	bool Initialize(int _screenHeight, int _screenWidth, HWND _windowHandle);
	void Shutdown();
	bool Frame();

private:
	bool Render();
};