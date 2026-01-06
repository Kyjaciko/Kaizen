////////////////////////////////////////////////////////////////////////////////
// Filename: RenderableGameObject.h
////////////////////////////////////////////////////////////////////////////////

#include "RenderableGameObject.h"

namespace DirectX11
{
	bool RenderableGameObject::Init(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_VertexBuffer)
	{
		if (!m_Model.Init(filePath, device, deviceContext, cb_vs_VertexBuffer))
			return false;

		SetPosition(0.0f, 0.0f, 0.0f);
		SetRotation(0.0f, 0.0f, 0.0f);
		UpdateMatrix();
		return true;
	}

	void RenderableGameObject::Draw(const DirectX::XMMATRIX& viewProjectionMatrix)
	{
		m_Model.Draw(m_WorldMatrix, viewProjectionMatrix);
	}

	void RenderableGameObject::UpdateMatrix()
	{
		m_WorldMatrix = DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z) * DirectX::XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
		UpdateDirectionVectors();
	}
}