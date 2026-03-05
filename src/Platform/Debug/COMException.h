////////////////////////////////////////////////////////////////////////////////
// Filename: COMException.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _COMEXCEPTION_H_
#define _COMEXCEPTION_H_

//////////////
// INCLUDES //
//////////////

#include <string>
#include <comdef.h>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "../../Core/StringHelper.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: COMException
////////////////////////////////////////////////////////////////////////////////

namespace windows
{
	class COMException
	{
	public:
        COMException(HRESULT hr, const std::string& msg, const std::string& file, const std::string& function, int line)
        {
            _com_error error(hr);
            m_message = L"Message: " + StringHelper::StringToWide(msg) + L"\n";
			m_message += error.ErrorMessage();
			m_message += L"\nFile: " + StringHelper::StringToWide(file) + L"\n";
			m_message += L"Function: " + StringHelper::StringToWide(function) + L"\n";
			m_message += L"Line: " + std::to_wstring(line) + L"\n";
        }

		COMException(HRESULT hr, const std::wstring& msg, const std::string& file, const std::string& function, int line)
		{
			_com_error error(hr);
			m_message = L"Message: " + msg + L"\n";
			m_message += error.ErrorMessage();
			m_message += L"\nFile: " + StringHelper::StringToWide(file) + L"\n";
			m_message += L"Function: " + StringHelper::StringToWide(function) + L"\n";
			m_message += L"Line: " + std::to_wstring(line) + L"\n";
		}

		const wchar_t* what() const
		{
			return m_message.c_str();
		}

	private:
		std::wstring m_message;
	};
}

#endif // !_COMEXCEPTION_H_
