////////////////////////////////////////////////////////////////////////////////
// Filename: Camera2D.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _CAMERA2D_H_
#define _CAMERA2D_H_

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "GameObject2D.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: Camera2D
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class Camera2D : public GameObject2D
	{
	public:
		Camera2D();

		void SetProjectionValues(float width, float height, float nearZ, float farZ);

		const DirectX::XMMATRIX& GetOrthoMatrix() const;
		const DirectX::XMMATRIX& GetWorldMatrix() const;

	private:
		DirectX::XMMATRIX m_OrthoMatrix;
		DirectX::XMMATRIX m_WorldMatrix;

	private:
		virtual void UpdateMatrix() override;
	};
}

#endif // !_CAMERA2D_H_