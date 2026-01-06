////////////////////////////////////////////////////////////////////////////////
// Filename: Texture.h
////////////////////////////////////////////////////////////////////////////////

#include "Texture.h"

namespace DirectX11
{
	Texture::Texture(ID3D11Device* device, const Color& color, aiTextureType type)
	{
		Init1x1ColorTexture(device, color, type);
	}

	Texture::Texture(ID3D11Device* device, const Color* colorData, UINT width, UINT height, aiTextureType type)
	{
		InitColorTexture(device, colorData, width, height, type);
	}

	Texture::Texture(ID3D11Device* device, const std::string& filePath, aiTextureType type)
	{
		m_Type = type;

		if (StringHelper::GetFileExtension(filePath) == ".dds")
		{
			HRESULT hr = DirectX::CreateDDSTextureFromFile(device, StringHelper::StringToWide(filePath).c_str(), m_Texture.GetAddressOf(), m_TextureView.GetAddressOf());
			if (FAILED(hr))
				Init1x1ColorTexture(device, Colors::UnloadedTextureColor, type);
		}
		else
		{
			HRESULT hr = DirectX::CreateWICTextureFromFile(device, StringHelper::StringToWide(filePath).c_str(), m_Texture.GetAddressOf(), m_TextureView.GetAddressOf());
			if (FAILED(hr))
				Init1x1ColorTexture(device, Colors::UnloadedTextureColor, type);
		}
	}

	Texture::Texture(ID3D11Device* device, const uint8_t* data, size_t size, aiTextureType type)
	{
		m_Type = type;

		HRESULT hr = DirectX::CreateWICTextureFromMemory(device, data, size, m_Texture.GetAddressOf(), m_TextureView.GetAddressOf());
		COM_ERROR_IF_FAILED_SHOW(hr, "Failed to create Texture from memory.");
	}

	aiTextureType Texture::GetType()
	{
		return m_Type;
	}

	ID3D11ShaderResourceView* Texture::GetTextureResourceView()
	{
		return m_TextureView.Get();
	}

	ID3D11ShaderResourceView** Texture::GetTextureResourceViewAddress()
	{
		return m_TextureView.GetAddressOf();
	}

	void Texture::Init1x1ColorTexture(ID3D11Device* device, const Color& colorData, aiTextureType type)
	{
		InitColorTexture(device, &colorData, 1, 1, type);
	}

	void Texture::InitColorTexture(ID3D11Device* device, const Color* colorData, UINT width, UINT height, aiTextureType type)
	{
		m_Type = type;

		CD3D11_TEXTURE2D_DESC texture_description(DXGI_FORMAT_R8G8B8A8_UNORM, width, height);
		ID3D11Texture2D* p_2D_texture = nullptr;
		D3D11_SUBRESOURCE_DATA initial_data{};
		initial_data.pSysMem	 = colorData;
		initial_data.SysMemPitch = width * sizeof(Color);

		HRESULT hr = device->CreateTexture2D(&texture_description, &initial_data, &p_2D_texture);
		COM_ERROR_IF_FAILED_SHOW(hr, "Failed to initialize texture from color data.");

		m_Texture = static_cast<ID3D11Texture2D*>(p_2D_texture);
		CD3D11_SHADER_RESOURCE_VIEW_DESC srv_description(D3D11_SRV_DIMENSION_TEXTURE2D, texture_description.Format);
		hr = device->CreateShaderResourceView(m_Texture.Get(), &srv_description, m_TextureView.GetAddressOf());
		COM_ERROR_IF_FAILED_SHOW(hr, "Failed to create shader resource view from texture generated from color data.");
	}
}
