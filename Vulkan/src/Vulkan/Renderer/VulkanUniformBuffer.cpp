#include "pch.h"
#include "VulkanUniformBuffer.h"

#include "Vulkan/Core/init.h"

VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size, VulkanCore* core)
	:m_Size(size), m_Core(core)
{
	createBuffer();
	setBufferInfo();
}

VulkanUniformBuffer::~VulkanUniformBuffer()
{
	vkDestroyBuffer(m_Core->GetDevice(), m_Buffer, nullptr);
	vkFreeMemory(m_Core->GetDevice(), m_DeviceMemory, nullptr);
}

void VulkanUniformBuffer::copyData(void* data, uint32_t size)
{
	VK_ASSERT(m_Size >= size, "Access to not initialized memory block!");

	void* mem;
	vkMapMemory(m_Core->GetDevice(), m_DeviceMemory, 0, m_Size, 0, &mem);
	memcpy(mem, data, size);
	vkUnmapMemory(m_Core->GetDevice(), m_DeviceMemory);
}

void VulkanUniformBuffer::createBuffer()
{
	VkBufferCreateInfo bufferCI = init::createBufferInfo(m_Size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &bufferCI, nullptr, &m_Buffer));

	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(m_Core->GetDevice(), m_Buffer, &memReq);

	VkMemoryAllocateInfo bufferMemoryAI = init::memAllocInfo();
	bufferMemoryAI.allocationSize = memReq.size;
	bufferMemoryAI.memoryTypeIndex = m_Core->getMemoryType(memReq.memoryTypeBits, (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

	vkAllocateMemory(m_Core->GetDevice(), &bufferMemoryAI, nullptr, &m_DeviceMemory);
	vkBindBufferMemory(m_Core->GetDevice(), m_Buffer, m_DeviceMemory, 0);
}

void VulkanUniformBuffer::setBufferInfo()
{
	m_BufferInfo.buffer = m_Buffer;
	m_BufferInfo.offset = 0;
	m_BufferInfo.range = m_Size;
}
