////////////////////////////////////////////////////////////////////////////////
// Filename: WindowEnumerator.cpp
////////////////////////////////////////////////////////////////////////////////

#include "WindowEnumerator.h"

namespace WherePenguinsDwell::Platform::Windows
{
    bool IsWindowVisibleToUser(HWND hWnd) noexcept
    {
        // Check if window is invisible or minimized.
        if (!IsWindowVisible(hWnd) || IsIconic(hWnd))
            return false;

        const LONG_PTR exStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
        if ((exStyle & WS_EX_TOOLWINDOW) && !(exStyle & WS_EX_APPWINDOW))
            return false;

        // Cloaked check: Many apps run in the background but are ‘visible’ according to the old API...
        int cloaked = 0;
        const HRESULT hr = DwmGetWindowAttribute(hWnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
        if (SUCCEEDED(hr) && cloaked)
            return false;

        // According to tests the cloaked check already does this.
        /*char className[256];
        GetClassNameA(hWnd, className, 256);
        if (strcmp(className, "Progman") == 0 || strcmp(className, "WorkerW") == 0)
            return false;*/

        return true;
    }

    BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) noexcept
    {
        std::vector<RECT>* windows = reinterpret_cast<std::vector<RECT>*>(lParam);
        if (!windows)
            return FALSE;

        if (!IsWindowVisibleToUser(hWnd))
            return TRUE;

        // Get window size without the offsets for resizing the window.
        // In case this failes fall back to normal method, that includes these offsets.
        RECT rect{};
        HRESULT hr = DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect));
        if (FAILED(hr))
        {
            if (!GetWindowRect(hWnd, &rect))
                return TRUE;
        }

        // Check if the window is on the screen, could use:
        //     - SM_{}VIRTUALSCREEN to support multi-monitor setups
        //     - Or SM_{}SCREEN for only the primary monitor
        const int virtualLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
        const int virtualTop = GetSystemMetrics(SM_YVIRTUALSCREEN);

        const RECT screenRect{
            .left   = virtualLeft,
            .top    = virtualTop,
            .right  = virtualLeft + GetSystemMetrics(SM_CXVIRTUALSCREEN),
            .bottom = virtualTop + GetSystemMetrics(SM_CYVIRTUALSCREEN)
        };

        RECT intersection{};
        if (!IntersectRect(&intersection, &rect, &screenRect))
            return TRUE;

        // NEVER throw a C++ exception back into a C API (especially windows API)!
        try
        {
            windows->push_back(rect);
        }
        catch (const std::bad_alloc&)
        {
            return FALSE;
        }

        return TRUE;
    }
}