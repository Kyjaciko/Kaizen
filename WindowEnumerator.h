////////////////////////////////////////////////////////////////////////////////
// Filename: WindowEnumerator.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _WINDOW_ENUMERATOR_H_
#define _WINDOW_ENUMERATOR_H_

/////////////
// LINKING //
/////////////

#pragma comment(lib, "dwmapi.lib")

//////////////
// INCLUDES //
//////////////

#include <new>
#include <vector>
#include <dwmapi.h>

////////////////
// COMPONENTS //
////////////////

namespace WherePenguinsDwell::Platform::Windows
{
	[[nodiscard]] bool IsWindowVisibleToUser(HWND hWnd) noexcept;
	BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) noexcept;
}

#endif // !_WINDOW_ENUMERATOR_H_