////////////////////////////////////////////////////////////////////////////////
// Filename: Graphics.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

//////////////
// INCLUDES //
//////////////

#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>

#include "Dear ImGui/imgui.h"
#include "Dear ImGui/imgui_impl_dx11.h"
#include "Dear ImGui/imgui_impl_win32.h"

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "AdapterReader.h"
#include "Shaders.h"
#include "RenderableGameObject.h"
#include "Camera2D.h"
#include "Camera3D.h"
#include "Light.h"
#include "Sprite.h"
#include "AxisGizmo.h"
#include "InfiniteGrid.h"

#include "../Demos/Demo.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: Graphics
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class Graphics
	{
	public:
		bool Init(HWND hWnd, int width, int height);
		void RenderFrame(double deltaTime);

		Camera3D* GetCamera(int x, int y) 
		{ 
			if (x < static_cast<float>(m_WindowWidth) / 2.f || !m_DualView)
				return &m_Camera; 
			else
				return &m_Camera2; 
		}

		GameObject*  GetGameObject()  { return &m_GameObject; }
		Light* GetLight() { return &m_Light; }

		void SwitchDualView() 
		{ 
			m_DualView = !m_DualView;
			if (m_DualView) 
				m_Camera.SetProjectionValues(90.0f, static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight), .1f, 10000.f);
			else
				m_Camera.SetProjectionValues(90.0f, (static_cast<float>(m_WindowWidth) / 2.f) / static_cast<float>(m_WindowHeight), .1f, 10000.f);
		}

	private:
		int m_WindowWidth;
		int m_WindowHeight;

		Camera2D m_Camera2D;

		bool m_DualView;
		CD3D11_VIEWPORT m_ViewportSingleView;
		CD3D11_VIEWPORT m_ViewportDualView[2];
		Camera3D m_Camera;
		Camera3D m_Camera2;

		Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pDeviceContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_pSwapChain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>	m_pDepthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState>	m_pDepthStencilState;

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_pRasterizerState;
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_pBlendState;

		std::unique_ptr<DirectX::SpriteBatch> m_pSpriteBatch;
		std::unique_ptr<DirectX::SpriteFont> m_pSpriteFont;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerState;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pPinkTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pGrassTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pTomNookTexture;

		// Crisis 3D model.
		RenderableGameObject m_GameObject;
		ConstantBuffer<CB_VS_vertexshader> m_CB_VS_vertexshader;
		PixelShader	m_PixelShader;
		VertexShader m_VertexShader;

		// Light bulb.
		Light m_Light;
		ConstantBuffer<CB_PS_light> m_CB_PS_light;
		PixelShader	m_PixelShaderWithNoLight;

		// Ocean.
		Ocean m_Ocean;
		VertexShader m_VertexShaderOcean;
		PixelShader m_PixelShaderOcean;
		ComputeShader m_ComputeShaderOcean;
		ConstantBuffer<PerlinCB> m_CB_CS_Perlin;
		ConstantBuffer<OceanCB> m_CB_VS_Ocean;

		// Axis.
		AxisGizmo m_Axis;

		// Grid.
		InfiniteGrid m_InfiniteGrid;
		VertexShader m_VertexShaderGrid;
		PixelShader	m_PixelShaderGrid;
		ConstantBuffer<CB_VS_vertexshader_grid> m_CB_VS_vertexshader_grid;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState>	m_pDepthStencilStateTransparant;

		// Penguin sprite.
		Sprite m_Sprite;
		ConstantBuffer<CB_VS_vertexshader_2d> m_CB_VS_vertexshader_2d;
		PixelShader	m_PixelShaderSprite;
		VertexShader m_VertexShaderSprite;

	private:
		bool InitDirectX(HWND hWnd);
		bool InitShaders();
		bool InitScene();

		void RenderScene(const Camera3D& camera, double deltaTime);
	};
}

#endif // !_GRAPHICS_H_