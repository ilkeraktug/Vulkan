#pragma once

#include "Test.h"
#include "Vulkan/Renderer/glTFModel.h"
#include "Vulkan/Renderer/Abstract/Camera.h"
#include "Vulkan/Renderer/V2/VulkanTexture.h"

class FullScreenRenderer;
class MaterialIDRenderer;
class VulkanFrameBuffer;

namespace V2
{
    class VulkanBuffer2;
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
        void preparePipeline();
        void prepareUniformBuffers();

        void updateUniformBuffers();
        void buildDeferredCommandBuffers();

        void AddLight(glm::vec4 position, glm::vec4 target, glm::vec4 color, glm::mat4 MVP);
    private:
        std::unique_ptr<Camera> m_Camera;

        std::unique_ptr<MaterialIDRenderer> m_MaterialIDRenderer;
        std::unique_ptr<FullScreenRenderer> m_FullScreenRenderer;
        
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
            VkFormat ColorFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
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

        struct
        {
            VkDescriptorPool gBuffer;
            VkDescriptorPool scene;
            VkDescriptorPool shadow;
        } descriptorPools;
        
        struct
        {
            VkDescriptorSetLayout gBuffer;
            VkDescriptorSetLayout scene;
            VkDescriptorSetLayout shadow;
        } descriptorSetLayouts;

        struct
        {
            VkPipelineLayout gBuffer;
            VkPipelineLayout scene;
            VkPipelineLayout shadow;
        } pipelineLayouts;
        
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
            std::shared_ptr<vkglTF::Model> model;
            std::shared_ptr<vkglTF::Model> background;
        } models;
        
        struct LightData
        {
            LightData() = default;
            LightData(glm::vec4 position, glm::vec4 target, glm::vec4 color)
                :   Position(position),
                    Target(target),
                    Color(color) {}
            
            glm::vec4 Position;
            glm::vec4 Target;
            glm::vec4 Color;
        };

        struct LightMVPs
        {
            LightMVPs() = default;
            LightMVPs(glm::mat4 mvp)
                :   MVP(mvp) {}
            
            glm::mat4 MVP;
        };
        
       struct ModelData
       {
           ModelData() = default;
           ModelData(glm::mat4 modelMatrix)
               :   ModelMatrix(modelMatrix) {}
           
           glm::mat4 ModelMatrix;
       };

        struct CameraData
        {
            CameraData() = default;
            CameraData(glm::vec4 position, glm::mat4 view, glm::mat4 projection)
                :   Position(position), View(view), Projection(projection) {}
            
            glm::vec4 Position;
            glm::mat4 View;
            glm::mat4 Projection;
        };

        std::vector<LightData> m_LightData;
        std::vector<LightMVPs> m_LightMVPs;
        std::vector<glm::vec4> m_InstancePositions;

        struct {
            VkDescriptorSet model;
            VkDescriptorSet background;
            VkDescriptorSet shadow;
            VkDescriptorSet scene;
        } descriptorSets;
        
        std::unique_ptr<V2::VulkanUniformBuffer2> modelUniformBuffer;
        std::unique_ptr<V2::VulkanUniformBuffer2> backgroundUniformBuffer;
        std::unique_ptr<V2::VulkanUniformBuffer2> cameraUniformBuffer;
        std::unique_ptr<V2::VulkanBuffer2> instancePositionBuffer;
        
        std::unique_ptr<V2::VulkanBuffer2> LightMVPsBuffer;
        std::unique_ptr<V2::VulkanBuffer2> LightDataBuffer;

        float timer = 0.0f;
        float timerSpeed = 0.25f;

        bool init = false;

        bool bVisTexture = false;
        bool bPressed = false;
    };
}
