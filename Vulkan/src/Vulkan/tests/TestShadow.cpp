#include "pch.h"
#include "TestShadow.h"

#include "Vulkan/Objects/CubeObj.h"
#include "Vulkan/Renderer/PerspectiveCamera.h"

test::TestShadow::TestShadow(VulkanCore* core)
{
    Test::Init(core);
    
    m_Camera = new PerspectiveCamera(m_Core->swapchain.extent.width, m_Core->swapchain.extent.height, core);
    m_Cube.reset(new CubeObj(core));

    void prepareOffscreenFramebuffer();
    void prepareOffscreenRenderpass();
    void prepareUniformBuffers();
    void setupDescriptorSetLayout();
    void preparePipelines();
    void setupDescriptorPool();
    void setupDescriptorSets();
    void buildCommandBuffers();
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
    offscreenImageSamplerCI.magFilter = shadowMapFilter;
    offscreenImageSamplerCI.minFilter = shadowMapFilter;
    offscreenImageSamplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    offscreenImageSamplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    offscreenImageSamplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    offscreenImageSamplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    offscreenImageSamplerCI.mipLodBias = 0.0f;
    offscreenImageSamplerCI.maxAnisotropy = 1.0f;
    offscreenImageSamplerCI.minLod = 0.0f;
    offscreenImageSamplerCI.maxLod = 1.0f;
    offscreenImageSamplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    vkCreateSampler(device, &offscreenImageSamplerCI, nullptr, &offscreenPass.depthSampler);

    prepareOffscreenRenderpass();

    VkFramebufferCreateInfo offscreenFramebufferCI{};
    offscreenFramebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    offscreenFramebufferCI.attachmentCount = 1;
    offscreenFramebufferCI.pAttachments = &offscreenPass.depth.View;
    offscreenFramebufferCI.renderPass = offscreenPass.renderPass;
    offscreenFramebufferCI.width = offscreenPass.width;
    offscreenFramebufferCI.height = offscreenPass.height;
    offscreenFramebufferCI.layers = 1;

    VK_CHECK(vkCreateFramebuffer(device, &offscreenFramebufferCI, nullptr, &offscreenPass.frameBuffer));
}

void test::TestShadow::prepareOffscreenRenderpass()
{
    VkAttachmentDescription attachmentDescription{};
    attachmentDescription.format = DEPTH_FORMAT;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkAttachmentReference depthReference{};
    depthReference.attachment = 0;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 0;
    subpass.pDepthStencilAttachment = &depthReference;

    std::array<VkSubpassDependency, 2> subpassDependencies;
    subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[0].dstSubpass = 0;
    subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpassDependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    subpassDependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    subpassDependencies[1].srcSubpass = 0;
    subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    subpassDependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subpassDependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo = init::renderPassCreateInfo();
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = subpassDependencies.size();
    renderPassCreateInfo.pDependencies = subpassDependencies.data();

    VK_CHECK(vkCreateRenderPass(m_Core->GetDevice(), &renderPassCreateInfo, nullptr, &offscreenPass.renderPass));
}

void test::TestShadow::prepareUniformBuffers()
{
}

void test::TestShadow::setupDescriptorSetLayout()
{
}

void test::TestShadow::preparePipelines()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = init::pipelineInputAssemblyState();
    inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo rasterizationStateCI = init::pipelineRasterizationState();
    rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCI.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateCI.depthClampEnable = VK_FALSE;
    rasterizationStateCI.lineWidth = 1.0f;

    VkPipelineColorBlendAttachmentState blendAttachmentState{};
    blendAttachmentState.colorWriteMask = 0xf;
    blendAttachmentState.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlendStateCI = init::pipelineColorBlendState();
    colorBlendStateCI.attachmentCount = 1;
    colorBlendStateCI.pAttachments = &blendAttachmentState;

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = init::pipelineDepthStencilState();
    depthStencilStateCI.depthTestEnable = VK_TRUE;
    depthStencilStateCI.depthWriteEnable = VK_TRUE;
    depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilStateCI.back.compareOp = VK_COMPARE_OP_ALWAYS;

    VkPipelineViewportStateCreateInfo viewportStateCI = init::pipelineViewportState();
    viewportStateCI.viewportCount = 1;
    viewportStateCI.scissorCount = 1;

    VkPipelineMultisampleStateCreateInfo multisampleStateCI = init::multiSampleState();
    multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicStateCI = init::dynamicState();
    dynamicStateCI.dynamicStateCount = dynamicStateEnables.size();
    dynamicStateCI.pDynamicStates = dynamicStateEnables.data();
    
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

    VkGraphicsPipelineCreateInfo pipelineCI = init::graphicsPipelineCreateInfo();
    pipelineCI.stageCount = shaderStages.size();
    pipelineCI.pStages = shaderStages.data();
    pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
    pipelineCI.pViewportState = &viewportStateCI;
    pipelineCI.pRasterizationState = &rasterizationStateCI;
    pipelineCI.pMultisampleState = &multisampleStateCI;
    pipelineCI.pDepthStencilState = &depthStencilStateCI;
    pipelineCI.pColorBlendState = &colorBlendStateCI;
    pipelineCI.pDynamicState = &dynamicStateCI;

    
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
