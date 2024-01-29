#include "vcpch.h"
#include "Mesh.hpp"

#include "VulkanCore/Core/Logging.hpp"

#include "VulkanCore/Renderer/InstanceManager.hpp"
#include "VulkanCore/Utils/BufferManager.hpp"

namespace VkApp
{

	Mesh::Mesh(const std::filesystem::path& path)
	{
		#ifdef VKAPP_DEBUG
		m_Path = path;
		#endif

		LoadModel(path, m_Vertices, m_Indices);
        CreateVertexBuffer(m_Vertices);
        CreateIndexBuffer(m_Indices);
	}

    void Mesh::Destroy()
    {
        vkDestroyBuffer(InstanceManager::Get()->GetLogicalDevice(), m_VertexBuffer, nullptr);
        vkFreeMemory(InstanceManager::Get()->GetLogicalDevice(), m_VertexBufferMemory, nullptr);

        vkDestroyBuffer(InstanceManager::Get()->GetLogicalDevice(), m_IndexBuffer, nullptr);
        vkFreeMemory(InstanceManager::Get()->GetLogicalDevice(), m_IndexBufferMemory, nullptr);
    }

    void Mesh::LoadModel(const std::filesystem::path& path, std::vector<MeshVertex>& vertices, std::vector<uint32_t>& indices) 
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
        {
            VKAPP_LOG_ERROR("Failed to load mesh from: \"{}\"", path.string());
            return;
        }

        ProcessNode(scene->mRootNode, scene, vertices, indices);
    }

    void Mesh::ProcessNode(aiNode* node, const aiScene* scene, std::vector<MeshVertex>& vertices, std::vector<uint32_t>& indices) 
    {
        // Process all the node's meshes
        for (unsigned int i = 0; i < node->mNumMeshes; i++) 
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            ProcessMesh(mesh, scene, vertices, indices);
        }

        // Then do the same for each of its children
        for (unsigned int i = 0; i < node->mNumChildren; i++)
            ProcessNode(node->mChildren[i], scene, vertices, indices);
    }

    void Mesh::ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<MeshVertex>& vertices, std::vector<uint32_t>& indices) 
    {
        // Vertex processing
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) 
        {
            MeshVertex vertex;
            glm::vec3 vector(0.0f);

            // Position
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            // Texture coordinates
            if (mesh->mTextureCoords[0]) 
            {
                glm::vec2 vec(0.0f);
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoord = vec; 
            }
            else
                vertex.TexCoord = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        // Index processing
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) 
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
    }
    
	void Mesh::CreateVertexBuffer(const std::vector<MeshVertex>& vertices)
	{
        //for (auto& vertice : vertices)
        //{
        //    VKAPP_LOG_TRACE("X: {0}, Y: {1}, Z: {2}", vertice.Position.x, vertice.Position.y, vertice.Position.z);
        //}

        BufferManager::CreateVertexBuffer(m_VertexBuffer, m_VertexBufferMemory, (void*)vertices.data(), sizeof(vertices[0]) * vertices.size());
	}

	void Mesh::CreateIndexBuffer(const std::vector<uint32_t>& indices)
	{
        BufferManager::CreateIndexBuffer(m_IndexBuffer, m_IndexBufferMemory, (void*)indices.data(), sizeof(indices[0]) * indices.size());
	}

}