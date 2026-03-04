////////////////////////////////////////////////////////////////////////////////
// Filename: Light.h
////////////////////////////////////////////////////////////////////////////////

#include "Light.h"

namespace DirectX11
{
	bool Light::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_VertexBuffer)
	{
		if (!m_Model.Init("../../src/Data/Objects/light.fbx", device, deviceContext, cb_vs_VertexBuffer))
			return false;
		
		SetPosition(0.0f, 0.0f, 0.0f);
		SetRotation(0.0f, 0.0f, 0.0f);
		UpdateMatrix();
		return true;
	}
}