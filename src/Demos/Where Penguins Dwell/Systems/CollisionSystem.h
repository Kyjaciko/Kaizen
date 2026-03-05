////////////////////////////////////////////////////////////////////////////////
// Filename: CollisionSystem.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _COLLISION_SYSTEM_H_
#define _COLLISION_SYSTEM_H_

#include <span>
#include <cmath>
#include <ranges>
#include <utility>
#include <concepts>
#include <optional>

#include "../Core/Constants.h"
#include "Components.h"
#include "../../../Systems/ECS/Coordinator.h"
#include "TransformComponent.h"
#include "Containers/UniformGrid.h"
#include "../Platform/WindowSystem.h"

namespace WherePenguinsDwell
{
    /**
     * @brief CollisionSystem handles pure collision detection and resolution.
     * Separated concerns: only detects and resolves, does not manage state or lifecycle.
     */
    class CollisionSystem : public Kaizen::Logic::System
    {
        using Grid = Kaizen::Container::UniformGrid<Components::ColliderBox>;

    public:
        struct CollisionResult
        {
            bool isColliding = false;
            float overlapX = 0.0f;
            float overlapY = 0.0f;
            float normalX = 0.0f;
            float normalY = 0.0f;
        };

        struct CollisionEvent
        {
            Entity penguinEntity;
            Entity windowEntity;
            CollisionResult result;
            bool shouldCrush = false;
        };

        void Init(Grid* grid, const float crushThreshold) noexcept
        {
            if (!grid) [[unlikely]]
                return;

            m_Grid = grid;
            m_CrushThreshold = crushThreshold;
        }

        /**
         * @brief Update collision detection and resolution.
         * Returns collision events for other systems to process.
         */
        [[nodiscard]] std::vector<CollisionEvent> Update(Kaizen::Logic::Coordinator& coordinator)
        {
            if (!m_Grid) [[unlikely]]
                return {};

            std::vector<CollisionEvent> events;
            events.reserve(m_Entities.size());

            // Reset grid and rebuild.
            m_Grid->Clear();
            FillGrid(coordinator);
            m_Grid->Build();

            m_Grid->CheckCollisions([&](Components::ColliderBox* a, Components::ColliderBox* b) {
                if (auto event = this->DetectCollision(coordinator, a, b); event.has_value())
                {
                    if (!event->shouldCrush)
                        ResolveCollision(coordinator, *event);

                    events.push_back(std::move(*event));
                }
            });

            return events;
        }

        /**
         * @brief Get entities that lost contact this frame (for behaviour system).
         */
        [[nodiscard]] std::vector<Entity> GetEntitiesWithoutContact(Kaizen::Logic::Coordinator& coordinator) const
        {
            std::vector<Entity> result;
            result.reserve(m_Entities.size() / 4); // Estimate: ~25% lose contact

            for (const auto& entity : m_Entities)
            {
                if (coordinator.HasComponent<Tags::WindowColliderTag>(entity))
                    continue;

                const auto& collider = coordinator.GetComponent<Components::ColliderBox>(entity);
                if (!collider.hasContactThisFrame)
                    result.push_back(entity);
            }

            return result;
        }

    private:
        float m_CrushThreshold = 0.0f;
        Grid* m_Grid = nullptr;

        [[nodiscard]] constexpr Kaizen::Container::Rect GetRect(
            const auto& transform,
            const auto& collider) const noexcept
        {
            return {
                transform.Position.x + collider.offsetX,
                transform.Position.y + collider.offsetY,
                collider.width,
                collider.height
            };
        }

        void FillGrid(Kaizen::Logic::Coordinator& coordinator) const
        {
            for (const auto& entity : m_Entities)
            {
                const auto& transform = coordinator.GetComponent<Kaizen::Components::TransformComponent>(entity);
                auto& collider = coordinator.GetComponent<Components::ColliderBox>(entity);

                collider.owner = entity;
                collider.hasContactThisFrame = false;

                m_Grid->Insert(&collider, GetRect(transform, collider));
            }
        }

        [[nodiscard]] std::optional<CollisionEvent> DetectCollision(
            Kaizen::Logic::Coordinator& coordinator,
            Components::ColliderBox* a,
            Components::ColliderBox* b) const
        {
            if (a->owner == b->owner) [[unlikely]]
                return std::nullopt;

            const bool windowA = coordinator.HasComponent<Tags::WindowColliderTag>(a->owner);
            const bool windowB = coordinator.HasComponent<Tags::WindowColliderTag>(b->owner);

            // Ignore Window-Window or Penguin-Penguin
            if (windowA == windowB) [[likely]]
                return std::nullopt;

            // Determine penguin and window
            const auto [penguinEntity, windowEntity] = windowA
                ? std::pair{ b->owner, a->owner }
            : std::pair{ a->owner, b->owner };

            // Skip if already exploding
            const auto& anim = coordinator.GetComponent<AnimationComponent>(penguinEntity);
            if (anim.CurrentState == EntityState::exploding) [[unlikely]]
                return std::nullopt;

            // Get collision data
            const auto& tP = coordinator.GetComponent<Kaizen::Components::TransformComponent>(penguinEntity);
            const auto& tW = coordinator.GetComponent<Kaizen::Components::TransformComponent>(windowEntity);
            auto& cP = coordinator.GetComponent<Components::ColliderBox>(penguinEntity);
            const auto& cW = coordinator.GetComponent<Components::ColliderBox>(windowEntity);

            const auto rP = GetRect(tP, cP);
            const auto rW = GetRect(tW, cW);

            CollisionResult collision = CalculateCollision(rP, rW, anim);

            if (!collision.isColliding)
                return std::nullopt;

            cP.hasContactThisFrame = true;

            // Determine if this should crush
            const float overlap = (collision.overlapX < collision.overlapY)
                ? collision.overlapX
                : collision.overlapY;

            return CollisionEvent{
                .penguinEntity = penguinEntity,
                .windowEntity = windowEntity,
                .result = collision,
                .shouldCrush = overlap > m_CrushThreshold
            };
        }

        [[nodiscard]] CollisionResult CalculateCollision(
            const Kaizen::Container::Rect& rP,
            const Kaizen::Container::Rect& rW,
            const AnimationComponent& anim) const noexcept
        {
            CollisionResult result{};

            const float aCenterX = rP.x + rP.w * 0.5f;
            const float aCenterY = rP.y + rP.h * 0.5f;
            const float bCenterX = rW.x + rW.w * 0.5f;
            const float bCenterY = rW.y + rW.h * 0.5f;

            const float dx = aCenterX - bCenterX;
            const float dy = aCenterY - bCenterY;

            const float minDistanceX = (rP.w + rW.w) * 0.5f;
            const float minDistanceY = (rP.h + rW.h) * 0.5f;

            // Climbing epsilon fix
            const float epsilon = (anim.CurrentState == EntityState::climbingOnTheLeft ||
                anim.CurrentState == EntityState::climbingOnTheRight)
                ? Constants::Systems::CLIMBING_EPSILON
                : 0.0f;

            if (std::abs(dx) >= minDistanceX + epsilon || std::abs(dy) >= minDistanceY)
                return result;

            result = {
                .isColliding = true,
                .overlapX = minDistanceX - std::abs(dx),
                .overlapY = minDistanceY - std::abs(dy),
                .normalX = dx > 0.0f ? 1.0f : -1.0f,
                .normalY = dy > 0.0f ? 1.0f : -1.0f
            };

            return result;
        }

        void ResolveCollision(
            Kaizen::Logic::Coordinator& coordinator,
            const CollisionEvent& event) const
        {
            auto& transform = coordinator.GetComponent<Kaizen::Components::TransformComponent>(event.penguinEntity);

            const auto& res = event.result;
            const float pushX = res.normalX;
            const float pushY = res.normalY;

            // Resolve on minimum overlap axis
            if (res.overlapX < res.overlapY)
                transform.Position.x += res.overlapX * pushX;
            else
                transform.Position.y += res.overlapY * pushY;
        }
    };
}

#endif // !_COLLISION_SYSTEM_H_