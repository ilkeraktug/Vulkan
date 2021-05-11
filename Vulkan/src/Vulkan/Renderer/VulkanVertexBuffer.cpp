#include "pch.h"
#include "VulkanVertexBuffer.h"

#include "Vulkan/Core/init.h"

VulkanVertexBuffer::VulkanVertexBuffer(float* vertices, uint32_t size, const VertexBufferLayout& layout, VulkanCore* core)
	:m_Layout(layout), m_BufferSize(size), m_Core(core)
{
	
	createBuffer(vertices);
	CalculateAttributes();
}

VulkanVertexBuffer::VulkanVertexBuffer(float* vertices, uint32_t size, VulkanCore* core)
	:m_BufferSize(size), m_Core(core)
{
	createBuffer(vertices);
}

VulkanVertexBuffer::~VulkanVertexBuffer()
{
	vkDestroyBuffer(m_Core->GetDevice(), m_Buffer, nullptr);
	vkFreeMemory(m_Core->GetDevice(), m_BufferMemory, nullptr);
}

void VulkanVertexBuffer::createBuffer(float* vertices)
{
	VkBuffer stageBuffer;
	VkDeviceMemory stageBufferMemory;

	VkBufferCreateInfo stageBufferInfo = init::createBufferInfo(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

	VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &stageBufferInfo, nullptr, &stageBuffer));

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(m_Core->GetDevice(), stageBuffer, &memReqs);

	VkMemoryAllocateInfo memAllocInfo = init::memAllocInfo();
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReqs.memoryTypeBits, (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

	VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &memAllocInfo, nullptr, &stageBufferMemory));

	vkBindBufferMemory(m_Core->GetDevice(), stageBuffer, stageBufferMemory, 0);

	void* data;
	vkMapMemory(m_Core->GetDevice(), stageBufferMemory, 0, m_BufferSize, 0, &data);
	memcpy(data, vertices, m_BufferSize);
	vkUnmapMemory(m_Core->GetDevice(), stageBufferMemory);

	VkBufferCreateInfo bufferInfo = init::createBufferInfo(m_BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &bufferInfo, nullptr, &m_Buffer));

	vkGetBufferMemoryRequirements(m_Core->GetDevice(), m_Buffer, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &memAllocInfo, nullptr, &m_BufferMemory));

	vkBindBufferMemory(m_Core->GetDevice(), m_Buffer, m_BufferMemory, 0);

	VkCommandBuffer copyCmdBuffer;

	VkCommandBufferAllocateInfo cmdAllocInfo = init::commandBufferAllocateInfo();
	cmdAllocInfo.commandPool = m_Core->resources.commandPool;
	cmdAllocInfo.commandBufferCount = 1;
	cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VK_CHECK(vkAllocateCommandBuffers(m_Core->GetDevice(), &cmdAllocInfo, &copyCmdBuffer));

	VkCommandBufferBeginInfo cmdBufferBeginInfo = init::cmdBufferBeginInfo();
	cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(copyCmdBuffer, &cmdBufferBeginInfo));

	VkBufferCopy bufferRegion;
	bufferRegion.srcOffset = 0;
	bufferRegion.dstOffset = 0;
	bufferRegion.size = m_BufferSize;

	vkCmdCopyBuffer(copyCmdBuffer, stageBuffer, m_Buffer, 1, &bufferRegion);

	VK_CHECK(vkEndCommandBuffer(copyCmdBuffer));

	VkSubmitInfo submitInfo = init::submitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCmdBuffer;

	VK_CHECK(vkQueueSubmit(m_Core->queue.GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
	vkQueueWaitIdle(m_Core->queue.GraphicsQueue);

	vkDestroyBuffer(m_Core->GetDevice(), stageBuffer, nullptr);
	vkFreeMemory(m_Core->GetDevice(), stageBufferMemory, nullptr);
}

void VulkanVertexBuffer::CalculateAttributes()
{
	m_VertexBinding.binding = 0;
	m_VertexBinding.stride = m_Layout.GetStride();
	m_VertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	m_VertexAttributes.clear();
	m_VertexAttributes.resize(m_Layout.GetElements().size());

	for (size_t i = 0; i < m_VertexAttributes.size(); i++)
	{
		m_VertexAttributes[i].binding = 0;
		m_VertexAttributes[i].location = i;
		m_VertexAttributes[i].format = m_Layout.GetElements().at(i).VkFormat;
		m_VertexAttributes[i].offset = m_Layout.GetElements().at(i).Offset;
	}
}

