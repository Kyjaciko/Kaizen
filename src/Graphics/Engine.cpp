////////////////////////////////////////////////////////////////////////////////
// Filename: Engine.cpp
////////////////////////////////////////////////////////////////////////////////

#include "Engine.h"

namespace EngineName
{
	bool Engine::Init(HINSTANCE hInstance, std::string windowTitle, std::string windowClass, int width, int height)
	{
		m_hpTimer = HPTimer();

		if (!m_windowFactory.Init(this, hInstance, windowTitle, windowClass, width, height))
			return false;

		if (!m_gfx.Init(m_windowFactory.GetHWnd(), width, height))
			return false;

		return true;
	}

	void Engine::Run()
	{
		// Loop until there is a quit message from the window or the user.
		while (m_windowFactory.HandleMessages())
		{
			m_hpTimer.Update();
			Update(m_hpTimer.GetDeltaTime());
			Render(m_hpTimer.GetDeltaTime());
		}
	}

	void Engine::Update(double deltaTime)
	{
		static DirectX11::Camera3D* camera = m_gfx.GetCamera(10, 10);

		while (!m_keyboard.IsCharBufferEmpty())
		{
			unsigned char character = m_keyboard.ReadChar();
		}

		while (!m_keyboard.IsKeyBufferEmpty())
		{
			windows::KeyboardEvent event = m_keyboard.ReadKey();
			unsigned char key = event.GetKey();
		}

		while (!m_mouse.IsEventBufferEmpty())
		{
			windows::MouseEvent event = m_mouse.ReadEvent();

			if (m_mouse.IsLeftPressed() && event.GetEventType() == windows::MouseEvent::EventType::LEFT_DOWN)
				camera = m_gfx.GetCamera(event.GetPosX(), event.GetPosY());

			// Change camera rotation based on mouse movement.
			if (m_mouse.IsRightPressed() && event.GetEventType() == windows::MouseEvent::EventType::RAW_MOVE && camera)
				camera->AdjustRotation(static_cast<float>(event.GetPosY()) * 0.001f, static_cast<float>(event.GetPosX()) * 0.001f, 0);

			if (m_mouse.IsMiddlePressed() && event.GetEventType() == windows::MouseEvent::EventType::MIDDLE_DOWN)
				m_gfx.SwitchDualView();
		}

		m_gfx.GetGameObject()->AdjustRotation(0.0f, 1.0f * deltaTime, 0.0f);

		///////////////////
		// ADJUST CAMERA //
		///////////////////
		using namespace DirectX;
		if (!camera) return;

		float camera_speed = 1.0f;
		if (m_keyboard.IsKeyPressed(VK_SPACE))
			camera_speed = 50.0f;
		if (m_keyboard.IsKeyPressed('Z'))
			camera->AdjustPosition(camera->GetForwardVector() * camera_speed * deltaTime);
		if (m_keyboard.IsKeyPressed('S'))
			camera->AdjustPosition(camera->GetBackwardVector() * camera_speed * deltaTime);
		if (m_keyboard.IsKeyPressed('Q'))
			camera->AdjustPosition(camera->GetLeftVector() * camera_speed * deltaTime);
		if (m_keyboard.IsKeyPressed('D'))
			camera->AdjustPosition(camera->GetRightVector() * camera_speed * deltaTime);
		if (m_keyboard.IsKeyPressed(VK_SHIFT))
			camera->AdjustPosition(0.0f, camera_speed * deltaTime, 0.0f);
		if (m_keyboard.IsKeyPressed(VK_CONTROL))
			camera->AdjustPosition(0.0f, -camera_speed * deltaTime, 0.0f);

		if (m_keyboard.IsKeyPressed('A'))
		{
			DirectX::XMVECTOR light_position = camera->GetPositionVector();
			light_position += camera->GetForwardVector();
			m_gfx.GetLight()->SetPosition(light_position);
			m_gfx.GetLight()->SetRotation(camera->GetRotationFloat3());
		}
	}

	void Engine::Render(double deltaTime)
	{
		m_gfx.RenderFrame(deltaTime);
	}
}