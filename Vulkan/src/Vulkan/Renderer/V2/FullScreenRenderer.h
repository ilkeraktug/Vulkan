#pragma once
#include "DescriptorSetBuilder.h"

class VulkanFrameBuffer;

class FullScreenRenderer
{
public:
    FullScreenRenderer(VulkanCore* core);

    void displayTexture(VkDescriptorImageInfo ImageInfo);
    void draw(VkCommandBuffer cmdBuffer);
    void draw2(VkCommandBuffer cmdBuffer);
private:
    void compileShaders();
    
    void createFrameBuffer();
    void createPipelineLayouts();

    void createPipeline();
    
private:
    VulkanCore* m_Core;
    
    std::unique_ptr<VulkanFrameBuffer> m_Framebuffer;

    VkDescriptorSetLayout m_DescriptorSetLayout;
    VkDescriptorSet m_DescriptorSet;

    VkPipelineLayout m_PipelineLayout;
    VkPipeline m_Pipeline;

   VkDescriptorImageInfo m_ImageInfo;
};
