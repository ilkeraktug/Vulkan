#pragma once
#include "Vulkan/Renderer/glTFModel.h"
#include "Vulkan/Renderer/Abstract/Camera.h"
#include "Vulkan/Renderer/V2/VulkanAccelerationStructure.h"
#include "Vulkan/tests/Test.h"

namespace test
{
    class RTXShadow : public Test
    {
    public:
        RTXShadow() = default;
        RTXShadow(VulkanCore* core);
        virtual ~RTXShadow();

        virtual void OnUpdate(float deltaTime) override;
        virtual void OnRender() override;
        virtual void OnImGuiRender() override;

        virtual void windowResized() override;
        void runBatchFile();
        
    private:
        bool initSuccess = false;
        std::unique_ptr<Camera> m_Camera;
        vkglTF::Model scene;
        vkglTF::Model scene2;

        VkBuffer raygenShaderBuffer;
        VkDeviceMemory raygenShaderMemory;
        VkStridedDeviceAddressRegionKHR raygenShaderDeviceAddressRegion;
        
        VkBuffer missShaderBuffer;
        VkDeviceMemory missShaderMemory;
        VkStridedDeviceAddressRegionKHR missShaderDeviceAddressRegion;
        
        VkBuffer cHitShaderBuffer;
        VkDeviceMemory cHitShaderMemory;
        VkStridedDeviceAddressRegionKHR cHitShaderDeviceAddressRegion;
        
        struct AccelerationStructure
        {
            VkBuffer Buffer;
            VkDeviceMemory DeviceMemory;
            VkAccelerationStructureKHR Handle;
            VkDeviceAddress DeviceAddress;
        };

        AccelerationStructure blAS;
        AccelerationStructure tlAS;

        struct ImageWrapper
        {
            VkImage Image;
            VkImageView ImageView;
            VkDeviceMemory Memory;
            VkSampler Sampler;
        };

        ImageWrapper m_OutputImage;

        struct UniformBufferData
        {
            glm::mat4 InverseProjection;
            glm::mat4 InverseView;
            glm::vec4 LightPosition;
        };
        
        struct UniformBuffer
        {
            VkBuffer Buffer;
            VkDeviceMemory Memory;
            void* Data;
        };

        UniformBuffer m_UniformBuffer;
        glm::vec4 m_LightPosition;

        float timer = 0.0f;
        float timerSpeed = 0.25f;

        VkDescriptorSetLayout m_RTXDescriptorSetLayout;
        VkPipelineLayout m_RTXPipelineLayout;
        VkPipeline m_RTXPipeline;

        VkDescriptorPool m_RTXDescriptorPool;
        VkDescriptorSet m_RTXDescriptorSet;

        std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroupCIs;

        VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_RayTracingPipelineProperties{};

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
        void createBottomLevelAccelerationStructure2();
        void createBottomLevelAccelerationStructureWithTransform();
        void createTopLevelAccelerationStructure();
        void createTopLevelAccelerationStructure2();
        
        void createStorageImage();
        void createUniformBuffer();
        void createRayTracingPipeline();
        void createPostProcessPipeline();
        void createShaderBindingTable();
        void createDescriptorSets();
        void buildCommandBuffers();

        //
        // void createAccelerationStructure(AccelerationStruct& as, VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
        // ScratchBuffer createScratchBuffer(VkDeviceSize size);
        // void deleteScratchBuffer(ScratchBuffer& scratchBuffer);

        void updateUniformBuffers();
        void updateLightPosition();

    private:

        std::shared_ptr<V2::VulkanAccelerationStructure> pblAS;
        std::shared_ptr<V2::VulkanAccelerationStructure> pblAS2;
        
        std::shared_ptr<V2::VulkanAccelerationStructure> ptlAS;
    };
}
