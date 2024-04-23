#include "pch.h"
#include "VulkanBufferStridedDeviceAddress.h"

#include "Vulkan/Renderer/VulkanCore.h"

namespace V2
{
    VulkanBufferStridedDeviceAddress::VulkanBufferStridedDeviceAddress(VulkanCore* core, VkDeviceSize size, VkDeviceSize stride, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, void* data)
    {
        m_MemoryAllocateFlagsInfo = &shaderDeviceAddressMemAllocFlagsInfo;

        init(core, size, usageFlags, memoryPropertyFlags, data);

        VkBufferDeviceAddressInfo bufferDeviceAddressInfo{};
        bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        bufferDeviceAddressInfo.buffer = GetHandle();

        m_StridedDeviceAddress.deviceAddress = vkGetBufferDeviceAddress(core->GetDevice(), &bufferDeviceAddressInfo);
        m_StridedDeviceAddress.stride = stride;
        m_StridedDeviceAddress.size = size;
    }

    VulkanBufferStridedDeviceAddress::~VulkanBufferStridedDeviceAddress()
    {
        m_StridedDeviceAddress = {};
    }
}
