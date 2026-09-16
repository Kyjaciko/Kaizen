////////////////////////////////////////////////////////////////////////////////
// Filename: Camera2D.cpp
////////////////////////////////////////////////////////////////////////////////

#include "Camera2D.h"

namespace DirectX11
{
	Camera2D::Camera2D()
	{
		m_Position		 = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_PositionVector = DirectX::XMVectorZero();
		m_Rotation		 = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_RotationVector = DirectX::XMVectorZero();
		UpdateMatrix();
	}

	void Camera2D::SetProjectionValues(float width, float height, float nearZ, float farZ)
	{
		// Set (0, 0) at the topleft of the screen.
		m_OrthoMatrix = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, width, height, 0.0f, nearZ, farZ);
	}

	const DirectX::XMMATRIX& Camera2D::GetOrthoMatrix() const
	{
		return m_OrthoMatrix;
	}

	const DirectX::XMMATRIX& Camera2D::GetWorldMatrix() const
	{
		return m_WorldMatrix;
	}

	void Camera2D::UpdateMatrix()
	{
		DirectX::XMMATRIX translation_offset_matrix = DirectX::XMMatrixTranslation(-m_Position.x, -m_Position.y, 0.0f);
		DirectX::XMMATRIX camera_rotation_matrix	= DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
		m_WorldMatrix = camera_rotation_matrix * translation_offset_matrix;
	}
}