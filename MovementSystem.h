////////////////////////////////////////////////////////////////////////////////
// Filename: MovementSystem.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _MOVEMENT_SYSTEM_H_
#define _MOVEMENT_SYSTEM_H_

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "System.h"
#include "Components.h"
#include "Coordinator.h"
#include "TransformComponent.h"
#include "AnimationComponent.h"

////////////////////////////////////////////////////////////////////////////////
// Filename: MovementSystem.h
////////////////////////////////////////////////////////////////////////////////

namespace WherePenguinsDwell
{
    class MovementSystem : public Kaizen::Logic::System
    {
    public:
        void Update(Kaizen::Logic::Coordinator& coordinator, const float dt) const
        {
            for (const auto& entity : m_Entities)
            {
                auto& transform = coordinator.GetComponent<Kaizen::Components::TransformComponent>(entity);
                auto& velocity = coordinator.GetComponent<Components::VelocityComponent>(entity);
                const auto& anim = coordinator.GetComponent<AnimationComponent>(entity);
                const auto& moveProps = coordinator.GetComponent<Components::MovementComponent>(entity);

                // TODO: Cheaper to reset or determine a 'EntityState' switch?
                velocity.x = 0.f;
                if (anim.CurrentState != EntityState::tumbling)
                    velocity.y = moveProps.fallingSpeed;

                switch (anim.CurrentState)
                {
                case EntityState::zap_walkingLeft:
                    velocity.y = 0.f;
                    [[fallthrough]];
                case EntityState::walkingLeft:
                    velocity.x = -moveProps.walkingSpeed;
                    break;

                case EntityState::zap_walkingRight:
                    velocity.y = 0.f;
                    [[fallthrough]];
                case EntityState::walkingRight:
                    velocity.x = moveProps.walkingSpeed;
                    break;

                case EntityState::climbingOnTheLeft:
                case EntityState::climbingOnTheRight:
                    velocity.y = -moveProps.climbingSpeed;
                    break;

                case EntityState::tumbling:
                    velocity.y += moveProps.tumbling_acceleration * dt;
                    velocity.y = std::min(velocity.y, moveProps.tumbling_terminalVelocity);
                    break;

				case EntityState::splash:
                case EntityState::exit_walkingLeft:
                case EntityState::exit_walkingRight:
                case EntityState::exploding:
                    velocity.y = 0.f;
					break;

                case EntityState::idle:
                case EntityState::falling:
                case EntityState::floating:
                default:
                    break;
                }

                transform.Position.x += velocity.x * dt;
                transform.Position.y += velocity.y * dt;
            }
        }
    };
}

#endif // !_MOVEMENT_SYSTEM_H_