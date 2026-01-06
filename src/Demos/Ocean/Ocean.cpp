////////////////////////////////////////////////////////////////////////////////
// Filename: Ocean.cpp
////////////////////////////////////////////////////////////////////////////////

#include "Ocean.h"

namespace DirectX11
{
	Ocean::~Ocean()
	{
		if (perlinUAV) perlinUAV->Release();
		if (perlinSRV) perlinSRV->Release();
	}

	bool Ocean::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<PerlinCB>& cbPerlin, ConstantBuffer<OceanCB>& cbOcean, UINT textureWidth, UINT textureHeight)
	{
		m_pDeviceContext = deviceContext;
		if (!m_pDeviceContext) return false;

		m_pCB_CS_Perlin = &cbPerlin;
		m_pCB_VS_Ocean = &cbOcean;

		m_Width = textureWidth;
		m_Height = textureHeight;

		// Create a grid for the ocean surface.
		std::vector<OceanVertex> vertex_data;
		std::vector<DWORD> index_data;
		CreateUniformGridOfVertices(vertex_data, index_data);

		HRESULT hr = m_Vertices.Init(device, vertex_data.data(), vertex_data.size());
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to initialize vertex buffer for ocean", false);

		hr = m_Indices.Init(device, index_data.data(), index_data.size());
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to initialize index buffer for ocean", false);

		SetupGrid(device);

		SetPosition(0.0f, 0.0f, 0.0f);
		SetRotation(0.0f, 0.0f, 0.0f);
		SetScale(1.0f, 1.0f, 1.0f);

		return true;
	}

	void Ocean::Draw(const Camera3D& camera, const DirectX::XMMATRIX& viewProjectionMatrix, ComputeShader& computeShader, PixelShader& ps, VertexShader& vs, ID3D11SamplerState* const* ppSamplers, double deltaTime)
	{
		DirectX::XMMATRIX projectorMatrix = DirectX::XMMatrixIdentity();
		float waveAmplitude = 2.0f; // moet gelijk zijn in SetupVertexShaderStage!
		//if (!CalculateProjectorMatrix(projectorMatrix, camera, DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), 0.1f, 1000.0f))
		if (!CalculateProjectorMatrix(projectorMatrix, camera, this->GetPositionVector(), waveAmplitude))
			return;

		GeneratePerlinNoise(computeShader, deltaTime);
		SetupVertexShaderStage(viewProjectionMatrix, projectorMatrix, vs, ppSamplers, deltaTime, camera.GetPositionFloat3());
		SetupPixelShaderStage(ps);
		SetupInputAssemblerStage(vs);

		ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
		m_pDeviceContext->VSSetShaderResources(0, 1, nullSRV);
		m_pDeviceContext->PSSetShaderResources(0, 1, nullSRV);
	}

	void Ocean::SetupGrid(ID3D11Device* device)
	{
		ID3D11Texture2D* perlin_texture = nullptr;

		// Create texture to store Perlin noise data.
		D3D11_TEXTURE2D_DESC texture_description = {};
		texture_description.Width = m_Width;
		texture_description.Height = m_Height;
		texture_description.MipLevels = 0;
		texture_description.ArraySize = 1;
		texture_description.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		texture_description.SampleDesc.Count = 1;
		texture_description.Usage = D3D11_USAGE_DEFAULT;
		texture_description.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texture_description.CPUAccessFlags = 0;
		texture_description.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		HRESULT hr = device->CreateTexture2D(&texture_description, nullptr, &perlin_texture);
		COM_ERROR_IF_FAILED_SHOW(hr, "Failed to create Perlin noise texture.");

		// Create unordered access view (UAV) for compute writing.
		D3D11_UNORDERED_ACCESS_VIEW_DESC uav_description = {};
		uav_description.Format = texture_description.Format;
		uav_description.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uav_description.Texture2D.MipSlice = 0;

		hr = device->CreateUnorderedAccessView(perlin_texture, &uav_description, &perlinUAV);
		COM_ERROR_IF_FAILED_SHOW(hr, "Failed to create Perlin noise UAV.");

		// Create shader resource view (SRV) for vertexshader/pixelshader reading.
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_description = {};
		srv_description.Format = texture_description.Format;
		srv_description.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_description.Texture2D.MostDetailedMip = 0;
		srv_description.Texture2D.MipLevels = -1;

		hr = device->CreateShaderResourceView(perlin_texture, &srv_description, &perlinSRV);
		COM_ERROR_IF_FAILED_SHOW(hr, "Failed to create Perlin noise SRV.");

		perlin_texture->Release();
	}

	void Ocean::SetupVertexShaderStage(const DirectX::XMMATRIX& viewProjectionMatrix, const DirectX::XMMATRIX& projectorMatrix, VertexShader& vs, ID3D11SamplerState* const* ppSamplers, double deltaTime, const DirectX::XMFLOAT3& pos)
	{
		m_pDeviceContext->VSSetShader(vs.GetShader(), nullptr, 0);

		// Update constant buffer.
		m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pCB_VS_Ocean->GetAddressOf());
		m_pCB_VS_Ocean->data.wvpMatrix		= m_WorldMatrix * viewProjectionMatrix;
		m_pCB_VS_Ocean->data.vpMatrix = viewProjectionMatrix; // Camera VP
		m_pCB_VS_Ocean->data.projectorMatrix = projectorMatrix; // Range * InvProj
		m_pCB_VS_Ocean->data.cameraPos = pos;
		m_pCB_VS_Ocean->data.heightScale = 5.0f; // Zet dit hoog genoeg om effect te zien! (bv. 2.0f - 10.0f)

		static double time = 0.0f;
		time += deltaTime;
		m_pCB_VS_Ocean->data.totalTime = static_cast<float>(time);
		m_pCB_VS_Ocean->ApplyChanges();

		m_pDeviceContext->VSSetShaderResources(0, 1, &perlinSRV);
		m_pDeviceContext->VSSetSamplers(0, 1, ppSamplers);
	}

	void Ocean::SetupPixelShaderStage(PixelShader& ps)
	{
		m_pDeviceContext->PSSetShader(ps.GetShader(), nullptr, 0);
		m_pDeviceContext->PSSetShaderResources(0, 1, &perlinSRV);
	}

	void Ocean::SetupInputAssemblerStage(VertexShader& vs)
	{
		const UINT offsets = 0;
		m_pDeviceContext->IASetInputLayout(vs.GetInputLayout());
		m_pDeviceContext->IASetVertexBuffers(0, 1, m_Vertices.GetAddressOf(), m_Vertices.StridePointer(), &offsets);
		m_pDeviceContext->IASetIndexBuffer(m_Indices.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
		m_pDeviceContext->DrawIndexed(m_Indices.IndexCount(), 0, 0);
	}

	void Ocean::GeneratePerlinNoise(ComputeShader computeShader, double deltaTime)
	{
		// Update constant buffer.
		static double time = 0.0f;
		time += deltaTime;
		m_pCB_CS_Perlin->data.width			= m_Width;
		m_pCB_CS_Perlin->data.height		= m_Height;
		m_pCB_CS_Perlin->data.tileCount		= 8.0f;
		m_pCB_CS_Perlin->data.gridSize		= 32.0f;
		m_pCB_CS_Perlin->data.numOctaves	= 4; // origineel 8
		m_pCB_CS_Perlin->data.totalTime		= time;
		m_pCB_CS_Perlin->ApplyChanges();

		// Bind compute shader, constant buffer (on b0) and UAV (on u0).
		m_pDeviceContext->CSSetShader(computeShader.GetShader(), nullptr, 0);
		m_pDeviceContext->CSSetConstantBuffers(0, 1, m_pCB_CS_Perlin->GetAddressOf());
		m_pDeviceContext->CSSetUnorderedAccessViews(0, 1, &perlinUAV, nullptr);

		// Dispatch, adjust for threadgroup.
		UINT tgX = (m_Width + 15) / 16;
		UINT tgY = (m_Height + 15) / 16;
		m_pDeviceContext->Dispatch(tgX, tgY, 1);

		// Unbind UAV and compute shader.
		ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
		m_pDeviceContext->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
		m_pDeviceContext->CSSetShader(nullptr, nullptr, 0);
		m_pDeviceContext->GenerateMips(perlinSRV);
	}

	void Ocean::CreateUniformGridOfVertices(std::vector<OceanVertex>& vertices, std::vector<DWORD>& indices)
	{
		const UINT NUM_VERTICES_X = m_Width + 1;
		const UINT NUM_VERTICES_Y = m_Height + 1;

		const float GRID_WIDTH = 1.0f;
		const float GRID_DEPTH = 1.0f;

		// Avoid recizes.
		vertices.reserve(NUM_VERTICES_X * NUM_VERTICES_Y);
		indices.reserve((NUM_VERTICES_X - 1) * (NUM_VERTICES_Y - 1) * 6);

		// Fill vertexbuffer.
		for (UINT row = 0; row < NUM_VERTICES_Y; ++row)
		{
			float v = float(row) / float(NUM_VERTICES_Y - 1);
			float posZ = v * GRID_DEPTH;

			for (UINT col = 0; col < NUM_VERTICES_X; ++col)
			{
				float u = float(col) / float(NUM_VERTICES_X - 1);
				float posX = u * GRID_WIDTH;

				vertices.emplace_back(posX, 0.0f, posZ, u, v);
			}
		}

		// Fill indexbuffer.
		for (UINT row = 0; row < NUM_VERTICES_Y - 1; ++row)
		{
			UINT rowStart = row * NUM_VERTICES_X;				// Index of first vertex.
			UINT nextRowStart = (row + 1) * NUM_VERTICES_X;		// Index of first vertex in the next row.

			for (UINT col = 0; col < NUM_VERTICES_X - 1; ++col)
			{
				UINT tl = rowStart + col;
				UINT tr = rowStart + col + 1;
				UINT bl = nextRowStart + col;
				UINT br = nextRowStart + col + 1;

				// Triangle 1: (tl, bl, tr)
				indices.push_back(tl);
				indices.push_back(bl);
				indices.push_back(tr);

				// Triangle 2: (tr, bl, br)
				indices.push_back(tr);
				indices.push_back(bl);
				indices.push_back(br);
			}
		}
	}

	void Ocean::UpdateMatrix()
	{
		m_WorldMatrix = DirectX::XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) * DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z) * DirectX::XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	}
}