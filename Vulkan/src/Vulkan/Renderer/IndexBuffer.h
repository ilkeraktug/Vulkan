#pragma once
#include "VulkanCore.h"

class IndexBuffer
{
public:
	IndexBuffer() = default;
	IndexBuffer(uint32_t* indices, uint32_t count);
	IndexBuffer(uint16_t* indices, uint32_t count);

	~IndexBuffer();

	inline const VkBuffer& GetBuffer() const { return m_Buffer; }
	inline const VkIndexType GetType() const { return m_Type; }
	inline const uint32_t GetCount() const { return m_Count; }
private:
	void createBuffer(uint32_t* indices, uint32_t count);
	void createBuffer(uint16_t* indices, uint32_t count);

	uint32_t getMemoryType(uint32_t type, VkMemoryPropertyFlags propertyFlags);
private:
	VkBuffer m_Buffer;
	VkDeviceMemory m_BufferMemory;

	VkIndexType m_Type;
	uint32_t m_Count;
};