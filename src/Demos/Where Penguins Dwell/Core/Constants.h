////////////////////////////////////////////////////////////////////////////////
// Filename: Constants.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _WHERE_PENGUINS_DWELL_CONSTANTS_H_
#define _WHERE_PENGUINS_DWELL_CONSTANTS_H_

///////////////
// CONSTANTS //
///////////////

namespace WherePenguinsDwell::Constants
{
    namespace Systems
    {
        //////////////////
        // Window System//
        //////////////////

        inline constexpr size_t ESTIMATED_MAX_WINDOW_QUANTITY = 40;

        //////////////////////
        // Collision System //
        //////////////////////

        inline constexpr float CLIMBING_EPSILON = 0.1f;

        //////////////////////
        // Behaviour System //
        //////////////////////

        inline constexpr int ACTIVITY_CHANGE_MULTIPLIER = 5; // 1/.. (in this case: 1/5 -> 20%) per time window.

        inline constexpr int READING_PROBABILITY = 0;
        inline constexpr int DIGGING_PROBABILITY = 1;
        inline constexpr int IDLE_PROBABILITY = 2;
        inline constexpr int MAX_IDLE_STATE = 3;

        ///////////////////////
        // Life Cycle System //
        ///////////////////////

        inline constexpr float EXPLOSION_CLEANUP_TIME = 1.0f;
 
        inline constexpr int ZAP_PROBABILITY = 0;
        inline constexpr int EXIT_PROBABILITY = 1;
    }
}

#endif // !_WHERE_PENGUINS_DWELL_CONSTANTS_H_