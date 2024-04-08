#pragma once
#include "glTFModel.h"
#include "V2/VulkanAccelerationStructure.h"

class VulkanCore;

class RTXBuilder
{
public:

    RTXBuilder() = default;

    RTXBuilder(VulkanCore* core);

    void Init(VulkanCore* core);

    std::shared_ptr<V2::VulkanAccelerationStructure> buildBLAS(const vkglTF::Model& model);

    std::shared_ptr<V2::VulkanAccelerationStructure> buildTLAS(const std::vector<VkAccelerationStructureInstanceKHR>& Instances);
private:
    VulkanCore* m_Core = nullptr;
};
