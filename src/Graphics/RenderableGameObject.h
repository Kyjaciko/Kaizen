////////////////////////////////////////////////////////////////////////////////
// Filename: RenderableGameObject.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERABLEGAMEOBJECT_H_
#define _RENDERABLEGAMEOBJECT_H_

//////////////
// INCLUDES //
//////////////

#include "GameObject3D.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: RenderableGameObject
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class RenderableGameObject : public GameObject3D
	{
	public:
		bool Init(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_VertexBuffer);

		void Draw(const DirectX::XMMATRIX& viewProjectionMatrix);

	protected:
		Model m_Model;
		DirectX::XMMATRIX m_WorldMatrix = DirectX::XMMatrixIdentity();

	protected:
		virtual void UpdateMatrix() override;
	};
}

#endif // !_RENDERABLEGAMEOBJECT_H_

