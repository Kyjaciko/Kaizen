////////////////////////////////////////////////////////////////////////////////
// Filename: WindowContainer.cpp
////////////////////////////////////////////////////////////////////////////////

#include "WindowContainer.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace windows
{
	WindowContainer::WindowContainer()
	{
		m_WindowFactory = new WindowFactory();
		m_Keyboard		= new Keyboard();
		m_Mouse			= new Mouse();
		m_gfx			= new DirectX11::Graphics();

		static bool raw_input_initialized = false;
		if (!raw_input_initialized)
		{
			// Mouse follows keyboard focus.
			RAWINPUTDEVICE raw_input_device;
			raw_input_device.usUsagePage = 0x01;
			raw_input_device.usUsage     = 0x02;
			raw_input_device.dwFlags	 = 0;
			raw_input_device.hwndTarget  = nullptr;

			if (RegisterRawInputDevices(&raw_input_device, 1, sizeof(raw_input_device)) == FALSE) 
				exit(-1);

			raw_input_initialized = true;
		}
	}

	WindowContainer::~WindowContainer()
	{
		delete m_WindowFactory;
		delete m_Keyboard;
		delete m_Mouse;
		delete m_gfx;
	}

	LRESULT WindowContainer::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Dear ImGui is handling the message.
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true;

		switch (uMsg)
		{
		
			///////////////////////
			// Keyboard messages //
			///////////////////////

		case WM_KEYDOWN:
		{
			unsigned char key = static_cast<unsigned char>(wParam);
			if (m_Keyboard->AreKeysAutoRepeat())
				m_Keyboard->OnKeyPress(key);
			else
			{
				const bool was_pressed = lParam & 0x40000000;   // Has been pressed before? Check bit 30.
				if (!was_pressed) m_Keyboard->OnKeyPress(key);
			}
			return 0;
		}
		case WM_KEYUP:
		{
			unsigned char key = static_cast<unsigned char>(wParam);
			m_Keyboard->OnKeyRelease(key);
			return 0;
		}
		case WM_CHAR:
		{
			unsigned char character = static_cast<unsigned char>(wParam);
			if (m_Keyboard->AreCharsAutoRepeat())
				m_Keyboard->OnChar(character);
			else
			{
				const bool was_pressed = lParam & 0x40000000;    // Has been pressed before? Check bit 30.
				if (!was_pressed) m_Keyboard->OnChar(character);
			}
			return 0;
		}

			////////////////////
			// Mouse messages //
			////////////////////

		case WM_LBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			m_Mouse->OnLeftPress(x, y);
			return 0;
		}
		case WM_LBUTTONUP:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			m_Mouse->OnLeftRelease(x, y);
			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			m_Mouse->OnRightPress(x, y);
			return 0;
		}
		case WM_RBUTTONUP:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			m_Mouse->OnRightRelease(x, y);
			return 0;
		}
		case WM_MBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			m_Mouse->OnMiddlePress(x, y);
			return 0;
		}
		case WM_MBUTTONUP:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			m_Mouse->OnMiddleRelease(x, y);
			return 0;
		}
		case WM_MOUSEWHEEL:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			int wheel_delta = GET_WHEEL_DELTA_WPARAM(wParam);
			if (wheel_delta > 0)
				m_Mouse->OnWheelUp(x, y);
			else
				m_Mouse->OnWheelDown(x, y);
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			m_Mouse->OnMouseMove(x, y);
			return 0;
		}
		case WM_INPUT:
		{
			UINT data_size = sizeof(RAWINPUTHEADER);

			GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &data_size, sizeof(RAWINPUTHEADER));
			if (data_size > 0)
			{
				std::unique_ptr<BYTE[]> raw_data = std::make_unique<BYTE[]>(data_size);
				if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, raw_data.get(), &data_size, sizeof(RAWINPUTHEADER)) == data_size)
				{
					RAWINPUT* raw_input = reinterpret_cast<RAWINPUT*>(raw_data.get());
					if (raw_input->header.dwType == RIM_TYPEMOUSE)
						m_Mouse->OnMouseRawMove(raw_input->data.mouse.lLastX, raw_input->data.mouse.lLastY);
				}
			}

			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

			////////////////////
			// Close messages //
			////////////////////

		case WM_TRAYICON:
		{
			if (lParam != WM_RBUTTONUP)
				return 0;
			
			POINT pt;
			GetCursorPos(&pt); // Haal muispositie op

			HMENU hMenu = CreatePopupMenu();
			InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, ID_TRAY_EXIT, TEXT("Close"));

			// Win32 Hack: Zorgt ervoor dat het menu sluit als je ernaast klikt
			SetForegroundWindow(hWnd);

			// Toon het menu op de muispositie
			TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);

			DestroyMenu(hMenu);
			return 0;
		}
		case WM_COMMAND:
		{
			if (LOWORD(wParam) != ID_TRAY_EXIT)
				return DefWindowProc(hWnd, uMsg, wParam, lParam);

			[[fallthrough]];
		}
		case WM_CLOSE:
		{
			// Note: Same logic holds for WM_COMMAND.
			// 
			// TODO: Mhmmm... If the window is transparant the only way for the user to close the window is to either:
			//     - use task manager.
			//     - using the tray icon.
			// In case of task manager WM_CLOSE will never be sent since it uses the TerminateProcess function through the OS.
			// How do we fix this?
			// 
			// TODO: Problem we have here is that this will call the same OnShutdown function
			// for every window. Since for efficiency and ease every window uses the same window procedure.
			// But if we use 2 windows with 2 totally different applications but in the same process
			// they will use totally different, if at all, OnShutdown functions.
			this->OnShutdown();
			DestroyWindow(hWnd);
			return 0;
		}
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}
}