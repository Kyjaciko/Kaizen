////////////////////////////////////////////////////////////////////////////////
// Filename: Engine.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _ENGINE_H_
#define _ENGINE_H_

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "../WindowsTypes.h"
#include "../Platform/WindowContainer.h"
#include "../Platform/Timer/HPTimer.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: Engine
////////////////////////////////////////////////////////////////////////////////

namespace Kaizen
{
	class Engine : public windows::WindowContainer
	{
	public:
		~Engine();

		bool Init(HINSTANCE hInstance, std::string windowTitle, std::string windowClass, int width, int height, windows::Types::WindowFlags flags);
		void Run();

	private:
		HPTimer* m_hpTimer = nullptr;

	private:
		void Update(double deltaTime);
		void Render(double deltaTime);
	};
}

#endif // !_ENGINE_H_