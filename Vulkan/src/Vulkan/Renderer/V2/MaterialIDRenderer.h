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

    void createDescriptorSetForModel();
    void createDescriptorSetForNode(VkDescriptorSet& descriptorSet, const std::unique_ptr<V2::VulkanUniformBuffer2>& uniformBuffer);

    void draw(VkCommandBuffer cmdBuffer);

    void updateUniformBuffer(const vkglTF::Model& model);

private:
    VulkanCore* m_Core;
    Camera* m_Camera;

    std::unique_ptr<VulkanFrameBuffer> m_Framebuffer;

    std::vector<std::unique_ptr<V2::VulkanUniformBuffer2>> m_ModelUniformBuffers;
    std::vector<std::unique_ptr<V2::VulkanUniformBuffer2>> m_ModelInstancePosUniformBuffers;

    VkPipelineLayout m_PipelineLayout;
    VkPipeline m_Pipeline;

    struct CameraUBOData
    {
        glm::mat4 View;
        glm::mat4 Projection;
    };

    struct ModelUBOData
    {
        glm::mat4 Model;
    };

    struct NodeData
    {
        // std::shared_ptr<vkglTF::Node> Node;
       vkglTF::Node* Node;
        
        VkDescriptorSet DescriptorSet;
        std::unique_ptr<V2::VulkanUniformBuffer2> UniformBuffer;
    };
    
    struct ModelData
    {
        std::shared_ptr<vkglTF::Model> Model;
        std::vector<NodeData> Nodes;
    };
    
    std::vector<ModelData> m_Models;

    std::unique_ptr<V2::VulkanUniformBuffer2> m_CameraDataUBO;

    VkDescriptorSet m_CameraDataSet;
    std::vector<VkDescriptorSet> m_ModelDataSets;

    std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
    VkDescriptorPool m_DescriptorPool;
};
