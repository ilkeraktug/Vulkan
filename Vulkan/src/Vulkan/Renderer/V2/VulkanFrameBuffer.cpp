#include "pch.h"
#include "VulkanFrameBuffer.h"

#include "Vulkan/Core/init.h"
#include "Vulkan/Renderer/VulkanCore.h"

void VulkanFrameBuffer::AddAttachment(VkFormat format, VkImageUsageFlags usage, uint32_t width, uint32_t height, uint32_t layer)
{
    FramebufferAttachment& attachment = m_Attachments.emplace_back();
    attachment.Format = format;
    attachment.layer = layer;
    
    m_Width = width;
    m_Height = height;
    
    VkImageCreateInfo imageCI = init::imageCreateInfo();
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = format;
    imageCI.extent = { width, height, 1 };
    imageCI.mipLevels = 1;
    imageCI.arrayLayers = layer;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage = usage;
    imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateImage(m_Core->GetDevice(), &imageCI, nullptr, &attachment.Image));

    VkMemoryRequirements memoryReqs{};
    
    vkGetImageMemoryRequirements(m_Core->GetDevice(), attachment.Image, &memoryReqs);
    
    VkMemoryAllocateInfo memAllocInfo = init::memAllocInfo();
    memAllocInfo.allocationSize = memoryReqs.size;
    memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memoryReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &memAllocInfo, nullptr, &attachment.Memory));

    VK_CHECK(vkBindImageMemory(m_Core->GetDevice(), attachment.Image, attachment.Memory, 0))

    VkImageAspectFlags aspectMask = 0;

    if(usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    {
        aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
        attachment.bIsColorAttachment = true;
    }
    if(usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask = aspectMask;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = layer;
    
    VkImageViewCreateInfo imageViewCI = init::imageViewCreateInfo();
    imageViewCI.image = attachment.Image;
    imageViewCI.viewType = layer == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    imageViewCI.format = format;
    imageViewCI.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    imageViewCI.subresourceRange = subresourceRange;

    VK_CHECK(vkCreateImageView(m_Core->GetDevice(), &imageViewCI, nullptr, &attachment.ImageView));
}

void VulkanFrameBuffer::Create()
{
    std::vector<VkAttachmentDescription> attachmentDescriptions;
    attachmentDescriptions.reserve(m_Attachments.size());

    std::vector<VkAttachmentReference> colorAttachmentReferences;
    colorAttachmentReferences.reserve(m_Attachments.size());

    std::vector<VkImageView> imageViews;
    imageViews.reserve(m_Attachments.size());

    VkAttachmentReference* pDepthStencilAttachment = nullptr;

    uint32_t maxLayer = 1;
    
    for(unsigned int i = 0; i < m_Attachments.size(); ++i)
    {
        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = m_Attachments[i].Format;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        
        if(m_Attachments[i].bIsColorAttachment)
        {
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            colorAttachmentReferences.push_back({i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        }
        else
        {
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

            pDepthStencilAttachment = new VkAttachmentReference{i, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        }

        if(m_Attachments[i].layer > maxLayer)
        {
            maxLayer = m_Attachments[i].layer;
        }

        attachmentDescriptions.push_back(attachmentDescription);

        imageViews.push_back(m_Attachments[i].ImageView);
    }

    VkSubpassDescription subpassDescription{};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = colorAttachmentReferences.size();
    subpassDescription.pColorAttachments = colorAttachmentReferences.data();
    subpassDescription.pDepthStencilAttachment = pDepthStencilAttachment;

    VkSubpassDependency subpassDependencies[2];

    subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[0].dstSubpass = 0;
    subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    subpassDependencies[1].srcSubpass = 0;
    subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    
    VkRenderPassCreateInfo renderPassCI = init::renderPassCreateInfo();
    renderPassCI.attachmentCount = attachmentDescriptions.size();
    renderPassCI.pAttachments = attachmentDescriptions.data();
    renderPassCI.subpassCount = 1;
    renderPassCI.pSubpasses = &subpassDescription;
    renderPassCI.dependencyCount = 2;
    renderPassCI.pDependencies = subpassDependencies;


    VK_CHECK(vkCreateRenderPass(m_Core->GetDevice(), &renderPassCI, nullptr, &m_RenderPass));
    
    delete pDepthStencilAttachment;
    
    VkFramebufferCreateInfo frameBufferCI{};
    frameBufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCI.renderPass = m_RenderPass;
    frameBufferCI.attachmentCount = imageViews.size();
    frameBufferCI.pAttachments = imageViews.data();
    frameBufferCI.width = m_Width;
    frameBufferCI.height = m_Height;
    frameBufferCI.layers = maxLayer;

    VK_CHECK(vkCreateFramebuffer(m_Core->GetDevice(), &frameBufferCI, nullptr, &m_Framebuffer));
}

VkFramebuffer VulkanFrameBuffer::GetHandle()
{
    return m_Framebuffer;
}

VkRenderPass VulkanFrameBuffer::GetRenderPass()
{
    return m_RenderPass;
}

VkSampler VulkanFrameBuffer::GetSampler()
{
    if(m_Sampler == VK_NULL_HANDLE)
    {
        createSampler();
    }

    return m_Sampler;
}

VkImageView VulkanFrameBuffer::GetImageView(int index)
{
    return m_Attachments[index].ImageView;
}

void VulkanFrameBuffer::createSampler()
{
    VkSamplerCreateInfo samplerCI = init::samplerCreateInfo();
    samplerCI.magFilter = VK_FILTER_LINEAR;
    samplerCI.minFilter = VK_FILTER_LINEAR;
    samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerCI.mipLodBias = 0.0f;
    samplerCI.anisotropyEnable = VK_FALSE;
    samplerCI.minLod = 0.0f;
    samplerCI.maxLod = 1.0f;
    samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

    vkCreateSampler(m_Core->GetDevice(), &samplerCI, nullptr, &m_Sampler);
}
