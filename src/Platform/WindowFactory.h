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

#include "Debug/ErrorLogger.h"

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

		bool Init(WindowContainer* pWindowContainer, HINSTANCE hInstance, std::string windowTitle, std::string windowClass, int width = 800, int height = 600);
		bool HandleMessages();

		HWND GetHWnd() const;

	private:
		int	m_width;
		int	m_height;

		std::string m_windowName;
		std::string	m_className;
		std::wstring m_wideWindowName;
		std::wstring m_wideClassName;

		HWND m_hWnd = nullptr;
		HINSTANCE m_hInstance = nullptr;

	private:
		std::wstring StringToWide(const std::string& str) { return std::wstring(str.begin(), str.end()); }

		void RegisterWindowClass();
	};
}

#endif // !_WINDOWFACTORY_H_
