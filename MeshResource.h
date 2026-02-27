////////////////////////////////////////////////////////////////////////////////
// Filename: MeshResource.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _MESH_RESOURCE_H_
#define _MESH_RESOURCE_H_

//////////////
// INCLUDES //
//////////////

#include <vector>
/*#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>*/

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "graphics/Vertex.h"
#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/ConstantBuffer.h"
#include "graphics/Texture.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: MeshResource
////////////////////////////////////////////////////////////////////////////////

namespace Kaizen::Graphics
{
	class MeshResource
	{
	public:
		//MeshResource(ID3D11Device* device, std::vector<VertexType>& vertices, std::vector<DWORD>& indices);
		template<typename VertexType>
		MeshResource(std::shared_ptr<DirectX11::VertexBuffer<VertexType>> vertexBuffer, std::shared_ptr<DirectX11::IndexBuffer> indexBuffer)
		{
			m_Format = VertexType::format;
			m_pVertexBuffer = vertexBuffer;
			m_pIndexBuffer = indexBuffer;
		}

		~MeshResource() = default;

		// Mesh is unique on GPU side.
		//MeshResource(const MeshResource&) = delete;
		//MeshResource& operator=(const MeshResource&) = delete;

		// Allow moving the mesh.
		MeshResource(MeshResource&&) = default;
		MeshResource& operator=(MeshResource&&) = default;

		void Bind(ID3D11DeviceContext* deviceContext, UINT startSlot = 0, UINT numBuffersconst = 1, const UINT offsets = 0);

		//const DirectX11::VertexBuffer<VertexType>& GetVertexBuffer() const { return m_VertexBuffer; }
		//const DirectX11::IndexBuffer& GetIndexBuffer() const { return m_IndexBuffer; }
		UINT GetIndexCount() const { return m_pIndexBuffer ? m_pIndexBuffer->IndexCount() : 0; }
		const DirectX11::VertexFormat GetFormat() const { return m_Format; }

	public:
		DirectX11::VertexFormat					  m_Format = DirectX11::VertexFormat::Unknown;
		std::shared_ptr<DirectX11::IVertexBuffer> m_pVertexBuffer;
		std::shared_ptr<DirectX11::IndexBuffer>   m_pIndexBuffer;

	private:
		// Mesh is unique on GPU side.
		MeshResource(const MeshResource&) = default;
		MeshResource& operator=(const MeshResource&) = default;
	};
}

#endif // !_MESH_RESOURCE_H_
