////////////////////////////////////////////////////////////////////////////////
// Filename: GeometryGenerator.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _GEOMETRY_GENERATOR_H_
#define _GEOMETRY_GENERATOR_H_

//////////////
// INCLUDES //
//////////////

#include <vector>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "Graphics/Vertex.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: GeometryGenerator
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
    class GeometryGenerator
    {
    public:
        // Een simpele container voor de ruwe data
        template<typename VertexType>
        struct MeshData {
            std::vector<VertexType> Vertices;
            std::vector<DWORD> Indices;
        };

        static MeshData<VertexPosUV> CreateQuad()
        {
            MeshData<VertexPosUV> mesh;
            mesh.Vertices =
            {
                VertexPosUV({-.5f, -.5f, 0.f}, {0.f, 0.f}),	// Top left.
                VertexPosUV({.5f, -.5f, 0.f}, {1.f, 0.f}),	// Top right.
                VertexPosUV({-.5f, .5f, 0.f}, {0.f, 1.f}),	// Bottom left.
                VertexPosUV({.5f, .5f, 0.f}, {1.f, 1.f})	// Bottom right.
            };

            mesh.Indices =
            {
                0, 1, 2,
                2, 1, 3
            };

            return mesh;
        }

        // Voor later: De Kubus (met Normals voor 3D licht)
        /*
        static MeshData<VertexPosNormalUV> CreateCube() {
            // ... 24 vertices ...
        }
        */
    };
}

#endif // !_GEOMETRY_GENERATOR_H_