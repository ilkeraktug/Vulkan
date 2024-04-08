#pragma once

#include <vulkan/vulkan_core.h>

#include "VulkanBuffer2.h"

class VulkanCore;

namespace V2
{
    class VulkanAccelerationStructure
    {
    public:

        VulkanAccelerationStructure();
        VulkanAccelerationStructure(VulkanCore* core, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo, VkAccelerationStructureTypeKHR asType);

        const VkAccelerationStructureKHR& GetHandle() const { return m_Handle; }
        //VkAccelerationStructureKHR GetHandle() { return m_Handle; }
        VkDeviceAddress GetDeviceAddress() const { return m_DeviceAddress; }
        
    private:
        VkAccelerationStructureKHR m_Handle;
        VkDeviceAddress m_DeviceAddress;

        std::unique_ptr<VulkanBuffer2> m_Buffer;
    };
}
