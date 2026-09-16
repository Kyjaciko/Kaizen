////////////////////////////////////////////////////////////////////////////////
// Filename: Model.cpp
////////////////////////////////////////////////////////////////////////////////

#include "Model.h"

namespace DirectX11
{
	bool Model::Init(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_VertexBuffer)
	{
		m_pDevice			  = device;
		m_pDeviceContext	  = deviceContext;
		m_pCB_VS_VertexShader = &cb_vs_VertexBuffer;

		if (!LoadModel(filePath))
			return false;

		return true;
	}

	void Model::Draw(const DirectX::XMMATRIX& worldMatrix, const DirectX::XMMATRIX& viewProjectionMatrix)
	{
		
		m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pCB_VS_VertexShader->GetAddressOf());
		
		for (int i = 0; i < m_Meshes.size(); i++)
		{
			m_pCB_VS_VertexShader->data.wvpMatrix = m_Meshes[i].GetTransformMatrix() * worldMatrix * viewProjectionMatrix;
			m_pCB_VS_VertexShader->data.worldMatrix = m_Meshes[i].GetTransformMatrix() * worldMatrix;
			//m_pCB_VS_VertexShader->data.wvpMatrix = DirectX::XMMatrixTranspose(m_pCB_VS_VertexShader->data.mat);			// Convert from row major -> column major (no longer needed, row_major specified in shader).
			m_pCB_VS_VertexShader->ApplyChanges();

			m_Meshes[i].Draw();
		}
	}

	bool Model::LoadModel(const std::string& filePath)
	{
		m_Directory = StringHelper::GetDirectoryFromPath(filePath);
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(
			filePath,
			aiProcess_Triangulate | aiProcess_ConvertToLeftHanded
		);

		if (!scene) return false;

		ProcessNode(scene->mRootNode, scene, DirectX::XMMatrixIdentity());
		return true;
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene, const DirectX::XMMATRIX& parentTransformMatrix)
	{
		DirectX::XMMATRIX node_transform_matrix = DirectX::XMMatrixTranspose(DirectX::XMMATRIX(&node->mTransformation.a1)) * parentTransformMatrix;

		for (UINT i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_Meshes.push_back(ProcessMesh(mesh, scene, node_transform_matrix));
		}

		for (UINT i = 0; i < node->mNumChildren; i++)
			ProcessNode(node->mChildren[i], scene, node_transform_matrix);
	}

	Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, const DirectX::XMMATRIX& transformMatrix)
	{
		// Data to fill.
		std::vector<Vertex3D> vertices;
		std::vector<DWORD>  indices;

		// Get vertices.
		for (UINT i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex3D vertex;
			vertex.position.x = mesh->mVertices[i].x;
			vertex.position.y = mesh->mVertices[i].y;
			vertex.position.z = mesh->mVertices[i].z;

			vertex.normal.x = mesh->mNormals[i].x;
			vertex.normal.y = mesh->mNormals[i].y;
			vertex.normal.z = mesh->mNormals[i].z;

			// Main texture of the object (should) always be the first index.
			if (mesh->mTextureCoords[0])
			{
				vertex.textCoord.x = static_cast<float>(mesh->mTextureCoords[0][i].x);
				vertex.textCoord.y = static_cast<float>(mesh->mTextureCoords[0][i].y);
			}

			vertices.push_back(vertex);
		}

		// Get indices.
		for (UINT i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];

			for (UINT j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		std::vector<Texture> textures;
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuse_textures = LoadMaterialTextures(material, aiTextureType_DIFFUSE, scene);
		textures.insert(textures.end(), diffuse_textures.begin(), diffuse_textures.end());

		return Mesh(m_pDevice, m_pDeviceContext, vertices, indices, textures, transformMatrix);
	}

	int Model::GetTextureIndex(aiString* string)
	{
		// The string should alwys be something like: *0, *4, etc.
		assert(string->length >= 2);
		return std::atoi(&string->C_Str()[1]);
	}

	TextureStorageType Model::DetermineTextureStorageType(const aiScene* scene, aiMaterial* material, unsigned int index, aiTextureType textureType)
	{
		if (!material->GetTextureCount(textureType))
			return NONE;

		aiString path;
		material->GetTexture(textureType, index, &path);
		std::string texturePath = path.C_Str();

		// Check if the texture is an embedded indexed texture by seeing if the file path starts with an index marker.
		if (texturePath[0] == '*')
		{
			if (scene->mTextures[0]->mHeight == 0)
				return EMBEDDED_INDEX_COMPRESSED;
			else
			{
				assert("SUPPORT DOES NOT EXIST YET FOR INDEXED NON COMPRESSED TEXTURES!" && 0);
				return EMBEDDED_INDEX_NON_COMPRESSED;
			}
		}

		// Check if the texture is an embedded texture referred to by name (not by index like "*0").
		if (auto texture = scene->GetEmbeddedTexture(texturePath.c_str()))
		{
			if (texture->mHeight == 0)
				return EMBEDDED_COMPRESSED;
			else
			{
				assert("SUPPORT DOES NOT EXIST YET FOR EMBEDDED NON COMPRESSED TEXTURES!" && 0);
				return EMBEDDED_NON_COMPRESSED;
			}
		}

		// Check if the texture is a filepath by checking for the period before the extension name.
		if (texturePath.find('.') != std::string::npos)
			return DISK;

		return NONE; // No texture exists.
	}

	std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* material, aiTextureType textureType, const aiScene* scene)
	{
		std::vector<Texture> material_textures;
		TextureStorageType store_type = INVALID;
		unsigned int texture_count = material->GetTextureCount(textureType);

		if (!texture_count)
		{
			store_type = NONE;
			aiColor3D aiColor(0.0f, 0.0f, 0.0f);
			switch (textureType)
			{
			case aiTextureType_DIFFUSE:
				material->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);

				// Color black? -> Use grey.
				if (aiColor.IsBlack()) 
				{
					material_textures.emplace_back(m_pDevice, Colors::UnloadedTextureColor, textureType);
					return material_textures;
				}
				material_textures.emplace_back(m_pDevice, Color(aiColor.r * 255, aiColor.g * 255, aiColor.b * 255), textureType);
				return material_textures;
			}
		}
		else
		{
			for (UINT i = 0; i < texture_count; i++)
			{
				aiString path;
				material->GetTexture(textureType, i, &path);
				TextureStorageType store_type = DetermineTextureStorageType(scene, material, i, textureType);
				switch (store_type)
				{
				case EMBEDDED_INDEX_COMPRESSED:
				{
					int index = GetTextureIndex(&path);
					material_textures.emplace_back(
						m_pDevice,
						reinterpret_cast<uint8_t*>(scene->mTextures[index]->pcData),
						scene->mTextures[index]->mWidth,
						textureType
					);
					break;
				}
				case EMBEDDED_COMPRESSED:
				{
					const aiTexture* texture = scene->GetEmbeddedTexture(path.C_Str());
					material_textures.emplace_back(
						m_pDevice,
						reinterpret_cast<uint8_t*>(texture->pcData),
						texture->mWidth,
						textureType
					);
					break;
				}
				case DISK:
				{
					std::string fileName = m_Directory + '\\' + path.C_Str();
					material_textures.emplace_back(m_pDevice, fileName, textureType);
					break;
				}
				}
			}
		}

		if (!material_textures.size())
			material_textures.emplace_back(m_pDevice, Colors::UnhandledTextureColor, textureType);

		return material_textures;
	}
}