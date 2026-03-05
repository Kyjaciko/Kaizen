////////////////////////////////////////////////////////////////////////////////
// Filename: Components.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _COMPONENTS_H_
#define _COMPONENTS_H_

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "../../../Systems/ECS/EntityManager.h"

////////////////
// COMPONENTS //
////////////////

namespace WherePenguinsDwell::Components
{
    struct ColliderBox
    {
        Entity owner;

        float width;
        float height;
        float offsetX = 0.f;
        float offsetY = 0.f;

        bool hasContactThisFrame = false;
    };

    struct VelocityComponent
    {
        float x = 0.f;
        float y = 0.f;
    };

    struct MovementComponent
    {
        float walkingSpeed = 24.f;
        float fallingSpeed = 48.f;

        float tumbling_acceleration = 36.f;
        float tumbling_terminalVelocity = 150.f;

        float climbingSpeed = 24.f;
    };

    struct TimerComponent
    {
        float deadTime = 0.f;
    };
}

#endif // !_COMPONENTS_H_