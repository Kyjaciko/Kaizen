////////////////////////////////////////////////////////////////////////////////
// Filename: IndexBuffer.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _INDEXBUFFER_H_
#define _INDEXBUFFER_H_

//////////////
// INCLUDES //
//////////////

#include <d3d11.h>
#include <wrl/client.h>

////////////////////////////////////////////////////////////////////////////////
// Class name: IndexBuffer
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class IndexBuffer
	{
	public:
		IndexBuffer() {}

		HRESULT Init(ID3D11Device* device, DWORD* data, UINT numIndices)
		{
			// Avoid memory leaks by resetting the index buffer if it already exists.
			if (m_pIndexBuffer.Get())
				m_pIndexBuffer.Reset();

			m_IndexCount = numIndices;

			D3D11_BUFFER_DESC buffer_description;
			ZeroMemory(&buffer_description, sizeof(D3D11_BUFFER_DESC));

			buffer_description.Usage = D3D11_USAGE_DEFAULT;			   // Usage of the buffer.
			buffer_description.ByteWidth = sizeof(DWORD) * numIndices; // Size of the buffer.
			buffer_description.BindFlags = D3D11_BIND_INDEX_BUFFER;	   // Set the buffer to be used as a index buffer.
			buffer_description.CPUAccessFlags = 0;
			buffer_description.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA buffer_data;
			ZeroMemory(&buffer_data, sizeof(D3D11_SUBRESOURCE_DATA));
			buffer_data.pSysMem = data;

			return device->CreateBuffer(&buffer_description, &buffer_data, m_pIndexBuffer.GetAddressOf());
		}

		ID3D11Buffer* Get() const { return m_pIndexBuffer.Get(); }
		ID3D11Buffer* const* GetAddressOf() const { return m_pIndexBuffer.GetAddressOf(); }
		UINT IndexCount() const { return m_IndexCount; }

	private:
		UINT m_IndexCount;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;

	private:
		IndexBuffer(const IndexBuffer& cpy);
	};
}

#endif // !_INDEXBUFFER_H_