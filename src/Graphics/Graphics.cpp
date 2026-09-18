////////////////////////////////////////////////////////////////////////////////
// Filename: Graphics.cpp
////////////////////////////////////////////////////////////////////////////////

#include "Graphics.h"

namespace DirectX11
{
	bool Graphics::Init(HWND hWnd, int width, int height)
	{
		m_DualView = false;
		m_WindowWidth = width;
		m_WindowHeight = height;

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

	void Graphics::RenderFrame(double deltaTime)
	{
		m_CB_PS_light.data.dynamicLightColor = m_Light.m_LightColor;
		m_CB_PS_light.data.dynamicLightStrength = m_Light.m_LightStrength;
		m_CB_PS_light.data.dynamicLightPosition = m_Light.GetPositionFloat3();
		m_CB_PS_light.data.dynamicLightAttenuationA = m_Light.m_LightAttenuationA;
		m_CB_PS_light.data.dynamicLightAttenuationB = m_Light.m_LightAttenuationB;
		m_CB_PS_light.data.dynamicLightAttenuationC = m_Light.m_LightAttenuationC;
		m_CB_PS_light.ApplyChanges();
		m_pDeviceContext->PSSetConstantBuffers(0, 1, m_CB_PS_light.GetAddressOf());

		///////////
		// CLEAR //
		///////////

		float background_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), background_color);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0); // Clear the depth and stencil buffer.

		////////////////
		// DRAWING 3D //
		////////////////

		m_pDeviceContext->IASetInputLayout(m_VertexShader.GetInputLayout());
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pDeviceContext->RSSetState(m_pRasterizerState.Get());
		m_pDeviceContext->OMSetDepthStencilState(m_pDepthStencilState.Get(), 0);
		m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);				 // Transparancy off.
		m_pDeviceContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());
		m_pDeviceContext->VSSetShader(m_VertexShader.GetShader(), nullptr, 0);
		m_pDeviceContext->PSSetShader(m_PixelShader.GetShader(), nullptr, 0);

		if (m_DualView) 
		{
			m_pDeviceContext->RSSetViewports(1, &m_ViewportDualView[1]);
			RenderScene(m_Camera2, deltaTime);
			m_pDeviceContext->RSSetViewports(1, &m_ViewportDualView[0]);
			RenderScene(m_Camera, deltaTime);
		}
		else
		{
			m_pDeviceContext->RSSetViewports(1, &m_ViewportSingleView);
			RenderScene(m_Camera, deltaTime);
		}

		////////////////
		// DRAWING 2D //
		////////////////

		// Spritesheet.
		/*{
			m_pDeviceContext->IASetInputLayout(m_VertexShaderSprite.GetInputLayout());
			m_pDeviceContext->PSSetShader(m_PixelShaderSprite.GetShader(), nullptr, 0);
			m_pDeviceContext->VSSetShader(m_VertexShaderSprite.GetShader(), nullptr, 0);
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
		
		// Start the ImGui frame.
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

	void Graphics::RenderScene(const Camera3D& camera, double deltaTime)
	{
		////////////////////
		// OPAQUE OBJECTS //
		////////////////////

		// Crisis model.
		/* {
			m_GameObject.Draw(camera.GetViewMatrix() * camera.GetProjectionMatrix());
		}*/

		// Light.
		/* {
			m_pDeviceContext->PSSetShader(m_PixelShaderWithNoLight.GetShader(), nullptr, 0);
			m_Light.Draw(camera.GetViewMatrix() * camera.GetProjectionMatrix());
		}*/

		// Ocean.
		/* {
			m_pDeviceContext->IASetInputLayout(m_VertexShaderOcean.GetInputLayout());
			m_pDeviceContext->PSSetShader(m_PixelShaderOcean.GetShader(), nullptr, 0);
			m_pDeviceContext->VSSetShader(m_VertexShaderOcean.GetShader(), nullptr, 0);
			m_Ocean.Draw(m_Camera, camera.GetViewMatrix() * camera.GetProjectionMatrix(), m_ComputeShaderOcean, m_PixelShaderOcean, m_VertexShaderOcean, m_pSamplerState.GetAddressOf(), deltaTime);
		}*/

		// Axis.
		/* {
			m_pDeviceContext->IASetInputLayout(m_VertexShaderSprite.GetInputLayout());
			m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			m_pDeviceContext->PSSetShader(m_PixelShaderSprite.GetShader(), nullptr, 0);
			m_pDeviceContext->VSSetShader(m_VertexShaderSprite.GetShader(), nullptr, 0);
			m_Axis.Draw(camera.GetViewMatrix() * camera.GetProjectionMatrix());
		}*/

		/////////////////////////
		// TRANSPARANT OBJECTS //
		/////////////////////////

		m_pDeviceContext->OMSetDepthStencilState(m_pDepthStencilStateTransparant.Get(), 0);
		m_pDeviceContext->OMSetBlendState(m_pBlendState.Get(), nullptr, 0xFFFFFFFF);
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Grid.
		{
			m_pDeviceContext->IASetInputLayout(m_VertexShaderGrid.GetInputLayout());
			m_pDeviceContext->PSSetShader(m_PixelShaderGrid.GetShader(), nullptr, 0);
			m_pDeviceContext->VSSetShader(m_VertexShaderGrid.GetShader(), nullptr, 0);
			m_InfiniteGrid.Draw(camera.GetViewMatrix(), camera.GetProjectionMatrix(), 0.1f, 10000.0f);
		}
	}

	bool Graphics::InitDirectX(HWND hWnd)
	{
		std::vector<AdapterData> adapters = AdapterReader::GetAdapters();
		if (adapters.size() < 1)
			return false;

		DXGI_SWAP_CHAIN_DESC swap_chain_description = { 0 };

		swap_chain_description.BufferDesc.Width = m_WindowWidth;
		swap_chain_description.BufferDesc.Height = m_WindowHeight;
		swap_chain_description.BufferDesc.RefreshRate.Numerator = 60;			// Set the refresh rate of the back buffer, currently hardcoded to 60 FPS.
		swap_chain_description.BufferDesc.RefreshRate.Denominator = 1;
		swap_chain_description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swap_chain_description.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swap_chain_description.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		swap_chain_description.SampleDesc.Count = 1;
		swap_chain_description.SampleDesc.Quality = 0;

		swap_chain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_description.BufferCount = 1;
		swap_chain_description.OutputWindow = hWnd;
		swap_chain_description.Windowed = TRUE;
		swap_chain_description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swap_chain_description.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		HRESULT hr;
		hr = D3D11CreateDeviceAndSwapChain(
			adapters[0].m_pAdapter,		// Currently we choose the adapter with the highest memory.
			D3D_DRIVER_TYPE_UNKNOWN,
			nullptr,
			NULL,
			nullptr,
			NULL,
			D3D11_SDK_VERSION,
			&swap_chain_description,
			m_pSwapChain.GetAddressOf(),
			m_pDevice.GetAddressOf(),
			nullptr,
			m_pDeviceContext.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create device and swapchain.", false);

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
			1,									// Number of render targets.
			m_pRenderTargetView.GetAddressOf(),
			m_pDepthStencilView.Get()
		);

		// Create depth stencil state.
		CD3D11_DEPTH_STENCIL_DESC depth_stencil_state_description(D3D11_DEFAULT);
		depth_stencil_state_description.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		hr = m_pDevice->CreateDepthStencilState(
			&depth_stencil_state_description,
			m_pDepthStencilState.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create depth stencil state.", false);

		CD3D11_DEPTH_STENCIL_DESC depth_stencil_state_description_transparant(D3D11_DEFAULT);
		depth_stencil_state_description_transparant.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		depth_stencil_state_description_transparant.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

		hr = m_pDevice->CreateDepthStencilState(
			&depth_stencil_state_description_transparant,
			m_pDepthStencilStateTransparant.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create depth stencil state.", false);

		////////////////
		// RASTERIZER //
		////////////////

		m_ViewportSingleView = CD3D11_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_WindowWidth), static_cast<float>(m_WindowHeight));

		m_ViewportDualView[0] = CD3D11_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_WindowWidth) / 2.f, static_cast<float>(m_WindowHeight));
		m_ViewportDualView[1] = CD3D11_VIEWPORT(static_cast<float>(m_WindowWidth) / 2.f, 0.0f, static_cast<float>(m_WindowWidth) / 2.f, static_cast<float>(m_WindowHeight));

		// Set viewport.
		// For example we could use multiple viewports here for split-screen.
		m_pDeviceContext->RSSetViewports(1, &m_ViewportSingleView);

		// Create rasterizer state.
		CD3D11_RASTERIZER_DESC rasterizer_description(D3D11_DEFAULT);
		//rasterizer_description.FillMode = D3D11_FILL_WIREFRAME;
		hr = m_pDevice->CreateRasterizerState(
			&rasterizer_description,
			m_pRasterizerState.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create rasterizer state.", false);

		// Create blend state.
		D3D11_BLEND_DESC blend_description = { 0 };
		D3D11_RENDER_TARGET_BLEND_DESC render_target_blend_description = { 0 };

		render_target_blend_description.BlendEnable = TRUE;
		render_target_blend_description.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		render_target_blend_description.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		render_target_blend_description.BlendOp = D3D11_BLEND_OP_ADD;
		render_target_blend_description.SrcBlendAlpha = D3D11_BLEND_ONE;
		render_target_blend_description.DestBlendAlpha = D3D11_BLEND_ZERO;
		render_target_blend_description.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		render_target_blend_description.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		blend_description.RenderTarget[0] = render_target_blend_description;

		hr = m_pDevice->CreateBlendState(
			&blend_description,
			m_pBlendState.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create blend state.", false);

		// Init fonts.
		m_pSpriteBatch = std::make_unique<DirectX::SpriteBatch>(m_pDeviceContext.Get());
		m_pSpriteFont  = std::make_unique<DirectX::SpriteFont>(m_pDevice.Get(), L"src/Data/Fonts/Comic_Sans_MS_16.spritefont");

		// Setup sampler state.
		CD3D11_SAMPLER_DESC sampler_description(D3D11_DEFAULT);
		sampler_description.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_description.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_description.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		hr = m_pDevice->CreateSamplerState(
			&sampler_description,
			m_pSamplerState.GetAddressOf()
		);
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to create sampler state.", false);

		return true;
	}

	bool Graphics::InitShaders()
	{
		/////////////////////
		// INPUT ASSEMBLER //
		/////////////////////

		// Macro to determine the shader folder path.
		std::wstring shader_folder;
#pragma region DetermineShaderPath
		if (IsDebuggerPresent())
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
		}

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

		if (!m_VertexShaderSprite.Init(m_pDevice, shader_folder + L"vertexshader_2D.cso", layout2D, ARRAYSIZE(layout2D)))
			return false;

		if (!m_VertexShaderGrid.Init(m_pDevice, shader_folder + L"vertexshader_grid.cso", layoutGrid, ARRAYSIZE(layoutGrid)))
			return false;

		if (!m_VertexShaderOcean.Init(m_pDevice, shader_folder + L"VS_Ocean.cso", layoutOcean, ARRAYSIZE(layoutOcean)))
			return false;

		//////////////////
		// PIXEL SHADER //
		//////////////////

		if (!m_PixelShaderSprite.Init(m_pDevice, shader_folder + L"pixelshader_2D.cso"))
			return false;

		if (!m_PixelShaderGrid.Init(m_pDevice, shader_folder + L"pixelshader_grid.cso"))
			return false;

		if (!m_PixelShaderOcean.Init(m_pDevice, shader_folder + L"PS_Ocean.cso"))
			return false;

		if (!m_ComputeShaderOcean.Init(m_pDevice, shader_folder + L"CS_PerlinNoise.cso"))
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

		if (!m_VertexShader.Init(m_pDevice, shader_folder + L"vertexshader.cso", layout3D, ARRAYSIZE(layout3D)))
			return false;

		//////////////////
		// PIXEL SHADER //
		//////////////////

		if (!m_PixelShader.Init(m_pDevice, shader_folder + L"pixelshader.cso"))
			return false;

		if (!m_PixelShaderWithNoLight.Init(m_pDevice, shader_folder + L"pixelshader_nolighteffect.cso"))
			return false;

		return true;
	}

	bool Graphics::InitScene()
	{
		// Load texture(s).
		HRESULT hr = DirectX::CreateWICTextureFromFile(
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

		/*hr = m_CB_PS_pixelshader.Init(m_pDevice.Get(), m_pDeviceContext.Get());
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to initialize constant pixel buffer.", false);*/

		hr = m_CB_PS_light.Init(m_pDevice.Get(), m_pDeviceContext.Get());
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to initialize constant pixel buffer.", false);

		m_CB_PS_light.data.ambientLightColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		m_CB_PS_light.data.ambientLightStrength = 1.0f;

		// Load object(s).
		if (!m_GameObject.Init("src/Data/Objects/Nanosuit/Nanosuit.obj", m_pDevice.Get(), m_pDeviceContext.Get(), m_CB_VS_vertexshader))
			return false;

		if (!m_Light.Init(m_pDevice.Get(), m_pDeviceContext.Get(), m_CB_VS_vertexshader))
			return false;

		if (!m_Axis.Init(m_pDevice.Get(), m_pDeviceContext.Get(), m_CB_VS_vertexshader_2d))
			return false;

		if (!m_InfiniteGrid.Init(m_pDevice.Get(), m_pDeviceContext.Get(), m_CB_VS_vertexshader_grid))
			return false;

		// Ocean
		if (!m_Ocean.Init(m_pDevice.Get(), m_pDeviceContext.Get(), m_CB_CS_Perlin, m_CB_VS_Ocean, 256, 256))
			return false;

		m_Axis.SetPosition(0.0f, 0.0f, 0.0f);
		m_Axis.SetScale(5.0f, 5.0f, 5.0f);

		m_Light.SetPosition(2.0f, 2.0f, 0.0f);

		//m_InfiniteGrid.SetPosition(0.0f, 0.0f, 0.0f);
		m_InfiniteGrid.SetScale(1.0f, 1.0f, 1.0f);

		m_Ocean.SetScale(5.0f, 5.0f, 5.0f);

		// Load sprite(s);
		if (!m_Sprite.Init(m_pDevice.Get(), m_pDeviceContext.Get(), 240, 304, "src/Data/Textures/Full Gimp SpriteSheet.png", m_CB_VS_vertexshader_2d)) /*240, 304*/
			return false;

		m_Camera2D.SetProjectionValues(static_cast<float>(m_WindowWidth), static_cast<float>(m_WindowHeight), 0.0f, 1.0f);

		m_Camera.SetPosition(5.f, 5.f, 0.f);
		m_Camera.SetProjectionValues(90.0f, static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight), .1f, 10000.f);
		m_Camera.SetLookAtPosition(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));

		m_Camera2.SetPosition(5.f, 100.f, 0.f);
		m_Camera2.SetProjectionValues(90.0f, (static_cast<float>(m_WindowWidth) / 2.f) / static_cast<float>(m_WindowHeight), .1f, 10000.f);
		m_Camera2.SetLookAtPosition(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));

		return true;
	}
}