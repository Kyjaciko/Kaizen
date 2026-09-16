////////////////////////////////////////////////////////////////////////////////
// Filename: main.cpp
////////////////////////////////////////////////////////////////////////////////

#include "Graphics/Engine.h"

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) 
{
	// Needed to properly load texture with DirectXTK.
	HRESULT hr = CoInitialize(nullptr);
	if (FAILED(hr)) return -1;

	EngineName::Engine engine;
	if (engine.Init(hInstance, "Invasion", "DirectX11WindowClass", 1920, 1080))
	{
		engine.Run();
	}

	return 0;
}