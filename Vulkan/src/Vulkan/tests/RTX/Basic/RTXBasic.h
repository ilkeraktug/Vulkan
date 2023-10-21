#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Vulkan/Renderer/VulkanBuffer.h"
#include "Vulkan/Renderer/Abstract/VK_VulkanBuffer.h"
#include "Vulkan/tests/Test.h"

class Camera;

struct RayTracingScratchBuffer
{
    uint64_t deviceAddress = 0;
    VkBuffer handle = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
};

// Ray tracing acceleration structure
struct AccelerationStructure {
    VkAccelerationStructureKHR handle;
    uint64_t deviceAddress = 0;
    VkDeviceMemory memory;
    VkBuffer buffer;
};

namespace test
{
    class RTXBasic : public Test
    {
    public:
        RTXBasic() = default;
        RTXBasic(VulkanCore* core);
        ~RTXBasic();

        virtual void OnUpdate(float deltaTime) override;
        virtual void OnRender() override;
        virtual void OnImGuiRender() override;

        virtual void windowResized() override;
        void runBatchFile();

        uint64_t getBufferDeviceAddress(VkBuffer buffer)
        {
            VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
            bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            bufferDeviceAI.buffer = buffer;
            return vkGetBufferDeviceAddressKHR(m_Core->GetDevice(), &bufferDeviceAI);
        }

        void createAccelerationStructureBuffer(AccelerationStructure &accelerationStructure, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
        {
            VkBufferCreateInfo bufferCreateInfo{};
            bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferCreateInfo.size = buildSizeInfo.accelerationStructureSize;
            bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
            VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &bufferCreateInfo, nullptr, &accelerationStructure.buffer));
            VkMemoryRequirements memoryRequirements{};
            vkGetBufferMemoryRequirements(m_Core->GetDevice(), accelerationStructure.buffer, &memoryRequirements);
            VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
            memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
            memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
            VkMemoryAllocateInfo memoryAllocateInfo{};
            memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
            memoryAllocateInfo.allocationSize = memoryRequirements.size;
            memoryAllocateInfo.memoryTypeIndex = m_Core->getMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &memoryAllocateInfo, nullptr, &accelerationStructure.memory));
            VK_CHECK(vkBindBufferMemory(m_Core->GetDevice(), accelerationStructure.buffer, accelerationStructure.memory, 0));
        }

        RayTracingScratchBuffer createScratchBuffer(VkDeviceSize size)
        {
            RayTracingScratchBuffer scratchBuffer{};

            VkBufferCreateInfo bufferCreateInfo = init::createBufferInfo(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
            VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &bufferCreateInfo, nullptr, &scratchBuffer.handle));

            VkMemoryRequirements memoryRequirements{};
            vkGetBufferMemoryRequirements(*m_Core, scratchBuffer.handle, &memoryRequirements);

            VkMemoryAllocateFlagsInfo allocateFlagsInfo{};
            allocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
            allocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
            
            VkMemoryAllocateInfo memoryAllocationInfo = init::memAllocInfo();
            memoryAllocationInfo.pNext = &allocateFlagsInfo;
            memoryAllocationInfo.allocationSize = memoryRequirements.size;
            memoryAllocationInfo.memoryTypeIndex = m_Core->getMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            VK_CHECK(vkAllocateMemory(*m_Core, &memoryAllocationInfo, nullptr, &scratchBuffer.memory));
            VK_CHECK(vkBindBufferMemory(*m_Core, scratchBuffer.handle, scratchBuffer.memory, 0));

            VkBufferDeviceAddressInfo bufferDeviceAddressInfo{};
            bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            bufferDeviceAddressInfo.buffer = scratchBuffer.handle;

            scratchBuffer.deviceAddress = vkGetBufferDeviceAddressKHR(*m_Core, &bufferDeviceAddressInfo);

            return scratchBuffer;
        }

        void deleteScratchBuffer(RayTracingScratchBuffer& scratchBuffer)
        {
            if(scratchBuffer.handle != VK_NULL_HANDLE)
            {
                vkDestroyBuffer(*m_Core, scratchBuffer.handle, nullptr);
            }
            if(scratchBuffer.memory != VK_NULL_HANDLE)
            {
                vkFreeMemory(*m_Core, scratchBuffer.memory, nullptr);
            }
        }
        
        uint32_t alignedSize(uint32_t value, uint32_t alignment)
        {
            return (value + alignment - 1) & ~(alignment - 1);
        }

        size_t alignedSize(size_t value, size_t alignment)
        {
            return (value + alignment - 1) & ~(alignment - 1);
        }

        Camera* m_Camera;

        void prepareStructureProperties();
        void createBottomLevelAccelerationStructure();
        void createTopLevelAccelerationStructure();
        
        void createStorageImage();
        void createUniformBuffer();
        void createRayTracingPipeline();
        void createShaderBindingTable();
        void createDescriptorSets();
        void buildCommandBuffers();

        void updateUniformBuffers();
        
        PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
        PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
        PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
        PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
        PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
        PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
        PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
        PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
        PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
        PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;

        VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties{};
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};

        AccelerationStructure bottomLevelAS{};
        AccelerationStructure topLevelAS{};
        
        VulkanBuffer vertexBuffer;
        VulkanBuffer indexBuffer;
        VulkanBuffer indexBuffer2;
        uint32_t indexCount;
        uint32_t indexCount2;
        VulkanBuffer transformBuffer;
        std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
        VulkanBuffer raygenShaderBindingTable;
        VulkanBuffer missShaderBindingTable;
        VulkanBuffer hitShaderBindingTable;

        struct StorageImage {
            VkDeviceMemory memory;
            VkImage image;
            VkImageView view;
            VkFormat format;
        } storageImage;

        struct UniformData {
            glm::mat4 viewInverse;
            glm::mat4 projInverse;
        } uniformData;
        VulkanBuffer ubo;
        
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
        VkDescriptorSet descriptorSet;
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorPool descriptorPool;

        bool initSuccess = false;

        static double cameraSpeed;

        struct
        {
            bool ctrl = false;
            bool shift = false;
            bool alt = false;
        } keys;
        
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

        static void onMouseScrollMoved(GLFWwindow* window, double dX, double dY);
    };
}
