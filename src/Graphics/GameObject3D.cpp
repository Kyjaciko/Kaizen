////////////////////////////////////////////////////////////////////////////////
// Filename: GameObject3D.cpp
////////////////////////////////////////////////////////////////////////////////

#include "GameObject3D.h"

namespace DirectX11
{
	void GameObject3D::SetLookAtPosition(DirectX::XMFLOAT3 lookAtPosition)
	{
		// Verify that the lookAtPostion is not the same as the camera position. 
		// They cannot be the same as that wouldn't make sense and would result in undefined behavior.
		if (lookAtPosition.x == m_Position.x && lookAtPosition.y == m_Position.y && lookAtPosition.z == m_Position.z)
			return;

		lookAtPosition.x = m_Position.x - lookAtPosition.x;
		lookAtPosition.y = m_Position.y - lookAtPosition.y;
		lookAtPosition.z = m_Position.z - lookAtPosition.z;

		float pitch = 0.0f;
		if (lookAtPosition.y != 0.0f)
		{
			const float distance = sqrt(lookAtPosition.x * lookAtPosition.x + lookAtPosition.z * lookAtPosition.z);
			pitch = atan(lookAtPosition.y / distance);
		}

		float yaw = 0.0f;
		if (lookAtPosition.x != 0.0f)
			yaw = atan(lookAtPosition.x / lookAtPosition.z);
		if (lookAtPosition.z > 0)
			yaw += DirectX::XM_PI;

		SetRotation(pitch, yaw, 0.0f);
	}

	const DirectX::XMVECTOR& GameObject3D::GetForwardVector(bool omitY) const
	{
		return omitY ? m_ForwardVector_NO_Y : m_ForwardVector;
	}

	const DirectX::XMVECTOR& GameObject3D::GetLeftVector(bool omitY) const
	{
		return omitY ? m_LeftVector_NO_Y : m_LeftVector;
	}

	const DirectX::XMVECTOR& GameObject3D::GetRightVector(bool omitY) const
	{
		return omitY ? m_RightVector_NO_Y : m_RightVector;
	}

	const DirectX::XMVECTOR& GameObject3D::GetBackwardVector(bool omitY) const
	{
		return omitY ? m_BackwardVector_NO_Y : m_BackwardVector;
	}

	void GameObject3D::UpdateDirectionVectors()
	{
		DirectX::XMMATRIX vector_rotation_matrix = DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, 0.0f);
		m_ForwardVector		= DirectX::XMVector3TransformCoord(DEFAULT_FORWARD_VECTOR, vector_rotation_matrix);
		m_BackwardVector	= DirectX::XMVector3TransformCoord(DEFAULT_BACKWARD_VECTOR, vector_rotation_matrix);
		m_LeftVector		= DirectX::XMVector3TransformCoord(DEFAULT_LEFT_VECTOR, vector_rotation_matrix);
		m_RightVector		= DirectX::XMVector3TransformCoord(DEFAULT_RIGHT_VECTOR, vector_rotation_matrix);

		DirectX::XMMATRIX vector_rotation_matrix_NO_Y = DirectX::XMMatrixRotationRollPitchYaw(0.0f, m_Rotation.y, 0.0f);
		m_ForwardVector_NO_Y	= DirectX::XMVector3TransformCoord(DEFAULT_FORWARD_VECTOR, vector_rotation_matrix_NO_Y);
		m_BackwardVector_NO_Y	= DirectX::XMVector3TransformCoord(DEFAULT_BACKWARD_VECTOR, vector_rotation_matrix_NO_Y);
		m_LeftVector_NO_Y		= DirectX::XMVector3TransformCoord(DEFAULT_LEFT_VECTOR, vector_rotation_matrix_NO_Y);
		m_RightVector_NO_Y		= DirectX::XMVector3TransformCoord(DEFAULT_RIGHT_VECTOR, vector_rotation_matrix_NO_Y);
	}
}