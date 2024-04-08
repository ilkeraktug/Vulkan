#include "pch.h"
#include "VulkanAccelerationStructure.h"

#include "VulkanBuffer2.h"
#include "Vulkan/Renderer/VulkanCore.h"
#include "Vulkan/Renderer/VulkanFunctions.h"

namespace V2
{
    VulkanAccelerationStructure::VulkanAccelerationStructure()
    {
    }

    VulkanAccelerationStructure::VulkanAccelerationStructure(VulkanCore* core, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo, VkAccelerationStructureTypeKHR asType)
    {
        m_Buffer = std::make_unique<VulkanBuffer2>(core, buildSizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        
        VkAccelerationStructureCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        createInfo.buffer = m_Buffer->GetHandle();
        createInfo.offset = 0;
        createInfo.size = buildSizeInfo.accelerationStructureSize;
        createInfo.type = asType;

        VK_CHECK(vkPFN::vkCreateAccelerationStructureKHR(core->GetDevice(), &createInfo, nullptr, &m_Handle));

        VkAccelerationStructureDeviceAddressInfoKHR deviceAddressInfo{};
        deviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
        deviceAddressInfo.accelerationStructure = m_Handle;

        m_DeviceAddress = vkPFN::vkGetAccelerationStructureDeviceAddressKHR(core->GetDevice(), &deviceAddressInfo);
    }
}
