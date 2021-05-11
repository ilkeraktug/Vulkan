#pragma once

#include "Abstract/Buffer.h"

#include "VulkanCore.h"

class VulkanUniformBuffer : public Buffer
{
public:
	VulkanUniformBuffer() = default;
	VulkanUniformBuffer(uint32_t size, VulkanCore* core);

	virtual ~VulkanUniformBuffer();

	void copyData(void* data, uint32_t size);

	VkBuffer GetBuffer() { return m_Buffer; }
	const VkBuffer& GetBuffer() const { return m_Buffer; }

	VkDescriptorBufferInfo GetBufferInfo() { return m_BufferInfo; }
	const VkDescriptorBufferInfo& GetBufferInfo() const { return m_BufferInfo; }
public:

private:
	void createBuffer();
	void setBufferInfo();
private:
	VkBuffer m_Buffer;
	VkDeviceMemory m_DeviceMemory;
	uint32_t m_Size;
	VkDescriptorBufferInfo m_BufferInfo;

	VulkanCore* m_Core;
};