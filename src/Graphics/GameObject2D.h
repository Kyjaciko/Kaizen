////////////////////////////////////////////////////////////////////////////////
// Filename: GameObject2D.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _GAMEOBJECT2D_H_
#define _GAMEOBJECT2D_H_

//////////////
// INCLUDES //
//////////////

#include "GameObject.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: GameObject2D
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class GameObject2D : public GameObject
	{
	public:
		const DirectX::XMMATRIX& GetWorldMatrix() const { return m_WorldMatrix; }

	protected:
		DirectX::XMMATRIX m_WorldMatrix = DirectX::XMMatrixIdentity();

		virtual void UpdateMatrix()
		{
			//assert("UpdateMatrix must be overridden!" && 0);
			m_WorldMatrix = DirectX::XMMatrixScaling(m_Scale.x, m_Scale.y, 1.0f) * DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z) * DirectX::XMMatrixTranslation(m_Position.x + m_Scale.x / 2.0f, m_Position.y + m_Scale.y / 2.0f, m_Position.z);
		}
	};
}

#endif // !_GAMEOBJECT2D_H_

