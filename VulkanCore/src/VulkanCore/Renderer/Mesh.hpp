#pragma once

#include <filesystem>

#include <assimp/Importer.hpp>   
#include <assimp/scene.h>        
#include <assimp/postprocess.h>  
#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

namespace VkApp
{

	struct MeshVertex
	{
	public:
		glm::vec3 Position = { };
		glm::vec2 TexCoord = { };

		static VkVertexInputBindingDescription GetBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(MeshVertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions()
		{
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};
			attributeDescriptions.resize((size_t)2);

			// Position
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(MeshVertex, Position);

			// TexCoord
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 2;
			attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(MeshVertex, TexCoord);

			return attributeDescriptions;
		}

	};

	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(const std::filesystem::path& path);
		void Destroy();

		#ifdef VKAPP_DEBUG
		std::filesystem::path& GetPath() { return m_Path; }
		#endif

		VkBuffer& GetVertexBuffer() { return m_VertexBuffer; }
		VkBuffer& GetIndexBuffer() { return m_IndexBuffer; }

		uint32_t GetAmountOfIndices() const { return m_Indices.size(); }

	private:
		void LoadModel(const std::filesystem::path& path, std::vector<MeshVertex>& vertices, std::vector<uint32_t>& indices);
		void ProcessNode(aiNode* node, const aiScene* scene, std::vector<MeshVertex>& vertices, std::vector<uint32_t>& indices);
		void ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<MeshVertex>& vertices, std::vector<uint32_t>& indices);

		void CreateVertexBuffer(const std::vector<MeshVertex>& vertices);
		void CreateIndexBuffer(const std::vector<uint32_t>& indices);

	private:
		#ifdef VKAPP_DEBUG
		std::filesystem::path m_Path; // For debugging purposes
		#endif

		std::vector<MeshVertex> m_Vertices = { };
		std::vector<uint32_t> m_Indices = { };

		VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;

		VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_IndexBufferMemory = VK_NULL_HANDLE;
	};

}