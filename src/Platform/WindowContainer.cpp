////////////////////////////////////////////////////////////////////////////////
// Filename: WindowContainer.cpp
////////////////////////////////////////////////////////////////////////////////

#include "WindowContainer.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace windows
{
	WindowContainer::WindowContainer()
	{
		static bool raw_input_initialized = false;
		if (!raw_input_initialized)
		{
			RAWINPUTDEVICE raw_input_device;
			raw_input_device.usUsagePage = 0x01;	// Generic desktop controls
			raw_input_device.usUsage     = 0x02;	// Mouse
			raw_input_device.dwFlags	 = 0;       // Flags
			raw_input_device.hwndTarget  = nullptr; // No target window, so it follows keyboard focus.

			if (RegisterRawInputDevices(&raw_input_device, 1, sizeof(raw_input_device)) == FALSE) exit(-1);

			raw_input_initialized = true;
		}
	}

	LRESULT WindowContainer::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true; // ImGui is handling the message.

		switch (uMsg)
		{

			///////////////////////
			// Keyboard messages //
			///////////////////////

		case WM_KEYDOWN:
		{
			unsigned char key = static_cast<unsigned char>(wParam);
			if (m_keyboard.AreKeysAutoRepeat())
				m_keyboard.OnKeyPress(key);
			else
			{
				const bool was_pressed = lParam & 0x40000000; // Has been pressed before? Check bit 30.
				if (!was_pressed) m_keyboard.OnKeyPress(key);
			}
			return 0;
		}
		case WM_KEYUP:
		{
			unsigned char key = static_cast<unsigned char>(wParam);
			m_keyboard.OnKeyRelease(key);
			return 0;
		}
		case WM_CHAR:
		{
			unsigned char character = static_cast<unsigned char>(wParam);
			if (m_keyboard.AreCharsAutoRepeat()) 
				m_keyboard.OnChar(character);
			else
			{
				const bool was_pressed = lParam & 0x40000000;   // Has been pressed before? Check bit 30.
				if (!was_pressed) m_keyboard.OnChar(character);
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
			m_mouse.OnLeftPress(x, y);
			return 0;
		}
		case WM_LBUTTONUP:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			m_mouse.OnLeftRelease(x, y);
			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			m_mouse.OnRightPress(x, y);
			return 0;
		}
		case WM_RBUTTONUP:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			m_mouse.OnRightRelease(x, y);
			return 0;
		}
		case WM_MBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			m_mouse.OnMiddlePress(x, y);
			return 0;
		}
		case WM_MBUTTONUP:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			m_mouse.OnMiddleRelease(x, y);
			return 0;
		}
		case WM_MOUSEWHEEL:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			int wheel_delta = GET_WHEEL_DELTA_WPARAM(wParam);
			if (wheel_delta > 0)
				m_mouse.OnWheelUp(x, y);
			else
				m_mouse.OnWheelDown(x, y);
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			m_mouse.OnMouseMove(x, y);
			return 0;
		}
		case WM_INPUT:
		{
			// Handle raw input.
			UINT data_size = sizeof(RAWINPUTHEADER);

			GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &data_size, sizeof(RAWINPUTHEADER));
			if (data_size > 0)
			{
				std::unique_ptr<BYTE[]> raw_data = std::make_unique<BYTE[]>(data_size);
				if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, raw_data.get(), &data_size, sizeof(RAWINPUTHEADER)) == data_size)
				{
					RAWINPUT* raw_input = reinterpret_cast<RAWINPUT*>(raw_data.get());
					if (raw_input->header.dwType == RIM_TYPEMOUSE)
					{
						m_mouse.OnMouseRawMove(raw_input->data.mouse.lLastX, raw_input->data.mouse.lLastY);
					}
				}
			}

			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}
}