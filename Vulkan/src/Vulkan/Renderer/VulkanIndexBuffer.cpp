#include "pch.h"
#include "VulkanIndexBuffer.h"

#include "Vulkan/Core/init.h"

VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t count, VulkanCore* core)
	:m_Core(core)
{
	m_Size = sizeof(uint32_t) * count;
	m_Count = count;
	m_IndexType = VK_INDEX_TYPE_UINT32;
	createBuffer<uint32_t>(indices);
}

VulkanIndexBuffer::VulkanIndexBuffer(uint16_t* indices, uint32_t count, VulkanCore* core)
	:m_Core(core)
{
	m_Size = sizeof(uint16_t) * count;
	m_Count = count;
	m_IndexType = VK_INDEX_TYPE_UINT16;
	createBuffer<uint16_t>(indices);
}

VulkanIndexBuffer::~VulkanIndexBuffer()
{
	vkDestroyBuffer(m_Core->GetDevice(), m_Buffer, nullptr);
	vkFreeMemory(m_Core->GetDevice(), m_Memory, nullptr);
}

