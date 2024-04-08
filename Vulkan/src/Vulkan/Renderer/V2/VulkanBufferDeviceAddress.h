#pragma once
#include "VulkanBuffer2.h"

namespace V2
{
    class VulkanBufferDeviceAddress : public VulkanBuffer2
    {
    public:
        VulkanBufferDeviceAddress() = default;
        VulkanBufferDeviceAddress(VulkanCore* core, VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, void* data = nullptr);

        virtual ~VulkanBufferDeviceAddress();

        VkDeviceAddress GetDeviceAddress() const { return m_DeviceAddress; }
    private:
        VkDeviceAddress m_DeviceAddress = 0;
    };
}
