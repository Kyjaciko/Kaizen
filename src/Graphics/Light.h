////////////////////////////////////////////////////////////////////////////////
// Filename: Light.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _LIGHT_H_
#define _LIGHT_H_

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "RenderableGameObject.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: Light
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class Light : public RenderableGameObject
	{
	public:
		bool Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_VertexBuffer);

	public:
		float m_LightStrength = 1.f;
		DirectX::XMFLOAT3 m_LightColor = DirectX::XMFLOAT3(1.f, 1.f, 1.f);

		// Light Attenuation.
		float m_LightAttenuationA = 1.f;
		float m_LightAttenuationB = .1f;
		float m_LightAttenuationC = .1f;
	};
}

#endif // !_LIGHT_H_

