////////////////////////////////////////////////////////////////////////////////
// Filename: ErrorLogger.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _ERRORLOGGER_H_
#define _ERRORLOGGER_H_

//////////////
// INCLUDES //
//////////////

#include <windows.h>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "COMException.h"
#include "../../Core/Types.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: ErrorLogger
////////////////////////////////////////////////////////////////////////////////

namespace windows
{
	class ErrorLogger
	{
	public:
		static void Log(const COMException& exception)
		{
			std::wstring error_message = exception.what();
			MessageBoxW(nullptr, error_message.c_str(), L"Error", MB_ICONERROR);
		}
	};
}

////////////
// MACROS //
////////////

// do { ... } while(0) is needed to avoid "dangling else".
#define COM_ERROR_IF_FAILED_RETURN(hr, msg, returnValue) do { if (FAILED(hr)) { windows::ErrorLogger::Log(windows::COMException(hr, msg, __FILE__, __FUNCTION__, __LINE__)); return returnValue; } } while(0)
#define COM_ERROR_IF_FAILED_EXIT(hr, msg) do { if (FAILED(hr)) { windows::ErrorLogger::Log(windows::COMException(hr, msg, __FILE__, __FUNCTION__, __LINE__)); exit(-1); } } while(0)
#define COM_ERROR_IF_FAILED_SHOW(hr, msg) do { if (FAILED(hr)) windows::ErrorLogger::Log(windows::COMException(hr, msg, __FILE__, __FUNCTION__, __LINE__)); } while(0)

#define VALIDATE_TRANSPARENCY(flags) do { \
		const bool transparencyEnabled = windows::Helpers::IsFlagEnabled(flags, windows::Types::WindowFlags::Transparent); \
        assert(!transparencyEnabled || USE_DIRECT_COMPOSITION && "Transparent requires USE_DIRECT_COMPOSITION to be enabled."); \
    } while(0)

#endif // !_ERRORLOGGER_H_

