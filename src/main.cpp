////////////////////////////////////////////////////////////////////////////////
// Filename: main.cpp
////////////////////////////////////////////////////////////////////////////////

#include "Graphics/Engine.h"

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) 
{
	// Needed to properly load texture with DirectXTK.
	HRESULT hr = CoInitialize(nullptr);
	if (FAILED(hr)) return -1;

	Kaizen::Engine* engine = new Kaizen::Engine();
	if (engine->Init(hInstance, "Kaizen Game Engine", "Kaizen", 640, 480,
		windows::Types::WindowFlags::Fullscreen | windows::Types::WindowFlags::Transparent))
	{
		engine->Run();
	}

	delete engine;
	return 0;
}