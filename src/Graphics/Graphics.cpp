////////////////////////////////////////////////////////////////////////////////
// Filename: Graphics.cpp
////////////////////////////////////////////////////////////////////////////////

#include "Graphics.h"
#include <filesystem>

static std::filesystem::path GetExecutableDir()
{
	wchar_t buffer[MAX_PATH];
	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	return std::filesystem::path(buffer).parent_path();
}

namespace DirectX11
{
	bool Graphics::Init(HWND hWnd, int width, int height, windows::Types::WindowFlags flags)
	{
		srand(time(nullptr));

		m_WindowWidth = width;
		m_WindowHeight = height;

		const bool transparent = windows::Helpers::IsFlagEnabled(flags, windows::Types::WindowFlags::Transparent);
		m_Alfa = transparent ? 0.f : 1.f;

		if (!InitDirectX(hWnd))
			return false;

		if (!InitShaders())
			return false;

		if (!InitScene())
			return false;

		// Setup Dear ImGui.
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplWin32_Init(hWnd);
		ImGui_ImplDX11_Init(m_pDevice.Get(), m_pDeviceContext.Get());
		ImGui::StyleColorsDark();

		return true;
	}

	void Graphics::RenderFrame(HWND hWnd, double deltaTime)
	{
		/*m_CB_PS_light.data.dynamicLightColor = m_Light.m_LightColor;
		m_CB_PS_light.data.dynamicLightStrength = m_Light.m_LightStrength;
		m_CB_PS_light.data.dynamicLightPosition = m_Light.GetPositionFloat3();
		m_CB_PS_light.data.dynamicLightAttenuationA = m_Light.m_LightAttenuationA;
		m_CB_PS_light.data.dynamicLightAttenuationB = m_Light.m_LightAttenuationB;
		m_CB_PS_light.data.dynamicLightAttenuationC = m_Light.m_LightAttenuationC;
		m_CB_PS_light.ApplyChanges();
		m_pDeviceContext->PSSetConstantBuffers(0, 1, m_CB_PS_light.GetAddressOf());*/

		///////////
		// CLEAR //
		///////////

		// Need to set render target because of this setting: DXGI_SWAP_EFFECT_FLIP_DISCARD, we need to explicitly ask windows for the back buffer
		//m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());
#if USE_DIRECT_COMPOSITION
		// Need to set render target because of this setting: DXGI_SWAP_EFFECT_FLIP_DISCARD, we need to explicitly ask windows for the back buffer
		m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());
#endif

		float background_color[4] = { 0.0f, 0.0f, 0.0f, m_Alfa };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), background_color);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0); // Clear the depth and stencil buffer.

		////////////////
		// DRAWING 3D //
		////////////////

		m_pDeviceContext->IASetInputLayout(m_VertexShader.GetInputLayout());
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pDeviceContext->RSSetState(m_pRasterizerState.Get());
		m_pDeviceContext->OMSetDepthStencilState(m_pDepthStencilState.Get(), 0); // Set the depth stencil state.
		m_pDeviceContext->OMSetBlendState(m_pBlendState.Get(), nullptr, 0xFFFFFFFF); // Set the blend state.
		//m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF); // Transparancy off.
		m_pDeviceContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf()); // Set the sampler state.
		m_pDeviceContext->VSSetShader(m_VertexShader.GetShader(), nullptr, 0);
		m_pDeviceContext->PSSetShader(m_PixelShader.GetShader(), nullptr, 0);

		////////////////////
		// OPAQUE OBJECTS //
		////////////////////

		// Crisis model.
		/* {
			m_GameObject.Draw(m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix());
		}

		// Light.
		{
			//m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_pDeviceContext->PSSetShader(m_PixelShaderWithNoLight.GetShader(), nullptr, 0);
			m_Light.Draw(m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix());
		}*/

		// Ocean
		/*{
			m_pDeviceContext->IASetInputLayout(m_VertexShaderOcean.GetInputLayout());
			m_pDeviceContext->PSSetShader(m_PixelShaderOcean.GetShader(), nullptr, 0);
			m_pDeviceContext->VSSetShader(m_VertexShaderOcean.GetShader(), nullptr, 0);
			m_Ocean.Draw(m_Camera, m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix(), m_ComputeShaderOcean, m_PixelShaderOcean, m_VertexShaderOcean, m_pSamplerState.GetAddressOf(), deltaTime);
		}*/

		// Axis.
		/* {
			m_pDeviceContext->IASetInputLayout(m_VertexShaderSprite.GetInputLayout());
			m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			m_pDeviceContext->PSSetShader(m_PixelShaderSprite.GetShader(), nullptr, 0);
			m_pDeviceContext->VSSetShader(m_VertexShaderSprite.GetShader(), nullptr, 0);
			m_Axis.Draw(m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix());
		}

		/////////////////////////
		// TRANSPARANT OBJECTS //
		/////////////////////////

		m_pDeviceContext->OMSetDepthStencilState(m_pDepthStencilStateTransparant.Get(), 0);
		m_pDeviceContext->OMSetBlendState(m_pBlendState.Get(), nullptr, 0xFFFFFFFF); // Set the blend state.
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Grid.
		{
			m_pDeviceContext->IASetInputLayout(m_VertexShaderGrid.GetInputLayout());
			m_pDeviceContext->PSSetShader(m_PixelShaderGrid.GetShader(), nullptr, 0);
			m_pDeviceContext->VSSetShader(m_VertexShaderGrid.GetShader(), nullptr, 0);
			//m_InfiniteGrid.Draw(m_Camera.GetViewMatrix(), m_Camera.GetProjectionMatrix(), 0.1f, 1000.0f);
		}

		// Ocean
		{
			m_pDeviceContext->IASetInputLayout(m_VertexShaderOcean.GetInputLayout());
			m_pDeviceContext->PSSetShader(m_PixelShaderOcean.GetShader(), nullptr, 0);
			m_pDeviceContext->VSSetShader(m_VertexShaderOcean.GetShader(), nullptr, 0);
			m_Ocean.Draw(m_Camera, m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix(), m_ComputeShaderOcean, m_PixelShaderOcean, m_VertexShaderOcean, m_pSamplerState.GetAddressOf(), deltaTime);
		}*/

		////////////////
		// DRAWING 2D //
		////////////////

		// Static Sprite Sheet
		{
			m_pDeviceContext->RSSetState(m_pRasterizerState.Get());
			m_pDeviceContext->OMSetDepthStencilState(m_pDepthStencilState.Get(), 0); // Set the depth stencil state.
			m_pDeviceContext->OMSetBlendState(m_pBlendState.Get(), nullptr, 0xFFFFFFFF);
			m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_pDeviceContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf()); // Set the sampler state.
			m_pDeviceContext->VSSetConstantBuffers(0, 1, m_CB_Frame.GetAddressOf());

			m_CB_Frame.data.viewProjectionMatrix = m_Camera2D.GetWorldMatrix() * m_Camera2D.GetOrthoMatrix();
			m_CB_Frame.ApplyChanges();

			m_WindowSystem->Update(m_Coordinator, deltaTime);
			m_MovementSystem->Update(m_Coordinator, deltaTime);
			auto collisionEvents = m_CollisionSystem->Update(m_Coordinator);
			std::vector<Entity> entitiesWithoutContact = m_CollisionSystem->GetEntitiesWithoutContact(m_Coordinator); // TODO: Should only update
			m_BehaviourSystem->Update(m_Coordinator, collisionEvents, entitiesWithoutContact);
			m_LifeCycleSystem->Update(m_Coordinator, deltaTime);
			m_AnimationSystem->Update(m_Coordinator, deltaTime);
			m_TransformSystem->Update(m_Coordinator);
			m_RenderSystem->Render(m_Coordinator, m_pDeviceContext.Get(), m_TextureManager.get(), m_CB_Object);
		}

		// Spritesheet.
		/*{
			// Reset for sprite drawing, should be called before Axis rendering.
			// But i keep this here for testing purposes.
			m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_Sprite.Draw(m_Camera2D.GetWorldMatrix() * m_Camera2D.GetOrthoMatrix());
		}*/

		// Draw text.
        static double elapsedTime = 0.0;
		static std::wstring fps_text = L"FPS: NULL";
        elapsedTime += deltaTime;  
		if (elapsedTime >= 1.0) // Update every second.
        {  
           double FPS = 1.0 / deltaTime;  
           fps_text = L"FPS: " + std::to_wstring(static_cast<int>(FPS));  
           elapsedTime = 0.0;  
        }

        m_pSpriteBatch->Begin();
        m_pSpriteFont->DrawString(m_pSpriteBatch.get(), fps_text.c_str(), DirectX::XMFLOAT2(10, 10), DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));
		m_pSpriteBatch->End();

		////////////////
		// Dear ImGui //
		////////////////
		
#if USE_DIRECT_COMPOSITION
		GetDearImGuiInput(hWnd);
#endif

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Light Controls");
		ImGui::SeparatorText("Ambient Lighting");
		ImGui::DragFloat3("Lighting color", &m_CB_PS_light.data.ambientLightColor.x, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Lighting strength", &m_CB_PS_light.data.ambientLightStrength, 0.01f, 0.0f, 1.0f);
		ImGui::SeparatorText("Dynamic Lighting");
		ImGui::DragFloat3("Light Color", &m_Light.m_LightColor.x, 0.01f, 0.0f, 10.0f);
		ImGui::DragFloat("Light strength", &m_Light.m_LightStrength, 0.01f, 0.0f, 10.0f);
		ImGui::DragFloat("Factor A", &m_Light.m_LightAttenuationA, 0.01f, 0.1f, 10.0f);
		ImGui::DragFloat("Factor B", &m_Light.m_LightAttenuationB, 0.01f, 0.0f, 10.0f);
		ImGui::DragFloat("Factor C", &m_Light.m_LightAttenuationC, 0.01f, 0.0f, 10.0f);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		/////////////
		// PRESENT //
		/////////////

		m_pSwapChain->Present(0, 0); // Vsync is OFF.
	}

	bool Graphics::OnShutdown(HWND hWnd, double deltaTime)
	{
		m_BehaviourSystem->ProcessShutdown(m_Coordinator);
		if (!std::ranges::distance(m_LifeCycleSystem->GetPendingDestructions(m_Coordinator)))
		{
			m_LifeCycleSystem->DestroyAll(m_Coordinator);
			return false;
		}

		RenderFrame(hWnd, deltaTime);
		return true;

		//ImGui_ImplDX11_Shutdown();
		//ImGui_ImplWin32_Shutdown();
		/*ImGui::DestroyContext();*/
	}

	bool Graphics::InitDirectX(HWND hWnd)
	{
#if USE_DIRECT_COMPOSITION
		InitDirectXForComposition(hWnd);
#else
		InitDirectXForHWND(hWnd);
#endif

		HRESULT hr;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
		hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(back_buffer.GetAddressOf()));
		COM_ERROR_IF_FAILED_RETURN(hr, "GetBuffer Failed.", false);

		hr = m_pDevice->CreateRenderTargetView(
			back_buffer.Get(),
			nullptr,
			m_pRenderTargetView.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create render target view.", false);

		// Enable depth.
		CD3D11_TEXTURE2D_DESC depth_stencil_buffer_description(DXGI_FORMAT_D24_UNORM_S8_UINT, m_WindowWidth, m_WindowHeight);
		depth_stencil_buffer_description.MipLevels = 1;
		depth_stencil_buffer_description.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		hr = m_pDevice->CreateTexture2D(
			&depth_stencil_buffer_description,
			nullptr,
			m_pDepthStencilBuffer.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create depth stencil buffer.", false);

		hr = m_pDevice->CreateDepthStencilView(
			m_pDepthStencilBuffer.Get(),
			nullptr,
			m_pDepthStencilView.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create depth stencil view.", false);

		///////////////////
		// OUTPUT MERGER //
		///////////////////

		m_pDeviceContext->OMSetRenderTargets(
			1, // Number of render targets.
			m_pRenderTargetView.GetAddressOf(),
			m_pDepthStencilView.Get()
		);

		CD3D11_DEPTH_STENCIL_DESC depth_stencil_state_description(D3D11_DEFAULT);
		depth_stencil_state_description.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		hr = m_pDevice->CreateDepthStencilState(
			&depth_stencil_state_description,
			m_pDepthStencilState.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create depth stencil state.", false);

		////////////////
		// RASTERIZER //
		////////////////

		CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_WindowWidth), static_cast<float>(m_WindowHeight));

		// For example we could use multiple viewports here for split-screen.
		m_pDeviceContext->RSSetViewports(1, &viewport);

		CD3D11_RASTERIZER_DESC rasterizer_description(D3D11_DEFAULT);
		//rasterizer_description.FillMode = D3D11_FILL_WIREFRAME;
		hr = m_pDevice->CreateRasterizerState(
			&rasterizer_description,
			m_pRasterizerState.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create rasterizer state.", false);

		D3D11_BLEND_DESC blend_description = { 0 };
		D3D11_RENDER_TARGET_BLEND_DESC render_target_blend_description = { 0 };

		// Boils down to: Final = (SrcColor * SrcBlend) OP (DestColor * DestBlend)
		// composition -> premultiplied
		// standard -> 
		render_target_blend_description.BlendEnable = TRUE;
		//render_target_blend_description.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		render_target_blend_description.SrcBlend = D3D11_BLEND_ONE;
		render_target_blend_description.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		render_target_blend_description.BlendOp = D3D11_BLEND_OP_ADD;

		render_target_blend_description.SrcBlendAlpha = D3D11_BLEND_ONE;
		//render_target_blend_description.DestBlendAlpha = D3D11_BLEND_ZERO;
		render_target_blend_description.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		render_target_blend_description.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		render_target_blend_description.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		blend_description.RenderTarget[0] = render_target_blend_description;

		hr = m_pDevice->CreateBlendState(
			&blend_description,
			m_pBlendState.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create blend state.", false);

		// Init fonts.
		OutputDebugStringW(std::filesystem::current_path().c_str());
		m_pSpriteBatch = std::make_unique<DirectX::SpriteBatch>(m_pDeviceContext.Get());
		/*if (IsDebuggerPresent())
			m_pSpriteFont  = std::make_unique<DirectX::SpriteFont>(m_pDevice.Get(), L"../../src/Data/Fonts/Comic_Sans_MS_16.spritefont");
		else*/
			m_pSpriteFont = std::make_unique<DirectX::SpriteFont>(m_pDevice.Get(), (GetExecutableDir() / L"Comic_Sans_MS_16.spritefont").c_str());

		// Setup sampler state.
		CD3D11_SAMPLER_DESC sampler_description(D3D11_DEFAULT);
		sampler_description.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // Wrap the texture.
		sampler_description.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_description.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		hr = m_pDevice->CreateSamplerState(
			&sampler_description,
			m_pSamplerState.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create sampler state.", false);

		return true;
	}

#if USE_DIRECT_COMPOSITION
	bool Graphics::InitDirectXForComposition(HWND hWnd)
	{
		UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		std::vector<D3D_FEATURE_LEVEL> featureLevels =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};

		HRESULT hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			createDeviceFlags,
			featureLevels.data(),
			(UINT)featureLevels.size(),
			D3D11_SDK_VERSION,
			m_pDevice.GetAddressOf(),
			nullptr,
			m_pDeviceContext.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create device.", false);

		Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
		m_pDevice.As(&dxgiDevice);

		Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
		dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());

		Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;
		dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(dxgiFactory.GetAddressOf()));

		// Setup SwapChain for composition.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
		swapChainDesc.Width = m_WindowWidth;
		swapChainDesc.Height = m_WindowHeight;
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;			  // Must be B8G8R8A8.
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;	  // Required for composition.
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		hr = dxgiFactory->CreateSwapChainForComposition(
			m_pDevice.Get(),
			&swapChainDesc,
			nullptr,
			m_pSwapChain.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create composition swapChain.", false);

		// DirectComposition setup.
		hr = DCompositionCreateDevice(
			dxgiDevice.Get(),
			__uuidof(IDCompositionDevice),
			reinterpret_cast<void**>(m_pDCompDevice.GetAddressOf())
		);
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create composition device.", false);

		hr = m_pDCompDevice->CreateTargetForHwnd(hWnd, TRUE, m_pDCompTarget.GetAddressOf());
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create target for windowhandle for composition device.", false);

		hr = m_pDCompDevice->CreateVisual(m_pDCompVisual.GetAddressOf());
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create visual for composition device.", false);

		hr = m_pDCompVisual->SetContent(m_pSwapChain.Get());
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to set content for composition visuak.", false);

		hr = m_pDCompTarget->SetRoot(m_pDCompVisual.Get());
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to set root for composition target.", false);

		hr = m_pDCompDevice->Commit();
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to commit composition device.", false);
		
		return true;
	}

#else

	bool Graphics::InitDirectXForHWND(HWND hWnd)
	{
		std::vector<AdapterData> adapters = AdapterReader::GetAdapters();
		if (adapters.size() < 1)
			return false;

		DXGI_SWAP_CHAIN_DESC swap_chain_description = { 0 };

		swap_chain_description.BufferDesc.Width = m_WindowWidth;				// Set the width of the back buffer.
		swap_chain_description.BufferDesc.Height = m_WindowHeight;				// Set the height of the back buffer.
		swap_chain_description.BufferDesc.RefreshRate.Numerator = 60;			// Set the refresh rate of the back buffer, currently hardcoded to 60 FPS.
		swap_chain_description.BufferDesc.RefreshRate.Denominator = 1;
		swap_chain_description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// Set the format of the back buffer to 32 bit color.
		swap_chain_description.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swap_chain_description.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		swap_chain_description.SampleDesc.Count = 1;	// Set the number of multisamples to use.
		swap_chain_description.SampleDesc.Quality = 0;	// Set the quality of the multisamples to use.

		swap_chain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// Set the swap chain to be used as a render target.
		swap_chain_description.BufferCount = 1;									// Set the number of back buffers to use.
		//swap_chain_description.BufferCount = 2;									// Set the number of back buffers to use.
		swap_chain_description.OutputWindow = hWnd;								// Set the window to render to.
		swap_chain_description.Windowed = TRUE;									// Set the swap chain to be windowed or not.
		swap_chain_description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;			// Set the swap effect to discard the old frame.
		//swap_chain_description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;			// Set the swap effect to discard the old frame.
		swap_chain_description.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// Set the swap chain to allow mode switching -> allows to switch between fullscreen, windowed mode and resize the window.

		HRESULT hr;
		hr = D3D11CreateDeviceAndSwapChain(
			adapters[0].m_pAdapter,		// Currently we choose the adapter with the highest memory.
			D3D_DRIVER_TYPE_UNKNOWN,	// We are specifying the adapter, so we use unknown.
			nullptr,					// Not using a software driver.
			NULL,						// Flags for runtime layers.
			nullptr,					// Feature levels array.
			NULL,						// Number of feature levels.
			D3D11_SDK_VERSION,			// SDK version.
			&swap_chain_description,
			m_pSwapChain.GetAddressOf(),
			m_pDevice.GetAddressOf(),
			nullptr, 					// Supported feature level.
			m_pDeviceContext.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create device and swapchain.", false);

		return true;
	}
#endif

	bool Graphics::InitShaders()
	{
		/////////////////////
		// INPUT ASSEMBLER //
		/////////////////////

		// Macro to determine the shader folder path.
		std::filesystem::path shader_folder = GetExecutableDir(); // See project propteries debugging !!!
		//OutputDebugStringW(shader_folder.c_str());
#pragma region DetermineShaderPath
		/*if (IsDebuggerPresent())
		{
#ifdef _DEBUG // Debug mode
	#ifdef _WIN64 // x64
			shader_folder = L"x64\\Debug\\";
	#else // x86 (Win32)
			shader_folder = L"Debug\\";
	#endif
#else // Release mode
	#ifdef _WIN64 // x64
			shader_folder = L"x64\\Release\\";
	#else // x86 (Win32)
			shader_folder = L"Release\\";
	#endif
#endif
		}*/

		////////
		// 2D //
		////////

		D3D11_INPUT_ELEMENT_DESC layout2D[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		D3D11_INPUT_ELEMENT_DESC layoutGrid[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		D3D11_INPUT_ELEMENT_DESC layoutOcean[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		///////////////////
		// VERTEX SHADER //
		///////////////////

		//if (!m_VertexShaderSprite.Init(m_pDevice, shader_folder + L"vertexshader_2D.cso", layout2D, ARRAYSIZE(layout2D)))
			//return false;

		if (!m_VertexShaderGrid.Init(m_pDevice, shader_folder / L"vertexshader_grid.cso", layoutGrid, ARRAYSIZE(layoutGrid)))
			return false;

		if (!m_VertexShaderOcean.Init(m_pDevice, shader_folder / L"VS_Ocean.cso", layoutOcean, ARRAYSIZE(layoutOcean)))
			return false;

		//////////////////
		// PIXEL SHADER //
		//////////////////

		//if (!m_PixelShaderSprite.Init(m_pDevice, shader_folder + L"pixelshader_2D.cso"))
			//return false;

		if (!m_PixelShaderGrid.Init(m_pDevice, shader_folder / L"pixelshader_grid.cso"))
			return false;

		if (!m_PixelShaderOcean.Init(m_pDevice, shader_folder / L"PS_Ocean.cso"))
			return false;

		if (!m_ComputeShaderOcean.Init(m_pDevice, shader_folder / L"CS_PerlinNoise.cso"))
			return false;

		////////////////////
		// 3D - triangles //
		////////////////////

		D3D11_INPUT_ELEMENT_DESC layout3D[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		///////////////////
		// VERTEX SHADER //
		///////////////////

		if (!m_VertexShader.Init(m_pDevice, shader_folder / L"vertexshader.cso", layout3D, ARRAYSIZE(layout3D)))
			return false;

		//////////////////
		// PIXEL SHADER //
		//////////////////

		if (!m_PixelShader.Init(m_pDevice, shader_folder / L"pixelshader.cso"))
			return false;

		if (!m_PixelShaderWithNoLight.Init(m_pDevice, shader_folder / L"pixelshader_nolighteffect.cso"))
			return false;

		return true;
	}

	bool Graphics::InitScene()
	{
		HRESULT hr;

		// Load texture(s).
		/*hr = DirectX::CreateWICTextureFromFile(
			m_pDevice.Get(),
			L"src/Data/Textures/Grass_Texture.png",
			nullptr,
			m_pGrassTexture.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to create wic texture from file.", false);

		hr = DirectX::CreateWICTextureFromFile(
			m_pDevice.Get(),
			L"src/Data/Textures/Pink.png",
			nullptr,
			m_pPinkTexture.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to create wic texture from file.", false);

		hr = DirectX::CreateWICTextureFromFile(
			m_pDevice.Get(),
			L"src/Data/Textures/TomNook.png",
			nullptr,
			m_pTomNookTexture.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to create wic texture from file.", false);

		// Load constant buffer(s).
		hr = m_CB_VS_vertexshader_2d.Init(m_pDevice.Get(), m_pDeviceContext.Get());
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to initialize 2d constant vertex buffer.", false);

		hr = m_CB_VS_vertexshader.Init(m_pDevice.Get(), m_pDeviceContext.Get());
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to initialize constant vertex buffer.", false);

		hr = m_CB_VS_vertexshader_grid.Init(m_pDevice.Get(), m_pDeviceContext.Get());
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to initialize constant vertex buffer.", false);

		hr = m_CB_VS_Ocean.Init(m_pDevice.Get(), m_pDeviceContext.Get());
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to initialize constant vertex buffer.", false);

		hr = m_CB_CS_Perlin.Init(m_pDevice.Get(), m_pDeviceContext.Get());
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to initialize constant vertex buffer.", false);

		hr = m_CB_PS_pixelshader.Init(m_pDevice.Get(), m_pDeviceContext.Get());
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to initialize constant pixel buffer.", false);*/

		hr = m_CB_PS_light.Init(m_pDevice.Get(), m_pDeviceContext.Get());
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to initialize constant pixel buffer.", false);

		m_CB_PS_light.data.ambientLightColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f); // White, will show full red, blue and green value of a pixel.
		m_CB_PS_light.data.ambientLightStrength = 1.0f; // Fully lit.

		// Load object(s).
		//if (!m_GameObject.Init("src/Data/Objects/Samples/dodge_challenger.fbx", m_pDevice.Get(), m_pDeviceContext.Get(), m_CB_VS_vertexshader))
		/*if (!m_GameObject.Init("src/Data/Objects/nanosuit/nanosuit.obj", m_pDevice.Get(), m_pDeviceContext.Get(), m_CB_VS_vertexshader))
			return false;*/

		//if (!m_Light.Init(m_pDevice.Get(), m_pDeviceContext.Get(), m_CB_VS_vertexshader))
			//return false;

		/*if (!m_Axis.Init(m_pDevice.Get(), m_pDeviceContext.Get(), m_CB_VS_vertexshader_2d))
			return false;

		if (!m_InfiniteGrid.Init(m_pDevice.Get(), m_pDeviceContext.Get(), m_CB_VS_vertexshader_grid))
			return false;*/

		// Ocean

		/*if (!m_Ocean.Init(m_pDevice.Get(), m_pDeviceContext.Get(), m_CB_CS_Perlin, m_CB_VS_Ocean, 256, 256))
			return false;*/

		// Ocean

		/*m_Axis.SetPosition(0.0f, 0.0f, 0.0f);
		m_Axis.SetScale(5.0f, 5.0f, 5.0f);*/

		//m_Light.SetPosition(10.0f, 5.0f, 0.0f);
		//m_Light.SetLookAtPosition(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));

		//m_InfiniteGrid.SetPosition(0.0f, 0.0f, 0.0f);
		//m_InfiniteGrid.SetScale(1.0f, 1.0f, 1.0f);

		//m_Ocean.SetScale(5.0f, 5.0f, 5.0f);
		
		// Load sprite(s);
		/*if (!m_Sprite.Init(m_pDevice.Get(), m_pDeviceContext.Get(), 240, 304, "src/Data/Textures/Full Gimp SpriteSheet.png", m_CB_VS_vertexshader_2d)) // 240, 304
			return false;*/
		
		m_Camera2D.SetProjectionValues(static_cast<float>(m_WindowWidth), static_cast<float>(m_WindowHeight), 0.0f, 1.0f);

		//m_Camera.SetPosition(2.0f, 2.0f, 2.0f);
		m_Camera.SetPosition(10.0f, 50.0f, 10.0f);
		m_Camera.SetProjectionValues(90.0f, static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight), 0.1f, 1000.0f);
		m_Camera.SetLookAtPosition(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));




		//////
		///////
		///////

		m_AssetManager			= std::make_shared<Kaizen::Resources::PrimitiveFactory>(m_pDevice);
		m_TextureManager		= std::make_shared<Kaizen::Resources::TextureManager>(m_pDevice);
		m_VertexShaderManager	= std::make_shared<Kaizen::Resources::VertexShaderManager>(m_pDevice);
		m_PixelShaderManager	= std::make_shared<Kaizen::Resources::PixelShaderManager>(m_pDevice);

		/*if (IsDebuggerPresent())
			m_SpritesheetTextureID  = m_TextureManager->Load("../../src/Data/Textures/Full Gimp SpriteSheetV3.png");
		else*/
			m_SpritesheetTextureID = m_TextureManager->Load((GetExecutableDir() / "Full Gimp SpriteSheetV3.png").string());

		m_VertexShaderSprite	= m_VertexShaderManager->Load<VertexPosUV>(m_pDevice, "VS_StaticSpriteShader.cso");
		m_PixelShaderSprite		= m_PixelShaderManager->Load(m_pDevice, "PS_StaticSpriteShader.cso");

		m_Coordinator.Init();
		m_Coordinator.RegisterComponent<Kaizen::Components::TransformComponent>();
		m_Coordinator.RegisterComponent<Kaizen::Graphics::SpriteComponent>();
		m_Coordinator.RegisterComponent<Kaizen::Graphics::MeshRendererComponent>();
		m_Coordinator.RegisterComponent<AnimationComponent>();
		m_Coordinator.RegisterComponent<WherePenguinsDwell::Components::ColliderBox>();
		m_Coordinator.RegisterComponent<WherePenguinsDwell::Tags::WindowColliderTag>();
		m_Coordinator.RegisterComponent<WherePenguinsDwell::Components::MovementComponent>();
		m_Coordinator.RegisterComponent<WherePenguinsDwell::Components::VelocityComponent>();
		m_Coordinator.RegisterComponent<WherePenguinsDwell::Components::TimerComponent>();

		m_RenderSystem = m_Coordinator.RegisterSystem<Kaizen::Graphics::RenderSystem>();
		{
			Signature signature;
			signature.set(m_Coordinator.GetComponentType<Kaizen::Components::TransformComponent>());
			signature.set(m_Coordinator.GetComponentType<Kaizen::Graphics::MeshRendererComponent>());
			m_Coordinator.SetSystemSignature<Kaizen::Graphics::RenderSystem>(signature);
		}

		m_TransformSystem = m_Coordinator.RegisterSystem<Kaizen::Graphics::TransformSystem>();
		{
			Signature signature;
			signature.set(m_Coordinator.GetComponentType<Kaizen::Components::TransformComponent>());
			m_Coordinator.SetSystemSignature<Kaizen::Graphics::TransformSystem>(signature);
		}

		m_AnimationSystem = m_Coordinator.RegisterSystem<Kaizen::Graphics::AnimationSystem>();
		{
			Signature signature;
			signature.set(m_Coordinator.GetComponentType<Kaizen::Graphics::SpriteComponent>());
			signature.set(m_Coordinator.GetComponentType<AnimationComponent>());
			m_Coordinator.SetSystemSignature<Kaizen::Graphics::AnimationSystem>(signature);
		}

		//std::default_random_engine generator;
		//std::uniform_real_distribution<float> randPosition(100.0f, 500.0f);
		RECT rect{};
		HWND taskbar_handle = FindWindowA("Shell_TrayWnd", NULL);
		GetWindowRect(taskbar_handle, &rect);
		assert(!(rect.left == 0 && rect.right == 0 && rect.top == 0 && rect.bottom == 0) && "Couldn't find taskbar dimensions.");

		std::mt19937 generator(std::random_device{}());
		std::uniform_real_distribution<float> dist_x(rect.left + 15, rect.right - 15);
		//std::uniform_real_distribution<float> dist_y(rect.top, rect.bottom);

		m_UniformGrid = std::make_shared<Kaizen::Container::UniformGrid<WherePenguinsDwell::Components::ColliderBox>>(m_WindowWidth, m_WindowHeight, 39);
		m_CollisionSystem = m_Coordinator.RegisterSystem<WherePenguinsDwell::CollisionSystem>();
		{
			Signature signature;
			signature.set(m_Coordinator.GetComponentType<Kaizen::Components::TransformComponent>());
			signature.set(m_Coordinator.GetComponentType<WherePenguinsDwell::Components::ColliderBox>());
			m_Coordinator.SetSystemSignature<WherePenguinsDwell::CollisionSystem>(signature);
		}
		m_CollisionSystem->Init(m_UniformGrid.get(), 10.f);

		m_WindowSystem = m_Coordinator.RegisterSystem<WherePenguinsDwell::Systems::WindowSystem>();
		{
			Signature signature;
			signature.set(m_Coordinator.GetComponentType<WherePenguinsDwell::Tags::WindowColliderTag>());
			m_Coordinator.SetSystemSignature<WherePenguinsDwell::Systems::WindowSystem>(signature);
		}
		m_WindowSystem->Init(WherePenguinsDwell::Settings::Systems::WINDOW_UPDATE_FREQUENCY, WherePenguinsDwell::Settings::Systems::EDGE_WINDOWS_THICKNESS);

		m_MovementSystem = m_Coordinator.RegisterSystem<WherePenguinsDwell::MovementSystem>();
		{
			Signature signature;
			signature.set(m_Coordinator.GetComponentType<Kaizen::Components::TransformComponent>());
			signature.set(m_Coordinator.GetComponentType<AnimationComponent>());
			signature.set(m_Coordinator.GetComponentType<WherePenguinsDwell::Components::MovementComponent>());
			signature.set(m_Coordinator.GetComponentType<WherePenguinsDwell::Components::VelocityComponent>());
			m_Coordinator.SetSystemSignature<WherePenguinsDwell::MovementSystem>(signature);
		}

		m_BehaviourSystem = m_Coordinator.RegisterSystem<WherePenguinsDwell::BehaviourSystem>();
		{
			Signature signature;
			signature.set(m_Coordinator.GetComponentType<AnimationComponent>());
			m_Coordinator.SetSystemSignature<WherePenguinsDwell::MovementSystem>(signature);
		}
		m_BehaviourSystem->Init(200);

		m_LifeCycleSystem = m_Coordinator.RegisterSystem<WherePenguinsDwell::LifeCycleSystem>();
		{
			Signature signature;
			signature.set(m_Coordinator.GetComponentType<AnimationComponent>());
			signature.set(m_Coordinator.GetComponentType<WherePenguinsDwell::Components::TimerComponent>());
			m_Coordinator.SetSystemSignature<WherePenguinsDwell::MovementSystem>(signature);
		}

		std::vector<Entity> entities(100);
		for (auto& entity : entities)
		{
			entity = m_Coordinator.CreateEntity();

			Kaizen::Graphics::SpriteComponent spriteData;
			m_Coordinator.AddComponent(entity, spriteData);

			Kaizen::Components::TransformComponent transform_comp;
			transform_comp.SetPosition(dist_x(generator), static_cast<float>(rect.top + 15), 0.0f);
			m_Coordinator.AddComponent(
				entity,
				transform_comp
			);

			/*Kaizen::Graphics::SpriteComponent spriteData;
			// Animation takes care of this!
			/*spriteData.w = 240.0f;
			spriteData.h = 270.0f;
			spriteData.cols = 8;
			spriteData.rows = 9;
			spriteData.uw = 1.0f / 8.0f;
			spriteData.vh = 1.0f / 9.0f;

			m_Coordinator.AddComponent(entity, spriteData);*/

			AnimationComponent anim;
			anim.SetupFromConfig(448.f, 584.f, {
				{ {30.f, 30.f}, 1, 1.f, 0.f, 0.f}, // Idle: 30x30, 1 column, 1 fps
				{ {30.f, 30.f}, 8, 8.f, 0.f, 0.f}, // walkingLeft: 30x30, 8 kolommen, 8 fps
				{ {30.f, 30.f}, 8, 8.f, 0.f, 0.f}, // walkingRight: 30x30, 8 kolommen, 8 fps
				{ {30.f, 30.f}, 8, 8.f, 0.f, 0.f}, // falling: 30x30, 8 kolommen, 8 fps
				{ {30.f, 30.f}, 8, 8.f, 0.f, 0.f}, // tumbling: 30x30, 1 column, 1 fps
				{ {30.f, 30.f}, 8, 8.f, 0.f, 0.f}, // floating: 30x30, 8 kolommen, 8 fps
				{ {30.f, 30.f}, 8, 8.f, 0.f, 0.f}, // walkingOnTheRight: 30x30, 8 kolommen, 8 fps
				{ {30.f, 30.f}, 8, 8.f, 0.f, 0.f}, // walkingOnTheLeft: 30x30, 8 kolommen, 8 fps

				{ {30.f, 30.f}, 12, 12.f, 0.f, 0.f}, // reading: 30x30, 12 column, 12 fps
				{ {32.f, 32.f}, 14, 14.f, 1.f, 1.f}, // digging: 32x32, 14 kolommen, 14 fps

				{ {30.f, 30.f}, 8, 8.f, 0.f, 0.f}, // supermanLeftHand: 30x30, 8 kolommen, 8 fps
				{ {30.f, 30.f}, 8, 8.f, 0.f, 0.f}, // supermanRightHand: 30x30, 8 kolommen, 8 fps

				{ {32.f, 33.f}, 10, 10.f, 1.f, 3.f / 2.0f}, // zap_walkingLeft: 32x33, 10 column, 10 fps
				{ {32.f, 33.f}, 10, 10.f, 1.f, 3.f / 2.0f}, // zap_walkingRight: 32x33, 10 kolommen, 10 fps

				{ {32.f, 32.f}, 9, 9.f, 1.f, 1.f}, // exit_walkingLeft: 32x32, 9 kolommen, 9 fps
				{ {32.f, 32.f}, 9, 9.f, 1.f, 1.f}, // exit_walkingRight: 32x32, 9 kolommen, 9 fps

				{ {32.f, 32.f}, 16, 16.f, 1.f, 1.f}, // bomber: 32x32, 16 kolommen, 16 fps
				{ {32.f, 32.f}, 12, 12.f, 1.f, 1.f}, // splash: 32x32, 12 kolommen, 12 fps
				{ {30.f, 30.f}, 1, 1.f, 0.f, 0.f}, // exploding: 30x30, 1 kolommen, 1 fps
			});
			m_Coordinator.AddComponent(entity, anim);

			WherePenguinsDwell::Components::ColliderBox collider;
			collider.offsetX = -15.f;
			collider.offsetY = -15.f;
			collider.width = 30.f; // Stem dit af op je sprite grootte / gewenste hitbox
			collider.height = 30.f;
			collider.owner = entity;
			m_Coordinator.AddComponent(entity, collider);

			WherePenguinsDwell::Components::MovementComponent mov;
			m_Coordinator.AddComponent(entity, mov);

			WherePenguinsDwell::Components::VelocityComponent vel;
			m_Coordinator.AddComponent(entity, vel);

			WherePenguinsDwell::Components::TimerComponent tim;
			m_Coordinator.AddComponent(entity, tim);

			auto sharedSpriteMaterial = std::make_shared<Kaizen::Graphics::SpriteMaterial>(m_pDevice.Get(), m_pDeviceContext.Get(), m_VertexShaderManager->GetTexture(m_VertexShaderSprite), m_PixelShaderManager->GetTexture(m_PixelShaderSprite), m_SpritesheetTextureID);
			m_Coordinator.AddComponent(
				entity,
				Kaizen::Graphics::MeshRendererComponent(
					m_AssetManager->GetPrimitive(PrimitiveType::Quad),
					sharedSpriteMaterial
				)
			);
		}

		hr = m_CB_Frame.Init(m_pDevice.Get(), m_pDeviceContext.Get());
		COM_ERROR_IF_FAILED_SHOW(hr, L"Failed to initialize frame constant buffer.");

		hr = m_CB_Object.Init(m_pDevice.Get(), m_pDeviceContext.Get());
		COM_ERROR_IF_FAILED_SHOW(hr, L"Failed to initialize object constant buffer");

		return true;
	}

	void Graphics::GetDearImGuiInput(HWND hWnd)
	{
		POINT p;
		if (!GetCursorPos(&p))
			return;

		// Convert input relative to window.
		ScreenToClient(hWnd, &p);

		// Even though the window is transparent, Dear ImGui can still receive mouse inputs, when the window is focused.
		// Once the (main) window is out of focus, if at all, Dear ImGui will stop receiving mouse inputs.
		// Can't solve this in the message loop since it requires focus...
		// So we bypass the message loop.
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(static_cast<float>(p.x), static_cast<float>(p.y));

		LONG_PTR exStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
		if (io.WantCaptureMouse)
		{
			if (!(exStyle & WS_EX_TRANSPARENT))
				return;

			exStyle &= ~WS_EX_TRANSPARENT;
			SetWindowLongPtr(hWnd, GWL_EXSTYLE, exStyle);
		}
		else
		{
			if (exStyle & WS_EX_TRANSPARENT)
				return;

			exStyle |= WS_EX_TRANSPARENT;
			SetWindowLongPtr(hWnd, GWL_EXSTYLE, exStyle);
		}
	}
}