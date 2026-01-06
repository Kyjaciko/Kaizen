////////////////////////////////////////////////////////////////////////////////
// Filename: Camera3D.cpp
////////////////////////////////////////////////////////////////////////////////

#include "Camera3D.h"

namespace DirectX11 
{
	Camera3D::Camera3D()
	{
		m_Position		 = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_PositionVector = DirectX::XMVectorZero();
		m_Rotation		 = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_RotationVector = DirectX::XMVectorZero();
		UpdateMatrix();
	}

	void Camera3D::SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ)
	{
		float fovRadians = DirectX::XMConvertToRadians(fovDegrees);
		m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ);
	}

	const DirectX::XMMATRIX& Camera3D::GetViewMatrix() const
	{
		return m_ViewMatrix;
	}

	const DirectX::XMMATRIX& Camera3D::GetProjectionMatrix() const
	{
		return m_ProjectionMatrix;
	}

	// Updates the view matrix and the movement vectors.
	void Camera3D::UpdateMatrix()
	{
		using namespace DirectX;

		// Calculate Camera3D rotation matrix.
		XMMATRIX rotation_matrix = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);

		// Calculate unit vector of Camera3D target based of Camera3D forward value transformend by Camera3D rotation matrix.
		XMVECTOR target = XMVector3TransformCoord(DEFAULT_FORWARD_VECTOR, rotation_matrix);

		// Adjust Camera3D target to be offset by the Camera3D's current position.
		target += m_PositionVector;

		// Calculate up direction based on current rotation.
		XMVECTOR up = XMVector3TransformCoord(DEFAULT_UP_VECTOR, rotation_matrix);

		// Rebuild view matrix.
		m_ViewMatrix = XMMatrixLookAtLH(m_PositionVector, target, up);

		// Calculate forward, left, right and backward vectors.
		UpdateDirectionVectors();
	}
}