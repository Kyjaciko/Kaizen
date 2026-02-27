////////////////////////////////////////////////////////////////////////////////
// Filename: Constants.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "Types.h"

///////////////
// CONSTANTS //
///////////////

namespace Kaizen::Constant
{
    inline constexpr int STANDARD_ANIMATION_FPS = 24;
    inline constexpr Types::ResourceID RESOURCE_FIRST_ID = 1;

    namespace Container
    {
        inline constexpr uint32_t ESTIMATED_QUANTITY_OF_OBJECTS = 100;
        inline constexpr int INDEX_UNINITIALIZED = std::numeric_limits<uint32_t>::max();
    }
}

#endif // !_CONSTANTS_H_