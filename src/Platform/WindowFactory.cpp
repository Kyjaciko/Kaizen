////////////////////////////////////////////////////////////////////////////////
// Filename: WindowFactory.cpp
////////////////////////////////////////////////////////////////////////////////

#include "WindowFactory.h"
#include "WindowContainer.h"

namespace windows
{
	WindowFactory::~WindowFactory()
	{
		if (!m_hWnd) 
			return;

		UnregisterClass(m_wideClassName.c_str(), m_hInstance);
		DestroyWindow(m_hWnd);
	}

	bool WindowFactory::Init(WindowContainer* pWindowContainer, HINSTANCE hInstance, std::string windowTitle, std::string windowClass, int width, int height, Types::WindowFlags flags)
	{
		// Direct Composition must be used or transparancy won't work!
		VALIDATE_TRANSPARENCY(flags);

		const bool fullscreen = Helpers::AreFlagsEnabled(flags, Types::WindowFlags::Fullscreen);
		const bool transparent = Helpers::AreFlagsEnabled(flags, Types::WindowFlags::Transparent);

		m_hInstance		 = hInstance;
		m_windowName	 = windowTitle;
		m_className		 = windowClass;
		m_wideWindowName = StringHelper::StringToWide(m_windowName);
		m_wideClassName  = StringHelper::StringToWide(m_className);

		if (fullscreen)
		{
			m_width = GetSystemMetrics(SM_CXSCREEN);
			m_height = GetSystemMetrics(SM_CYSCREEN);
		}
		else
		{
			m_width = width;
			m_height = height;
		}

		// Register the window class.
		RegisterWindowClass();

		// Determine window styles.
		DWORD style = WS_OVERLAPPEDWINDOW;
		DWORD exStyle = WS_EX_APPWINDOW;
		if (fullscreen || transparent) 
			style = WS_POPUP;

		if (transparent)
		{
			exStyle |= WS_EX_NOREDIRECTIONBITMAP; // Tested -> Not necessary.
			exStyle |= WS_EX_NOACTIVATE;

			// Force window always on top, even over the taskbar.
			exStyle |= WS_EX_TOPMOST;

			// Don't show window in ALT+TAB.
			exStyle &= ~WS_EX_APPWINDOW;
			exStyle |= WS_EX_TOOLWINDOW;

			// Enable transparency.
			exStyle |= WS_EX_LAYERED;
			//exStyle |= WS_EX_TRANSPARENT; -> Dynamically adjusted while rendering.
		}

		RECT wr = { 0, 0, m_width, m_height };
		if (!fullscreen && !transparent)
		{
			// Adjust if needed for the title and sidebars.
			wr.left = 50;
			wr.top = 50;
			wr.right = wr.left + m_width;
			wr.bottom = wr.top + m_height;
			AdjustWindowRect(&wr, style, FALSE);
		}

		// Create the actual window.
		m_hWnd = CreateWindowEx(
			exStyle,
			m_wideClassName.c_str(),
			m_wideClassName.c_str(),
			style,
			fullscreen ? 0 : wr.left,
			fullscreen ? 0 : wr.top,
			fullscreen ? m_width : (wr.right - wr.left),
			fullscreen ? m_height : (wr.bottom - wr.top),
			nullptr,
			nullptr,
			m_hInstance,
			pWindowContainer
		);

		if (!m_hWnd) 
			return false;

		// Bring the window up on the screen and set it as the main focus.
		ShowWindow(m_hWnd, SW_SHOW);
		SetForegroundWindow(m_hWnd);
		if (!transparent)
			SetFocus(m_hWnd);

		if (transparent)
		{
			NOTIFYICONDATA nid = {};
			nid.cbSize = sizeof(NOTIFYICONDATA);
			nid.hWnd = m_hWnd;
			nid.uID = TRAY_ICON_ID;
			nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
			nid.uCallbackMessage = WM_TRAYICON;
			nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
			wcscpy_s(nid.szTip, L"My app");

			Shell_NotifyIcon(NIM_ADD, &nid);
		}

		return true;
	}

	bool WindowFactory::HandleMessages()
	{
		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));

		// PeekMessage is the key to real-time rendering.
		while (PeekMessage(&msg, m_hWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_NULL && !IsWindow(m_hWnd))
		{
			m_hWnd = nullptr; // Message loop takes care of destroying the window.
			UnregisterClass(m_wideClassName.c_str(), m_hInstance);
			return false;
		}

		return true;
	}

	HWND WindowFactory::GetHWnd() const
	{
		return m_hWnd;
	}

	LRESULT CALLBACK HandleMsgRedirect(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		/*case WM_CLOSE:
			DestroyWindow(hWnd);
			return 0;*/
		default:
		{
			WindowContainer* const p_window = reinterpret_cast<WindowContainer*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
			
			// Forward message to the WindowContainer class handler.
			return p_window->WindowProc(hWnd, uMsg, wParam, lParam);
		}
		}
	}

	LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_NCCREATE:
		{
			// This function will just be a setup.
			// We will call a different WindowProc that will redirect the messages to the WindowContainer class.
			const CREATESTRUCT* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			WindowContainer* p_window = reinterpret_cast<WindowContainer*>(pCreate->lpCreateParams);
			if (!p_window) 
				exit(-1);

			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(p_window));
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HandleMsgRedirect));
			return p_window->WindowProc(hWnd, uMsg, wParam, lParam);
		}
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

	void WindowFactory::RegisterWindowClass()
	{
		WNDCLASSEX wc;
		wc.cbSize		 = sizeof(WNDCLASSEX);
		wc.style		 = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc	 = HandleMsgSetup;
		wc.cbClsExtra	 = 0;
		wc.cbWndExtra	 = 0;
		wc.hInstance	 = m_hInstance;
		wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = m_wideClassName.c_str();
		wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);

		RegisterClassEx(&wc);
	}
}