////////////////////////////////////////////////////////////////////////////////
// Filename: Texture.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _TEXTURE_H_
#define _TEXTURE_H_

//////////////
// INCLUDES //
//////////////

#include <d3d11.h>
#include <wrl/client.h>
#include <assimp/material.h>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "Color.h"
#include "../Platform/Debug/ErrorLogger.h"

//////////////////
// ENUMERATIONS //
//////////////////

enum TextureStorageType
{
	INVALID,
	NONE,
	EMBEDDED_INDEX_COMPRESSED,
	EMBEDDED_INDEX_NON_COMPRESSED,
	EMBEDDED_COMPRESSED,
	EMBEDDED_NON_COMPRESSED,
	DISK
};

////////////////////////////////////////////////////////////////////////////////
// Class name: Texture
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class Texture
	{
	public:
		Texture(ID3D11Device* device, const Color& color, aiTextureType type);
		Texture(ID3D11Device* device, const Color* colorData, UINT width, UINT height, aiTextureType type);
		Texture(ID3D11Device* device, const std::string& filePath, aiTextureType type);
		Texture(ID3D11Device* device, const uint8_t* data, size_t size, aiTextureType type);

		aiTextureType GetType();
		ID3D11ShaderResourceView* GetTextureResourceView();
		ID3D11ShaderResourceView** GetTextureResourceViewAddress();

	private:
		aiTextureType m_Type = aiTextureType_UNKNOWN;
		Microsoft::WRL::ComPtr<ID3D11Resource> m_Texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_TextureView;

	private:
		void Init1x1ColorTexture(ID3D11Device* device, const Color& colorData, aiTextureType type);
		void InitColorTexture(ID3D11Device* device, const Color* colorData, UINT width, UINT height, aiTextureType type);
	};
}

#endif // !_TEXTURE_H_