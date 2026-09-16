////////////////////////////////////////////////////////////////////////////////
// Filename: Keyboard.cpp
////////////////////////////////////////////////////////////////////////////////

#include "Keyboard.h"

namespace windows
{
	Keyboard::Keyboard()
	{
		// Initialize the key states to off (false).
		for (int i = 0; i < 256; i++)
			m_keyStates[i] = false;
	}

	KeyboardEvent Keyboard::ReadKey()
	{
		if (m_keyBuffer.empty()) return KeyboardEvent();

		// Get first keyboard event from queue and remove it.
		KeyboardEvent event = m_keyBuffer.front();
		m_keyBuffer.pop();
		return event;
	}

	unsigned char Keyboard::ReadChar()
	{
		if (m_charBuffer.empty()) return 0u;

		// Get first character from queue and remove it.
		unsigned char character = m_charBuffer.front();
		m_charBuffer.pop();
		return character;
	}

	void Keyboard::OnKeyPress(const unsigned char key)
	{
		m_keyStates[key] = true;									   // Set key state to pressed (true).
		m_keyBuffer.push(KeyboardEvent(KeyboardEvent::KEY_DOWN, key)); // Add key event to buffer.
	}

	void Keyboard::OnKeyRelease(const unsigned char key)
	{
		m_keyStates[key] = false;									 // Set key state to released (false).
		m_keyBuffer.push(KeyboardEvent(KeyboardEvent::KEY_UP, key)); // Add key event to buffer.
	}

	void Keyboard::OnChar(const unsigned char key)
	{
		m_charBuffer.push(key);
	}

	bool Keyboard::IsKeyPressed(const unsigned char key) const
	{
		return m_keyStates[key];
	}

	bool Keyboard::IsKeyBufferEmpty() const
	{
		return m_keyBuffer.empty();
	}

	bool Keyboard::IsCharBufferEmpty() const
	{
		return m_charBuffer.empty();
	}

	bool Keyboard::AreKeysAutoRepeat() const
	{
		return m_autoRepeatKeys;
	}

	bool Keyboard::AreCharsAutoRepeat() const
	{
		return m_autoRepeatChars;
	}

	void Keyboard::EnableAutoRepeatKeys()
	{
		m_autoRepeatKeys = true;
	}

	void Keyboard::DisableAutoRepeatKeys()
	{
		m_autoRepeatKeys = false;
	}

	void Keyboard::EnableAutoRepeatChars()
	{
		m_autoRepeatChars = true;
	}

	void Keyboard::DisableAutoRepeatChars()
	{
		m_autoRepeatChars = false;
	}
}