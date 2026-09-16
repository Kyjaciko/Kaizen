////////////////////////////////////////////////////////////////////////////////
// Filename: InfiniteGrid.cpp
////////////////////////////////////////////////////////////////////////////////

#include "InfiniteGrid.h"

namespace DirectX11
{
	bool InfiniteGrid::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader_grid>& cb_vs_VertexShader_2d)
	{
		m_pDeviceContext = deviceContext;
		if (!m_pDeviceContext) return false;

		m_pCB_VS_VertexShader_grid = &cb_vs_VertexShader_2d;

		// In normalized device coordinates (NDC).
		std::vector<Vertex2D_StaticColor> vertex_data =
		{
			Vertex2D_StaticColor(1.0f, 1.0f, 0.0f, 0.2f, 0.2f, 0.2f, 1.0f),
			Vertex2D_StaticColor(-1.0f, -1.0f, 0.0f, 0.2f, 0.2f, 0.2f, 1.0f),
			Vertex2D_StaticColor(-1.0f, 1.0f, 0.0f, 0.2f, 0.2f, 0.2f, 1.0f),
			Vertex2D_StaticColor(1.0f, -1.0f, 0.0f, 0.2f, 0.2f, 0.2f, 1.0f)
		};

		std::vector<DWORD> index_data =
		{
			0, 1, 2, 
			1, 0, 3
		};

		HRESULT hr = m_Vertices.Init(device, vertex_data.data(), vertex_data.size());
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to initialize vertex buffer for sprite", false);

		hr = m_Indices.Init(device, index_data.data(), index_data.size());
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to initialize vertex buffer for sprite", false);

		SetPosition(0.0f, 0.0f, 0.0f);
		SetRotation(0.0f, 0.0f, 0.0f);

		return true;
	}

	void InfiniteGrid::Draw(const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix, float nearZ, float farZ)
	{
		DirectX::XMMATRIX viewProjectionMatrix = viewMatrix * projectionMatrix;
		DirectX::XMMATRIX wvp_matrix = m_WorldMatrix * viewProjectionMatrix;
		m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pCB_VS_VertexShader_grid->GetAddressOf());
		m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pCB_VS_VertexShader_grid->GetAddressOf());
		m_pCB_VS_VertexShader_grid->data.wvpMatrix = wvp_matrix;
		m_pCB_VS_VertexShader_grid->data.invViewMatrix = DirectX::XMMatrixInverse(nullptr, viewMatrix);
		m_pCB_VS_VertexShader_grid->data.invProjectionMatrix = DirectX::XMMatrixInverse(nullptr, projectionMatrix);
		m_pCB_VS_VertexShader_grid->data.cameraNear = nearZ;
		m_pCB_VS_VertexShader_grid->data.cameraFar = farZ;
		m_pCB_VS_VertexShader_grid->ApplyChanges();

		const UINT offsets = 0;
		m_pDeviceContext->IASetVertexBuffers(0, 1, m_Vertices.GetAddressOf(), m_Vertices.StridePointer(), &offsets);
		m_pDeviceContext->IASetIndexBuffer(m_Indices.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
		m_pDeviceContext->DrawIndexed(m_Indices.IndexCount(), 0, 0);
	}

	void InfiniteGrid::UpdateMatrix()
	{
		m_WorldMatrix = DirectX::XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) * DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z) * DirectX::XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	}
}