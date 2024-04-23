#pragma once
#include "glTFModel.h"
#include "V2/VulkanAccelerationStructure.h"

namespace V2
{
    class VulkanBufferDeviceAddress;
}

class VulkanCore;

class RTXBuilder
{
public:

    RTXBuilder() = default;

    RTXBuilder(VulkanCore* core);

    void Init(VulkanCore* core);

    std::shared_ptr<V2::VulkanAccelerationStructure> buildBLAS(const vkglTF::Model& model);

    std::shared_ptr<V2::VulkanAccelerationStructure> buildBLAS(V2::VulkanBufferDeviceAddress* vertexBuffer, VkDeviceSize vertexBufferStride, uint32_t maxVertex, V2::VulkanBufferDeviceAddress* indexBuffer, uint32_t indexCount, V2::VulkanBufferDeviceAddress* transformBuffer = nullptr);
    
    std::shared_ptr<V2::VulkanAccelerationStructure> buildBLAS(VkDeviceAddress vertexBufferDeviceAddress, VkDeviceSize vertexBufferStride, uint32_t maxVertex, VkDeviceAddress indexBufferDeviceAddress, uint32_t indexCount, VkDeviceAddress transformBufferDeviceAddress = 0);

    std::shared_ptr<V2::VulkanAccelerationStructure> buildTLAS(const std::vector<VkAccelerationStructureInstanceKHR>& Instances);
private:
    VulkanCore* m_Core = nullptr;
};
