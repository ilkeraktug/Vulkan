#include "pch.h"
#include "VertexBuffer.h"

#include "VulkanCore.h"

VertexBuffer::VertexBuffer(float* vertices, uint32_t size, const BufferLayout& layout)
	:m_Layout(layout), m_Size(size)
{
	createVertexBindingDescription();
	createVertexAttributeDescription();
	createBuffer(vertices, size);
}

VertexBuffer::~VertexBuffer()
{
	vkQueueWaitIdle(VulkanCore::GetGraphicsQueue());
	//vkDestroyBuffer(VulkanCore::GetDevice(), m_Buffer, nullptr);
	//vkFreeMemory(VulkanCore::GetDevice(), m_BufferMemory, nullptr);
}

void VertexBuffer::createVertexBindingDescription()
{
	m_VertexBinding.binding = 0;
	m_VertexBinding.stride = m_Layout.GetStride();
	m_VertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void VertexBuffer::createVertexAttributeDescription()
{
	m_VertexAttribute.resize(m_Layout.GetElements().size());

	for (size_t i = 0; i < m_VertexAttribute.size(); i++)
	{
		m_VertexAttribute.at(i).binding = 0;
		m_VertexAttribute.at(i).location = i;
		m_VertexAttribute.at(i).format = ShaderTypeToVkFormat(m_Layout.GetElements().at(i).Type);
		m_VertexAttribute.at(i).offset = m_Layout.GetElements().at(i).Offset;
	}
}

void VertexBuffer::createBuffer(float* vertices, uint32_t size)
{
	VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	createInfo.size = size;
	createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_ASSERT(vkCreateBuffer(VulkanCore::GetDevice(), &createInfo, nullptr, &m_Buffer) == VK_SUCCESS, "Cant vkCreateBuffer!");

	VkMemoryRequirements memoryRequirement;
	vkGetBufferMemoryRequirements(VulkanCore::GetDevice(), m_Buffer, &memoryRequirement);

	VkMemoryAllocateInfo memoryAllocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	memoryAllocateInfo.allocationSize = memoryRequirement.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirement.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	
	//getMemoryType(memoryRequirement.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VK_ASSERT(vkAllocateMemory(VulkanCore::GetDevice(), &memoryAllocateInfo, nullptr, &m_BufferMemory) == VK_SUCCESS, "Cant allocate memory!");

	vkBindBufferMemory(VulkanCore::GetDevice(), m_Buffer, m_BufferMemory, 0);

	void* data;
	vkMapMemory(VulkanCore::GetDevice(), m_BufferMemory, 0, size, 0, &data);
	memcpy(data, vertices, size);
	vkUnmapMemory(VulkanCore::GetDevice(), m_BufferMemory);

}

/*uint32_t VertexBuffer::getMemoryType(uint32_t type, VkMemoryPropertyFlags propertyFlags)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(VulkanCore::GetPhysicalDevice(), &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((type & (1 << i)) && ((memoryProperties.memoryTypes[i].propertyFlags & propertyFlags)))
			return type;
	}

	VK_ASSERT(false, "Cant find required memory flags!");
}*/

uint32_t VertexBuffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(VulkanCore::GetPhysicalDevice(), &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}