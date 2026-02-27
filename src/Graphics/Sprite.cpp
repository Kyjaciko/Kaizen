////////////////////////////////////////////////////////////////////////////////
// Filename: Sprite.cpp
////////////////////////////////////////////////////////////////////////////////

#include "Sprite.h"

namespace DirectX11
{
	bool Sprite::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float width, float height, Kaizen::Types::ResourceID textureID/*const std::string& spritePath*/, ConstantBuffer<CB_VS_vertexshader_2d>& cb_vs_VertexShader2D)
	{
		m_pDeviceContext = deviceContext;
		if (!m_pDeviceContext) return false;

		m_pCB_VS_VertexShader2D = &cb_vs_VertexShader2D;
		//m_Texture = std::make_unique<Texture>(device, spritePath, aiTextureType_DIFFUSE);
		m_TextureID = textureID;

		std::vector<Vertex2D> vertex_data =
		{
			Vertex2D(-0.5f, -0.5f, 0.0f, 0.0f, 0.0f),	// Top left.
			Vertex2D(0.5f, -0.5f, 0.0f, 1.0f, 0.0f),	// Top right.
			Vertex2D(-0.5f, 0.5f, 0.0f, 0.0f, 1.0f),	// Bottom left.
			Vertex2D(0.5f, 0.5f, 0.0f, 1.0f, 1.0f)		// Bottom right.
		};

		std::vector<DWORD> index_data =
		{
			0, 1, 2,
			2, 1, 3
		};

		HRESULT hr = m_Vertices.Init(device, vertex_data.data(), vertex_data.size());
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to initialize vertex buffer for sprite", false);

		hr = m_Indices.Init(device, index_data.data(), index_data.size());
		COM_ERROR_IF_FAILED_RETURN(hr, "Failed to initialize vertex buffer for sprite", false);

		SetPosition(0.0f, 0.0f, 0.0f);
		SetRotation(0.0f, 0.0f, 0.0f);
		SetScale(width, height);

		return true;
	}

	void Sprite::Draw(DirectX::XMMATRIX orthoMatrix)
	{
		DirectX::XMMATRIX wvp_matrix = m_WorldMatrix * orthoMatrix;
		m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pCB_VS_VertexShader2D->GetAddressOf());
		m_pCB_VS_VertexShader2D->data.wvpMatrix = wvp_matrix;
		m_pCB_VS_VertexShader2D->ApplyChanges();

		//m_pDeviceContext->PSSetShaderResources(0, 1, m_Texture->GetTextureResourceViewAddress());

		const UINT offsets = 0;
		m_pDeviceContext->IASetVertexBuffers(0, 1, m_Vertices.GetAddressOf(), m_Vertices.StridePointer(), &offsets);
		m_pDeviceContext->IASetIndexBuffer(m_Indices.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
		m_pDeviceContext->DrawIndexed(m_Indices.IndexCount(), 0, 0);
	}

	float Sprite::GetWidth() const
	{
		return m_Scale.x;
	}

	float Sprite::GetHeight() const
	{
		return m_Scale.y;
	}

	void Sprite::UpdateMatrix()
	{
		m_WorldMatrix = DirectX::XMMatrixScaling(m_Scale.x, m_Scale.y, 1.0f) * DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z) * DirectX::XMMatrixTranslation(m_Position.x + m_Scale.x / 2.0f, m_Position.y + m_Scale.y / 2.0f, m_Position.z);
	}
}