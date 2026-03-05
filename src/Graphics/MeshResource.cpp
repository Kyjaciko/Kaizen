////////////////////////////////////////////////////////////////////////////////
// Filename: MeshResource.cpp
////////////////////////////////////////////////////////////////////////////////

#include "MeshResource.h"
#include "Graphics/Vertex.h"

//template class Kaizen::Graphics::MeshResource<DirectX11::VertexPosUV>;

namespace Kaizen::Graphics
{
	/*template<typename VertexType>
	MeshResource::MeshResource(ID3D11Device* device, std::vector<VertexType>& vertices, std::vector<DWORD>& indices)
	{
		HRESULT hr = m_VertexBuffer.Init(device, vertices.data(), vertices.size());
		COM_ERROR_IF_FAILED_SHOW(hr, "Failed to initialize vertex buffer for mesh.");

		hr = m_IndexBuffer.Init(device, indices.data(), indices.size());
		COM_ERROR_IF_FAILED_SHOW(hr, "Failed to initialize vertex buffer for mesh.");
	}*/

	void MeshResource::Bind(ID3D11DeviceContext* deviceContext, UINT startSlot, UINT numBuffersconst, const UINT offsets)
	{
		//const UINT offsets = 0;
		deviceContext->IASetVertexBuffers(startSlot, numBuffersconst, m_pVertexBuffer->GetAddressOf(), m_pVertexBuffer->StridePointer(), &offsets);
		deviceContext->IASetIndexBuffer(m_pIndexBuffer->Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	}
}
