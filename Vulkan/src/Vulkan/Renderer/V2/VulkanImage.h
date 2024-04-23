#pragma once
#include <vulkan/vulkan_core.h>

class VulkanCore;

enum class EInit
{
    CreateOnlyImage,
    CreateImageAndSampler
};

namespace V2
{
    class VulkanImage
    {
    public:
        VulkanImage() = default;
        VulkanImage(EInit initType, VulkanCore* core = nullptr, VkFormat format = VK_FORMAT_MAX_ENUM, VkExtent3D extent = {}, VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM, VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM);

        virtual ~VulkanImage();

        virtual void Init(EInit initType, VulkanCore* core, VkFormat format, VkExtent3D extent, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);

        void createImage(VulkanCore* core, VkFormat format, VkExtent3D extent, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
        void createSampler();

        const VkImage& GetImage() const { return m_Image; }
        const VkImageView& GetImageView() const { return m_ImageView; }
        const VkSampler& GetSampler() const { return m_Sampler; }
        const VkImageLayout& GetImageLayout() const { return imageLayout; }
        
        const VkDescriptorImageInfo& GetDescriptorInfo() const { return m_DescriptorInfo; }

    protected:
        VkImageType imageType;
        VkImageViewType viewType;

        VkImageLayout imageLayout;

    private:
        VulkanCore* m_Core;

    private:
        VkImage m_Image = VK_NULL_HANDLE;
        VkImageView m_ImageView = VK_NULL_HANDLE;
        VkSampler m_Sampler = VK_NULL_HANDLE;
        VkDeviceMemory m_DeviceMemory = VK_NULL_HANDLE;

        VkDescriptorImageInfo m_DescriptorInfo{};
    };
}