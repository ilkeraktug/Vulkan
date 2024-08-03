#pragma once
#include <vulkan/vulkan.h>

class VulkanCore;

struct FramebufferAttachment
{
    VkImage Image;
    VkImageView ImageView;

    VkDeviceMemory Memory;
    VkFormat Format;

    bool bIsColorAttachment;
    
    uint32_t Layer; 
};

class VulkanFrameBuffer
{
public:

    VulkanFrameBuffer() = default;
    VulkanFrameBuffer(VulkanCore* core);
    ~VulkanFrameBuffer();
    
    void AddAttachment(VkFormat format, VkImageUsageFlags usage, uint32_t width, uint32_t height, uint32_t layer = 1);
    void Create();

    VkFramebuffer GetHandle();
    VkRenderPass GetRenderPass();
    VkSampler GetSampler();

    VkImageView GetImageView(int index);
private:
    void createSampler();
private:
    VulkanCore* m_Core;
    
    std::vector<FramebufferAttachment> m_Attachments;

    VkSampler m_Sampler = VK_NULL_HANDLE;
    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;

    uint32_t m_Width; 
    uint32_t m_Height;
};
