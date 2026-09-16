////////////////////////////////////////////////////////////////////////////////
// Filename: VertexBuffer.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _VERTEXBUFFER_H_
#define _VERTEXBUFFER_H_

//////////////
// INCLUDES //
//////////////

#include <memory>
#include <d3d11.h>
#include <wrl/client.h>

////////////////////////////////////////////////////////////////////////////////
// Class name: VertexBuffer
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	template<typename T>
	class VertexBuffer
	{
	public:
		VertexBuffer() {}

		VertexBuffer(const VertexBuffer<T>& cpy)
		{
			m_pVertexBuffer = cpy.m_pVertexBuffer;
			m_VertexCount	= cpy.m_VertexCount;
			m_Stride		= cpy.m_Stride;
		}

		VertexBuffer<T>& operator=(const VertexBuffer<T>& eq)
		{
			m_pVertexBuffer = eq.m_pVertexBuffer;
			m_VertexCount	= eq.m_VertexCount;
			m_Stride		= eq.m_Stride;
			return *this;
		}
		

		HRESULT Init(ID3D11Device* device, T* data, UINT numVertices)
		{
			// Avoid memory leaks by resetting the index buffer if it already exists.
			if (m_pVertexBuffer.Get())
				m_pVertexBuffer.Reset();

			m_VertexCount = numVertices;

			D3D11_BUFFER_DESC buffer_description;
			ZeroMemory(&buffer_description, sizeof(D3D11_BUFFER_DESC));

			buffer_description.Usage = D3D11_USAGE_DEFAULT;
			buffer_description.ByteWidth = m_Stride * numVertices;
			buffer_description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			buffer_description.CPUAccessFlags = 0;
			buffer_description.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA buffer_data;
			ZeroMemory(&buffer_data, sizeof(D3D11_SUBRESOURCE_DATA));
			buffer_data.pSysMem = data;

			return device->CreateBuffer(&buffer_description, &buffer_data, m_pVertexBuffer.GetAddressOf());
		}

		ID3D11Buffer* Get() const { return m_pVertexBuffer.Get(); }
		ID3D11Buffer* const * GetAddressOf() const { return m_pVertexBuffer.GetAddressOf(); }
		UINT VertexCount() const { return m_VertexCount; }
		const UINT Stride() const { return m_Stride; }
		const UINT* StridePointer() const { return &m_Stride; }

	private:
		UINT m_VertexCount;
		UINT m_Stride = sizeof(T);
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	};
}

#endif // !_VERTEXBUFFER_H_