#pragma once

class InputClass
{
public:
	InputClass();
	~InputClass();

	void Initialize();

	void KeyDown(unsigned int _input);
	void KeyUp(unsigned int _input);
	bool IsKeyDown(unsigned int _input);

private:
	bool m_keys[256];
};