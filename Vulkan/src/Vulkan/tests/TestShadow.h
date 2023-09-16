#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Vulkan/Renderer/VulkangltfModel.h"

#include "Test.h"

namespace vkglTF
{
    class Model;
}

class VulkanUniformBuffer;
class VulkanVertexBuffer;
class CubeObj;
class Camera;

#define SHADOWMAP_DIM 1024
#define DEPTH_FORMAT VK_FORMAT_D16_UNORM

namespace test
{
    class TestShadow : public Test
    {
    public:
        TestShadow() = default;
        TestShadow(VulkanCore* core);
        virtual ~TestShadow();


        float timer = 0.0f;
        // Multiplier for speeding up (or slowing down) the global timer
        float timerSpeed = 0.25f;
        
        bool displayDebug = true;

        std::vector<vkglTF::Model> scenes;
        std::vector<std::string> sceneNames;
        int32_t sceneIndex = 0;
        
        virtual void OnUpdate(float deltaTime) override;
        virtual void OnRender() override;
        virtual void OnImGuiRender() override;

    private:

        void runBatchFile();
        void loadAssets();
        void prepareOffscreenFramebuffer();
        void prepareOffscreenRenderpass();
        void prepareUniformBuffers();
        void setupDescriptorSetLayout();
        void preparePipelines();
        void setupDescriptorPool();
        void setupDescriptorSets();
        void buildCommandBuffers();
        
        virtual void windowResized() override;

        void updateLight();
        void updateUniformBufferOffscreen();
        void updateUniformBuffers();

    private:
        static void onMouseScrollMoved(GLFWwindow*, double, double);
    public:
        std::unique_ptr<VulkanVertexBuffer> VertexBuffer;
        std::unique_ptr<VulkanVertexBuffer> OffscreenVertexBuffer;

        struct UniformBuffers
        {
            std::unique_ptr<VulkanUniformBuffer> SceneBuffer;
            std::unique_ptr<VulkanUniformBuffer> OffscreenBuffer;
            std::unique_ptr<VulkanUniformBuffer> ILKER;
        } m_UniformBuffers;
        
    private:

        bool displayShadowMap = true;
        
        Camera* m_Camera;

        static double cameraSpeed;

        struct
        {
            bool ctrl = false;
            bool shift = false;
            bool alt = false;
        } keys;
        
        std::unique_ptr<CubeObj> m_Cube;

        VkPipelineCache m_PipelineCache;
        
        VkDescriptorPool m_DescriptorPool;
        VkDescriptorSetLayout m_DescriptorSetLayout;

        bool initSuccess = false;

        float zNear = 1.0f;
        float zFar = 96.0f;

        // Depth bias (and slope) are used to avoid shadowing artifacts
        // Constant depth bias factor (always applied)
        float depthBiasConstant = 1.25f;
        // Slope depth bias factor, applied depending on polygon's slope
        float depthBiasSlope = 1.75f;
        
        glm::vec3 lightPos = glm::vec3();
        float lightFOV = 45.0f;
        
        VkClearColorValue defaultClearColor = { { 0.025f, 0.025f, 0.025f, 1.0f } };

        struct
        {
            bool left = false;
            bool middle = false;
            bool right = false;
        } mouseButtons;

        struct
        {
            double x = 0.0f ,y = 0.0f;
        } mousePos;
        
    private:
        
        struct FrameBufferAttachment {
            VkImage Image;
            VkDeviceMemory Memory;
            VkImageView View;
        };
        
        struct OffscreenPass {
            int32_t width, height;
            VkFramebuffer frameBuffer;
            FrameBufferAttachment depth;
            VkRenderPass renderPass;
            VkSampler depthSampler;
            VkDescriptorImageInfo descriptor;
        } offscreenPass;

        struct {
            VkPipeline Offscreen;
            VkPipeline SceneShadow;
            VkPipeline SceneShadowPCF;
            VkPipeline Debug;
        } m_Pipelines;

        struct {
            VkDescriptorSet offscreen;
            VkDescriptorSet scene;
            VkDescriptorSet debug;
            VkDescriptorSet ILKER;
        } descriptorSets;

        VkPipelineLayout pipelineLayout;

        struct SceneUniformBuffer {
            glm::mat4 projection;
            glm::mat4 view;
            glm::mat4 model;
            glm::mat4 depthBiasMVP;
            glm::vec4 lightPos;
            // Used for depth map visualization
            float zNear;
            float zFar;
        };
        
        SceneUniformBuffer uboVSscene;
        SceneUniformBuffer uboVSILKER;
        
        struct {
            glm::mat4 depthMVP;
        } uboOffscreenVS;
    };
}
