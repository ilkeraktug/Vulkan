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
	vkDestroyBuffer(VulkanCore::GetDevice(), m_Buffer, nullptr);
	vkFreeMemory(VulkanCore::GetDevice(), m_BufferMemory, nullptr);
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
	VkBuffer stageBuffer;
	VkDeviceMemory stageBufferMemory;

	VkBufferCreateInfo stageBufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	stageBufferCreateInfo.size = size;
	stageBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stageBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_ASSERT(vkCreateBuffer(VulkanCore::GetDevice(), &stageBufferCreateInfo, nullptr, &stageBuffer) == VK_SUCCESS, "Cant vkCreateBuffer!");

	VkMemoryRequirements stageBufferMemoryRequriment;
	vkGetBufferMemoryRequirements(VulkanCore::GetDevice(), stageBuffer, &stageBufferMemoryRequriment);

	VkMemoryAllocateInfo stageBufferMemoryAllocate{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	stageBufferMemoryAllocate.allocationSize = stageBufferMemoryRequriment.size;
	stageBufferMemoryAllocate.memoryTypeIndex = getMemoryType(stageBufferMemoryRequriment.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VK_ASSERT(vkAllocateMemory(VulkanCore::GetDevice(), &stageBufferMemoryAllocate, nullptr, &stageBufferMemory) == VK_SUCCESS, "Cant vkAllocateMemory!");

	vkBindBufferMemory(VulkanCore::GetDevice(), stageBuffer, stageBufferMemory, 0);

	void* data;
	vkMapMemory(VulkanCore::GetDevice(), stageBufferMemory, 0, size, 0, &data);
	memcpy(data, vertices, size);
	vkUnmapMemory(VulkanCore::GetDevice(), stageBufferMemory);

	VkBufferCreateInfo VertexBufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	VertexBufferCreateInfo.size = size;
	VertexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VertexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_ASSERT(vkCreateBuffer(VulkanCore::GetDevice(), &VertexBufferCreateInfo, nullptr, &m_Buffer) == VK_SUCCESS, "Cant vkCreateBuffer!");

	VkMemoryRequirements VertexBufferMemoryRequriment;
	vkGetBufferMemoryRequirements(VulkanCore::GetDevice(), m_Buffer, &VertexBufferMemoryRequriment);

	VkMemoryAllocateInfo VertexBufferMemoryAllocate{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	VertexBufferMemoryAllocate.allocationSize = VertexBufferMemoryRequriment.size;
	VertexBufferMemoryAllocate.memoryTypeIndex = getMemoryType(VertexBufferMemoryRequriment.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_ASSERT(vkAllocateMemory(VulkanCore::GetDevice(), &VertexBufferMemoryAllocate, nullptr, &m_BufferMemory) == VK_SUCCESS, "Cant vkAllocateMemory!");

	vkBindBufferMemory(VulkanCore::GetDevice(), m_Buffer, m_BufferMemory, 0);


	VkCommandPoolCreateInfo commandPoolCreateInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	commandPoolCreateInfo.queueFamilyIndex = VulkanCore::GetQueueIndices().TransferIndex.value();

	VkCommandPool commandPool;
	vkCreateCommandPool(VulkanCore::GetDevice(), &commandPoolCreateInfo, nullptr, &commandPool);

	VkCommandBufferAllocateInfo commandBufferAllocate{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	commandBufferAllocate.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocate.commandPool = commandPool;
	commandBufferAllocate.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(VulkanCore::GetDevice(), &commandBufferAllocate, &commandBuffer);

	VkCommandBufferBeginInfo commandBufferBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

	VkBufferCopy region{};
	region.size = size;

	vkCmdCopyBuffer(commandBuffer, stageBuffer, m_Buffer, 1, &region);
	VK_ASSERT(vkEndCommandBuffer(commandBuffer) == VK_SUCCESS, "Cant end commandBuffer");

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(VulkanCore::GetTransfernQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(VulkanCore::GetTransfernQueue());

	vkDestroyCommandPool(VulkanCore::GetDevice(), commandPool, nullptr);

	vkDestroyBuffer(VulkanCore::GetDevice(), stageBuffer, nullptr);
	vkFreeMemory(VulkanCore::GetDevice(), stageBufferMemory, nullptr);
}	

uint32_t VertexBuffer::getMemoryType(uint32_t type, VkMemoryPropertyFlags propertyFlags)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(VulkanCore::GetPhysicalDevice(), &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((type & (1 << i)) && ((memoryProperties.memoryTypes[i].propertyFlags & propertyFlags)))
			return i;
	}

	VK_ASSERT(false, "Cant find required memory flags!");
}