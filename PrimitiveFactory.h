////////////////////////////////////////////////////////////////////////////////
// Filename: PrimitiveFactory.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _PRIMITIVE_FACTORY_H_
#define _PRIMITIVE_FACTORY_H_

//////////////
// INCLUDES //
//////////////

#include <memory>
#include <unordered_map>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "MeshResource.h"
#include "GeometryGenerator.h"

enum class PrimitiveType 
{
    Quad,
    Cube,
    Sphere
};

////////////////////////////////////////////////////////////////////////////////
// Class name: PrimitiveFactory
////////////////////////////////////////////////////////////////////////////////

namespace Kaizen::Resources
{
    class PrimitiveFactory
    {
    public:
        PrimitiveFactory(Microsoft::WRL::ComPtr<ID3D11Device> device) : m_pDevice(device) {}

        std::shared_ptr<Graphics::MeshResource> GetPrimitive(PrimitiveType type)
        {
            // Check for doubles.
            if (m_PrimitiveCache.find(type) != m_PrimitiveCache.end())
                return m_PrimitiveCache[type];

            std::shared_ptr<Graphics::MeshResource> mesh;
            switch (type)
            {
            case PrimitiveType::Quad:
                mesh = CreateQuadMesh();
                break;
            }

            m_PrimitiveCache[type] = mesh;
            return mesh;
        }

    private:
        std::unordered_map<PrimitiveType, std::shared_ptr<Graphics::MeshResource>> m_PrimitiveCache;

        Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;

    private:
        std::shared_ptr<Graphics::MeshResource> CreateQuadMesh()
        {
            auto data = DirectX11::GeometryGenerator::CreateQuad();

            std::shared_ptr<DirectX11::VertexBuffer<DirectX11::VertexPosUV>> vertex_buffer = std::make_shared<DirectX11::VertexBuffer<DirectX11::VertexPosUV>>();
            vertex_buffer->Init(m_pDevice.Get(), data.Vertices.data(), data.Vertices.size());

            std::shared_ptr<DirectX11::IndexBuffer> index_buffer = std::make_shared<DirectX11::IndexBuffer>();
            index_buffer->Init(m_pDevice.Get(), data.Indices.data(), data.Indices.size());

            std::shared_ptr<Graphics::MeshResource> mesh = std::make_shared<Graphics::MeshResource>(vertex_buffer, index_buffer);
            return mesh;
        }
    };
}

#endif // !_PRIMITIVE_FACTORY_H_