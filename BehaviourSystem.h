////////////////////////////////////////////////////////////////////////////////
// Filename: BehaviourSystem.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _BEHAVIOUR_SYSTEM_H_
#define _BEHAVIOUR_SYSTEM_H_

#include <random>
#include <ranges>
#include <concepts>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "src/Demos/Where Penguins Dwell/Core/Constants.h"
#include "Coordinator.h"
#include "Components.h"
#include "CollisionSystem.h"
#include "WindowSystem.h"
#include "AnimationComponent.h"

namespace WherePenguinsDwell
{
    /**
     * @brief BehaviourSystem manages penguin AI and state transitions.
     */
    class BehaviourSystem : public Kaizen::Logic::System
    {
        // Keep this here to stop the compiler from bitching :D
        [[nodiscard]] auto GetPenguinEntities(Kaizen::Logic::Coordinator& coordinator) const
        {
            return m_Entities | std::views::filter([&](const Entity& e)
                {
                    return !coordinator.HasComponent<Tags::WindowColliderTag>(e);
                });
        }

    public:
        void Init(const int gameFPS) noexcept
        {
            m_ActivityDist = std::uniform_int_distribution<>(1, Constants::Systems::ACTIVITY_CHANGE_MULTIPLIER *
                60 * // seconds per minute
                gameFPS);
            m_StateDist = std::uniform_int_distribution<>(0, Constants::Systems::MAX_IDLE_STATE - 1);
        }

        /**
         * @brief Update penguin behaviours based on collision events and random AI.
         */
        void Update(
            Kaizen::Logic::Coordinator& coordinator,
            const std::vector<CollisionSystem::CollisionEvent>& collisionEvents,
            const std::vector<Entity>& entitiesWithoutContact)
        {
            // Handle collision-based state changes
            ProcessCollisionStates(coordinator, collisionEvents);

            // Handle tumbling for entities without contact
            ProcessTumblingStates(coordinator, entitiesWithoutContact);

            // Random activity changes
            ProcessRandomAI(coordinator);
        }

        /**
         * @brief Handle shutdown animations.
         */
        void ProcessShutdown(Kaizen::Logic::Coordinator& coordinator)
        {
            auto penguins = GetPenguinEntities(coordinator);

            for (const auto& entity : penguins)
            {
                auto& anim = coordinator.GetComponent<AnimationComponent>(entity);

                if (anim.CurrentState == EntityState::walkingLeft)
                {
                    SetState(anim, FlipCoin()
                        ? EntityState::zap_walkingLeft
                        : EntityState::exit_walkingLeft);
                }
                else if (anim.CurrentState == EntityState::walkingRight)
                {
                    SetState(anim, FlipCoin()
                        ? EntityState::zap_walkingRight
                        : EntityState::exit_walkingRight);
                }
                else if (IsClimbingState(anim.CurrentState))
                {
                    SetState(anim, FlipCoin()
                        ? EntityState::tumbling
                        : EntityState::floating);
                }
                else if (anim.CurrentState == EntityState::falling || anim.CurrentState == EntityState::floating || anim.CurrentState == EntityState::tumbling)
                {
                    continue;
                }
                else if (!IsShutdownState(anim.CurrentState))
                {
                    SetState(anim, EntityState::splash);
                }
            }
        }

    private:
        // Thread-safe random generators
        std::mt19937 s_Generator{ std::random_device{}() };
        std::bernoulli_distribution s_CoinFlip{ 0.5 };
        std::uniform_int_distribution<> m_ActivityDist;
        std::uniform_int_distribution<> m_StateDist;

        [[nodiscard]] static constexpr bool IsShutdownState(const EntityState state) noexcept
        {
            return state == EntityState::splash ||
                state == EntityState::zap_walkingLeft ||
                state == EntityState::exit_walkingLeft ||
                state == EntityState::zap_walkingRight ||
                state == EntityState::exit_walkingRight;
        }

        [[nodiscard]] static constexpr bool IsWalkingState(const EntityState state) noexcept
        {
            return state == EntityState::walkingLeft ||
                state == EntityState::walkingRight;
        }

        [[nodiscard]] static constexpr bool IsClimbingState(const EntityState state) noexcept
        {
            return state == EntityState::climbingOnTheLeft ||
                state == EntityState::climbingOnTheRight;
        }

        [[nodiscard]] static constexpr bool IsIdleActivityState(const EntityState state) noexcept
        {
            return state == EntityState::reading ||
                state == EntityState::digging ||
                state == EntityState::idle;
        }

        void ProcessCollisionStates(
            Kaizen::Logic::Coordinator& coordinator,
            const std::vector<CollisionSystem::CollisionEvent>& events)
        {
            for (const auto& event : events)
            {
                auto& anim = coordinator.GetComponent<AnimationComponent>(event.penguinEntity);

                if (event.shouldCrush)
                {
                    SetState(anim, EntityState::exploding);
                    continue;
                }

                const auto& res = event.result;

                // Resolve state based on collision axis
                if (res.overlapX < res.overlapY)
                    HandleHorizontalCollision(anim, res.normalX);
                else
                    HandleVerticalCollision(anim, res.normalY);
            }
        }

        /*void ProcessTumblingStates(
            Kaizen::Logic::Coordinator& coordinator,
            const auto& entitiesWithoutContact)
        {
            for (const auto& entity : entitiesWithoutContact)
            {
                auto& anim = coordinator.GetComponent<AnimationComponent>(entity);

                if (IsClimbingState(anim.CurrentState) || IsWalkingState(anim.CurrentState)) [[likely]]
                    SetState(anim, EntityState::tumbling);
            }
        }*/

        void ProcessTumblingStates(
            Kaizen::Logic::Coordinator& coordinator,
            const auto& entitiesWithoutContact)
        {
            for (const auto& entity : entitiesWithoutContact)
            {
                auto& anim = coordinator.GetComponent<AnimationComponent>(entity);

                if (IsWalkingState(anim.CurrentState))
                {
                    // Van een richel afgelopen: val naar beneden
                    SetState(anim, EntityState::tumbling);
                }
                else if (IsClimbingState(anim.CurrentState))
                {
                    // Contact verloren tijdens het klimmen = de top bereikt!
                    if (FlipCoin())
                    {
                        // Bovenop het raam beland! 
                        // Als hij rechts klom, moet hij nu naar links lopen over het dak.
                        auto& transform = coordinator.GetComponent<Kaizen::Components::TransformComponent>(entity);
                        const auto& collider = coordinator.GetComponent<Components::ColliderBox>(entity);

                        // Een offset (bijv. de helft van zijn breedte) zorgt dat hij stevig boven het dak staat
                        const float ledgeBoost = collider.width * 0.1f;

                        if (anim.CurrentState == EntityState::climbingOnTheRight)
                        {
                            SetState(anim, EntityState::walkingLeft);
                            // Pinguïn was rechts, we duwen hem naar links het dak op
                            transform.Position.x -= ledgeBoost;
                        }
                        else // climbingOnTheLeft
                        {
                            SetState(anim, EntityState::walkingRight);
                            // Pinguïn was links, we duwen hem naar rechts het dak op
                            transform.Position.x += ledgeBoost;
                        }
                    }
                    else
                    {
                        // Oeps, misgepakt bij de rand...
                        SetState(anim, EntityState::tumbling);
                    }
                }
            }
        }

        void ProcessRandomAI(Kaizen::Logic::Coordinator& coordinator)
        {
            auto penguins = GetPenguinEntities(coordinator);

            for (const auto& entity : penguins)
            {
                auto& anim = coordinator.GetComponent<AnimationComponent>(entity);

                //if (!IsWalkingState(anim.CurrentState)) [[unlikely]]
                    //continue;

                //static std::random_device rd;
                //static std::mt19937 s_Generator(rd());
                //static std::uniform_int_distribution<> m_ActivityDist(1, 1 * 30 * 200); // 10% * 60 (s/min) * FPS
                //static std::uniform_int_distribution<> m_StateDist(0, Constants::Systems::MAX_IDLE_STATE - 1);

                if (m_ActivityDist(s_Generator) != 1) [[likely]]
                    continue;

                if (IsWalkingState(anim.CurrentState)) [[likely]]
                {
                    const int stateChoice = m_StateDist(s_Generator);

                    if (stateChoice == Constants::Systems::READING_PROBABILITY)
                        SetState(anim, EntityState::reading);
                    else if (stateChoice == Constants::Systems::DIGGING_PROBABILITY)
                        SetState(anim, EntityState::digging);
                    else
                        SetState(anim, EntityState::idle);
                }
                else if (IsIdleActivityState(anim.CurrentState)) [[unlikely]]
                {
                    SetState(anim, FlipCoin() ? EntityState::walkingLeft : EntityState::walkingRight);
                }
            }
        }

        void HandleHorizontalCollision(AnimationComponent& anim, const float normalX) noexcept
        {
            if (IsIdleActivityState(anim.CurrentState))
                return;

            if (IsClimbingState(anim.CurrentState))
                return;

            const bool shouldClimb = FlipCoin();

            if (normalX < 0.0f)
                SetState(anim, shouldClimb ? EntityState::climbingOnTheLeft : EntityState::walkingLeft);
            else
                SetState(anim, shouldClimb ? EntityState::climbingOnTheRight : EntityState::walkingRight);
        }

        void HandleVerticalCollision(AnimationComponent& anim, const float normalY) noexcept
        {
            if (normalY < 0.0f)
            {
                // Landing on surface
                if (anim.CurrentState == EntityState::falling ||
                    anim.CurrentState == EntityState::tumbling ||
                    anim.CurrentState == EntityState::floating) [[likely]]
                {
                    SetState(anim, FlipCoin() ? EntityState::walkingRight : EntityState::walkingLeft);
                }
            }
            else
            {
                // Hit from below
                SetState(anim, FlipCoin() ? EntityState::falling : EntityState::tumbling);
            }
        }

        void SetState(AnimationComponent& anim, const EntityState newState) noexcept
        {
            if (anim.CurrentState == newState)
                return;

            anim.CurrentState = newState;
            anim.CurrentFrame = 0;
            anim.Timer = 0.0f;
        }

        [[nodiscard]] bool FlipCoin() noexcept
        {
            //static std::random_device rd;
            //static std::mt19937 s_Generator(rd());
            return s_CoinFlip(s_Generator);
        }
    };
}

#endif // !_BEHAVIOUR_SYSTEM_H_