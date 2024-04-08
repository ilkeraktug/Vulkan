#include "pch.h"
#include "VulkanBufferDeviceAddress.h"

#include "Vulkan/Renderer/VulkanCore.h"

namespace V2
{
    VulkanBufferDeviceAddress::VulkanBufferDeviceAddress(VulkanCore* core, VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, void* data)
    {
        m_MemoryAllocateFlagsInfo = &shaderDeviceAddressMemAllocFlagsInfo;

        init(core, size, usageFlags, memoryPropertyFlags, data);

        VkBufferDeviceAddressInfo bufferDeviceAddressInfo{};
        bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        bufferDeviceAddressInfo.buffer = GetHandle();
        
        m_DeviceAddress = vkGetBufferDeviceAddress(core->GetDevice(), &bufferDeviceAddressInfo);
    }

    VulkanBufferDeviceAddress::~VulkanBufferDeviceAddress()
    {
        m_DeviceAddress = 0;
    }
}
