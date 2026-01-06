////////////////////////////////////////////////////////////////////////////////
// Filename: Camera3D.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _CAMERA3D_H_
#define _CAMERA3D_H_

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "GameObject3D.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: Camera3D
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class Camera3D : public GameObject3D
	{
	public:
		Camera3D();

		void SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ);

		const DirectX::XMMATRIX& GetViewMatrix() const;
		const DirectX::XMMATRIX& GetProjectionMatrix() const;

	private:
		DirectX::XMMATRIX	m_ViewMatrix;
		DirectX::XMMATRIX	m_ProjectionMatrix;

	private:
		virtual void UpdateMatrix() override;
	};
}

#endif // !_CAMERA3D_H_