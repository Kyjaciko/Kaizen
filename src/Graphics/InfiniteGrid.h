////////////////////////////////////////////////////////////////////////////////
// Filename: InfiniteGrid.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _INFINITEGRID_H_
#define _INFINITEGRID_H_

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "GameObject3D.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: InfiniteGrid
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class InfiniteGrid : public GameObject3D
	{
	public:
		bool Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader_grid>& cb_vs_VertexShader_2d);

		void Draw(const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix, float nearZ, float farZ);

	private:
		IndexBuffer	m_Indices;
		VertexBuffer<Vertex2D_StaticColor> m_Vertices;
		ConstantBuffer<CB_VS_vertexshader_grid>* m_pCB_VS_VertexShader_grid;

		DirectX::XMMATRIX m_WorldMatrix = DirectX::XMMatrixIdentity();

		ID3D11DeviceContext* m_pDeviceContext;

	private:
		virtual void UpdateMatrix() override;
	};
}

#endif // !_INFINITEGRID_H_


