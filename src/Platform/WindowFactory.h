////////////////////////////////////////////////////////////////////////////////
// Filename: WindowFactory.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _WINDOWFACTORY_H_
#define _WINDOWFACTORY_H_

//////////////
// INCLUDES //
//////////////

#include <Windows.h>
#include <string>
#include <memory>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "../Settings.h"
#include "../WindowsTypes.h"
#include "../WindowsHelpers.h"
#include "Debug/ErrorLogger.h"

#include <shellapi.h>

inline constexpr UINT WM_TRAYICON = WM_USER + 1;
inline constexpr UINT ID_TRAY_EXIT = 1001;
inline constexpr UINT TRAY_ICON_ID = 1;

//////////////////////////
// FORWARD DECLARATIONS //
//////////////////////////

namespace windows
{
	class WindowContainer;
}

////////////////////////////////////////////////////////////////////////////////
// Class name: WindowFactory
////////////////////////////////////////////////////////////////////////////////

namespace windows
{
	class WindowFactory
	{
	public:
		~WindowFactory();

		bool Init(WindowContainer* pWindowContainer, HINSTANCE hInstance, std::string windowTitle, std::string windowClass, int width, int height, Types::WindowFlags flags);
		bool HandleMessages();

		HWND GetHWnd() const;

		int GetWidth() const { return m_width; }
		int GetHeight() const { return m_height; }

	private:
		int				m_width;
		int				m_height;
		std::string		m_windowName;
		std::string		m_className;
		std::wstring	m_wideWindowName;
		std::wstring	m_wideClassName;

		HWND		    m_hWnd = nullptr;
		HINSTANCE	    m_hInstance = nullptr;

	private:
		void RegisterWindowClass();
	};
}

#endif // !_WINDOWFACTORY_H_
