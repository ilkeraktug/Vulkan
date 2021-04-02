#pragma once
#include <vulkan\vulkan_core.h>
#include "Buffer.h"

#include "glm\glm.hpp"

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};


class VertexBuffer
{
public:
	VertexBuffer() = default;
	VertexBuffer(float* vertices, uint32_t size, const BufferLayout& layout);
	VertexBuffer(const std::vector<Vertex>& vertices, uint32_t size, const BufferLayout& layout);
	~VertexBuffer();

	inline const BufferLayout& GetLayout() const { return m_Layout; }
	void SetLayout(const BufferLayout& layout) { m_Layout = layout; }

	inline const VkVertexInputBindingDescription& GetVertexBinding() const { return m_VertexBinding; }
	inline const std::vector<VkVertexInputAttributeDescription>& GetVertexAttribute() const { return m_VertexAttribute; }
	inline const VkBuffer& GetBuffer() const { return m_Buffer; }
	inline const uint32_t GetVertexCount() const { return m_Size / m_Layout.GetStride(); }
private:
	void createVertexBindingDescription();
	void createVertexAttributeDescription();
	void createBuffer(float* vertices, uint32_t size);

	uint32_t getMemoryType(uint32_t type, VkMemoryPropertyFlags propertyFlags);
private:
	uint32_t m_Size;
	BufferLayout m_Layout;
	VkBuffer m_Buffer;
	VkDeviceMemory m_BufferMemory;

	VkVertexInputBindingDescription m_VertexBinding{};
	std::vector<VkVertexInputAttributeDescription> m_VertexAttribute;
};