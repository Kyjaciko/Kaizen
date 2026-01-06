////////////////////////////////////////////////////////////////////////////////
// Filename: ConstantBufferTypes.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _CONSTANTBUFFERTYPES_H_
#define _CONSTANTBUFFERTYPES_H_

//////////////
// INCLUDES //
//////////////

#include <DirectXMath.h>

////////////////////////////////////////////////////////////////////////////////
// Structure: CB_VS_vertexshader
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	struct CB_VS_vertexshader
	{
		DirectX::XMMATRIX	wvpMatrix;
		DirectX::XMMATRIX	worldMatrix;
	};

	struct CB_VS_vertexshader_2d
	{
		DirectX::XMMATRIX	wvpMatrix;
	};

	struct CB_VS_vertexshader_grid
	{
		DirectX::XMMATRIX	wvpMatrix;
		DirectX::XMMATRIX	invViewMatrix;
		DirectX::XMMATRIX	invProjectionMatrix;
		float				cameraNear;
		float				cameraFar;
	};

	/*struct CB_PS_pixelshader
	{
		float alpha = 1.0f;
	};*/

	// Must be 16 byte aligned.
	struct CB_PS_light
	{
		DirectX::XMFLOAT3	ambientLightColor;
		float				ambientLightStrength;

		DirectX::XMFLOAT3	dynamicLightColor;
		float				dynamicLightStrength;
		DirectX::XMFLOAT3	dynamicLightPosition;
		float				dynamicLightAttenuationA;
		float				dynamicLightAttenuationB;
		float				dynamicLightAttenuationC;
	};
}

#endif // !_CONSTANTBUFFERTYPES_H_
