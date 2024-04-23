#pragma once
#include "VulkanBuffer2.h"


namespace V2
{
    class VulkanUniformBuffer2 : public VulkanBuffer2
    {
    public:
        VulkanUniformBuffer2() = default;
        VulkanUniformBuffer2(VulkanCore* core, VkDeviceSize size, void* data = nullptr);
    };
}
