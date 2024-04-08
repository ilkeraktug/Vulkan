#pragma once
#include "Vulkan/Renderer/glTFModel.h"
#include "Vulkan/tests/Test.h"

class Camera;

namespace test
{
    class RTXReflection : public Test
    {
    public:
        RTXReflection() = default;
        RTXReflection(VulkanCore* core);
        virtual ~RTXReflection();

        virtual void OnUpdate(float deltaTime) override;
        virtual void OnRender() override;
        virtual void OnImGuiRender() override;

        virtual void windowResized() override;
        void runBatchFile();

        uint32_t getAlignedSize(uint32_t size, uint32_t alignment);

    private:

        std::unique_ptr<Camera> m_Camera;
        vkglTF::Model scene;

        VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_RayTracingPipelineProperties{};
        VkPhysicalDeviceAccelerationStructurePropertiesKHR m_AccelerationStructureProperties{};
        
        struct AccelerationStruct
        {
            VkAccelerationStructureKHR Handle;
            VkDeviceAddress DeviceAddress;

            VkBuffer Buffer;
            VkDeviceMemory Memory;
        };

        struct ScratchBuffer
        {
            // VkDeviceAddress DeviceAddress;
            VkDeviceMemory Memory;
            VkBuffer Buffer;
            VkDeviceAddress DeviceAddress;
        };

        AccelerationStruct blAs;
        AccelerationStruct tlAs;

        struct StorageImage
        {
            VkImage Image;
            VkImageView ImageView;
            VkDeviceMemory Memory;
            VkSampler Sampler;
        } m_StorageImage;

        struct UniformData {
            glm::mat4 viewInverse;
            glm::mat4 projInverse;
            glm::vec4 lightPos;
            int32_t vertexSize;
        } m_UniformData;
        
        struct UniformBuffer
        {
            VkBuffer Buffer;
            VkDeviceMemory Memory;
            void* data = nullptr;
        } m_UniformBuffer;

        struct ShaderBindingTable
        {
            VkBuffer Buffer;
            VkDeviceMemory Memory;
            VkStridedDeviceAddressRegionKHR StridedDeviceAdress;
            void* data;
        };

        ShaderBindingTable m_Raygen;
        ShaderBindingTable m_Miss;
        ShaderBindingTable m_Hit;

        VkDescriptorSetLayout rtxDescriptorSetLayout;
        VkPipelineLayout rtxPipelineLayout;
        VkPipeline rtxPipeline;

        VkDescriptorPool descriptorPool;
        VkDescriptorSet descriptorSet;

        std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;


        VkDescriptorSet m_PostProcessDescriptorSet;
        VkDescriptorPool m_PostProcessDescriptorPool;
        VkDescriptorSetLayout m_PostProcessDescriptorSetLayout;
        VkPipelineLayout m_PostProcessPipelineLayout;
        VkPipeline m_PostProcessGraphicsPipeline;
        
        float timer = 0.0f;
        // Multiplier for speeding up (or slowing down) the global timer
        float timerSpeed = 0.25f;

        bool initSuccess = false;
        
    private:
        void prepareStructureProperties();
        void createBottomLevelAccelerationStructure();
        void createBottomLevelAccelerationStructureWithTransform();
        void createTopLevelAccelerationStructure();
        
        void createStorageImage();
        void createUniformBuffer();
        void createRayTracingPipeline();
        void createPostProcessPipeline();
        void createShaderBindingTable();
        void createDescriptorSets();
        void buildCommandBuffers();

        void createAccelerationStructure(AccelerationStruct& as, VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
        ScratchBuffer createScratchBuffer(VkDeviceSize size);
        void deleteScratchBuffer(ScratchBuffer& scratchBuffer);

        void updateUniformBuffers();
        void updateLightPosition();

        VkDeviceAddress getBufferDeviceAddress(VkDevice device, VkBuffer buffer);

    };
}
