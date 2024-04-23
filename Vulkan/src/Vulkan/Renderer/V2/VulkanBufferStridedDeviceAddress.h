#pragma once
#include "VulkanBuffer2.h"

namespace V2
{
    class VulkanBufferStridedDeviceAddress : public VulkanBuffer2
    {
    public:
        VulkanBufferStridedDeviceAddress() = default;
        VulkanBufferStridedDeviceAddress(VulkanCore* core, VkDeviceSize size, VkDeviceSize stride, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, void* data = nullptr);

        virtual ~VulkanBufferStridedDeviceAddress();

        const VkStridedDeviceAddressRegionKHR& GetStridedDeviceAddress() const { return m_StridedDeviceAddress; }
    private:
        VkStridedDeviceAddressRegionKHR m_StridedDeviceAddress;
    };
}
