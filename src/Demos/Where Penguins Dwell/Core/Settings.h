////////////////////////////////////////////////////////////////////////////////
// Filename: Settings.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _WHERE_PENGUINS_DWELL_SETTINGS_H_
#define _WHERE_PENGUINS_DWELL_SETTINGS_H_

#include <stdint.h>

///////////////
// CONSTANTS //
///////////////

namespace WherePenguinsDwell::Settings
{
    namespace Systems
    {
        inline constexpr uint8_t WINDOW_UPDATE_FREQUENCY = 5;
        inline constexpr float EDGE_WINDOWS_THICKNESS = 100.f;
    }
}

#endif // !_WHERE_PENGUINS_DWELL_SETTINGS_H_