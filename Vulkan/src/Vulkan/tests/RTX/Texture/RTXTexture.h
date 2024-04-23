#pragma once

#include "Vulkan/Renderer/glTFModel.h"
#include "Vulkan/Renderer/Abstract/Camera.h"
#include "Vulkan/Renderer/V2/VulkanAccelerationStructure.h"
#include "Vulkan/Renderer/V2/VulkanBufferDeviceAddress.h"
#include "Vulkan/Renderer/V2/VulkanBufferStridedDeviceAddress.h"
#include "Vulkan/Renderer/V2/VulkanTexture.h"
#include "Vulkan/Renderer/V2/VulkanUniformBuffer2.h"
#include "Vulkan/tests/Test.h"

namespace test
{
    class RTXTexture : public Test
    {
    public:
        RTXTexture() = default;
        RTXTexture(VulkanCore* core);
        
        virtual ~RTXTexture();

        virtual void OnUpdate(float deltaTime) override;
        virtual void OnRender() override;
        virtual void OnImGuiRender() override;

        virtual void windowResized() override;
        void runBatchFile();
    private:
        std::unique_ptr<Camera> m_Camera;

        bool initSuccess = false;
        std::shared_ptr<V2::VulkanAccelerationStructure> pblAS;

        struct
        {
            VkDescriptorSetLayout DescriptorSetLayout;
            VkDescriptorPool DescriptorPool;
            VkDescriptorSet DescriptorSet;

            VkPipelineLayout PipelineLayout;
            VkPipeline Pipeline;
        } m_PostProcess;
    private:
        void prepareStructureProperties();
        void createBottomLevelAccelerationStructure();
        void createTopLevelAccelerationStructure();

        void loadTexture();

        void createUniformBuffer();

        void createRTXPipeline();
        void createPostProcessPipeline();
        void createShaderBindingTable();
        void createDescriptorSets();
        void buildCmdBuffer();

    private:
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_RayTracingPipelineProperties{};

    private:
        vks::Texture2D m_Texture;
        vkglTF::Model m_Scene;

        std::shared_ptr<V2::VulkanAccelerationStructure> m_SingleSquareBLAS;
        std::shared_ptr<V2::VulkanAccelerationStructure> m_sceneBLAS;
        
        std::shared_ptr<V2::VulkanAccelerationStructure> m_tlAS;

        std::unique_ptr<V2::VulkanBufferDeviceAddress> m_VertexBuffer;
        std::unique_ptr<V2::VulkanBufferDeviceAddress> m_IndexBuffer;

        std::unique_ptr<V2::VulkanBuffer2> m_VertexBufferDeviceAddressBuffer;
        std::unique_ptr<V2::VulkanBuffer2> m_IndexBufferDeviceAddressBuffer;

        struct UniformData
        {
            glm::mat4 InvProjection;
            glm::mat4 InvView;
        };
        
        std::unique_ptr<V2::VulkanUniformBuffer2> uniformBuffer;

        VkDescriptorSetLayout m_RTXDescriptorSetLayout;
        VkPipelineLayout m_RTXPipelineLayout;
        VkPipeline m_RTXPipeline;

        VkDescriptorPool m_RTXDescriptorPool;
        VkDescriptorSet m_RTXDescriptorSet;

        std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;

        std::unique_ptr<V2::VulkanBufferStridedDeviceAddress> raygenStridedBuffer;
        std::unique_ptr<V2::VulkanBufferStridedDeviceAddress> closestHitStridedBuffer;
        //std::unique_ptr<V2::VulkanBufferStridedDeviceAddress> anyHitStridedBuffer;
        std::unique_ptr<V2::VulkanBufferStridedDeviceAddress> missStridedBuffer;
    };
}