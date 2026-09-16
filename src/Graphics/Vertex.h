////////////////////////////////////////////////////////////////////////////////
// Filename: Vertex.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _VERTEX_H_
#define _VERTEX_H_

//////////////
// INCLUDES //
//////////////

#include <DirectXMath.h>

#include "Color.h"

////////////////////////////////////////////////////////////////////////////////
// Structure name: Vertex2D
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	struct Vertex2D
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 textCoord	= DirectX::XMFLOAT2(0.0f, 0.0f); // Do this to make sure the texture coordinates are always set to zero, so the if statement in the pixelshader works correctly.
		DirectX::XMFLOAT4 color		= DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

		Vertex2D() {}
		Vertex2D(float x, float y, float z, float u, float v)
			: position(x, y, z)
			, textCoord(u, v)
		{
		}

		Vertex2D(float x, float y, float z, float r, float g, float b, float a)
			: position(x, y, z)
			, color(r, g, b, a)
		{
		}
	};

	struct Vertex2D_StaticColor
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

		Vertex2D_StaticColor(float x, float y, float z, float r, float g, float b, float a)
			: position(x, y, z)
			, color(r, g, b, a)
		{
		}
	};
}

////////////////////////////////////////////////////////////////////////////////
// Structure name: Vertex3D
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	struct Vertex3D
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 textCoord;
		DirectX::XMFLOAT3 normal;

		Vertex3D() {}
		Vertex3D(float x, float y, float z, float u, float v, float normal_x, float normal_y, float normal_z)
			: position(x, y, z)
			, textCoord(u, v)
			, normal(normal_x, normal_y, normal_z) 
		{
		}
	};
}

#endif // !_VERTEX_H_


