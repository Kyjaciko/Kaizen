////////////////////////////////////////////////////////////////////////////////
// Filename: WindowSystem.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _WINDOW_SYSTEM_H_
#define _WINDOW_SYSTEM_H_

//////////////
// INCLUDES //
//////////////

#include <vector>
#include <windows.h>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "Demos/Where Penguins Dwell/Core/Constants.h"
#include "../../../Systems/ECS/System.h"
#include "../../../Systems/ECS/Coordinator.h"
#include "WindowEnumerator.h"
#include "TransformComponent.h"
#include "../Systems/Components.h"

//////////
// TAGS //
//////////

namespace WherePenguinsDwell::Tags
{
    struct WindowColliderTag{}; // Purely as a label, to distinguish a window from other entities.
}

////////////////////////////////////////////////////////////////////////////////
// Class name: WindowSystem
////////////////////////////////////////////////////////////////////////////////

namespace WherePenguinsDwell::Systems
{
    class WindowSystem : public Kaizen::Logic::System
    {
    public:
        void Init(const uint32_t fps, const float edgeThickness)
        {
            m_Timer = 0.f;
            m_UpdatePeriod = 1.f / static_cast<float>(fps);

            m_EdgeThickness = edgeThickness;
			m_ScreenWidth = static_cast<float>(GetSystemMetrics(SM_CXSCREEN)); // TODO: graphics should pass this, unnecessary call to windows API.
            m_ScreenHeight = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
        }

        void Update(Kaizen::Logic::Coordinator& coordinator, const float dt)
        {
			// Since windows API calls are expensive, we only update the window entities a few times per second.
            m_Timer += dt;
            if (m_Timer < m_UpdatePeriod)
                return;

            m_Timer -= m_UpdatePeriod;

            // Since we only roughly expect 10-40 windows on the monitors and a max update rate (fps) of 30.
            // So 'Full rebuild' is a good enough choice.
            // 
            // TODO: Avoid destroying entities while iterating over a system container.
            // 'DestroyEntity()' mutates the system entity set, which invalidates their iterators.
            // Fix this code >:[, possible solution:
            //      -> Mark entities for destruction in a queue.
            //      -> Process the queue.
            std::vector<Entity> entitiesToDestroy(m_Entities.begin(), m_Entities.end());
            for (auto entity : entitiesToDestroy)
                coordinator.DestroyEntity(entity);

            std::vector<RECT> windows;
            windows.reserve(Constants::Systems::ESTIMATED_MAX_WINDOW_QUANTITY);

            // Fetch visible windows from the windows api and transform them into entities.
            EnumWindows(Platform::Windows::EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));
            for (const auto& rect : windows)
            {
                // CAREFULL! Since the main monitor is used, the coordinate system (origin point) is the same.
                // Otherwise an offset or transform must be used.
                CreateWindowEntity(
                    coordinator,
                    static_cast<float>(rect.left),
                    static_cast<float>(rect.top),
                    static_cast<float>(rect.right - rect.left),
                    static_cast<float>(rect.bottom - rect.top)
                );
            }

            // Create display edges.
            CreateWindowEntity(coordinator, 0.f, -m_EdgeThickness, m_ScreenWidth, m_EdgeThickness);  // Top.
            CreateWindowEntity(coordinator, 0.f, m_ScreenHeight, m_ScreenWidth, m_EdgeThickness);    // Bottom.
            CreateWindowEntity(coordinator, -m_EdgeThickness, 0.f, m_EdgeThickness, m_ScreenHeight); // Left.
            CreateWindowEntity(coordinator, m_ScreenWidth, 0.f, m_EdgeThickness, m_ScreenHeight);    // Right.
        }

    private:
        float m_EdgeThickness = 0.f;
        float m_ScreenWidth = 0.f, m_ScreenHeight = 0.f;

        float m_Timer = 0.f;
        float m_UpdatePeriod = 0.f;

    private:
        void CreateWindowEntity(Kaizen::Logic::Coordinator& coordinator, float x, float y, float w, float h, float offsetX = 0.f, float offsetY = 0.f) const
        {
            Entity window = coordinator.CreateEntity();

            Kaizen::Components::TransformComponent transform;
            transform.SetPosition(x, y, 0.f);
            coordinator.AddComponent(window, std::move(transform));

            coordinator.AddComponent(window, Components::ColliderBox{
                .owner = window,
                .width = w,
                .height = h,
                .offsetX = offsetX,
                .offsetY = offsetY
            });

            coordinator.AddComponent(window, Tags::WindowColliderTag{});
        }
    };
}

#endif // !_WINDOW_SYSTEM_H_