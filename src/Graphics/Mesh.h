////////////////////////////////////////////////////////////////////////////////
// Filename: Mesh.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _MESH_H_
#define _MESH_H_

//////////////
// INCLUDES //
//////////////

#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "Vertex.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "Texture.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: Mesh
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class Mesh
	{
	public:
		Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<Vertex3D>& vertices, std::vector<DWORD>& indices, std::vector<Texture>& textures, const DirectX::XMMATRIX& transformMatrix);
		Mesh(const Mesh& cpy);

		void Draw();
		const DirectX::XMMATRIX& GetTransformMatrix();

	public:
		DirectX::XMMATRIX		m_TransformMatrix;
		VertexBuffer<Vertex3D>	m_VertexBuffer;
		IndexBuffer				m_IndexBuffer;
		ID3D11DeviceContext*	m_pDeviceContext;
		std::vector<Texture>	m_Textures;
	};
}

#endif // !_MESH_H_
