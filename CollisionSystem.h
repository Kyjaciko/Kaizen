////////////////////////////////////////////////////////////////////////////////
// Filename: CollisionSystem.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _COLLISION_SYSTEM_H_
#define _COLLISION_SYSTEM_H_

//////////////
// INCLUDES //
//////////////

#include <span>
#include <cmath>
#include <random>
#include <vector>
#include <ranges>
#include <utility>
#include <concepts>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "Coordinator.h"
#include "Components.h"
#include "TransformComponent.h"
#include "Containers/UniformGrid.h"
#include "WindowSystem.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: CollisionSystem
////////////////////////////////////////////////////////////////////////////////

namespace WherePenguinsDwell
{
    /**
     * @brief CollisionSystem handles the physical interaction between colliders.
     */
    class CollisionSystem : public Kaizen::Logic::System
    {
        using Grid = Kaizen::Container::UniformGrid<Components::ColliderBox>;

        struct CollisionResult 
        {
            bool isColliding = false;

            float overlapX = 0.0f;
            float overlapY = 0.0f;
            float normalX = 0.0f;
            float normalY = 0.0f;
        };

    public:
        void Init(Grid* grid, const float crushThreshold, const int gameFPS) noexcept
        {
            if (!grid) [[unlikely]]
                return;

            m_Grid = grid;
			m_CrushThreshold = crushThreshold;
			m_FPS = gameFPS;
        }

        /**
         * @brief Main update loop for collisions.
         */
        void Update(Kaizen::Logic::Coordinator& coordinator) const
        {
            if (!m_Grid) [[unlikely]]
                return;

            m_Grid->Clear();
			this->FillGrid(coordinator);
            m_Grid->Build(); // Sort & indexing.

            m_Grid->CheckCollisions([&](Components::ColliderBox* a, Components::ColliderBox* b) {
				this->CheckCollisions(coordinator, a, b, this->m_CrushThreshold);
            });




            auto penguins = m_Entities | std::views::filter([&](const Entity& e) {
                return !coordinator.HasComponent<Tags::WindowColliderTag>(e);
            });

            for (const auto& entity : penguins)
            {
                const auto& collider = coordinator.GetComponent<Components::ColliderBox>(entity);
                if (collider.hasContactThisFrame) [[likely]]
                    continue;

                auto& anim = coordinator.GetComponent<AnimationComponent>(entity);
                if (anim.CurrentState != EntityState::climbingOnTheLeft && anim.CurrentState != EntityState::climbingOnTheRight &&
                    anim.CurrentState != EntityState::walkingLeft && anim.CurrentState != EntityState::walkingRight) [[unlikely]]
                    continue;

                SetState(anim, EntityState::tumbling);
            }


            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_int_distribution<> dis(1, 1 * 30 * m_FPS); // 10% * 60 (s/min) * FPS
            for (const auto& entity : penguins)
            {
                auto& anim = coordinator.GetComponent<AnimationComponent>(entity);
                if (anim.CurrentState != EntityState::walkingLeft && anim.CurrentState != EntityState::walkingRight) [[unlikely]]
                    continue;
                if (dis(gen) == 1)
                {
                    int r = rand() % 3;
                    if (r == 0)
                        SetState(anim, EntityState::reading);
                    else if (r == 1)
                        SetState(anim, EntityState::digging);
                    else
                        SetState(anim, EntityState::idle);
                }
            }

            for (const auto& entity : penguins)
            {
                auto& anim = coordinator.GetComponent<AnimationComponent>(entity);
                if (anim.CurrentState != EntityState::reading && anim.CurrentState != EntityState::digging && anim.CurrentState != EntityState::idle) [[unlikely]]
                    continue;

                if (dis(gen) == 1)
                {
                    if (rand() % 2 == 0)
                        SetState(anim, EntityState::walkingLeft);
                    else
                        SetState(anim, EntityState::walkingRight);
                }
            }
        }

        // API for Gameplay: Give me all entities wihtin a area.
        /*std::vector<Entity> GetEntitiesInRect(const Rect& searchRect, Kaizen::Logic::Coordinator& coordinator)
        {
            std::vector<Entity> foundEntities;
            std::vector<BoxColliderComponent*> potentialColliders;

            // Broad Phase: Get all entities in the cells that intersect with the rectangle.
            m_Grid->Query(searchRect, potentialColliders);

            // Narrow Phase: Exact check
            for (auto* collider : potentialColliders)
            {
                auto& transform = coordinator.GetComponent<Kaizen::Components::TransformComponent>(collider->owner);

                if (AABBIntersect(searchRect, collider, transform))
                {
                    foundEntities.push_back(collider->owner);
                }
            }

            return foundEntities;
        }*/

    private:
		int m_FPS = 0;
        float m_CrushThreshold = 0.f;
        Grid* m_Grid = nullptr; // TODO: Reference should be used instead of a raw pointer.
		                        // But the current ECS system doesn't support construction based implementations.
                                // Fix this cheesy code problem in the ECS system >:[.

    private:
        [[nodiscard]] constexpr Kaizen::Container::Rect GetRect(const auto& transform, const auto& collider) const noexcept
        {
            return { 
                transform.Position.x + collider.offsetX, 
                transform.Position.y + collider.offsetY, 
                collider.width, 
                collider.height 
            };
        }

        void SetState(AnimationComponent& anim, const EntityState newState) const noexcept
        {
            if (anim.CurrentState == newState)
                return;

            anim.CurrentState = newState;
            anim.CurrentFrame = 0;
            anim.Timer = 0.0f;
        }

        void FillGrid(Kaizen::Logic::Coordinator& coordinator) const
        {
            for (const auto& entity : m_Entities)
            {
                const auto& transform = coordinator.GetComponent<Kaizen::Components::TransformComponent>(entity);
                auto& collider = coordinator.GetComponent<Components::ColliderBox>(entity);

                collider.owner = entity;
                collider.hasContactThisFrame = false;

                m_Grid->Insert(&collider, Kaizen::Container::Rect{
                    .x = transform.Position.x + collider.offsetX,
                    .y = transform.Position.y + collider.offsetY,
                    .w = collider.width,
                    .h = collider.height
                });
            }
        }

        [[nodiscard]] CollisionResult CalculateCollision(const Kaizen::Container::Rect& rP, const Kaizen::Container::Rect& rW, const AnimationComponent& anim) const noexcept
        {
            CollisionResult result{};

            const float aCenterX = rP.x + rP.w / 2.0f;
            const float aCenterY = rP.y + rP.h / 2.0f;
            const float bCenterX = rW.x + rW.w / 2.0f;
            const float bCenterY = rW.y + rW.h / 2.0f;

            const float dx = aCenterX - bCenterX;
            const float dy = aCenterY - bCenterY;

            // Calculate the minimum distance to NOT have a collision.
            const float minDistanceX = (rP.w / 2.0f) + (rW.w / 2.0f);
            const float minDistanceY = (rP.h / 2.0f) + (rW.h / 2.0f);

            // TODO: Get rid of this stupid fix!
            // Since there is no 'gravity' in the x direction, there will be no collision once the penguin starts climbing
            // So it will immediately start falling again, quick fix by applying a offset...
            float EPSILON = 0.f;
            if (anim.CurrentState == EntityState::climbingOnTheLeft || anim.CurrentState == EntityState::climbingOnTheRight)
                EPSILON = 0.1f;

            // If we are not within the minimum distance, there will be NO collision.
            if (std::abs(dx) >= minDistanceX + EPSILON || std::abs(dy) >= minDistanceY)
				return result; // TODO: Should return invalid result...

            result = {
                .isColliding = true,
                .overlapX = minDistanceX - std::abs(dx),
                .overlapY = minDistanceY - std::abs(dy),
                .normalX = dx > 0 ? 1.0f : -1.0f,
                .normalY = dy > 0 ? 1.0f : -1.0f
            };

            return result;
        }

        void ResolveCollision(Kaizen::Logic::Coordinator& coordinator, const Entity penguin, const CollisionResult& res, const float crushThreshold) const
        {
            auto& transform = coordinator.GetComponent<Kaizen::Components::TransformComponent>(penguin);
            auto& anim = coordinator.GetComponent<AnimationComponent>(penguin);

            const float pushX = res.normalX;
            const float pushY = res.normalY;

            if (res.overlapX < res.overlapY) // X-axis resolution.
            {
                if (res.overlapX > crushThreshold)
                    SetState(anim, EntityState::exploding);
                else 
                {
                    transform.Position.x += res.overlapX * pushX;
                    HandleHorizontalState(anim, pushX);
                }
            }
            else // Y-axis resolution.
            {
                if (res.overlapY > crushThreshold)
                    SetState(anim, EntityState::exploding);
                else 
                {
                    transform.Position.y += res.overlapY * pushY;
                    HandleVerticalState(anim, pushY);
                }
            }
        }

        void CheckCollisions(Kaizen::Logic::Coordinator& coordinator, Components::ColliderBox* a, Components::ColliderBox* b, const float crushThreshold) const
        {
			if (a->owner == b->owner) [[unlikely]]
                return;

            const bool windowA = coordinator.HasComponent<Tags::WindowColliderTag>(a->owner);
            const bool windowB = coordinator.HasComponent<Tags::WindowColliderTag>(b->owner);

            // Ignore Window-Window (true-true) or Penguin-Penguin collisions (false-false).
            if (windowA == windowB) [[likely]]
                return;

			// Determine which entity is the penguin and which is the window.
            const auto [penguinEntity, windowEntity] = windowA ? std::pair{ b->owner, a->owner } : std::pair{ a->owner, b->owner };

			// No need for a collision check if penguin is already dead :(.
            auto& anim = coordinator.GetComponent<AnimationComponent>(penguinEntity);
            if (anim.CurrentState == EntityState::exploding) [[unlikely]]
                return;

            // Fetch all data required for collision check.
            const auto& tP = coordinator.GetComponent<Kaizen::Components::TransformComponent>(penguinEntity);
            const auto& tW = coordinator.GetComponent<Kaizen::Components::TransformComponent>(windowEntity);
            auto& cP = coordinator.GetComponent<Components::ColliderBox>(penguinEntity);
            const auto& cW = coordinator.GetComponent<Components::ColliderBox>(windowEntity);

            const auto rP = GetRect(tP, cP);
            const auto rW = GetRect(tW, cW);

            CollisionResult collision = CalculateCollision(rP, rW, anim);
            if (!collision.isColliding)
                return;

            cP.hasContactThisFrame = true;
            ResolveCollision(coordinator, penguinEntity, collision, crushThreshold);
        }

        [[nodiscard]] bool FlipCoin() const noexcept
        {
            thread_local std::mt19937 rng{ std::random_device{}() };
            thread_local std::bernoulli_distribution dist(0.5);
            return dist(rng);
        }

        void HandleHorizontalState(AnimationComponent& anim, const float pushX) const noexcept
        {
            const bool flip = FlipCoin();
            if (pushX < 0)
                SetState(anim, flip ? EntityState::walkingLeft : EntityState::climbingOnTheLeft);
            else
                SetState(anim, flip ? EntityState::walkingRight : EntityState::climbingOnTheRight);
        }

        void HandleVerticalState(AnimationComponent& anim, const float pushY) const noexcept
        {
            const bool flip = FlipCoin();
            if (pushY < 0)
            {
                if (anim.CurrentState != EntityState::falling && anim.CurrentState != EntityState::tumbling && anim.CurrentState != EntityState::floating)
                    return;

                SetState(anim, flip ? EntityState::walkingRight : EntityState::walkingLeft);
            }
            else
                SetState(anim, flip ? EntityState::falling : EntityState::tumbling);
        }
    };
}

#endif // !_COLLISION_SYSTEM_H_