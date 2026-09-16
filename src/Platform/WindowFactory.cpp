////////////////////////////////////////////////////////////////////////////////
// Filename: WindowFactory.cpp
////////////////////////////////////////////////////////////////////////////////

#include "WindowFactory.h"
#include "WindowContainer.h"

namespace windows
{
	WindowFactory::~WindowFactory()
	{
		if (!m_hWnd) return;

		// Unregister the window class and destroy the window.
		UnregisterClass(m_wideClassName.c_str(), m_hInstance);
		DestroyWindow(m_hWnd);
	}

	bool WindowFactory::Init(WindowContainer* pWindowContainer, HINSTANCE hInstance, std::string windowTitle, std::string windowClass, int width, int height)
	{
		m_hInstance		 = hInstance;
		m_windowName	 = windowTitle;
		m_className		 = windowClass;
		m_wideWindowName = StringToWide(m_windowName);
		m_wideClassName  = StringToWide(m_className);
		m_width          = width;
		m_height		 = height;

		// Register the window class.
		RegisterWindowClass();

		// Adjust if needed for the title and sidebars.
		RECT wr;
		wr.left   = 50;
		wr.top	  = 50;
		wr.right  = wr.left + m_width;
		wr.bottom = wr.top + m_height;
		AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

		// Create the actual window.
		m_hWnd = CreateWindowEx(
			WS_EX_APPWINDOW,
			m_wideClassName.c_str(),
			m_wideClassName.c_str(),
			WS_OVERLAPPEDWINDOW,
			wr.left,				// X position of the window. If this is CW_USEDEFAULT, the system chooses the X position.
			wr.top,					// Y position of the window. If this is CW_USEDEFAULT, the system chooses the Y position.
			wr.right - wr.left, 	// Width of the window. If this is CW_USEDEFAULT, the system chooses the width.
			wr.bottom - wr.top,		// Height of the window. If this is CW_USEDEFAULT, the system chooses the height.
			nullptr,
			nullptr,
			m_hInstance,
			pWindowContainer        // Pointer to any value to be passed to the window procedure.
		);

		if (!m_hWnd) return false;

		// Bring the window up on the screen and set it as the main focus.
		ShowWindow(m_hWnd, SW_SHOW);
		SetForegroundWindow(m_hWnd);
		SetFocus(m_hWnd);

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
			m_hWnd = nullptr; // Message processing loop takes care of destroying the window.
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
		case WM_CLOSE:
			DestroyWindow(hWnd);
			return 0;
		default:
		{
			// Get the pointer to the WindowContainer class.
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
			if (!p_window) exit(-1);

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
		wc.style		 = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // Flags [HRedraw and VRedraw] to redraw the window when it is resized.
		wc.lpfnWndProc	 = HandleMsgSetup;					   // Pointer to the window procedure for handling messages for this window.
		wc.cbClsExtra	 = 0;
		wc.cbWndExtra	 = 0;
		wc.hInstance	 = m_hInstance;
		wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);    // Handle to the class icon (must be a handle to an icon resource). Currently using the default icon.
		wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);		   // Handle to the class cursor. Currently using the default arrow cursor. If this is NULL, we have to explicitly set the cursor's shape each time it enters the window.
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = m_wideClassName.c_str();
		wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);	   // Handle to the small icon that appears in the taskbar and in the window's title bar. Currently using the default icon.

		// Register the window class.
		RegisterClassEx(&wc);
	}
}