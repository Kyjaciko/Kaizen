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

		/*float SetAttenuationFactorA(float A) { m_LightAttenuationA = A; }
		float SetAttenuationFactorB(float B) { m_LightAttenuationB = B; }
		float SetAttenuationFactorC(float C) { m_LightAttenuationC = C; }

		float GetStrength() { return m_LightStrength; }
		DirectX::XMFLOAT3 GetColor() { return m_LightColor; }
		float GetAttenuationFactorA() { return m_LightAttenuationA; }
		float GetAttenuationFactorB() { return m_LightAttenuationB; }
		float GetAttenuationFactorC() { return m_LightAttenuationC; }*/

	public:
		float				m_LightStrength = 1.0f;
		DirectX::XMFLOAT3	m_LightColor    = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

		// Light Attenuation.
		float	m_LightAttenuationA = 1.0f;
		float	m_LightAttenuationB = 0.1f;
		float	m_LightAttenuationC = 0.1f;
	};
}

#endif // !_LIGHT_H_

