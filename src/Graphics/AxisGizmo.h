////////////////////////////////////////////////////////////////////////////////
// Filename: AxisGizmo.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _AXISGIZMO_H_
#define _AXISGIZMO_H_

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "GameObject3D.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: AxisGizmo
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class AxisGizmo : public GameObject3D
	{
	public:
		bool Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader_2d>& cb_vs_VertexShader_2d);

		void Draw(const DirectX::XMMATRIX& viewProjectionMatrix);

	private:
		IndexBuffer								m_Indices;
		VertexBuffer<Vertex2D>					m_Vertices;
		ConstantBuffer<CB_VS_vertexshader_2d>*	m_pCB_VS_VertexShader_2d;

		DirectX::XMMATRIX						m_WorldMatrix = DirectX::XMMatrixIdentity();

		ID3D11DeviceContext*					m_pDeviceContext;

	private:
		virtual void UpdateMatrix() override;
	};
}

#endif // !_AXISGIZMO_H_

