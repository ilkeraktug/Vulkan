#include "pch.h"
#include "VulkanImage.h"

#include <vulkan/vulkan_core.h>

#include "Vulkan/Core/tools.h"
#include "Vulkan/Renderer/VulkanCore.h"

namespace V2
{
    VulkanImage::VulkanImage(EInit initType, VulkanCore* core, VkFormat format, VkExtent3D extent, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags)
        : m_Core(core)
    {
        Init(initType, core, format, extent, usageFlags, memoryPropertyFlags);
    }

    VulkanImage::~VulkanImage()
    {
        if(m_Core)
        {
            if(m_Image != VK_NULL_HANDLE)
            {
               vkDestroyImage(m_Core->GetDevice(), m_Image, nullptr); 
            }

            if(m_ImageView != VK_NULL_HANDLE)
            {
                vkDestroyImageView(m_Core->GetDevice(), m_ImageView, nullptr); 
            }

            if(m_Sampler != VK_NULL_HANDLE)
            {
                vkDestroySampler(m_Core->GetDevice(), m_Sampler, nullptr); 
            }

            if(m_DeviceMemory != VK_NULL_HANDLE)
            {
                vkFreeMemory(m_Core->GetDevice(), m_DeviceMemory, nullptr); 
            }
        }
    }

    void VulkanImage::Init(EInit initType, VulkanCore* core, VkFormat format, VkExtent3D extent, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags)
    {
        if(initType == EInit::CreateOnlyImage)
        {
            createImage(core, format, extent, usageFlags, memoryPropertyFlags);
        }
        else if(initType == EInit::CreateImageAndSampler)
        {
            createImage(core, format, extent, usageFlags, memoryPropertyFlags);
            createSampler();
        }
    }

    void VulkanImage::createImage(VulkanCore* core, VkFormat format, VkExtent3D extent, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags)
    {
        m_Core = core;

        imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        
        VkImageCreateInfo imageCI{};
        imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCI.imageType = imageType;
        imageCI.format = format;
        imageCI.extent = extent;
        imageCI.mipLevels = 1;
        imageCI.arrayLayers = 1;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage = usageFlags;
        imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VK_CHECK(vkCreateImage(m_Core->GetDevice(), &imageCI, nullptr, &m_Image));

        VkMemoryRequirements memReqs{};
    
        vkGetImageMemoryRequirements(m_Core->GetDevice(), m_Image, &memReqs);

        VkMemoryAllocateInfo memAllocInfo{};
        memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
    
        VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &memAllocInfo, nullptr, &m_DeviceMemory));

        VK_CHECK(vkBindImageMemory(m_Core->GetDevice(), m_Image, m_DeviceMemory, 0));
    
        VkImageViewCreateInfo imageViewCI{};
        imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCI.image = m_Image;
        imageViewCI.viewType = viewType;
        imageViewCI.format = format;
        imageViewCI.components.r = VK_COMPONENT_SWIZZLE_R;
        imageViewCI.components.g = VK_COMPONENT_SWIZZLE_G;
        imageViewCI.components.b = VK_COMPONENT_SWIZZLE_B;
        imageViewCI.components.a = VK_COMPONENT_SWIZZLE_A;
        imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCI.subresourceRange.baseMipLevel = 0;
        imageViewCI.subresourceRange.levelCount = 1;
        imageViewCI.subresourceRange.baseArrayLayer = 0;
        imageViewCI.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(m_Core->GetDevice(), &imageViewCI, nullptr, &m_ImageView));

        m_DescriptorInfo.sampler = m_Sampler;
        m_DescriptorInfo.imageView = m_ImageView;
        m_DescriptorInfo.imageLayout = imageLayout;
    }

    void VulkanImage::createSampler()
    {
        VkSamplerCreateInfo samplerCI{};
        samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCI.magFilter = VK_FILTER_LINEAR;
        samplerCI.minFilter = VK_FILTER_LINEAR;
        samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.mipLodBias = 0;
        samplerCI.anisotropyEnable = VK_FALSE;
        samplerCI.compareEnable = VK_FALSE;
        samplerCI.minLod = 0;
        samplerCI.maxLod = 0;
        samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        samplerCI.unnormalizedCoordinates = VK_FALSE;

        VK_CHECK(vkCreateSampler(m_Core->GetDevice(), &samplerCI, nullptr, &m_Sampler));
    }
}