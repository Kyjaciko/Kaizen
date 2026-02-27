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

// Must be 16 byte aligned.
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



	// For everything (usual slot: b0)
	struct CB_Frame 
	{
		DirectX::XMMATRIX viewProjectionMatrix;
	};

	// For all changing objects (usual slot: b1)
	struct CB_Object 
	{
		DirectX::XMMATRIX worldMatrix;
	};
}

////////////////////////////////////////////////////////////////////////////////
// Specific data for components (usual slot: b2)
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	struct CB_SpriteData
	{
		DirectX::XMFLOAT4 uvTransform; // x=u, y=v, z=width, w=height
	};
}

#endif // !_CONSTANTBUFFERTYPES_H_
