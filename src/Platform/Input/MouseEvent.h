////////////////////////////////////////////////////////////////////////////////
// Filename: MouseEvent.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _MOUSEEVENT_H_
#define _MOUSEEVENT_H_

////////////////////////////////////////////////////////////////////////////////
// Class name: MouseEvent
////////////////////////////////////////////////////////////////////////////////

namespace windows
{
	struct MousePoint
	{
		int x;
		int y;
	};

	class MouseEvent
	{
	public:
		enum class EventType
		{
            MOVE,
            LEFT_DOWN,
            LEFT_UP,
            RIGHT_DOWN,
            RIGHT_UP,
			MIDDLE_DOWN,
			MIDDLE_UP,
			WHEEL_DOWN,
            WHEEL_UP,
			RAW_MOVE,
			INVALID
		};

	public:
		MouseEvent();
		MouseEvent(const EventType eventType, const int x, const int y);

		bool IsValid() const;
		EventType GetEventType() const;
		MousePoint GetPosition() const;
		int GetPosX() const;
		int GetPosY() const;

	private:
		EventType		m_eventType;
		MousePoint		m_position;
	};
}

#endif // !_MOUSEEVENT_H_

