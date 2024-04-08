#pragma once
#include <vulkan/vulkan_core.h>

class VulkanCore;

namespace V2
{
    static VkMemoryAllocateFlagsInfo shaderDeviceAddressMemAllocFlagsInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO, nullptr, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, 0};
    
    class VulkanBuffer2
    {
    public:

        VulkanBuffer2() = default;
        VulkanBuffer2(VulkanCore* core, VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, void* data = nullptr);

        ~VulkanBuffer2();
        
        void copyToBuffer(void* data, VkDeviceSize size);
    
        void init(VulkanCore* core, VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, void* data);

        VkBuffer GetHandle() const { return m_Handle; }
        VkDeviceMemory GetMemory() const { return m_DeviceMemory; }

    protected:
        VkMemoryAllocateFlagsInfo* m_MemoryAllocateFlagsInfo = nullptr;
    private:

        VkBuffer m_Handle = VK_NULL_HANDLE;
        VkDeviceMemory m_DeviceMemory = VK_NULL_HANDLE;
        void* m_MappedMemory = nullptr;

        VkDescriptorBufferInfo m_DescriptorInfo{};

        VulkanCore* m_Core = nullptr;


        VkBufferUsageFlags m_UsageFlags;
        VkMemoryPropertyFlags m_MemoryPropertyFlags;
        
        bool m_IsMapped = false;
    };
}
