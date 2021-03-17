#include "pch.h"
#include "IndexBuffer.h"


IndexBuffer::IndexBuffer(uint32_t* indices, uint32_t count)
	: m_Count(count), m_Type(VK_INDEX_TYPE_UINT32)
{
	createBuffer(indices, count);
}

IndexBuffer::IndexBuffer(uint16_t* indices, uint32_t count)
	: m_Count(count), m_Type(VK_INDEX_TYPE_UINT16)
{
	createBuffer(indices, count);
}

IndexBuffer::~IndexBuffer()
{
	vkQueueWaitIdle(VulkanCore::GetGraphicsQueue());
	vkDestroyBuffer(VulkanCore::GetDevice(), m_Buffer, nullptr);
	vkFreeMemory(VulkanCore::GetDevice(), m_BufferMemory, nullptr);
}

void IndexBuffer::createBuffer(uint32_t* indices, uint32_t count)
{
	uint32_t size = count * sizeof(uint32_t);

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
	memcpy(data, indices, size);
	vkUnmapMemory(VulkanCore::GetDevice(), stageBufferMemory);

	VkBufferCreateInfo indexBufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	indexBufferCreateInfo.size = size;
	indexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	indexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_ASSERT(vkCreateBuffer(VulkanCore::GetDevice(), &indexBufferCreateInfo, nullptr, &m_Buffer) == VK_SUCCESS, "Cant vkCreateBuffer!");

	VkMemoryRequirements indexBufferMemoryRequriment;
	vkGetBufferMemoryRequirements(VulkanCore::GetDevice(), m_Buffer, &indexBufferMemoryRequriment);

	VkMemoryAllocateInfo indexBufferMemoryAllocate{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	indexBufferMemoryAllocate.allocationSize = indexBufferMemoryRequriment.size;
	indexBufferMemoryAllocate.memoryTypeIndex = getMemoryType(indexBufferMemoryRequriment.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_ASSERT(vkAllocateMemory(VulkanCore::GetDevice(), &indexBufferMemoryAllocate, nullptr, &m_BufferMemory) == VK_SUCCESS, "Cant vkAllocateMemory!");

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

void IndexBuffer::createBuffer(uint16_t* indices, uint32_t count)
{
	uint32_t size = count * sizeof(uint16_t);

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
	memcpy(data, indices, size);
	vkUnmapMemory(VulkanCore::GetDevice(), stageBufferMemory);

	VkBufferCreateInfo indexBufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	indexBufferCreateInfo.size = size;
	indexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	indexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_ASSERT(vkCreateBuffer(VulkanCore::GetDevice(), &indexBufferCreateInfo, nullptr, &m_Buffer) == VK_SUCCESS, "Cant vkCreateBuffer!");

	VkMemoryRequirements indexBufferMemoryRequriment;
	vkGetBufferMemoryRequirements(VulkanCore::GetDevice(), m_Buffer, &indexBufferMemoryRequriment);

	VkMemoryAllocateInfo indexBufferMemoryAllocate{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	indexBufferMemoryAllocate.allocationSize = indexBufferMemoryRequriment.size;
	indexBufferMemoryAllocate.memoryTypeIndex = getMemoryType(indexBufferMemoryRequriment.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_ASSERT(vkAllocateMemory(VulkanCore::GetDevice(), &indexBufferMemoryAllocate, nullptr, &m_BufferMemory) == VK_SUCCESS, "Cant vkAllocateMemory!");

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

uint32_t IndexBuffer::getMemoryType(uint32_t type, VkMemoryPropertyFlags propertyFlags)
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
