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



//
#include <random>
#include "../Settings.h"
#include "../ResourceManager.h"
#include "../MeshResource.h"
#include "../Coordinator.h"
#include "../RenderSystem.h"
#include "../Material.h"
#include "../PrimitiveFactory.h"
#include "../Containers/UniformGrid.h"
#include "../Components.h"
#include "../CollisionSystem.h"
#include "../WindowSystem.h"
#include "../MovementSystem.h"
#include "../WindowsTypes.h"
#include "../WindowsHelpers.h"
#include "../Demos/Where Penguins Dwell/Core/Settings.h"
#include "../BehaviourSystem.h"
#include "../LifeCycleSystem.h"

//////////////
// SETTINGS //
//////////////

#if USE_DIRECT_COMPOSITION
	#include <dcomp.h>
	#pragma comment(lib, "dcomp.lib")
#endif

#if USE_DIRECT_COMPOSITION
	using SwapChainType = IDXGISwapChain1;
#else
	using SwapChainType = IDXGISwapChain;
#endif

////////////////////////////////////////////////////////////////////////////////
// Class name: Graphics
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class Graphics
	{
	public:
		bool Init(HWND hWnd, int width, int height, windows::Types::WindowFlags flags);
		void RenderFrame(HWND hWnd, double deltaTime);

		[[nodiscard]] bool OnShutdown(HWND hWnd, double deltaTime);

		Camera3D* GetCamera() { return &m_Camera; }
		GameObject*  GetGameObject()  { return &m_GameObject; }
		Light* GetLight() { return &m_Light; }

	private:
		float m_Alfa;
		int m_WindowWidth;
		int m_WindowHeight;

		AxisGizmo											m_Axis;

		Camera2D											m_Camera2D;
		Camera3D											m_Camera;

		Microsoft::WRL::ComPtr<ID3D11Device>				m_pDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>			m_pDeviceContext;
		Microsoft::WRL::ComPtr<SwapChainType>				m_pSwapChain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_pRenderTargetView;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		m_pDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>				m_pDepthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState>		m_pDepthStencilState;

		Microsoft::WRL::ComPtr<ID3D11RasterizerState>		m_pRasterizerState;
		Microsoft::WRL::ComPtr<ID3D11BlendState>			m_pBlendState;
		Microsoft::WRL::ComPtr<ID3D11SamplerState>			m_pSamplerState;

		// TEST
		VertexShader										m_VertexShaderLine;
		PixelShader											m_PixelShaderLine;
		// TEST

		VertexShader										m_VertexShader;
		//VertexShader										m_VertexShaderSprite;
		PixelShader											m_PixelShader;
		PixelShader											m_PixelShaderWithNoLight;
		//PixelShader											m_PixelShaderSprite;

		ConstantBuffer<CB_VS_vertexshader>					m_CB_VS_vertexshader;
		//ConstantBuffer<CB_PS_pixelshader>					m_CB_PS_pixelshader;
		ConstantBuffer<CB_PS_light>							m_CB_PS_light;
		ConstantBuffer<CB_VS_vertexshader_2d>				m_CB_VS_vertexshader_2d;

		RenderableGameObject								m_GameObject;
		Light												m_Light;
		Sprite												m_Sprite;

		std::unique_ptr<DirectX::SpriteBatch>				m_pSpriteBatch;
		std::unique_ptr<DirectX::SpriteFont>				m_pSpriteFont;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_pPinkTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_pGrassTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_pTomNookTexture;

		// Grid
		InfiniteGrid										m_InfiniteGrid;
		VertexShader										m_VertexShaderGrid;
		PixelShader											m_PixelShaderGrid;
		ConstantBuffer<CB_VS_vertexshader_grid>				m_CB_VS_vertexshader_grid;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState>		m_pDepthStencilStateTransparant;

		// Ocean
		Ocean												m_Ocean;
		VertexShader										m_VertexShaderOcean;
		PixelShader											m_PixelShaderOcean;
		ComputeShader										m_ComputeShaderOcean;
		ConstantBuffer<PerlinCB>							m_CB_CS_Perlin;
		ConstantBuffer<OceanCB>								m_CB_VS_Ocean;



		// 
		//Kaizen::Graphics::TextureManager					m_TextureManager;
		//ConstantBuffer<CB_VS_vertexshader_2d>				m_CB_VS_vertexshader_2d;
		//VertexShader										m_VertexShader;

		Kaizen::Logic::Coordinator				m_Coordinator;

		std::shared_ptr<Kaizen::Resources::PrimitiveFactory>	m_AssetManager;
		std::shared_ptr<Kaizen::Resources::TextureManager>		m_TextureManager;
		std::shared_ptr<Kaizen::Resources::VertexShaderManager>	m_VertexShaderManager;
		std::shared_ptr<Kaizen::Resources::PixelShaderManager>	m_PixelShaderManager;

		std::shared_ptr<Kaizen::Graphics::RenderSystem>			m_RenderSystem;
		std::shared_ptr<Kaizen::Graphics::TransformSystem>		m_TransformSystem;
		std::shared_ptr<Kaizen::Graphics::AnimationSystem>		m_AnimationSystem;
		std::shared_ptr<WherePenguinsDwell::CollisionSystem>	m_CollisionSystem;
		std::shared_ptr<WherePenguinsDwell::Systems::WindowSystem>		m_WindowSystem;
		std::shared_ptr<WherePenguinsDwell::MovementSystem>		m_MovementSystem;
		std::shared_ptr<WherePenguinsDwell::BehaviourSystem>		m_BehaviourSystem;
		std::shared_ptr<WherePenguinsDwell::LifeCycleSystem>		m_LifeCycleSystem;

		ConstantBuffer<CB_Frame>	m_CB_Frame;
		ConstantBuffer<CB_Object>	m_CB_Object;

		Kaizen::Types::ResourceID	m_SpritesheetTextureID;
		Kaizen::Types::ResourceID	m_VertexShaderSprite;
		Kaizen::Types::ResourceID	m_PixelShaderSprite;

		std::shared_ptr<Kaizen::Container::UniformGrid<WherePenguinsDwell::Components::ColliderBox>>	m_UniformGrid;

#if USE_DIRECT_COMPOSITION
		Microsoft::WRL::ComPtr<IDCompositionDevice> m_pDCompDevice;
		Microsoft::WRL::ComPtr<IDCompositionTarget> m_pDCompTarget;
		Microsoft::WRL::ComPtr<IDCompositionVisual> m_pDCompVisual;
#endif

	private:
		bool InitDirectX(HWND hWnd);

#if USE_DIRECT_COMPOSITION
		bool InitDirectXForComposition(HWND hWnd);
#else
		bool InitDirectXForHWND(HWND hWnd);
#endif

		bool InitShaders();
		bool InitScene();

		void GetDearImGuiInput(HWND hWnd);
	};
}

#endif // !_GRAPHICS_H_

