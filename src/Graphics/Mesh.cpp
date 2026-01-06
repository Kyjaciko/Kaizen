////////////////////////////////////////////////////////////////////////////////
// Filename: Mesh.h
////////////////////////////////////////////////////////////////////////////////

#include "Mesh.h"

namespace DirectX11
{
	Mesh::Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<Vertex3D>& vertices, std::vector<DWORD>& indices, std::vector<Texture>& textures, const DirectX::XMMATRIX& transformMatrix)
	{
		m_pDeviceContext  = deviceContext;
		m_Textures		  = textures;
		m_TransformMatrix = transformMatrix;

		HRESULT hr = m_VertexBuffer.Init(device, vertices.data(), vertices.size());
		COM_ERROR_IF_FAILED_SHOW(hr, "Failed to initialize vertex buffer for mesh.");

		hr = m_IndexBuffer.Init(device, indices.data(), indices.size());
		COM_ERROR_IF_FAILED_SHOW(hr, "Failed to initialize vertex buffer for mesh.");
	}

	Mesh::Mesh(const Mesh& cpy)
	{
		m_pDeviceContext  = cpy.m_pDeviceContext;
		m_IndexBuffer	  = cpy.m_IndexBuffer;
		m_VertexBuffer	  = cpy.m_VertexBuffer;
		m_Textures		  = cpy.m_Textures;
		m_TransformMatrix = cpy.m_TransformMatrix;
	}

	void Mesh::Draw()
	{
		UINT offset = 0;

		for (int i = 0; i < m_Textures.size(); i++)
		{
			if (m_Textures[i].GetType() == aiTextureType_DIFFUSE)
			{
				m_pDeviceContext->PSSetShaderResources(0, 1, m_Textures[i].GetTextureResourceViewAddress());
				break;
			}
		}

		m_pDeviceContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_pDeviceContext->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), m_VertexBuffer.StridePointer(), &offset);
		m_pDeviceContext->DrawIndexed(m_IndexBuffer.IndexCount(), 0, 0);
	}

	const DirectX::XMMATRIX& Mesh::GetTransformMatrix()
	{
		return m_TransformMatrix;
	}
}
