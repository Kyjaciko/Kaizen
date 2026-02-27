////////////////////////////////////////////////////////////////////////////////
// Filename: Vertex.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _VERTEX_H_
#define _VERTEX_H_

//////////////
// INCLUDES //
//////////////

#include <d3d11.h>
#include <DirectXMath.h>

#include "Color.h"

#include <vector> // Added to fix undefined 'std::vector' and initializer-list assignment errors.

////////////////////////////////////////////////////////////////////////////////
// Structure name: Vertex2D
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	struct Vertex2D
	{
		DirectX::XMFLOAT3	position;
		DirectX::XMFLOAT2	textCoord	= DirectX::XMFLOAT2(0.0f, 0.0f); // Do this to make sure the texture coordinates are always set to zero, so the if statement in the pixelshader works correctly.
		DirectX::XMFLOAT4	color		= DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

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
		DirectX::XMFLOAT3	position;
		DirectX::XMFLOAT4	color = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

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
		DirectX::XMFLOAT3	position;
		DirectX::XMFLOAT2	textCoord;
		DirectX::XMFLOAT3	normal;

		Vertex3D() {}
		Vertex3D(float x, float y, float z, float u, float v, float normal_x, float normal_y, float normal_z)
			: position(x, y, z)
			, textCoord(u, v)
			, normal(normal_x, normal_y, normal_z) 
		{}
	};
}

////////////////////////////////////////////////////////////////////////////////
// Standard Vertex Formats
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	enum class VertexFormat 
	{ 
		Unknown,
		PositionUV, 
		PositionNormalUV 
	};

	// Minimal data: Sprites, UI, Particles, Skyboxes...
	struct VertexPosUV 
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texCoord;

		// Vertex info
		static const VertexFormat format = VertexFormat::PositionUV;
		inline static D3D11_INPUT_ELEMENT_DESC layoutDescription[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		inline static UINT numElements = ARRAYSIZE(layoutDescription);

		VertexPosUV(const float x, const float y, const float z, const float u, const float v)
			: position(x, y, z)
			, texCoord(u, v)
		{
		}

		VertexPosUV(const DirectX::XMFLOAT3 xyz, const DirectX::XMFLOAT2 uv)
			: position(xyz)
			, texCoord(uv)
		{
		}
	};

	// For models.
	struct VertexStandard 
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texCoord;
		// Eventueel Tangent/Binormal voor Normal mapping

		//static const D3D11_INPUT_ELEMENT_DESC layout[];
		VertexStandard(const float x, const float y, const float z, const float u, const float v, const float normal_x, const float normal_y, const float normal_z)
			: position(x, y, z)
			, texCoord(u, v)
			, normal(normal_x, normal_y, normal_z)
		{
		}

		VertexStandard(const DirectX::XMFLOAT3 xyz, const DirectX::XMFLOAT2 uv, const DirectX::XMFLOAT3 normal)
			: position(xyz)
			, texCoord(uv)
			, normal(normal)
		{
		}
	};

	// 4. Template Traits (De Koppeling: Struct -> Enum)
	// Dit zorgt ervoor dat de compiler weet: "Als ik VertexPosUV zie, bedoel ik format PositionUV"
	//template<typename T> struct VertexTraits { static const VertexFormat Format = VertexFormat::Unknown; };

	// Specialisaties
	//template<> struct VertexTraits<VertexPosUV> { static const VertexFormat Format = VertexFormat::PositionUV; };
	//template<> struct VertexTraits<VertexStandard> { static const VertexFormat Format = VertexFormat::PositionNormalUV; };
}

#endif // !_VERTEX_H_


