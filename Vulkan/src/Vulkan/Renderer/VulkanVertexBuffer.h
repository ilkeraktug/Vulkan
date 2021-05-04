#pragma once
#include "VulkanCore.h"

#include "Abstract/Buffer.h"
#include "VertexBufferLayout.h"


class VulkanVertexBuffer : public Buffer
{
public:
	VulkanVertexBuffer() {};
	VulkanVertexBuffer(float* vertices, uint32_t size, const VertexBufferLayout& layout, VulkanCore* core);
	VulkanVertexBuffer(float* vertices, uint32_t size, VulkanCore* core);

	virtual ~VulkanVertexBuffer();

	void SetLayout(const VertexBufferLayout& layout) { m_Layout = layout; CalculateAttributes(); }

	const VkBuffer& GetBuffer() const { return m_Buffer; }
	const VkVertexInputBindingDescription GetVertexInput() const { return m_VertexBinding; };
	const std::vector<VkVertexInputAttributeDescription>& GetVertexAttributes() const { return m_VertexAttributes; };
private:

	void createBuffer(float* vertices);
	void CalculateAttributes();
private:
	VulkanCore* m_Core = nullptr;

	VertexBufferLayout m_Layout;

	VkBuffer m_Buffer;
	VkDeviceMemory m_BufferMemory;
	VkDeviceSize m_BufferSize;

	VkVertexInputBindingDescription m_VertexBinding{};
	std::vector<VkVertexInputAttributeDescription> m_VertexAttributes;
};