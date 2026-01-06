////////////////////////////////////////////////////////////////////////////////
// Filename: Sprite.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _SPRITE_H_
#define _SPRITE_H_

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "GameObject2D.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: Sprite
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class Sprite : public GameObject2D
	{
	public:
		bool Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float width, float height, const std::string& spritePath, ConstantBuffer<CB_VS_vertexshader_2d>& cb_vs_VertexShader2D);

		void Draw(DirectX::XMMATRIX orthoMatrix);

		float GetWidth() const;
		float GetHeight() const;

	private:
		IndexBuffer								m_Indices;
		VertexBuffer<Vertex2D>					m_Vertices;
		
		std::unique_ptr<Texture>				m_Texture;
		ID3D11DeviceContext*					m_pDeviceContext;

		ConstantBuffer<CB_VS_vertexshader_2d>*	m_pCB_VS_VertexShader2D;
		DirectX::XMMATRIX						m_WorldMatrix = DirectX::XMMatrixIdentity();

	private:
		virtual void UpdateMatrix() override;
	};
}

#endif // !_SPRITE_H_
