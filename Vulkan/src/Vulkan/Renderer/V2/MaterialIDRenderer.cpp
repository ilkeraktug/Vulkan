#include "pch.h"
#include "MaterialIDRenderer.h"

#include "DescriptorSetBuilder.h"
#include "GraphicsPipelineBuilder.h"
#include "VulkanFrameBuffer.h"
#include "VulkanUniformBuffer2.h"
#include "Vulkan/Renderer/VulkanCore.h"

MaterialIDRenderer::MaterialIDRenderer(VulkanCore* core, Camera* camera)
    : m_Core(core), m_Camera(camera)
{
    m_CameraDataUBO = std::make_unique<V2::VulkanUniformBuffer2>(m_Core, sizeof(CameraUBOData));
    
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
    ModelData& modelData = m_Models.emplace_back();
    modelData.Model = model;
    
    m_ModelUniformBuffers.emplace_back(std::make_unique<V2::VulkanUniformBuffer2>(m_Core, sizeof(ModelUBOData)));
    createDescriptorSetForModel();

    if(instancePosData)
    {
        std::unique_ptr<V2::VulkanUniformBuffer2>& uniformBuffer_InstancePos = m_ModelInstancePosUniformBuffers.emplace_back(std::make_unique<V2::VulkanUniformBuffer2>(m_Core, sizeof(float) * size));
    
        uniformBuffer_InstancePos->copyToBuffer(instancePosData, sizeof(float) * size);
    }

    for(int i = 0; i < model->nodes.size(); ++i)
    {
        NodeData& nodeData = modelData.Nodes.emplace_back();
        nodeData.UniformBuffer = std::make_unique<V2::VulkanUniformBuffer2>(m_Core, sizeof(ModelUBOData));
        nodeData.Node = model->nodes[i];
        
        createDescriptorSetForNode(nodeData.DescriptorSet, nodeData.UniformBuffer);
    }
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
    DescriptorSetBuilder::Get().Begin(m_Core->GetDevice()).
    AddDescriptorLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT).
    CreateDescriptorLayout(m_DescriptorSetLayouts).
    ResetDescriptorLayoutBinding().
    AddDescriptorLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT).
    CreateDescriptorLayout(m_DescriptorSetLayouts).
    AddDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 20).
    CreateDescriptorPool(m_DescriptorPool).
    AllocateDescriptorSet(0, m_CameraDataSet).
    AddBufferInfo(0, m_CameraDataUBO->GetHandle()).
    UpdateDescriptorSet();

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(float);
    
    VkPipelineLayoutCreateInfo pipelineLayoutCI{};
    pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCI.setLayoutCount = m_DescriptorSetLayouts.size();
    pipelineLayoutCI.pSetLayouts = m_DescriptorSetLayouts.data();
    pipelineLayoutCI.pushConstantRangeCount = 1;
    pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;

    VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCI, nullptr, &m_PipelineLayout));
}

void MaterialIDRenderer::updateUniformBuffer()
{
    for(int i = 0; i < m_Models.size(); ++i)
    {
        for(auto&& nodeData : m_Models[i].Nodes)
        {
            ModelUBOData ubo{};
            ubo.Model = nodeData.Node->getMatrix();
        
            nodeData.UniformBuffer->copyToBuffer(&ubo, sizeof(ModelUBOData));
        }
    }
    
    CameraUBOData cameraUboData{};
    cameraUboData.View = m_Camera->getViewMatrix();
    cameraUboData.Projection = m_Camera->getProjectionMatrix();

    m_CameraDataUBO->copyToBuffer(&cameraUboData, sizeof(CameraUBOData));
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
    AddRasterizationState(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE).
    AddMultisampleState().
    AddDepthStencilState(VK_TRUE, VK_TRUE).
    AddColorBlendAttachment(VK_FALSE, 0xf).
    Create(m_Core->GetDevice(), m_PipelineLayout, m_Framebuffer->GetRenderPass());
}

void MaterialIDRenderer::createDescriptorSetForModel()
{
    VkDescriptorSet& modelDescriptorSet = m_ModelDataSets.emplace_back();
    
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_DescriptorSetLayouts[1];

    VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &allocInfo, &modelDescriptorSet));

    VkDescriptorBufferInfo uniformBufferInfo{};
    uniformBufferInfo.buffer = m_ModelUniformBuffers.back()->GetHandle();
    uniformBufferInfo.offset = 0;
    uniformBufferInfo.range = VK_WHOLE_SIZE;
    
    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = modelDescriptorSet;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pBufferInfo = &uniformBufferInfo;

    vkUpdateDescriptorSets(m_Core->GetDevice(), 1, &writeDescriptorSet, 0, nullptr);
    
}

void MaterialIDRenderer::createDescriptorSetForNode(VkDescriptorSet& descriptorSet, const std::unique_ptr<V2::VulkanUniformBuffer2>& uniformBuffer)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_DescriptorSetLayouts[1];

    VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &allocInfo, &descriptorSet));

    VkDescriptorBufferInfo uniformBufferInfo{};
    uniformBufferInfo.buffer = uniformBuffer->GetHandle();
    uniformBufferInfo.offset = 0;
    uniformBufferInfo.range = VK_WHOLE_SIZE;
    
    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pBufferInfo = &uniformBufferInfo;

    vkUpdateDescriptorSets(m_Core->GetDevice(), 1, &writeDescriptorSet, 0, nullptr);
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
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_CameraDataSet, 0, nullptr);
    
    for(int i = 0; i < m_Models.size(); ++i)
    {
    
        //m_Models[j]->draw(cmdBuffer);
        const VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &m_Models[i].Model->vertices.buffer, offsets);
        vkCmdBindIndexBuffer(cmdBuffer, m_Models[i].Model->indices.buffer, 0, VK_INDEX_TYPE_UINT32);

        for(int j = 0; j < m_Models[i].Nodes.size(); ++j)
        {
            vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 1, 1, &m_Models[i].Nodes[j].DescriptorSet, 0, nullptr);
            float id = i * j + i + 1;
            vkCmdPushConstants(cmdBuffer, m_PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &id);
            
            vkCmdDrawIndexed(cmdBuffer, m_Models[i].Model->indices.count, 1, 0, 0, 0);
        }

    }
    
    
    vkCmdEndRenderPass(cmdBuffer);
    //VK_CHECK(vkEndCommandBuffer(cmdBuffer));
}

void MaterialIDRenderer::updateUniformBuffer(const vkglTF::Model& model)
{
}
