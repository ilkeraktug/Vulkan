#include "pch.h"
#include "VulkanBuffer2.h"

#include "Vulkan/Renderer/VulkanCore.h"


namespace V2
{
    VulkanBuffer2::VulkanBuffer2(VulkanCore* core, VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, void* data)
    {
        init(core, size, usageFlags, memoryPropertyFlags, data);
    }

    VulkanBuffer2::~VulkanBuffer2()
    {
        VkMemoryAllocateFlagsInfo* m_MemoryAllocateFlagsInfo = nullptr;

        vkDestroyBuffer(m_Core->GetDevice(), m_Handle, nullptr);
        vkFreeMemory(m_Core->GetDevice(), m_DeviceMemory, nullptr);

        if(m_MappedMemory)
        {
            std::cout << m_MappedMemory << std::endl;
        }
    }

    void VulkanBuffer2::init(VulkanCore* core, VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, void* data)
    {
        m_Core = core;
        
        m_UsageFlags = usageFlags;
        m_MemoryPropertyFlags = memoryPropertyFlags;
        
        VkBufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.size = size;
        createInfo.usage = usageFlags;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &createInfo, nullptr, &m_Handle));

        VkMemoryRequirements memoryRequirements{};

        vkGetBufferMemoryRequirements(m_Core->GetDevice(), m_Handle, &memoryRequirements);


        if((usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) > 0)
        {
            m_MemoryAllocateFlagsInfo = &shaderDeviceAddressMemAllocFlagsInfo;
        }
        
        VkMemoryAllocateInfo memAllocInfo{};
        memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAllocInfo.pNext = m_MemoryAllocateFlagsInfo;
        memAllocInfo.allocationSize = memoryRequirements.size;
        memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlags);

        VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &memAllocInfo, nullptr, &m_DeviceMemory));

        VK_CHECK(vkBindBufferMemory(m_Core->GetDevice(), m_Handle, m_DeviceMemory, 0));

        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) > 0 && data)
        {
            VK_CHECK(vkMapMemory(m_Core->GetDevice(), m_DeviceMemory, 0, memoryRequirements.size, 0, &m_MappedMemory));
            m_IsMapped = true;

            memcpy(m_MappedMemory, data, memoryRequirements.size);
        }
        else if(data)
        {
            VK_ERROR("VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT is not specified {0}", __FUNCTION__);
        }
        
        m_DescriptorInfo.buffer = m_Handle;
        m_DescriptorInfo.offset = 0;
        m_DescriptorInfo.range = VK_WHOLE_SIZE;
    }

    void VulkanBuffer2::copyToBuffer(void* data, VkDeviceSize size)
    {
        if(m_Core)
        {
            if ((m_MemoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) > 0)
            {
                if (m_IsMapped && data)
                {
                    memcpy(m_MappedMemory, data, size);
                }
                else if(data)
                {
                    VK_CHECK(vkMapMemory(m_Core->GetDevice(), m_DeviceMemory, 0, size, 0, &m_MappedMemory));
                    m_IsMapped = true;
            
                    memcpy(m_MappedMemory, data, size);
                }
            }
            else
            {
                VK_ERROR("VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT is not specified {0}", __FUNCTION__);
            }
        }
        else
        {
            VK_ERROR("Init function was not invoked {0}", __FUNCTION__);
        }
    }
}
