#pragma once
#include "GraphicsPipelineBuilder.h"

namespace V2
{
    class VulkanUniformBuffer2;
}

class VulkanFrameBuffer;
class VulkanCore;

class MaterialIDRenderer
{
public:
    MaterialIDRenderer(VulkanCore* core, Camera* camera);

    void OnUpdate(float deltaTime);
    void OnRender(VkCommandBuffer cmdBuffer);

    void AddModel(const vkglTF::Model& model, uint32_t size = 1, float* instancePosData = nullptr);
    void AddModel(std::shared_ptr<vkglTF::Model> model, uint32_t size = 1, float* instancePosData = nullptr);

    const std::unique_ptr<VulkanFrameBuffer>& GetFrameBuffer() const { return m_Framebuffer; }
    
    inline static uint64_t GlobalID = 1;
private:
    void compileShaders();
    void setupFramebuffer();
    void setupLayouts();
    void updateUniformBuffer();
    void setupDescriptorSets();
    void setupGraphicsPipeline();

    void draw(VkCommandBuffer cmdBuffer);

    void updateUniformBuffer(const vkglTF::Model& model);

private:
    VulkanCore* m_Core;
    Camera* m_Camera;

    std::unique_ptr<VulkanFrameBuffer> m_Framebuffer;

    std::vector<std::unique_ptr<V2::VulkanUniformBuffer2>> m_UniformBuffers;
    std::vector<std::unique_ptr<V2::VulkanUniformBuffer2>> m_UniformBuffers_InstancePos;

    VkDescriptorSetLayout m_DescriptorSetLayout;
    VkDescriptorPool m_DescriptorPool;
    std::vector<VkDescriptorSet> m_DescriptorSets;

    VkPipelineLayout m_PipelineLayout;
    VkPipeline m_Pipeline;

    struct UBO
    {
        glm::mat4 Model;
        glm::mat4 View;
        glm::mat4 Projection;
    };

    std::vector<std::shared_ptr<vkglTF::Model>> m_Models;
};
