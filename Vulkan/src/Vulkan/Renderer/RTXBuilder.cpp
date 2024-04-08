#include "pch.h"
#include "RTXBuilder.h"

#include "VulkanFunctions.h"
#include "V2/VulkanAccelerationStructure.h"
#include "V2/VulkanBufferDeviceAddress.h"

RTXBuilder::RTXBuilder(VulkanCore* core)
    :m_Core(core)
{
}

void RTXBuilder::Init(VulkanCore* core)
{
    m_Core = core;
}

std::shared_ptr<V2::VulkanAccelerationStructure> RTXBuilder::buildBLAS(const vkglTF::Model& model)
{
    struct
    {
        VkDeviceAddress Vertex;
        VkDeviceAddress Index;
    } modelDeviceAddresses{};

    VkBufferDeviceAddressInfo deviceAddressInfo{};
    deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    deviceAddressInfo.buffer = model.vertices.buffer;
    
    modelDeviceAddresses.Vertex = vkGetBufferDeviceAddress(m_Core->GetDevice(), &deviceAddressInfo);

    deviceAddressInfo.buffer = model.indices.buffer;
    modelDeviceAddresses.Index = vkGetBufferDeviceAddress(m_Core->GetDevice(), &deviceAddressInfo);

    uint32_t triangleCount = model.indices.count / 3;
    
    VkAccelerationStructureGeometryTrianglesDataKHR triangleData{};
    triangleData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    triangleData.vertexFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
    triangleData.vertexData.deviceAddress = modelDeviceAddresses.Vertex;
    triangleData.vertexStride = sizeof(vkglTF::Vertex);
    triangleData.maxVertex = model.vertices.count;
    triangleData.indexType = VK_INDEX_TYPE_UINT32;
    triangleData.indexData.deviceAddress = modelDeviceAddresses.Index;

    VkAccelerationStructureGeometryKHR geometryInfo{};
    geometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometryInfo.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometryInfo.geometry.triangles = triangleData;
    geometryInfo.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
    buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    buildGeometryInfo.geometryCount = 1;
    buildGeometryInfo.pGeometries = &geometryInfo;

    VkAccelerationStructureBuildSizesInfoKHR asBuildSizeInfo{};
    asBuildSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    
    vkPFN::vkGetAccelerationStructureBuildSizesKHR(m_Core->GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildGeometryInfo, &triangleCount, &asBuildSizeInfo);

    V2::VulkanBufferDeviceAddress scratchBuffer(m_Core,
        asBuildSizeInfo.buildScratchSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    std::shared_ptr<V2::VulkanAccelerationStructure> blAS = std::make_shared<V2::VulkanAccelerationStructure>(m_Core, asBuildSizeInfo, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);
    
    buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildGeometryInfo.dstAccelerationStructure = blAS->GetHandle();
    buildGeometryInfo.scratchData.deviceAddress = scratchBuffer.GetDeviceAddress();

    VkAccelerationStructureBuildRangeInfoKHR asBuildRangeInfo{};
    asBuildRangeInfo.primitiveCount = triangleCount;
    asBuildRangeInfo.primitiveOffset = 0;
    asBuildRangeInfo.firstVertex = 0;
    asBuildRangeInfo.transformOffset = 0;

    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> buildRangeInfos{ &asBuildRangeInfo };

    VkCommandBuffer cmdBuffer = m_Core->createComputeCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    
    vkPFN::vkCmdBuildAccelerationStructuresKHR(cmdBuffer, 1, &buildGeometryInfo, buildRangeInfos.data());
    m_Core->flushComputeCommandBuffer(cmdBuffer, m_Core->queue.ComputeQueue);
    
    return blAS;
}

std::shared_ptr<V2::VulkanAccelerationStructure> RTXBuilder::buildTLAS(const std::vector<VkAccelerationStructureInstanceKHR>& Instances)
{
    static_assert(sizeof(std::decay<decltype(*Instances.data())>::type) == sizeof(VkAccelerationStructureInstanceKHR),"");

    V2::VulkanBufferDeviceAddress instancesBuffer(m_Core,
        sizeof(VkAccelerationStructureInstanceKHR) * Instances.size(),
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, const_cast<VkAccelerationStructureInstanceKHR*>(Instances.data()));

    VkAccelerationStructureGeometryInstancesDataKHR instancesData{};
    instancesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    instancesData.arrayOfPointers = VK_FALSE;
    instancesData.data.deviceAddress = instancesBuffer.GetDeviceAddress();
    
    VkAccelerationStructureGeometryKHR asGeometry{};
    asGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    asGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    asGeometry.geometry.instances = instancesData;
    asGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR asBuildGeometryInfo{};
    asBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    asBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    //asBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    asBuildGeometryInfo.geometryCount = 1;
    asBuildGeometryInfo.pGeometries = &asGeometry;

    VkAccelerationStructureBuildSizesInfoKHR asBuildSizeInfo{};
    asBuildSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

    uint32_t primitiveCount = static_cast<uint32_t>(Instances.size());
    vkPFN::vkGetAccelerationStructureBuildSizesKHR(m_Core->GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &asBuildGeometryInfo, &primitiveCount, &asBuildSizeInfo);

    V2::VulkanBufferDeviceAddress scratchBuffer(m_Core,
        asBuildSizeInfo.buildScratchSize,
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    std::shared_ptr<V2::VulkanAccelerationStructure> tlAS = std::make_unique<V2::VulkanAccelerationStructure>(m_Core, asBuildSizeInfo, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
    
    asBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    asBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    asBuildGeometryInfo.dstAccelerationStructure = tlAS->GetHandle();
    asBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.GetDeviceAddress();

    VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
    buildRangeInfo.primitiveCount = static_cast<uint32_t>(Instances.size());
    buildRangeInfo.primitiveOffset = 0;
    buildRangeInfo.firstVertex = 0;
    buildRangeInfo.transformOffset = 0;

    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> buildRangeInfos { &buildRangeInfo };
    
    VkCommandBuffer cmdBuffer = m_Core->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_Core->resources.computeCommandPool, true);
    vkPFN::vkCmdBuildAccelerationStructuresKHR(cmdBuffer, 1, &asBuildGeometryInfo, buildRangeInfos.data());
    m_Core->flushComputeCommandBuffer(cmdBuffer, m_Core->queue.ComputeQueue, true);

    return tlAS;
}
