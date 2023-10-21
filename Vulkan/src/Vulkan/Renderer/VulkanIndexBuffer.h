#pragma once

#include <vulkan/vulkan.h>

#include "Abstract/VK_VulkanBuffer.h"

#include "VulkanCore.h"

class VulkanIndexBuffer : public VK_VulkanBuffer
{
public:
	VulkanIndexBuffer() = default;
	VulkanIndexBuffer(uint32_t* indices, uint32_t count, VulkanCore* core);
	VulkanIndexBuffer(uint16_t* indices, uint32_t count, VulkanCore* core);

	virtual ~VulkanIndexBuffer();

	VkBuffer& GetBuffer() { return m_Buffer; }
	const VkBuffer& GetBuffer() const { return m_Buffer; }

	VkIndexType& GetIndexType() { return m_IndexType; }
	const VkIndexType& GetIndexType() const { return m_IndexType; }

	uint32_t GetCount() { return m_Count; }

private:
	VkBuffer m_Buffer;
	VkDeviceMemory m_Memory;
	VkDeviceSize m_Size;

	uint32_t m_Count;
	VkIndexType m_IndexType;

	VulkanCore* m_Core;
private:
	template<typename T>
	void createBuffer(T* indices)
	{
		VkBuffer stageBuffer;
		VkDeviceMemory stageMemory;

		VkBufferCreateInfo stageBufferCI = init::createBufferInfo(m_Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &stageBufferCI, nullptr, &stageBuffer));

		VkMemoryRequirements memReq;
		vkGetBufferMemoryRequirements(m_Core->GetDevice(), stageBuffer, &memReq);

		VkMemoryAllocateInfo stageBufferMemoryAI = init::memAllocInfo();
		stageBufferMemoryAI.allocationSize = memReq.size;
		stageBufferMemoryAI.memoryTypeIndex = m_Core->getMemoryType(memReq.memoryTypeBits, (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

		vkAllocateMemory(m_Core->GetDevice(), &stageBufferMemoryAI, nullptr, &stageMemory);
		vkBindBufferMemory(m_Core->GetDevice(), stageBuffer, stageMemory, 0);

		void* data;
		vkMapMemory(m_Core->GetDevice(), stageMemory, 0, m_Size, 0, &data);
		memcpy(data, indices, m_Size);
		vkUnmapMemory(m_Core->GetDevice(), stageMemory);

		VkBufferCreateInfo bufferCI = init::createBufferInfo(m_Size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &bufferCI, nullptr, &m_Buffer));

		vkGetBufferMemoryRequirements(m_Core->GetDevice(), m_Buffer, &memReq);

		VkMemoryAllocateInfo bufferMemoryAI = init::memAllocInfo();
		bufferMemoryAI.allocationSize = memReq.size;
		bufferMemoryAI.memoryTypeIndex = m_Core->getMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		vkAllocateMemory(m_Core->GetDevice(), &bufferMemoryAI, nullptr, &m_Memory);
		vkBindBufferMemory(m_Core->GetDevice(), m_Buffer, m_Memory, 0);

		VkBufferCopy region{};
		region.size = m_Size;

		VkCommandBuffer copyCmd = m_Core->createCopyCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		vkCmdCopyBuffer(copyCmd, stageBuffer, m_Buffer, 1, &region);
		VK_CHECK(vkEndCommandBuffer(copyCmd));

		VkSubmitInfo submitInfo = init::submitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &copyCmd;

		vkQueueSubmit(m_Core->queue.TransferQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_Core->queue.GraphicsQueue);
		vkDeviceWaitIdle(m_Core->GetDevice());

		vkFreeCommandBuffers(m_Core->GetDevice(), m_Core->resources.copyCommandPool, 1, &copyCmd);

		vkDestroyBuffer(m_Core->GetDevice(), stageBuffer, nullptr);
		vkFreeMemory(m_Core->GetDevice(), stageMemory, nullptr);
	}
};