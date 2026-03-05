////////////////////////////////////////////////////////////////////////////////
// Filename: WindowsTypes.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _WINDOWS_TYPES_H_
#define _WINDOWS_TYPES_H_

//////////////
// INCLUDES //
//////////////

#include <cstdint>

///////////////
// CONSTANTS //
///////////////

namespace windows::Types
{
	enum class WindowFlags : uint8_t
	{
		None = 0,
		Fullscreen = 1 << 0,
		Transparent = 1 << 1
	};

	[[nodiscard]] constexpr WindowFlags operator&(WindowFlags a, WindowFlags b) noexcept {
		return static_cast<WindowFlags>(
			static_cast<uint8_t>(a) & static_cast<uint8_t>(b)
			);
	}

	[[nodiscard]] constexpr WindowFlags operator|(WindowFlags a, WindowFlags b) noexcept {
		return static_cast<WindowFlags>(
			static_cast<uint8_t>(a) | static_cast<uint8_t>(b)
			);
	}
}

#endif // !_WINDOWS_TYPES_H_