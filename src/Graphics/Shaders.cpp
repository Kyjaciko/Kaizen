////////////////////////////////////////////////////////////////////////////////
// Filename: Shaders.cpp
////////////////////////////////////////////////////////////////////////////////

#include "Shaders.h"

namespace DirectX11
{
	bool VertexShader::Init(Microsoft::WRL::ComPtr<ID3D11Device>& device, std::wstring shaderPath, D3D11_INPUT_ELEMENT_DESC* layoutDescription, UINT numElements)
	{
		HRESULT hr = D3DReadFileToBlob(shaderPath.c_str(), m_pShaderBuffer.GetAddressOf());
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to load shader: " + shaderPath, false);
		
		hr = device->CreateVertexShader(m_pShaderBuffer->GetBufferPointer(), m_pShaderBuffer->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf());
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to create vertex shader: " + shaderPath, false);

		hr = device->CreateInputLayout(
			layoutDescription,
			numElements,
			m_pShaderBuffer->GetBufferPointer(),
			m_pShaderBuffer->GetBufferSize(),
			m_pInputLayout.GetAddressOf()
		);

		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to create input layout.", false);

		return true;
	}

	ID3D11VertexShader* VertexShader::GetShader()
	{
		return m_pVertexShader.Get();
	}

	ID3D10Blob* VertexShader::GetBuffer()
	{
		return m_pShaderBuffer.Get();
	}

	ID3D11InputLayout* VertexShader::GetInputLayout()
	{
		return m_pInputLayout.Get();
	}
}

namespace DirectX11
{
	bool PixelShader::Init(Microsoft::WRL::ComPtr<ID3D11Device>& device, std::wstring shaderPath)
	{
		HRESULT hr = D3DReadFileToBlob(shaderPath.c_str(), m_pShaderBuffer.GetAddressOf());
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to load shader: " + shaderPath, false);

		hr = device->CreatePixelShader(m_pShaderBuffer->GetBufferPointer(), m_pShaderBuffer->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf());
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to create pixel shader: " + shaderPath, false);

		return true;
	}

	ID3D11PixelShader* PixelShader::GetShader()
	{
		return m_pPixelShader.Get();
	}

	ID3D10Blob* PixelShader::GetBuffer()
	{
		return m_pShaderBuffer.Get();
	}
}

namespace DirectX11
{
	bool ComputeShader::Init(Microsoft::WRL::ComPtr<ID3D11Device>& device, std::wstring shaderPath)
	{
		HRESULT hr = D3DReadFileToBlob(shaderPath.c_str(), m_pShaderBuffer.GetAddressOf());
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to load shader: " + shaderPath, false);

		hr = device->CreateComputeShader(m_pShaderBuffer->GetBufferPointer(), m_pShaderBuffer->GetBufferSize(), nullptr, m_pComputeShader.GetAddressOf());
		COM_ERROR_IF_FAILED_RETURN(hr, L"Failed to create compute shader: " + shaderPath, false);

		return true;
	}

	ID3D11ComputeShader* ComputeShader::GetShader()
	{
		return m_pComputeShader.Get();
	}

	ID3D10Blob* ComputeShader::GetBuffer()
	{
		return m_pShaderBuffer.Get();
	}
}