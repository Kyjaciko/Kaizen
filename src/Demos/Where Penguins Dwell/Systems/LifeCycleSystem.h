////////////////////////////////////////////////////////////////////////////////
// Filename: LifeCycleSystem.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _LIFECYCLE_SYSTEM_H_
#define _LIFECYCLE_SYSTEM_H_

#include <vector>
#include <ranges>

#include "../../../Systems/ECS/Coordinator.h"
#include "Components.h"
#include "../Core/Constants.h"
#include "../Platform/WindowSystem.h"
#include "AnimationComponent.h"

namespace WherePenguinsDwell
{
    /**
     * @brief LifeCycleSystem manages entity spawning, destruction, and cleanup.
     */
    class LifeCycleSystem : public Kaizen::Logic::System
    {
    public:
        /**
         * @brief Update lifecycle - clean up dead entities.
         */
        void Update(Kaizen::Logic::Coordinator& coordinator, const float deltaTime)
        {
            ProcessExplodingEntities(coordinator, deltaTime);
        }

        /**
         * @brief Destroy all managed entities (for level cleanup).
         */
        void DestroyAll(Kaizen::Logic::Coordinator& coordinator)
        {
            // Copy to avoid iterator invalidation
            std::vector<Entity> entitiesToDestroy(m_Entities.begin(), m_Entities.end());

            for (const auto& entity : entitiesToDestroy)
                coordinator.DestroyEntity(entity);
        }

        /**
         * @brief Get entities pending destruction this frame.
         */
        [[nodiscard]] auto GetPendingDestructions(Kaizen::Logic::Coordinator& coordinator) const
        {
            return m_Entities | std::views::filter([&](const Entity& e) 
            {
                if (coordinator.HasComponent<Tags::WindowColliderTag>(e)) [[unlikely]]
                    return false;

                const auto& anim = coordinator.GetComponent<AnimationComponent>(e);
                return anim.CurrentState == EntityState::exploding ||
                    anim.CurrentState == EntityState::exploding ||
                    anim.CurrentState == EntityState::splash ||
                    anim.CurrentState == EntityState::zap_walkingLeft ||
                    anim.CurrentState == EntityState::exit_walkingLeft ||
                    anim.CurrentState == EntityState::zap_walkingRight ||
                    anim.CurrentState == EntityState::exit_walkingRight ||
                    anim.CurrentState == EntityState::falling || anim.CurrentState == EntityState::floating || anim.CurrentState == EntityState::tumbling;
            });
        }

    private:
        void ProcessExplodingEntities(Kaizen::Logic::Coordinator& coordinator, const float deltaTime)
        {
            auto penguins = m_Entities | std::views::filter([&](const Entity& e) {
                return !coordinator.HasComponent<Tags::WindowColliderTag>(e);
                });

            // Collect entities to destroy (avoid iterator invalidation)
            std::vector<Entity> toDestroy;
            toDestroy.reserve(8); // Reasonable default

            for (const auto& entity : penguins)
            {
                const auto& anim = coordinator.GetComponent<AnimationComponent>(entity);

                if (anim.CurrentState != EntityState::exploding &&
                    anim.CurrentState != EntityState::splash &&
                    anim.CurrentState != EntityState::zap_walkingLeft &&
                    anim.CurrentState != EntityState::exit_walkingLeft &&
                    anim.CurrentState != EntityState::zap_walkingRight &&
                    anim.CurrentState != EntityState::exit_walkingRight) [[likely]]
                    continue;

                auto& timer = coordinator.GetComponent<Components::TimerComponent>(entity);
				timer.deadTime += deltaTime;
                if (timer.deadTime <= Constants::Systems::EXPLOSION_CLEANUP_TIME) // TODO: No need for a timer, just use FrameCount.
                    continue;

                toDestroy.push_back(entity);
            }

            // Destroy all collected entities
            for (const auto& entity : toDestroy)
                coordinator.DestroyEntity(entity);
        }
    };
}

#endif // !_LIFECYCLE_SYSTEM_H_