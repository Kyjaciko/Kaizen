////////////////////////////////////////////////////////////////////////////////
// Filename: WindowsHelpers.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _WINDOWS_HELPERS_H_
#define _WINDOWS_HELPERS_H_

//////////////
// INCLUDES //
//////////////

#include <cstdint>

/////////////
// HELPERS //
/////////////

namespace windows::Helpers
{
	// Check if EVERY flag is enabled.
	[[nodiscard]] constexpr bool AreFlagsEnabled(Types::WindowFlags setFlags, Types::WindowFlags checkFlags) noexcept
	{
		return (setFlags & checkFlags) == checkFlags;
	}

	// Check if atleast 1 flag is enabled.
	[[nodiscard]] constexpr bool IsFlagEnabled(Types::WindowFlags setFlags, Types::WindowFlags checkFlags) noexcept
	{
		return (setFlags & checkFlags) != Types::WindowFlags::None;
	}
}

#endif // !_WINDOWS_HELPERS_H_