////////////////////////////////////////////////////////////////////////////////
// Filename: Shaders.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _SHADERS_H_
#define _SHADERS_H_

//////////////
// INCLUDES //
//////////////

#include <vector>
#include <string>
#include <comdef.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <d3dcompiler.h>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "../Platform/Debug/ErrorLogger.h"

#include "Vertex.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: VertexShader
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class VertexShader
	{
	public:
		template<typename VertexType>
		bool Init(Microsoft::WRL::ComPtr<ID3D11Device>& device, std::wstring shaderPath)
		{
			m_Format = VertexType::format;
			return Init(device, shaderPath, VertexType::layoutDescription, VertexType::numElements);
		}

		bool Init(Microsoft::WRL::ComPtr<ID3D11Device>& device, std::wstring shaderPath, D3D11_INPUT_ELEMENT_DESC* layoutDescription, UINT numElements);
		const VertexFormat GetFormat() const { return m_Format; }
		ID3D11VertexShader* GetShader();
		ID3D10Blob* GetBuffer();
		ID3D11InputLayout* GetInputLayout();

	private:
		VertexFormat m_Format = VertexFormat::Unknown;

		Microsoft::WRL::ComPtr<ID3D11VertexShader>		m_pVertexShader;
		Microsoft::WRL::ComPtr<ID3D10Blob>				m_pShaderBuffer;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>		m_pInputLayout;		// Used to determine how the vertex data is laid out in memory.
	};
}

////////////////////////////////////////////////////////////////////////////////
// Class name: PixelShader
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class PixelShader
	{
	public:
		bool Init(Microsoft::WRL::ComPtr<ID3D11Device>& device, std::wstring shaderPath);
		ID3D11PixelShader* GetShader();
		ID3D10Blob* GetBuffer();

	private:
		Microsoft::WRL::ComPtr<ID3D11PixelShader>		m_pPixelShader;
		Microsoft::WRL::ComPtr<ID3D10Blob>				m_pShaderBuffer;
	};
}

////////////////////////////////////////////////////////////////////////////////
// Class name: ComputeShader
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class ComputeShader
	{
	public:
		bool Init(Microsoft::WRL::ComPtr<ID3D11Device>& device, std::wstring shaderPath);
		ID3D11ComputeShader* GetShader();
		ID3D10Blob* GetBuffer();

	private:
		Microsoft::WRL::ComPtr<ID3D11ComputeShader>		m_pComputeShader;
		Microsoft::WRL::ComPtr<ID3D10Blob>				m_pShaderBuffer;
	};
}

#endif // !_SHADERS_H_
