#include "pch.h"
#include "TestShadow.h"

#include "Vulkan/Objects/CubeObj.h"
#include "Vulkan/Renderer/PerspectiveCamera.h"

test::TestShadow::TestShadow(VulkanCore* core)
{
    Test::Init(core);
    
    m_Camera = new PerspectiveCamera(m_Core->swapchain.extent.width, m_Core->swapchain.extent.height, core);
    m_Cube.reset(new CubeObj(core));
}

test::TestShadow::~TestShadow()
{
}

void test::TestShadow::OnUpdate(float deltaTime)
{
}

void test::TestShadow::OnRender()
{
}

void test::TestShadow::OnImGuiRender()
{
}

void test::TestShadow::prepareOffscreenFramebuffer()
{
    VkDevice device = m_Core->GetDevice();
    offscreenPass.width = SHADOWMAP_DIM;
    offscreenPass.height = SHADOWMAP_DIM;

    VkImageCreateInfo offscreenImageCI = init::imageCreateInfo();
    offscreenImageCI.imageType = VK_IMAGE_TYPE_2D;
    offscreenImageCI.format = DEPTH_FORMAT;
    offscreenImageCI.extent.height = offscreenPass.width;
    offscreenImageCI.extent.width = offscreenPass.height;
    offscreenImageCI.mipLevels = 1;
    offscreenImageCI.arrayLayers = 1;
    offscreenImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    offscreenImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    offscreenImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    
    VK_CHECK(vkCreateImage(device, &offscreenImageCI, nullptr, &offscreenPass.depth.Image));

    VkMemoryRequirements offscreenImageMemReq;
    vkGetImageMemoryRequirements(device, offscreenPass.depth.Image, &offscreenImageMemReq);
    VkMemoryAllocateInfo imageMemAllocInfo = init::memAllocInfo();
    imageMemAllocInfo.allocationSize = offscreenImageMemReq.size;
    imageMemAllocInfo.memoryTypeIndex = m_Core->getMemoryType(offscreenImageMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
     VK_CHECK(vkAllocateMemory(device, &imageMemAllocInfo, nullptr, &offscreenPass.depth.Memory));
     VK_CHECK(vkBindImageMemory(device, offscreenPass.depth.Image, offscreenPass.depth.Memory, 0));

    VkImageViewCreateInfo offscreenImageViewCI = init::imageViewCreateInfo();
    offscreenImageViewCI.image = offscreenPass.depth.Image;
    offscreenImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    offscreenImageViewCI.format = DEPTH_FORMAT;
    offscreenImageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    offscreenImageViewCI.subresourceRange.levelCount = 1;
    offscreenImageViewCI.subresourceRange.layerCount = 1;
    offscreenImageViewCI.subresourceRange.baseMipLevel = 0;
    offscreenImageViewCI.subresourceRange.baseArrayLayer = 0;
    
   VK_CHECK(vkCreateImageView(device, &offscreenImageViewCI, nullptr, &offscreenPass.depth.View));

    VkFilter shadowMapFilter = VK_FILTER_NEAREST;

    VkSamplerCreateInfo offscreenImageSamplerCI = init::samplerCreateInfo();
    VkFilter                magFilter = shadowMapFilter;
    VkFilter                minFilter = shadowMapFilter;
    VkSamplerMipmapMode     mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    VkSamplerAddressMode    addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    VkSamplerAddressMode    addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    VkSamplerAddressMode    addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    float                   mipLodBias = 0.0f;
    float                   maxAnisotropy = 1.0f;
    float                   minLod = 0.0f;
    float                   maxLod = 1.0f;
    VkBorderColor           borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    vkCreateSampler(device, &offscreenImageSamplerCI, nullptr, &offscreenPass.depthSampler);

    prepareOffscreenRenderpass();
}

void test::TestShadow::prepareOffscreenRenderpass()
{
    VkAttachmentDescription attachmentDescription{};
    VkFormat                        format = DEPTH_FORMAT;
    VkSampleCountFlagBits           samples = VK_SAMPLE_COUNT_1_BIT;
    VkAttachmentLoadOp              loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    VkAttachmentStoreOp             storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkAttachmentLoadOp              stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    VkAttachmentStoreOp             stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    VkImageLayout                   initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout                   finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkAttachmentReference depthReference{};
    depthReference.attachment = 0; 
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}

void test::TestShadow::prepareUniformBuffers()
{
}

void test::TestShadow::setupDescriptorSetLayout()
{
}

void test::TestShadow::preparePipelines()
{
}

void test::TestShadow::setupDescriptorPool()
{
}

void test::TestShadow::setupDescriptorSets()
{
}

void test::TestShadow::buildCommandBuffers()
{
}

void test::TestShadow::windowResized()
{
    Test::windowResized();
}
