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
    
    return buildBLAS(modelDeviceAddresses.Vertex, sizeof(vkglTF::Vertex), model.vertices.count, modelDeviceAddresses.Index, model.indices.count, 0);
}

std::shared_ptr<V2::VulkanAccelerationStructure> RTXBuilder::buildBLAS(V2::VulkanBufferDeviceAddress* vertexBuffer, VkDeviceSize vertexBufferStride, uint32_t maxVertex, V2::VulkanBufferDeviceAddress* indexBuffer, uint32_t indexCount, V2::VulkanBufferDeviceAddress* transformBuffer)
{
    VkDeviceAddress vertexBufferDeviceAddress = 0;
    VkDeviceAddress indexBufferDeviceAddress = 0;
    VkDeviceAddress transformBufferDeviceAddress = 0;

    if(vertexBuffer)
    {
        vertexBufferDeviceAddress = vertexBuffer->GetDeviceAddress();
    }
    if(indexBuffer)
    {
        indexBufferDeviceAddress = indexBuffer->GetDeviceAddress();
    }
    if(transformBuffer)
    {
        transformBufferDeviceAddress = transformBuffer->GetDeviceAddress();
    }

    return buildBLAS(vertexBufferDeviceAddress, vertexBufferStride, maxVertex, indexBufferDeviceAddress, indexCount, transformBufferDeviceAddress);
}

std::shared_ptr<V2::VulkanAccelerationStructure> RTXBuilder::buildBLAS(VkDeviceAddress vertexBufferDeviceAddress, VkDeviceSize vertexBufferStride, uint32_t maxVertex, VkDeviceAddress indexBufferDeviceAddress, uint32_t indexCount, VkDeviceAddress transformBufferDeviceAddress)
{
    uint32_t triangleCount = indexCount / 3;
    
    VkAccelerationStructureGeometryTrianglesDataKHR triangleData{};
    triangleData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    triangleData.vertexFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
    triangleData.vertexData.deviceAddress = vertexBufferDeviceAddress;
    triangleData.vertexStride = vertexBufferStride;
    triangleData.maxVertex = maxVertex;
    triangleData.indexType = VK_INDEX_TYPE_UINT32;
    triangleData.indexData.deviceAddress = indexBufferDeviceAddress;
    triangleData.transformData.deviceAddress = transformBufferDeviceAddress;

    VkAccelerationStructureGeometryKHR asGeometry{};
    asGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    asGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    asGeometry.geometry.triangles = triangleData;
    asGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
    buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    buildGeometryInfo.geometryCount = 1;
    buildGeometryInfo.pGeometries = &asGeometry;

    VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo{};
    buildSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

    vkPFN::vkGetAccelerationStructureBuildSizesKHR(m_Core->GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildGeometryInfo, &triangleCount, &buildSizeInfo);

    std::shared_ptr<V2::VulkanAccelerationStructure> blAS = std::make_shared<V2::VulkanAccelerationStructure>(m_Core, buildSizeInfo, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);

    V2::VulkanBufferDeviceAddress scratchBuffer(m_Core, buildSizeInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);

    buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildGeometryInfo.dstAccelerationStructure = blAS->GetHandle();
    buildGeometryInfo.scratchData.deviceAddress = scratchBuffer.GetDeviceAddress();

    VkAccelerationStructureBuildRangeInfoKHR rangeInfo{};
    rangeInfo.primitiveCount = triangleCount;
    rangeInfo.primitiveOffset = 0;
    rangeInfo.firstVertex = 0;
    rangeInfo.transformOffset = 0;
    
    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> rangeInfos{ &rangeInfo };
    
    VkCommandBuffer cmdBuffer = m_Core->createComputeCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    vkPFN::vkCmdBuildAccelerationStructuresKHR(cmdBuffer, 1, &buildGeometryInfo, rangeInfos.data());
    m_Core->flushComputeCommandBuffer(cmdBuffer, m_Core->queue.ComputeQueue, true);

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
