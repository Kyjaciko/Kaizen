////////////////////////////////////////////////////////////////////////////////
// Filename: Model.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _MODEL_H_
#define _MODEL_H_

///////////////////////
// MY CLASS INCLUDES //
///////////////////////

#include "Mesh.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: Model
////////////////////////////////////////////////////////////////////////////////

namespace DirectX11
{
	class Model
	{
	public:
		bool Init(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_VertexBuffer);

		void Draw(const DirectX::XMMATRIX& worldMatrix, const DirectX::XMMATRIX& viewProjectionMatrix);

	private:
		std::string	m_Directory;

		// Model data.
		std::vector<Mesh> m_Meshes;

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		ConstantBuffer<CB_VS_vertexshader>* m_pCB_VS_VertexShader;

	private:
		bool LoadModel(const std::string& filePath);
		void ProcessNode(aiNode* node, const aiScene* scene, const DirectX::XMMATRIX& parentTransformMatrix);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const DirectX::XMMATRIX& transformMatrix);

		int GetTextureIndex(aiString* string);
		TextureStorageType DetermineTextureStorageType(const aiScene* scene, aiMaterial* material, unsigned int index, aiTextureType textureType);
		std::vector<Texture> LoadMaterialTextures(aiMaterial* material, aiTextureType textureType, const aiScene* scene);
	};
}

#endif // !_MODEL_H_
