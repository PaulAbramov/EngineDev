#include "InputClass.h"

/*
	Constructor
*/
InputClass::InputClass()
{

}

/*
	Destructor
*/
InputClass::~InputClass()
{

}

/*
	First set all keys to false, so they are all not pressed
*/
void InputClass::Initialize()
{
	for (int i = 0; i < 256; i++)
	{
		m_keys[i] = false;
	}
}

/*
	Set the key at the given position to pressed
*/
void InputClass::KeyDown(unsigned int _input)
{
	m_keys[_input] = true;
}

/*
	Set the key at the given position to released
*/
void InputClass::KeyUp(unsigned int _input)
{
	m_keys[_input] = false;
}

/*
	Get the current state of a Key
*/
bool InputClass::IsKeyDown(unsigned int _input)
{
	return m_keys[_input];
}