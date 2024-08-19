#include "pch.h"
#include "FullScreenRenderer.h"

#include "GraphicsPipelineBuilder.h"
#include "VulkanFrameBuffer.h"
#include "Vulkan/Renderer/VulkanCore.h"

FullScreenRenderer::FullScreenRenderer(VulkanCore* core)
    : m_Core(core)
{
    compileShaders();
    createFrameBuffer();
}

void FullScreenRenderer::displayTexture(VkDescriptorImageInfo* ImageInfo)
{
    m_ImageInfo = ImageInfo;
    createPipelineLayouts();
    createPipeline();
}

void FullScreenRenderer::draw(VkCommandBuffer cmdBuffer)
{
    if(m_ImageInfo)
    {
        VkClearValue colorClearValue{};
        colorClearValue.color = {0.0f, 0.0f, 0.0f, 1.0f };
    
        VkRenderPassBeginInfo renderPassBI{};
        renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBI.renderPass = m_Framebuffer->GetRenderPass();
        renderPassBI.framebuffer = m_Framebuffer->GetHandle();
        renderPassBI.renderArea = { {0, 0}, {m_Core->swapchain.extent.width, m_Core->swapchain.extent.height} };
        renderPassBI.clearValueCount = 1;
        renderPassBI.pClearValues = &colorClearValue;

        vkCmdBeginRenderPass(cmdBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);
        vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
        vkCmdEndRenderPass(cmdBuffer);
    }
}

void FullScreenRenderer::compileShaders()
{
    std::system("@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\fullscreenpass & for /r %i in (*.vert) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tvs_6_1 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo C:%~pi%~ni.vspv)");
    std::system("@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\fullscreenpass & for /r %i in (*.frag) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tvs_6_1 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo C:%~pi%~ni.fspv)");
}

void FullScreenRenderer::createFrameBuffer()
{
    m_Framebuffer = std::make_unique<VulkanFrameBuffer>(m_Core);

    m_Framebuffer->AddAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_Core->swapchain.extent.width, m_Core->swapchain.extent.height);
    m_Framebuffer->Create();
}

void FullScreenRenderer::createPipelineLayouts()
{
    if(m_ImageInfo)
    {
        DescriptorSetBuilder::Get().Begin(m_Core->GetDevice()).
        AddDescriptorLayoutBinding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT).
        CreateDescriptorLayout(m_DescriptorSetLayout).
        AddDescriptorPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1).
        CreateDescriptorPool().
        AllocateDescriptorSet(m_DescriptorSet).
        AddImageInfo(0, m_ImageInfo).
        UpdateDescriptorSet();
    
        VkPipelineLayoutCreateInfo pipelineLayoutCI{};
        pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount = 1;
        pipelineLayoutCI.pSetLayouts = &m_DescriptorSetLayout;
        pipelineLayoutCI.pushConstantRangeCount = 0;
        pipelineLayoutCI.pPushConstantRanges = nullptr;

        VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCI, nullptr, &m_PipelineLayout));
    }
}

void FullScreenRenderer::createPipeline()
{
    GraphicsPipelineBuilder().Get().Reset().
    AddShaderStage(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/fullscreenpass/fullscreenVS.vspv", VK_SHADER_STAGE_VERTEX_BIT).
    AddShaderStage(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/fullscreenpass/fullscreenFS.fspv", VK_SHADER_STAGE_FRAGMENT_BIT).
    AddEmptyVertexInputState().
    AddInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST).
    AddRasterizationState(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE).
    AddViewportState(m_Core->swapchain.extent.width, m_Core->swapchain.extent.height).
    AddDepthStencilState(VK_FALSE, VK_FALSE).
    AddEmptyColorBlendAttachment().
    AddMultisampleState().
    AddDynamicPipelineState({}).
    Create(m_Core->GetDevice(), m_PipelineLayout, m_Framebuffer->GetRenderPass());
}
