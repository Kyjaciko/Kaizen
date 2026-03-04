////////////////////////////////////////////////////////////////////////////////
// Filename: WindowContainer.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _WINDOWCONTAINER_H_
#define _WINDOWCONTAINER_H_

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "WindowFactory.h"
#include "Input/Keyboard.h"
#include "Input/Mouse.h"
#include "../Graphics/Graphics.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: WindowContainer
////////////////////////////////////////////////////////////////////////////////

namespace windows
{
	class WindowContainer
	{
	public:
		WindowContainer();
		~WindowContainer();

		virtual void OnShutdown() const noexcept {}

		LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	protected:
		WindowFactory*			m_WindowFactory;
		Keyboard*				m_Keyboard;
		Mouse*					m_Mouse;
		DirectX11::Graphics*	m_gfx;
	};
}

#endif // !_WINDOWCONTAINER_H_

