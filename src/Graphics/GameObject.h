////////////////////////////////////////////////////////////////////////////////
// Filename: GameObject.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _GAMEOBJECT_H_
#define _GAMEOBJECT_H_

//////////////
// INCLUDES //
//////////////

#include "Model.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: GameObject
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class GameObject
	{
	public:
		void SetPosition(const DirectX::XMVECTOR& position);
		void SetPosition(const DirectX::XMFLOAT3& position);
		void SetPosition(float x, float y, float z);
		void AdjustPosition(const DirectX::XMVECTOR& position);
		void AdjustPosition(const DirectX::XMFLOAT3& position);
		void AdjustPosition(float x, float y, float z);

		void SetRotation(const DirectX::XMVECTOR& rotation);
		void SetRotation(const DirectX::XMFLOAT3& rotation);
		void SetRotation(float x, float y, float z);
		void AdjustRotation(const DirectX::XMVECTOR& rotation);
		void AdjustRotation(const DirectX::XMFLOAT3& rotation);
		void AdjustRotation(float x, float y, float z);

		void SetScale(float xScale, float yScale, float zScale = 1.0f);

		const DirectX::XMVECTOR& GetPositionVector() const;
		const DirectX::XMFLOAT3& GetPositionFloat3() const;
		const DirectX::XMVECTOR& GetRotationVector() const;
		const DirectX::XMFLOAT3& GetRotationFloat3() const;

	protected:
		// Position.
		DirectX::XMFLOAT3	m_Position;
		DirectX::XMFLOAT3	m_Rotation;
		DirectX::XMVECTOR	m_PositionVector;
		DirectX::XMVECTOR	m_RotationVector;

		// Size.
		DirectX::XMFLOAT3	m_Scale;

	protected: 
		virtual void UpdateMatrix()
		{
			assert("UpdateMatrix must be overridden!" && 0);
		}
	};
}

#endif // !_GAMEOBJECT_H_
