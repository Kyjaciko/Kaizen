////////////////////////////////////////////////////////////////////////////////
// Filename: KeyboardEvent.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _KEYBOARDEVENT_H_
#define _KEYBOARDEVENT_H_

////////////////////////////////////////////////////////////////////////////////
// Class name: KeyboardEvent
////////////////////////////////////////////////////////////////////////////////

namespace windows
{
	class KeyboardEvent
	{
	public:
		enum EventType
		{
			KEY_DOWN,
			KEY_UP,
			INVALID
		};

	public:
		KeyboardEvent();
		KeyboardEvent(const EventType type, const unsigned char key);

		bool IsPressed() const;
		bool IsReleased() const;
		bool IsValid() const;
		unsigned char GetKey() const;

	private:
		EventType		m_type;
		unsigned char	m_key;
	};
}

#endif // !_KEYBOARDEVENT_H_
