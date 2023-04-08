#pragma once
#include "Test.h"

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

        virtual void OnUpdate(float deltaTime) override;
        virtual void OnRender() override;
        virtual void OnImGuiRender() override;

    private:
        void prepareOffscreenFramebuffer();
        void prepareOffscreenRenderpass();
        void prepareUniformBuffers();
        void setupDescriptorSetLayout();
        void preparePipelines();
        void setupDescriptorPool();
        void setupDescriptorSets();
        void buildCommandBuffers();
        
        virtual void windowResized() override;

    private:
        Camera* m_Camera;
        
        std::unique_ptr<CubeObj> m_Cube;

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
    };
}
