////////////////////////////////////////////////////////////////////////////////
// Filename: KeyboardEvent.cpp
////////////////////////////////////////////////////////////////////////////////

#include "KeyboardEvent.h"

namespace windows
{
	KeyboardEvent::KeyboardEvent()
		: m_type(INVALID)
		, m_key(0u)
	{
	}

	KeyboardEvent::KeyboardEvent(const EventType type, const unsigned char key)
		: m_type(type)
		, m_key(key)
	{
	}

	bool KeyboardEvent::IsPressed() const
	{
		return m_type == KEY_DOWN;
	}

	bool KeyboardEvent::IsReleased() const
	{
		return m_type == KEY_UP;
	}

	bool KeyboardEvent::IsValid() const
	{
		return m_type != INVALID;
	}

	unsigned char KeyboardEvent::GetKey() const
	{
		return m_key;
	}
}