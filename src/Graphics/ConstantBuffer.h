////////////////////////////////////////////////////////////////////////////////
// Filename: ConstantBuffer.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _CONSTANTBUFFER_H_
#define _CONSTANTBUFFER_H_

//////////////
// INCLUDES //
//////////////

#include <d3d11.h>
#include <wrl/client.h>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "ConstantBufferTypes.h"
#include "../Platform/Debug/ErrorLogger.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: ConstantBuffer
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	template<typename T>
	class ConstantBuffer
	{
	public:
		T data;

	public:
		ConstantBuffer() {}

		HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
		{
			if (m_pConstantBuffer.Get()) 
				m_pConstantBuffer.Reset();

			m_pDeviceContext = deviceContext;

			D3D11_BUFFER_DESC constant_buffer_description;
			ZeroMemory(&constant_buffer_description, sizeof(D3D11_BUFFER_DESC));

			constant_buffer_description.Usage = D3D11_USAGE_DYNAMIC;
			constant_buffer_description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			constant_buffer_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			constant_buffer_description.MiscFlags = 0;
			constant_buffer_description.ByteWidth = static_cast<UINT>(sizeof(T) + (16 - (sizeof(T) % 16))); // Constant buffers have to be 16-byte aligned.
			constant_buffer_description.StructureByteStride = 0;

			return device->CreateBuffer(&constant_buffer_description, nullptr, m_pConstantBuffer.GetAddressOf());
		}

		bool ApplyChanges()
		{
			D3D11_MAPPED_SUBRESOURCE mapped_resource;
			HRESULT hr = m_pDeviceContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
			COM_ERROR_IF_FAILED_RETURN(hr, "Failed to map the constant buffer.", false);

			CopyMemory(mapped_resource.pData, &data, sizeof(T)); // Copy the data to the constant buffer.
			m_pDeviceContext->Unmap(m_pConstantBuffer.Get(), 0);
			return true;
		}

		ID3D11Buffer* Get() const { return m_pConstantBuffer.Get(); }
		ID3D11Buffer* const* GetAddressOf() const { return m_pConstantBuffer.GetAddressOf(); }

	private:
		ID3D11DeviceContext* m_pDeviceContext;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pConstantBuffer;

	private:
		ConstantBuffer(const ConstantBuffer& cpy);
	};
}

#endif // !_CONSTANTBUFFER_H_
