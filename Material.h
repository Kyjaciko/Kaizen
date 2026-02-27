////////////////////////////////////////////////////////////////////////////////
// Filename: Material.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _MATERIAL_H_
#define _MATERIAL_H_

//////////////
// INCLUDES //
//////////////

#include <d3d11.h>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "Types.h"
#include "graphics/Shaders.h"
#include "MeshResource.h"
#include "Graphics/ConstantBuffer.h"
#include "Graphics/ConstantBufferTypes.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: Model
////////////////////////////////////////////////////////////////////////////////
//class ConstantBuffer;

namespace Kaizen::Graphics
{
    // TODO: ConstantBuffer<CBType>* ConstantBuffer; make Interface class for CB
    /*template <typename CBType>
    struct Material
    {
        Kaizen::Types::TextureID DiffuseTexture = 0;
        //Kaizen::Types::ShaderID VertexShaderID = 0;
        //Kaizen::Types::ShaderID PixelShaderID = 0;
        DirectX11::VertexShader* VertexShader; // beter use ID's
        DirectX11::PixelShader* PixelShader;
        DirectX11::ConstantBuffer<CBType>* ConstantBuffer;
        //ID3D11InputLayout* InputLayout; -> Available in VertexShader.

        // Render States (optioneel, voor later)
        // ID3D11BlendState* BlendState;
    };*/

    // Graphics/Material.h
    class IMaterial
    {
    public:
        IMaterial(DirectX11::VertexShader* vertexShader, DirectX11::PixelShader* pixelShader, Kaizen::Types::ResourceID textureID)
        {
            m_RequiredFormat = vertexShader->GetFormat();
            m_pVertexShader  = vertexShader;
            m_pPixelShader   = pixelShader;
            m_DiffuseTexture = textureID;
        }

        virtual void Bind(ID3D11DeviceContext* deviceContext, Kaizen::Resources::TextureManager* textureManager)
        {
            deviceContext->IASetInputLayout(m_pVertexShader->GetInputLayout());
            deviceContext->VSSetShader(m_pVertexShader->GetShader(), nullptr, 0);
            deviceContext->PSSetShader(m_pPixelShader->GetShader(), nullptr, 0);

            if (!textureManager || !m_DiffuseTexture)
                return;

            DirectX11::Texture* temp_texture = textureManager->GetTexture(m_DiffuseTexture);
            deviceContext->PSSetShaderResources(0, 1, temp_texture->GetTextureResourceViewAddress());
        }

        virtual void UpdateData(const void* data) = 0;
        const DirectX11::VertexFormat GetRequiredFormat() const { return m_RequiredFormat; }

    protected:
        Kaizen::Types::ResourceID m_DiffuseTexture = 0;
        //Kaizen::Types::ShaderID VertexShaderID = 0;
        //Kaizen::Types::ShaderID PixelShaderID = 0;

        DirectX11::VertexFormat m_RequiredFormat;
        DirectX11::VertexShader* m_pVertexShader; // beter use ID's
        DirectX11::PixelShader* m_pPixelShader;
    };

    /// <summary>
    /// /
    /// </summary>

    class SpriteMaterial : public IMaterial
    {
    public:
		SpriteMaterial(ID3D11Device* device, ID3D11DeviceContext* deviceContext, DirectX11::VertexShader* vertexShader, DirectX11::PixelShader* pixelShader, Kaizen::Types::ResourceID textureID) 
            : IMaterial(vertexShader, pixelShader, textureID)
        {
            HRESULT hr = m_ConstantBuffer.Init(device, deviceContext);
            COM_ERROR_IF_FAILED_SHOW(hr, L"Failed to initialize sprite constant buffer.");
        }

        virtual void Bind(ID3D11DeviceContext* deviceContext, Kaizen::Resources::TextureManager* textureManager) override
        {
            IMaterial::Bind(deviceContext, textureManager);
            deviceContext->VSSetConstantBuffers(2, 1, m_ConstantBuffer.GetAddressOf());
        }

        virtual void UpdateData(const void* data) override
        {
            const DirectX11::CB_SpriteData* pData = static_cast<const DirectX11::CB_SpriteData*>(data);
            m_ConstantBuffer.data = *pData;
            m_ConstantBuffer.ApplyChanges();
        }

    private:
        DirectX11::ConstantBuffer<DirectX11::CB_SpriteData> m_ConstantBuffer;
    };
}

#endif // !_MATERIAL_H_
