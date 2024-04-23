#include "pch.h"
#include "VulkanUniformBuffer2.h"

namespace V2
{
    VulkanUniformBuffer2::VulkanUniformBuffer2(VulkanCore* core, VkDeviceSize size, void* data)
    : VulkanBuffer2(core, size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, data)
    {
    }
}
