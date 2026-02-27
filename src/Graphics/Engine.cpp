////////////////////////////////////////////////////////////////////////////////
// Filename: Engine.cpp
////////////////////////////////////////////////////////////////////////////////

#include "Engine.h"

namespace Kaizen
{
	Engine::~Engine()
	{
		delete m_hpTimer;
	}

	bool Engine::Init(HINSTANCE hInstance, std::string windowTitle, std::string windowClass, int width, int height, windows::Types::WindowFlags flags)
	{
		m_hpTimer = new HPTimer();

		if (!m_WindowFactory->Init(this, hInstance, windowTitle, windowClass, width, height, flags))
			return false;

		if (!m_gfx->Init(m_WindowFactory->GetHWnd(), m_WindowFactory->GetWidth(), m_WindowFactory->GetHeight(), flags))
			return false;

		return true;
	}

	void Engine::Run()
	{
		// Loop until there is a quit message from the window or the user.
		while (m_WindowFactory->HandleMessages())
		{
			m_hpTimer->Update();
			Update(m_hpTimer->GetDeltaTime());
			Render(m_hpTimer->GetDeltaTime());
		}
	}

	void Engine::Update(double deltaTime)
	{
		while (!m_Keyboard->IsCharBufferEmpty())
		{
			unsigned char character = m_Keyboard->ReadChar();
		}

		while (!m_Keyboard->IsKeyBufferEmpty())
		{
			windows::KeyboardEvent event = m_Keyboard->ReadKey();
			unsigned char key = event.GetKey();
		}

		while (!m_Mouse->IsEventBufferEmpty())
		{
			windows::MouseEvent event = m_Mouse->ReadEvent();

			// Change camera rotation based on mouse movement.
			if (m_Mouse->IsRightPressed() && event.GetEventType() == windows::MouseEvent::EventType::RAW_MOVE)
				m_gfx->GetCamera()->AdjustRotation(static_cast<float>(event.GetPosY()) * 0.001f, static_cast<float>(event.GetPosX()) * 0.001f, 0);
		}

		m_gfx->GetGameObject()->AdjustRotation(0.0f, 1.0f * deltaTime, 0.0f);

		///////////////////
		// ADJUST CAMERA //
		///////////////////
		using namespace DirectX;

		float camera_speed = 1.0f;
		DirectX11::Camera3D* camera = m_gfx->GetCamera();
		if (m_Keyboard->IsKeyPressed(VK_SPACE))
			camera_speed = 50.0f;
		if (m_Keyboard->IsKeyPressed('Z'))
			camera->AdjustPosition(camera->GetForwardVector() * camera_speed * deltaTime);
		if (m_Keyboard->IsKeyPressed('S'))
			camera->AdjustPosition(camera->GetBackwardVector() * camera_speed * deltaTime);
		if (m_Keyboard->IsKeyPressed('Q'))
			camera->AdjustPosition(camera->GetLeftVector() * camera_speed * deltaTime);
		if (m_Keyboard->IsKeyPressed('D'))
			camera->AdjustPosition(camera->GetRightVector() * camera_speed * deltaTime);
		if (m_Keyboard->IsKeyPressed(VK_SHIFT))
			camera->AdjustPosition(0.0f, camera_speed * deltaTime, 0.0f);
		if (m_Keyboard->IsKeyPressed(VK_CONTROL))
			camera->AdjustPosition(0.0f, -camera_speed * deltaTime, 0.0f);

		if (m_Keyboard->IsKeyPressed('A'))
		{
			DirectX::XMVECTOR light_position = camera->GetPositionVector();
			light_position += camera->GetForwardVector();
			m_gfx->GetLight()->SetPosition(light_position);
			m_gfx->GetLight()->SetRotation(camera->GetRotationFloat3());
		}
	}

	void Engine::Render(double deltaTime)
	{
		m_gfx->RenderFrame(m_WindowFactory->GetHWnd(), deltaTime);
	}
}