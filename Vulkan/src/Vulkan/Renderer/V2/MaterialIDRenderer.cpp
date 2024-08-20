#include "pch.h"
#include "MaterialIDRenderer.h"

#include "GraphicsPipelineBuilder.h"
#include "VulkanFrameBuffer.h"
#include "VulkanUniformBuffer2.h"
#include "Vulkan/Renderer/VulkanCore.h"

MaterialIDRenderer::MaterialIDRenderer(VulkanCore* core, Camera* camera)
    : m_Core(core), m_Camera(camera)
{
    compileShaders();
    setupFramebuffer();
    setupLayouts();
    updateUniformBuffer();
    setupDescriptorSets();
    setupGraphicsPipeline();
}

void MaterialIDRenderer::OnUpdate(float deltaTime)
{
    updateUniformBuffer();
}

void MaterialIDRenderer::OnRender(VkCommandBuffer cmdBuffer)
{
    draw(cmdBuffer);
}

void MaterialIDRenderer::AddModel(const vkglTF::Model& model, uint32_t size /*= 1*/, float* instancePosData /*= nullptr*/)
{
    AddModel(std::make_shared<vkglTF::Model>(model), size, instancePosData);
}

void MaterialIDRenderer::AddModel(std::shared_ptr<vkglTF::Model> model, uint32_t size /*= 1*/, float* instancePosData /*= nullptr*/)
{
    m_Models.push_back(model);
    
    std::unique_ptr<V2::VulkanUniformBuffer2>& uniformBuffer = m_UniformBuffers.emplace_back(std::make_unique<V2::VulkanUniformBuffer2>(m_Core, sizeof(UBO)));
    UBO ubo{};
    ubo.Model = model->nodes[0]->getMatrix();
    ubo.View = m_Camera->getViewMatrix();
    ubo.Projection = m_Camera->getProjectionMatrix();
    
    uniformBuffer->copyToBuffer(&ubo, sizeof(UBO));
        
    std::unique_ptr<V2::VulkanUniformBuffer2>& uniformBuffer_InstancePos = m_UniformBuffers_InstancePos.emplace_back(std::make_unique<V2::VulkanUniformBuffer2>(m_Core, sizeof(float) * size));

    uniformBuffer_InstancePos->copyToBuffer(instancePosData, sizeof(float) * size);
    
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = m_DescriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &m_DescriptorSetLayout;
    
    VkDescriptorSet& newDescritproSet = m_DescriptorSets.emplace_back();

    VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &descriptorSetAllocInfo, &newDescritproSet));

    VkDescriptorBufferInfo uniformBufferDescriptorInfo{};
    uniformBufferDescriptorInfo.buffer = uniformBuffer->GetHandle();
    uniformBufferDescriptorInfo.offset = 0;
    uniformBufferDescriptorInfo.range = VK_WHOLE_SIZE;
    
    VkDescriptorBufferInfo uniformBufferDescriptorInfo2{};
    uniformBufferDescriptorInfo2.buffer = uniformBuffer_InstancePos->GetHandle();
    uniformBufferDescriptorInfo2.offset = 0;
    uniformBufferDescriptorInfo2.range = VK_WHOLE_SIZE;
    
    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = newDescritproSet;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pBufferInfo = &uniformBufferDescriptorInfo;
    
    VkWriteDescriptorSet writeDescriptorSet2{};
    writeDescriptorSet2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet2.dstSet = newDescritproSet;
    writeDescriptorSet2.dstBinding = 1;
    writeDescriptorSet2.descriptorCount = 1;
    writeDescriptorSet2.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet2.pBufferInfo = &uniformBufferDescriptorInfo2;

    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {writeDescriptorSet, writeDescriptorSet2};
    
    vkUpdateDescriptorSets(m_Core->GetDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}

void MaterialIDRenderer::compileShaders()
{
    std::system(
        "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\material & for /r %i in (*.vert) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tvs_6_1 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo C:%~pi%~ni.vspv)");
    std::system(
        "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\material & for /r %i in (*.frag) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tps_6_1 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo C:%~pi%~ni.fspv)");
}

void MaterialIDRenderer::setupFramebuffer()
{
    m_Framebuffer = std::make_unique<VulkanFrameBuffer>(m_Core);
    m_Framebuffer->AddAttachment(VK_FORMAT_R16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, m_Core->swapchain.extent.width, m_Core->swapchain.extent.height, 1);
    m_Framebuffer->AddAttachment(VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, m_Core->swapchain.extent.width, m_Core->swapchain.extent.height, 1);
    m_Framebuffer->Create();
}

void MaterialIDRenderer::setupLayouts()
{
    VkDescriptorSetLayoutBinding descriptorSetLayoutBindings{};
    descriptorSetLayoutBindings.binding = 0;
    descriptorSetLayoutBindings.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorSetLayoutBindings.descriptorCount = 1;
    descriptorSetLayoutBindings.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding descriptorSetLayoutBindings2{};
    descriptorSetLayoutBindings2.binding = 1;
    descriptorSetLayoutBindings2.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorSetLayoutBindings2.descriptorCount = 1;
    descriptorSetLayoutBindings2.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    std::vector<VkDescriptorSetLayoutBinding> bindings = {descriptorSetLayoutBindings, descriptorSetLayoutBindings2};
    
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
    descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCI.bindingCount = bindings.size();
    descriptorSetLayoutCI.pBindings = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_Core->GetDevice(), &descriptorSetLayoutCI, nullptr, &m_DescriptorSetLayout));

    VkDescriptorPoolSize descriptorPoolSize{};
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSize.descriptorCount = 100;
    
    VkDescriptorPoolCreateInfo descriptorPoolCI{};
    descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCI.maxSets = 100;
    descriptorPoolCI.poolSizeCount = 1;
    descriptorPoolCI.pPoolSizes = &descriptorPoolSize;
    
    VK_CHECK(vkCreateDescriptorPool(m_Core->GetDevice(), &descriptorPoolCI, nullptr, &m_DescriptorPool));

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(float);
    
    VkPipelineLayoutCreateInfo pipelineLayoutCI{};
    pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCI.setLayoutCount = 1;
    pipelineLayoutCI.pSetLayouts = &m_DescriptorSetLayout;
    pipelineLayoutCI.pushConstantRangeCount = 1;
    pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;

    VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCI, nullptr, &m_PipelineLayout));
}

void MaterialIDRenderer::updateUniformBuffer()
{
    for(int i = 0; i < m_Models.size(); ++i)
    {
        UBO ubo{};
        ubo.Model = m_Models[i]->nodes[0]->getMatrix();
        ubo.View = m_Camera->getViewMatrix();
        ubo.Projection = m_Camera->getProjectionMatrix();
        
        m_UniformBuffers[i]->copyToBuffer(&ubo, sizeof(UBO));
    }
}

void MaterialIDRenderer::setupDescriptorSets()
{

}

void MaterialIDRenderer::setupGraphicsPipeline()
{
    m_Pipeline = GraphicsPipelineBuilder::Get().Reset().
    AddShaderStage(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/material/material.vspv", VK_SHADER_STAGE_VERTEX_BIT).
    AddShaderStage(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/material/material.fspv", VK_SHADER_STAGE_FRAGMENT_BIT).
    AddVertexInputState(0, {vkglTF::VertexComponent::Position}).
    AddInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST).
    AddViewportState(m_Core->swapchain.extent.width, m_Core->swapchain.extent.height).
    AddRasterizationState(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE).
    AddMultisampleState().
    AddDepthStencilState(VK_TRUE, VK_TRUE).
    AddColorBlendAttachment(VK_FALSE, 0xf).
    Create(m_Core->GetDevice(), m_PipelineLayout, m_Framebuffer->GetRenderPass());
}

void MaterialIDRenderer::draw(VkCommandBuffer cmdBuffer)
{
    VkClearValue clearValues[2];

    clearValues[0].color = { 0.0f,0.0f,0.0f,0.0f};
    clearValues[1].depthStencil = { 1.0f, 0};
    
    //VkCommandBufferBeginInfo cmdBufferBI = init::cmdBufferBeginInfo();
    
    VkRenderPassBeginInfo renderPassBI{};
    renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBI.renderPass = m_Framebuffer->GetRenderPass();
    renderPassBI.framebuffer = m_Framebuffer->GetHandle();
    renderPassBI.renderArea = {0, 0, m_Core->swapchain.extent};
    renderPassBI.clearValueCount = 2;
    renderPassBI.pClearValues = clearValues;
    
    //vkBeginCommandBuffer(cmdBuffer, &cmdBufferBI);
    vkCmdBeginRenderPass(cmdBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
    
    for(int j = 0; j < m_Models.size(); ++j)
    {
        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSets[j], 0, nullptr);
    
        //m_Models[j]->draw(cmdBuffer);
        const VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &m_Models[j]->vertices.buffer, offsets);
        vkCmdBindIndexBuffer(cmdBuffer, m_Models[j]->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

        float id = j;
        vkCmdPushConstants(cmdBuffer, m_PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &id);

        vkCmdDrawIndexed(cmdBuffer, m_Models[j]->indices.count, 1, 0, 0, 0);
    }
    
    
    vkCmdEndRenderPass(cmdBuffer);
    //VK_CHECK(vkEndCommandBuffer(cmdBuffer));
}

void MaterialIDRenderer::updateUniformBuffer(const vkglTF::Model& model)
{
}
