#pragma once

#include "Test.h"
#include "Vulkan/Renderer/glTFModel.h"
#include "Vulkan/Renderer/Abstract/Camera.h"
#include "Vulkan/Renderer/V2/VulkanTexture.h"

class VulkanFrameBuffer;

namespace V2
{
    class VulkanUniformBuffer2;
}

namespace test
{
    class TestRenderPass : public Test
    {
    public:
        TestRenderPass() = default;
        TestRenderPass(VulkanCore* core);
        
        virtual ~TestRenderPass();

        virtual void OnUpdate(float deltaTime) override;
        virtual void OnRender() override;
        virtual void OnImGuiRender() override;
        
        void runBatchFile();
        void loadassets();
        void deferredSetup();
        void shadowSetup();
        void prepareDescriptorSetLayout();
        void writeDescriptors();
        void preparePipeline();
        void prepareUniformBuffers();

        void updateUniformBuffers();
        void buildDeferredCommandBuffers();
    private:
        std::unique_ptr<Camera> m_Camera;
        
        struct Shadow
        {
            VkFormat Format = VK_FORMAT_D32_SFLOAT_S8_UINT;
            VkExtent3D Extent{1024, 1024, 1};
            uint32_t LightCount = 3;
            
            VkRenderPass renderPass;
            
            VkImage depthImage;
            VkDeviceMemory depthImageMemory;
            VkImageView depthImageView;
            
            std::unique_ptr<VulkanFrameBuffer> FrameBuffer;            
        } shadow;

        struct Scene
        {
            VkFormat ColorFormat = VK_FORMAT_R8G8B8A8_UNORM;
            VkFormat DepthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
            
            VkRenderPass renderPass;

            VkExtent3D ColorExtent{1024, 1024, 1};
            VkExtent3D DepthExtent{1024, 1024, 1};
            
            VkImage colorImage;
            VkDeviceMemory colorImageMemory;
            VkImageView colorImageView;
            
            VkImage depthImage;
            VkDeviceMemory depthImageMemory;
            VkImageView depthImageView;
            
            std::unique_ptr<VulkanFrameBuffer> FrameBuffer;
        } scene;

        struct
        {
            VkPipeline shadow;
            VkPipeline scene;
            VkPipeline gBuffer;
        } pipelines;

        VkDescriptorSetLayout m_DescriptorSetLayout;
        VkPipelineLayout m_PipelineLayout;

        VkDescriptorPool m_DescriptorPool;
        VkDescriptorSet m_DescriptorSet;

        struct {
            struct {
                vks::Texture2D colorMap;
                vks::Texture2D normalMap;
            } model;
            struct {
                vks::Texture2D colorMap;
                vks::Texture2D normalMap;
            } background;
        } textures;

        struct {
            vkglTF::Model model;
            vkglTF::Model background;
        } models;
        
        struct Light
        {
            glm::vec4 Position;
            glm::vec4 Target;
            glm::vec4 Color;
            glm::mat4 MVP;
        };

        struct {
            VkDescriptorSet model;
            VkDescriptorSet background;
            VkDescriptorSet shadow;
        } descriptorSets;

        struct SceneUniformBufferStruct
        {
            glm::vec3 viewPosition;
            Light lights[3];
            int debugShadow;
        } SceneUniformBufferStructObject;

        struct ShadowUniformBufferStruct
        {
            glm::mat4 MVPs[3];
            glm::vec4 InstancePos[3];
        } ShadowUniformBufferStructObject;

        struct gBufferUniformBufferStruct
        {
            glm::mat4 Model;
            glm::mat4 View;
            glm::mat4 Projection;
            glm::vec4 InstancePos[3];
        } gBufferUniformBufferStructObject;
        
        std::unique_ptr<V2::VulkanUniformBuffer2> gBufferUniformBuffer;
        std::unique_ptr<V2::VulkanUniformBuffer2> shadowUniformBuffer;
        std::unique_ptr<V2::VulkanUniformBuffer2> sceneUniformBuffer;

        float timer = 0.0f;
        // Multiplier for speeding up (or slowing down) the global timer
        float timerSpeed = 0.25f;

        bool init = false;
    };
}
