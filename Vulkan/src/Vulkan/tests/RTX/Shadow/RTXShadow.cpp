#include "pch.h"
#include "RTXShadow.h"

#include "Vulkan/Core/tools.h"
#include "Vulkan/Math/Transform.h"
#include "Vulkan/Renderer/glTFModel.h"
#include "Vulkan/Renderer/PerspectiveCamera.h"
#include "Vulkan/Renderer/RTXBuilder.h"
#include "Vulkan/Renderer/VulkanFunctions.h"
#include "Vulkan/Renderer/VulkanShader.h"

static uint32_t getAlignedSize(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

test::RTXShadow::RTXShadow(VulkanCore* core)
{
    Init(core);
    m_Camera = std::make_unique<PerspectiveCamera>(m_Core->swapchain.extent.width, m_Core->swapchain.extent.height, core);
    m_Camera->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
    m_Camera->setPosition(glm::vec3(0.0f, 0.5f, -2.0f));
    runBatchFile();
    
    prepareStructureProperties();
    // createBottomLevelAccelerationStructure();
    // createTopLevelAccelerationStructure();
    createBottomLevelAccelerationStructure2();
    createTopLevelAccelerationStructure2();
        
    createStorageImage();
    createUniformBuffer();
    createRayTracingPipeline();
    createShaderBindingTable();
    createDescriptorSets();
    createPostProcessPipeline();
    buildCommandBuffers();
    initSuccess = true;
}

test::RTXShadow::~RTXShadow()
{
}

void test::RTXShadow::OnUpdate(float deltaTime)
{
    m_Camera->OnUpdate(deltaTime);
    timer += deltaTime * timerSpeed;

    if(timer > 1)
    {
        timer = 0.0f;
    }

    updateLightPosition();
    updateUniformBuffers();
}

void test::RTXShadow::OnRender()
{
    if(!initSuccess)
    {
        return;
    }
		
    m_Core->BeginScene();

    m_Core->resources.submitInfo.commandBufferCount = 1;
    m_Core->resources.submitInfo.pCommandBuffers = &m_Core->resources.drawCmdBuffers[m_Core->resources.imageIndex];

    buildCommandBuffers();

    VK_CHECK(vkQueueSubmit(m_Core->queue.GraphicsQueue, 1, &m_Core->resources.submitInfo, VK_NULL_HANDLE));

    VkResult err = m_Core->Submit();
		
    //TODO : Fences and Semaphores !
    vkDeviceWaitIdle(m_Core->GetDevice());
}

void test::RTXShadow::OnImGuiRender()
{
}

void test::RTXShadow::windowResized()
{
    Test::windowResized();
}

void test::RTXShadow::runBatchFile()
{
    std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingshadow & for /r %i in (*.vert) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tvs_6_1 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo C:%~pi%~ni.vspv)");
    std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingshadow & for /r %i in (*.frag) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tps_6_1 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo C:%~pi%~ni.fspv)");
    std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingshadow & for %i in (*.rchit) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tlib_6_3 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo %~ni.rchit.spv)");
    std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingshadow & for %i in (*.rmiss) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tlib_6_3 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo %~ni.rmiss.spv)");
    std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingshadow & for %i in (*.rgen) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tlib_6_3 -Emain   -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing  -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo %~ni.rgen.spv)");
}

void test::RTXShadow::prepareStructureProperties()
{
    m_RayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

    VkPhysicalDeviceProperties2 physicalDeviceProperties2{}; 
    physicalDeviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    physicalDeviceProperties2.pNext = &m_RayTracingPipelineProperties;
    
    vkGetPhysicalDeviceProperties2(m_Core->GetPhysicalDevice(), &physicalDeviceProperties2);
}

void test::RTXShadow::createBottomLevelAccelerationStructure()
{
    vkglTF::memoryPropertyFlags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::FlipY | vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors;

    scene.loadFromFile("C:/dev/Vulkan/Vulkan/assets/models/vulkanscene_shadow.gltf", m_Core, m_Core->queue.TransferQueue, glTFLoadingFlags);

    VkDeviceOrHostAddressConstKHR vertexDataAddress{};
    VkDeviceOrHostAddressConstKHR indexDataAddress{};

    VkBufferDeviceAddressInfo deviceAddressInfo{};
    deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    deviceAddressInfo.buffer = scene.vertices.buffer;
    vertexDataAddress.deviceAddress = vkGetBufferDeviceAddress(m_Core->GetDevice(), &deviceAddressInfo);
    
    deviceAddressInfo.buffer = scene.indices.buffer;
    indexDataAddress.deviceAddress = vkGetBufferDeviceAddress(m_Core->GetDevice(), &deviceAddressInfo);

    int sceneVertexCount = scene.vertices.count;
    int sceneIndexCount = scene.indices.count;
    uint32_t maxPrimitiveCount = scene.indices.count / 3;

    VkAccelerationStructureGeometryKHR blasGeometry{};
    blasGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    blasGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    blasGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    blasGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    blasGeometry.geometry.triangles.vertexData = vertexDataAddress;
    blasGeometry.geometry.triangles.vertexStride = sizeof(vkglTF::Vertex);
    blasGeometry.geometry.triangles.maxVertex = sceneVertexCount;
    blasGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    blasGeometry.geometry.triangles.indexData = indexDataAddress;
    blasGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;


    VkAccelerationStructureBuildGeometryInfoKHR blasBuildGeometryInfo{};
    blasBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    blasBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    blasBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    //blasBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    blasBuildGeometryInfo.geometryCount = 1;
    blasBuildGeometryInfo.pGeometries = &blasGeometry;

    VkAccelerationStructureBuildSizesInfoKHR blasBuildSizeInfo{};
    blasBuildSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

    vkPFN::vkGetAccelerationStructureBuildSizesKHR(m_Core->GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &blasBuildGeometryInfo, &maxPrimitiveCount, &blasBuildSizeInfo);

    VkBufferCreateInfo blasBufferCI = init::createBufferInfo(blasBuildSizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

    VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &blasBufferCI, nullptr, &blAS.Buffer));

    VkMemoryRequirements blasMemRequirements{};
    vkGetBufferMemoryRequirements(m_Core->GetDevice(), blAS.Buffer, &blasMemRequirements);
    
    VkMemoryAllocateFlagsInfo deviceAddressBitAllocFlagsInfo{};
    deviceAddressBitAllocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    deviceAddressBitAllocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    
    VkMemoryAllocateInfo blasMemAllocInfo = init::memAllocInfo();
    blasMemAllocInfo.pNext = &deviceAddressBitAllocFlagsInfo;
    blasMemAllocInfo.allocationSize = blasMemRequirements.size;
    blasMemAllocInfo.memoryTypeIndex = m_Core->getMemoryType(blasMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &blasMemAllocInfo, nullptr, &blAS.DeviceMemory));

    VK_CHECK(vkBindBufferMemory(m_Core->GetDevice(), blAS.Buffer, blAS.DeviceMemory, 0));
    
    VkBufferDeviceAddressInfo blasBufferAddressInfo{};
    blasBufferAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    blasBufferAddressInfo.buffer = blAS.Buffer;
    
    blAS.DeviceAddress = vkGetBufferDeviceAddress(m_Core->GetDevice(), &blasBufferAddressInfo);
    
    VkAccelerationStructureCreateInfoKHR blasCI{};
    blasCI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    blasCI.buffer = blAS.Buffer;
    blasCI.offset = 0;
    blasCI.size = blasBuildSizeInfo.accelerationStructureSize;
    blasCI.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    //blasCI.deviceAddress = blasBufferDeviceAddress;
    
    VK_CHECK(vkPFN::vkCreateAccelerationStructureKHR(m_Core->GetDevice(), &blasCI, nullptr, &blAS.Handle));
    
    VkBufferCreateInfo scratchBufferCI = init::createBufferInfo(blasBuildSizeInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

    VkBuffer scratchBuffer;
    VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &scratchBufferCI, nullptr, &scratchBuffer));

    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements(m_Core->GetDevice(), scratchBuffer, &memRequirements);
    
    VkMemoryAllocateInfo memAllocInfo = init::memAllocInfo();
    memAllocInfo.pNext = &deviceAddressBitAllocFlagsInfo;
    memAllocInfo.allocationSize = memRequirements.size;
    memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceMemory scracthBufferMem;
    VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &memAllocInfo, nullptr, &scracthBufferMem));

    VK_CHECK(vkBindBufferMemory(m_Core->GetDevice(), scratchBuffer, scracthBufferMem, 0));
    
    VkBufferDeviceAddressInfo scratchBufferDeviceAddressInfo{};
    scratchBufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    scratchBufferDeviceAddressInfo.buffer = scratchBuffer;

    VkDeviceOrHostAddressKHR scratchBufferDeviceAddress{};
    
    scratchBufferDeviceAddress.deviceAddress = vkGetBufferDeviceAddress(m_Core->GetDevice(), &scratchBufferDeviceAddressInfo);
    
    VkAccelerationStructureBuildGeometryInfoKHR blasBuildGeometryInfo2{};
    blasBuildGeometryInfo2.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    blasBuildGeometryInfo2.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    blasBuildGeometryInfo2.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    blasBuildGeometryInfo2.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    blasBuildGeometryInfo2.dstAccelerationStructure = blAS.Handle;
    blasBuildGeometryInfo2.geometryCount = 1;
    blasBuildGeometryInfo2.pGeometries = &blasGeometry;
    blasBuildGeometryInfo2.scratchData = scratchBufferDeviceAddress;

    VkAccelerationStructureBuildRangeInfoKHR blasBuildRangeInfo{};
    blasBuildRangeInfo.primitiveCount = maxPrimitiveCount;
    blasBuildRangeInfo.primitiveOffset = 0;
    blasBuildRangeInfo.firstVertex = 0;
    blasBuildRangeInfo.transformOffset = 0;

    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &blasBuildRangeInfo };
    
    auto cmdBuffer = m_Core->createComputeCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    vkPFN::vkCmdBuildAccelerationStructuresKHR(cmdBuffer, 1, &blasBuildGeometryInfo2, accelerationBuildStructureRangeInfos.data());
    m_Core->flushComputeCommandBuffer(cmdBuffer, m_Core->queue.ComputeQueue);
}

void test::RTXShadow::createBottomLevelAccelerationStructure2()
{
    vkglTF::memoryPropertyFlags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::FlipY | vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors;

    scene.loadFromFile("C:/dev/Vulkan/Vulkan/assets/models/vulkanscene_shadow.gltf", m_Core, m_Core->queue.TransferQueue, glTFLoadingFlags);
    scene2.loadFromFile("C:/dev/Vulkan/Vulkan/assets/models/shadowscene_fire.gltf", m_Core, m_Core->queue.TransferQueue, glTFLoadingFlags);
    
    pblAS = m_Core->GetRTXBuilder()->buildBLAS(scene);
    pblAS2 = m_Core->GetRTXBuilder()->buildBLAS(scene2);
}

void test::RTXShadow::createBottomLevelAccelerationStructureWithTransform()
{
}

void test::RTXShadow::createTopLevelAccelerationStructure()
{
    VkTransformMatrixKHR tlasInstanceTransform =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f
    };
    
    VkAccelerationStructureInstanceKHR tlasInstance{};
    tlasInstance.transform = tlasInstanceTransform;
    tlasInstance.instanceCustomIndex = 0;
    tlasInstance.mask = 0xFF;
    tlasInstance.instanceShaderBindingTableRecordOffset = 0;
    tlasInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
    tlasInstance.accelerationStructureReference = blAS.DeviceAddress;

    VkBuffer tlasInstanceBuffer;
    VkDeviceMemory tlasInstanceBufferDeviceMemory;

    VkBufferCreateInfo tlasInstanceBufferCI = init::createBufferInfo(
        sizeof(VkAccelerationStructureInstanceKHR),
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

    VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &tlasInstanceBufferCI, nullptr, &tlasInstanceBuffer));
    
    VkMemoryRequirements tlasInstanceBufferMemRequirements{};
    vkGetBufferMemoryRequirements(m_Core->GetDevice(), tlasInstanceBuffer, &tlasInstanceBufferMemRequirements);

    VkMemoryAllocateFlagsInfo tlasInstanceBufferMemAllocFlagInfo{};
    tlasInstanceBufferMemAllocFlagInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    tlasInstanceBufferMemAllocFlagInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

    VkMemoryAllocateInfo tlasInstanceBufferMemAllocInfo = init::memAllocInfo();
    tlasInstanceBufferMemAllocInfo.pNext = &tlasInstanceBufferMemAllocFlagInfo;
    tlasInstanceBufferMemAllocInfo.allocationSize = tlasInstanceBufferMemRequirements.size;
    tlasInstanceBufferMemAllocInfo.memoryTypeIndex = m_Core->getMemoryType(tlasInstanceBufferMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &tlasInstanceBufferMemAllocInfo, nullptr, &tlasInstanceBufferDeviceMemory));

    VK_CHECK(vkBindBufferMemory(m_Core->GetDevice(), tlasInstanceBuffer, tlasInstanceBufferDeviceMemory, 0));

    void* mappedMemory = nullptr;

    VK_CHECK(vkMapMemory(m_Core->GetDevice(), tlasInstanceBufferDeviceMemory, 0, sizeof(VkAccelerationStructureInstanceKHR), 0, &mappedMemory));

    if(mappedMemory)
    {
        memcpy(mappedMemory, &tlasInstance, sizeof(tlasInstance));
    }

    vkUnmapMemory(m_Core->GetDevice(), tlasInstanceBufferDeviceMemory);

    VkBufferDeviceAddressInfo tlasInstanceDeviceAddressInfo{};
    tlasInstanceDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    tlasInstanceDeviceAddressInfo.buffer = tlasInstanceBuffer;
    
    VkDeviceOrHostAddressConstKHR tlasInstanceBufferDeviceHostAddress{};
    tlasInstanceBufferDeviceHostAddress.deviceAddress = vkGetBufferDeviceAddress(m_Core->GetDevice(), &tlasInstanceDeviceAddressInfo);
    
    VkAccelerationStructureGeometryKHR tlasGeometry{};
    tlasGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    tlasGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    tlasGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    tlasGeometry.geometry.instances.data = tlasInstanceBufferDeviceHostAddress;
    tlasGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
    tlasGeometry.geometry.instances.arrayOfPointers = VK_FALSE;

    VkAccelerationStructureBuildGeometryInfoKHR tlasBuildGeometryInfo{};
    tlasBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    tlasBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    tlasBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    tlasBuildGeometryInfo.geometryCount = 1;
    tlasBuildGeometryInfo.pGeometries = &tlasGeometry;
    
    VkAccelerationStructureBuildSizesInfoKHR tlasBuildSizeInfo{};
    tlasBuildSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

    uint32_t numTriangle = scene.vertices.count / 3;
    uint32_t numPrimitive = 1;

    vkPFN::vkGetAccelerationStructureBuildSizesKHR(m_Core->GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &tlasBuildGeometryInfo, &numPrimitive, &tlasBuildSizeInfo);

    VkBufferCreateInfo tlasBufferCI = init::createBufferInfo(tlasBuildSizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
    VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &tlasBufferCI, nullptr, &tlAS.Buffer));

    VkMemoryRequirements tlasBufferMemReqs{};
    vkGetBufferMemoryRequirements(m_Core->GetDevice(), tlAS.Buffer, &tlasBufferMemReqs);
    
    VkMemoryAllocateFlagsInfo tlasMemAllocFlagsInfo{};
    tlasMemAllocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    tlasMemAllocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    VkMemoryAllocateInfo tlasMemAllocInfo = init::memAllocInfo();
    tlasMemAllocInfo.pNext = &tlasMemAllocFlagsInfo;
    tlasMemAllocInfo.allocationSize = tlasBufferMemReqs.size;
    tlasMemAllocInfo.memoryTypeIndex = m_Core->getMemoryType(tlasBufferMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &tlasMemAllocInfo, nullptr, &tlAS.DeviceMemory));

    VK_CHECK(vkBindBufferMemory(m_Core->GetDevice(), tlAS.Buffer, tlAS.DeviceMemory, 0));
    
    VkAccelerationStructureCreateInfoKHR tlasCreateInfo{};
    tlasCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    tlasCreateInfo.buffer = tlAS.Buffer;
    tlasCreateInfo.offset = 0;
    tlasCreateInfo.size = tlasBuildSizeInfo.accelerationStructureSize;
    tlasCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

    VK_CHECK(vkPFN::vkCreateAccelerationStructureKHR(m_Core->GetDevice(), &tlasCreateInfo, nullptr, &tlAS.Handle));
    
    VkBufferCreateInfo scratchBufferCI = init::createBufferInfo(tlasBuildSizeInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
    VkBuffer scratchBuffer;

    VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &scratchBufferCI, nullptr, &scratchBuffer));

    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements(m_Core->GetDevice(), scratchBuffer, &memRequirements);
    
    VkMemoryAllocateFlagsInfo memAllocFlagsInfo{};
    memAllocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memAllocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    VkMemoryAllocateInfo memAllocInfo = init::memAllocInfo();
    memAllocInfo.pNext = &memAllocFlagsInfo;
    memAllocInfo.allocationSize = memRequirements.size;
    memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceMemory scratchBufferDeviceMemory;
    VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &memAllocInfo, nullptr, &scratchBufferDeviceMemory));

    VK_CHECK(vkBindBufferMemory(m_Core->GetDevice(), scratchBuffer, scratchBufferDeviceMemory, 0));

    VkBufferDeviceAddressInfo scratchBufferDeviceAddressInfo{};
    scratchBufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    scratchBufferDeviceAddressInfo.buffer = scratchBuffer;
    VkDeviceAddress scratchBufferDeviceAddress = vkGetBufferDeviceAddress(m_Core->GetDevice(), &scratchBufferDeviceAddressInfo);

    // VkBufferCreateInfo tlasBufferCI = init::createBufferInfo(tlasBuildSizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR);
    //
    // VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &tlasBufferCI, nullptr, &tlAS.Buffer));
    //
    // vkGetBufferMemoryRequirements(m_Core->GetDevice(), tlAS.Buffer, &memRequirements);
    //
    // VkMemoryAllocateInfo tlasMemAllocInfo = init::memAllocInfo();
    // tlasMemAllocInfo.pNext = &memAllocFlagsInfo;
    // tlasMemAllocInfo.allocationSize = memRequirements.size;
    // tlasMemAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    //
    // VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &tlasMemAllocInfo, nullptr, &tlAS.DeviceMemory));
    //
    // VK_CHECK(vkBindBufferMemory(m_Core->GetDevice(), tlAS.Buffer, tlAS.DeviceMemory, 0));
    
    tlasBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    tlasBuildGeometryInfo.dstAccelerationStructure = tlAS.Handle;
    tlasBuildGeometryInfo.scratchData.deviceAddress = scratchBufferDeviceAddress;

    VkAccelerationStructureBuildRangeInfoKHR tlasBuildRangeInfo{};
    tlasBuildRangeInfo.primitiveCount = 1;
    tlasBuildRangeInfo.primitiveOffset = 0;
    tlasBuildRangeInfo.firstVertex = 0;
    tlasBuildRangeInfo.transformOffset = 0;

    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &tlasBuildRangeInfo };
    
    VkCommandBuffer cmdBuffer = m_Core->createComputeCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    vkPFN::vkCmdBuildAccelerationStructuresKHR(cmdBuffer, 1, &tlasBuildGeometryInfo, accelerationBuildStructureRangeInfos.data());
    m_Core->flushComputeCommandBuffer(cmdBuffer, m_Core->queue.ComputeQueue);
}

void test::RTXShadow::createTopLevelAccelerationStructure2()
{
    
    VkAccelerationStructureInstanceKHR tlasInstance{};
    tlasInstance.transform = Transform::Identity.GetAsVkTransformMatrixKHR();
    tlasInstance.instanceCustomIndex = 0;
    tlasInstance.mask = 0xFF;
    tlasInstance.instanceShaderBindingTableRecordOffset = 0;
    tlasInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
    tlasInstance.accelerationStructureReference = pblAS->GetDeviceAddress();

    
    Transform instance2Transform(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 90.0f, 0.0f), glm::vec3(1.0f));
    
    VkAccelerationStructureInstanceKHR tlasInstance2{};
    tlasInstance2.transform = instance2Transform;
    tlasInstance2.instanceCustomIndex = 0;
    tlasInstance2.mask = 0xFF;
    tlasInstance2.instanceShaderBindingTableRecordOffset = 0;
    tlasInstance2.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
    tlasInstance2.accelerationStructureReference = pblAS->GetDeviceAddress();
    
    Transform instance3Transform(glm::vec3(10.0f, 10.0f, 0.0f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    
    VkAccelerationStructureInstanceKHR tlasInstance3{};
    tlasInstance3.transform = instance3Transform;
    tlasInstance3.instanceCustomIndex = 0;
    tlasInstance3.mask = 0xFF;
    tlasInstance3.instanceShaderBindingTableRecordOffset = 0;
    tlasInstance3.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
    tlasInstance3.accelerationStructureReference = pblAS2->GetDeviceAddress();
    
    ptlAS = m_Core->GetRTXBuilder()->buildTLAS({tlasInstance, tlasInstance2, tlasInstance3});
}

void test::RTXShadow::createStorageImage()
{
    VkFormat ImageFormat = m_Core->swapchain.colorFormat;
    VkImageCreateInfo storageImageInfo = init::imageCreateInfo();
    storageImageInfo.imageType = VK_IMAGE_TYPE_2D;
    storageImageInfo.format = ImageFormat;
    storageImageInfo.extent.width = m_Core->swapchain.extent.width;
    storageImageInfo.extent.height = m_Core->swapchain.extent.height;
    storageImageInfo.extent.depth = 1;
    storageImageInfo.mipLevels = 1;
    storageImageInfo.arrayLayers = 1;
    storageImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    storageImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    storageImageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    storageImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    storageImageInfo.initialLayout = VK_IMAGE_LAYOUT_GENERAL ;

    VK_CHECK(vkCreateImage(m_Core->GetDevice(), &storageImageInfo, nullptr, &m_OutputImage.Image));

    VkMemoryRequirements imageMemRequirements{};

    vkGetImageMemoryRequirements(m_Core->GetDevice(), m_OutputImage.Image, &imageMemRequirements);
    
    VkMemoryAllocateInfo imageMemAllocInfo = init::memAllocInfo();
    imageMemAllocInfo.allocationSize = imageMemRequirements.size;
    imageMemAllocInfo.memoryTypeIndex = m_Core->getMemoryType(imageMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &imageMemAllocInfo, nullptr, &m_OutputImage.Memory));

    VK_CHECK(vkBindImageMemory(m_Core->GetDevice(), m_OutputImage.Image, m_OutputImage.Memory, 0));

    VkImageViewCreateInfo imageViewCI = init::imageViewCreateInfo();
    imageViewCI.image = m_OutputImage.Image;
    imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCI.format = ImageFormat;
    imageViewCI.components.r = VK_COMPONENT_SWIZZLE_R;
    imageViewCI.components.g = VK_COMPONENT_SWIZZLE_G;
    imageViewCI.components.b = VK_COMPONENT_SWIZZLE_B;
    imageViewCI.components.a = VK_COMPONENT_SWIZZLE_A;
    imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCI.subresourceRange.baseMipLevel = 0;
    imageViewCI.subresourceRange.levelCount = 1;
    imageViewCI.subresourceRange.baseArrayLayer = 0;
    imageViewCI.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(m_Core->GetDevice(), &imageViewCI, nullptr, &m_OutputImage.ImageView));
    
    VkSamplerCreateInfo imageSamplerCI{};
    imageSamplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    imageSamplerCI.magFilter = VK_FILTER_LINEAR;
    imageSamplerCI.minFilter = VK_FILTER_LINEAR;
    imageSamplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    imageSamplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    imageSamplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    imageSamplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    imageSamplerCI.mipLodBias = 0;
    imageSamplerCI.anisotropyEnable = VK_FALSE;
    imageSamplerCI.compareEnable = VK_FALSE;
    imageSamplerCI.minLod = 0;
    imageSamplerCI.maxLod = 0;
    imageSamplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    imageSamplerCI.unnormalizedCoordinates = VK_FALSE;

    VK_CHECK(vkCreateSampler(m_Core->GetDevice(), &imageSamplerCI, nullptr, &m_OutputImage.Sampler));
}

void test::RTXShadow::createUniformBuffer()
{
    VkBufferCreateInfo bufferCI = init::createBufferInfo(sizeof(UniformBufferData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &bufferCI, nullptr, &m_UniformBuffer.Buffer));

    VkMemoryRequirements memReqs{};
    vkGetBufferMemoryRequirements(m_Core->GetDevice(), m_UniformBuffer.Buffer, &memReqs);

    VkMemoryAllocateInfo memAllocInfo = init::memAllocInfo();
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &memAllocInfo, nullptr, &m_UniformBuffer.Memory));

    VK_CHECK(vkBindBufferMemory(m_Core->GetDevice(), m_UniformBuffer.Buffer, m_UniformBuffer.Memory, 0));

    VK_CHECK(vkMapMemory(m_Core->GetDevice(), m_UniformBuffer.Memory, 0, sizeof(UniformBuffer), 0, &m_UniformBuffer.Data));
}

void test::RTXShadow::createRayTracingPipeline()
{
    
    std::array<VkDescriptorSetLayoutBinding, 5> rtxPipelineDescriptorSetLayoutBindings = 
    {
        init::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 0, 1),
        init::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 1, 1),
        init::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 2, 1),
        init::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 3, 1),
        init::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 4, 1)
    };
    
    VkDescriptorSetLayoutCreateInfo rtxDescriptorSetLayoutCI = init::descriptorSetLayoutCreateInfo();
    rtxDescriptorSetLayoutCI.bindingCount = rtxPipelineDescriptorSetLayoutBindings.size();
    rtxDescriptorSetLayoutCI.pBindings = rtxPipelineDescriptorSetLayoutBindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_Core->GetDevice(), &rtxDescriptorSetLayoutCI, nullptr, &m_RTXDescriptorSetLayout));
    
    VkPipelineLayoutCreateInfo rtxPipelineLayoutCI = init::pipelineLayoutCreateInfo();
    rtxPipelineLayoutCI.setLayoutCount = 1;
    rtxPipelineLayoutCI.pSetLayouts = &m_RTXDescriptorSetLayout;

    VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &rtxPipelineLayoutCI, nullptr, &m_RTXPipelineLayout));
    
    // std::array<VkPipelineShaderStageCreateInfo, 4> rtxPipelineShaderStageCIs =
    //     {
    //         VulkanShader::GetShaderModule(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingshadow/raygen.rgen.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR),
    //         VulkanShader::GetShaderModule(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingshadow/miss.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR),
    //         VulkanShader::GetShaderModule(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingshadow/miss_shadow.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR),
    //         VulkanShader::GetShaderModule(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingshadow/closesthit.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR),
    //     };

    std::vector<VkPipelineShaderStageCreateInfo> rtxPipelineShaderStageCIs;

    uint32_t maxRecursion = 16;
    uint32_t maxTest = 16;

    VkSpecializationMapEntry specializationMapEntry{};
    specializationMapEntry.constantID = 0;
    specializationMapEntry.offset = 0;
    specializationMapEntry.size = sizeof(maxRecursion);

    VkSpecializationMapEntry specializationMapEntry2{};
    specializationMapEntry2.constantID = 1;
    specializationMapEntry2.offset = sizeof(maxRecursion);
    specializationMapEntry2.size = sizeof(maxTest);
    
    std::array<VkSpecializationMapEntry, 2> MapEntries
    {
        specializationMapEntry, specializationMapEntry2
    };

    std::array<uint32_t, 2> entryData
    {
        maxRecursion, maxTest
    };

    VkSpecializationInfo specializationInfo{};
    specializationInfo.mapEntryCount = MapEntries.size();
    specializationInfo.pMapEntries = MapEntries.data();
    specializationInfo.dataSize = entryData.size() * sizeof(entryData[0]);
    specializationInfo.pData = entryData.data();
    
    {
        VkPipelineShaderStageCreateInfo shaderStageCI = VulkanShader::GetShaderModule(*m_Core, "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingshadow/raygen.rgen.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        rtxPipelineShaderStageCIs.push_back(shaderStageCI);

        VkRayTracingShaderGroupCreateInfoKHR shaderGroupCI{};
        shaderGroupCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroupCI.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroupCI.generalShader = 0;
        shaderGroupCI.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroupCI.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroupCI.intersectionShader = VK_SHADER_UNUSED_KHR;

        shaderGroupCIs.push_back(shaderGroupCI);
    }
    {
        VkPipelineShaderStageCreateInfo shaderStageCI = VulkanShader::GetShaderModule(*m_Core, "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingshadow/miss.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR);
        rtxPipelineShaderStageCIs.push_back(shaderStageCI);
        
        VkRayTracingShaderGroupCreateInfoKHR shaderGroupCI{};
        shaderGroupCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroupCI.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroupCI.generalShader = 1;
        shaderGroupCI.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroupCI.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroupCI.intersectionShader = VK_SHADER_UNUSED_KHR;

        shaderGroupCIs.push_back(shaderGroupCI);
    }
    {
        VkPipelineShaderStageCreateInfo shaderStageCI = VulkanShader::GetShaderModule(*m_Core, "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingshadow/miss_shadow.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR);
        rtxPipelineShaderStageCIs.push_back(shaderStageCI);
        
        VkRayTracingShaderGroupCreateInfoKHR shaderGroupCI{};
        shaderGroupCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroupCI.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroupCI.generalShader = 2;
        shaderGroupCI.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroupCI.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroupCI.intersectionShader = VK_SHADER_UNUSED_KHR;

        shaderGroupCIs.push_back(shaderGroupCI);
    }
    {
        VkPipelineShaderStageCreateInfo shaderStageCI = VulkanShader::GetShaderModule(*m_Core, "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingshadow/closesthit.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
        rtxPipelineShaderStageCIs.push_back(shaderStageCI);
        
        VkRayTracingShaderGroupCreateInfoKHR shaderGroupCI{};
        shaderGroupCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroupCI.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        shaderGroupCI.generalShader = VK_SHADER_UNUSED_KHR;
        shaderGroupCI.closestHitShader = 3;
        shaderGroupCI.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroupCI.intersectionShader = VK_SHADER_UNUSED_KHR;

        shaderGroupCIs.push_back(shaderGroupCI);
    }
    
    VkRayTracingPipelineCreateInfoKHR rtxPipelineCI{};
    rtxPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    rtxPipelineCI.stageCount = rtxPipelineShaderStageCIs.size();
    rtxPipelineCI.pStages = rtxPipelineShaderStageCIs.data();
    rtxPipelineCI.groupCount = shaderGroupCIs.size();
    rtxPipelineCI.pGroups = shaderGroupCIs.data();
    rtxPipelineCI.maxPipelineRayRecursionDepth = 2;
    rtxPipelineCI.layout = m_RTXPipelineLayout;

    VK_CHECK(vkPFN::vkCreateRayTracingPipelinesKHR(m_Core->GetDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rtxPipelineCI, nullptr, &m_RTXPipeline));
}

void test::RTXShadow::createPostProcessPipeline()
{
    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings = { init::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1) };

    VkDescriptorSetLayoutCreateInfo descriptorLayoutCI = init::descriptorSetLayoutCreateInfo();
    descriptorLayoutCI.bindingCount = descriptorSetLayoutBindings.size();
    descriptorLayoutCI.pBindings = descriptorSetLayoutBindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_Core->GetDevice(), &descriptorLayoutCI, nullptr, &m_PostProcess.DescriptorSetLayout));

    VkPipelineLayoutCreateInfo pipelineLayoutCI = init::pipelineLayoutCreateInfo();
    pipelineLayoutCI.setLayoutCount = 1;
    pipelineLayoutCI.pSetLayouts = &m_PostProcess.DescriptorSetLayout;

    VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCI, nullptr, &m_PostProcess.PipelineLayout));

    std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStages =
    {
        VulkanShader::GetShaderModule(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingshadow/postprocess/postprocess.vspv", VK_SHADER_STAGE_VERTEX_BIT),    
        VulkanShader::GetShaderModule(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingshadow/postprocess/postprocess.fspv", VK_SHADER_STAGE_FRAGMENT_BIT)    
    };

    VkPipelineVertexInputStateCreateInfo vertexInputStateCI = init::pipelineVertexInputState();
    vertexInputStateCI.vertexBindingDescriptionCount = 0;
    vertexInputStateCI.pVertexBindingDescriptions = nullptr;
    vertexInputStateCI.vertexAttributeDescriptionCount = 0;
    vertexInputStateCI.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = init::pipelineInputAssemblyState();
    inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;
    
    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = m_Core->swapchain.extent.width;
    viewport.height = m_Core->swapchain.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = viewport.width;
    scissor.extent.height = viewport.height;
    
    VkPipelineViewportStateCreateInfo viewportStateCI = init::pipelineViewportState();
    viewportStateCI.viewportCount = 1;
    viewportStateCI.pViewports = &viewport;
    viewportStateCI.scissorCount = 1;
    viewportStateCI.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizationStateCI = init::pipelineRasterizationState();
    rasterizationStateCI.depthClampEnable = VK_FALSE;
    rasterizationStateCI.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
    rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateCI.depthBiasEnable = VK_FALSE;
    rasterizationStateCI.depthBiasConstantFactor = 0.0f;
    rasterizationStateCI.depthBiasClamp = 0.0f;
    rasterizationStateCI.depthBiasSlopeFactor = 0.0f;
    rasterizationStateCI.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampleStateCI = init::multiSampleState();
    multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleStateCI.sampleShadingEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = init::pipelineDepthStencilState();
    depthStencilStateCI.depthTestEnable = VK_FALSE;
    depthStencilStateCI.depthWriteEnable = VK_FALSE;
    depthStencilStateCI.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateCI.stencilTestEnable = VK_FALSE;
    depthStencilStateCI.minDepthBounds = 0.0f;
    depthStencilStateCI.maxDepthBounds = 1.0f;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    
    VkPipelineColorBlendStateCreateInfo colorBlendStateCI = init::pipelineColorBlendState();
    colorBlendStateCI.logicOpEnable = VK_FALSE;
    colorBlendStateCI.attachmentCount = 1;
    colorBlendStateCI.pAttachments = &colorBlendAttachment;

    VkPipelineDynamicStateCreateInfo dynamicStateCI = init::dynamicState();
    dynamicStateCI.dynamicStateCount = 0;
    dynamicStateCI.pDynamicStates = nullptr;
    
    VkGraphicsPipelineCreateInfo graphicsPipelineCI = init::graphicsPipelineCreateInfo();
    graphicsPipelineCI.stageCount = pipelineShaderStages.size();
    graphicsPipelineCI.pStages = pipelineShaderStages.data();
    graphicsPipelineCI.pVertexInputState = &vertexInputStateCI;
    graphicsPipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
    graphicsPipelineCI.pTessellationState = nullptr;
    graphicsPipelineCI.pViewportState = &viewportStateCI;
    graphicsPipelineCI.pRasterizationState = &rasterizationStateCI;
    graphicsPipelineCI.pMultisampleState = &multisampleStateCI;
    graphicsPipelineCI.pDepthStencilState = &depthStencilStateCI;
    graphicsPipelineCI.pColorBlendState = &colorBlendStateCI;
    graphicsPipelineCI.pDynamicState = &dynamicStateCI;
    graphicsPipelineCI.layout = m_PostProcess.PipelineLayout;
    graphicsPipelineCI.renderPass = m_Core->resources.renderPass;
    graphicsPipelineCI.subpass = 0;

    VK_CHECK(vkCreateGraphicsPipelines(m_Core->GetDevice(), VK_NULL_HANDLE, 1, &graphicsPipelineCI, nullptr, &m_PostProcess.Pipeline));

    std::vector<VkDescriptorPoolSize> poolSizes =
        {
            init::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
        };
    
    VkDescriptorPoolCreateInfo descriptorPoolCI = init::descriptorPoolCreateInfo();
    descriptorPoolCI.maxSets = poolSizes.size();
    descriptorPoolCI.poolSizeCount = poolSizes.size();
    descriptorPoolCI.pPoolSizes = poolSizes.data();

    VK_CHECK(vkCreateDescriptorPool(m_Core->GetDevice(), &descriptorPoolCI, nullptr, &m_PostProcess.DescriptorPool));

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = init::descriptorSetAllocateInfo();
    descriptorSetAllocInfo.descriptorPool = m_PostProcess.DescriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &m_PostProcess.DescriptorSetLayout;
    
    VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &descriptorSetAllocInfo, &m_PostProcess.DescriptorSet));

    VkDescriptorImageInfo descriptorImageInfo{};
    descriptorImageInfo.sampler = m_OutputImage.Sampler;
    descriptorImageInfo.imageView = m_OutputImage.ImageView;
    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    
    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = m_PostProcess.DescriptorSet;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.pImageInfo = &descriptorImageInfo;

    vkUpdateDescriptorSets(m_Core->GetDevice(), 1, &writeDescriptorSet, 0, nullptr);
}

void test::RTXShadow::createShaderBindingTable()
{
    uint32_t shaderGroupHandleSize = m_RayTracingPipelineProperties.shaderGroupHandleSize;
    uint32_t shaderGroupHandleAlignment = m_RayTracingPipelineProperties.shaderGroupHandleAlignment;
    uint32_t shaderGroupHandleAlignedSize = getAlignedSize(shaderGroupHandleSize, shaderGroupHandleAlignment);
    uint32_t shaderGroupSize = shaderGroupCIs.size() * shaderGroupHandleAlignedSize;

    std::vector<uint8_t> shaderGroupData(shaderGroupSize);
    VK_CHECK(vkPFN::vkGetRayTracingShaderGroupHandlesKHR(m_Core->GetDevice(), m_RTXPipeline, 0, shaderGroupCIs.size(), shaderGroupSize, shaderGroupData.data()));

    VkBufferCreateInfo raygenShaderBufferCI = init::createBufferInfo(shaderGroupHandleAlignedSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR);
    VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &raygenShaderBufferCI, nullptr, &raygenShaderBuffer));

    void* raygenShaderData;
    void* missShaderData;
    void* cHitShaderData;
    
    VkMemoryAllocateFlagsInfo memAllocFlagsInfo{};
    memAllocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memAllocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    VkMemoryRequirements memReqs{};
    vkGetBufferMemoryRequirements(m_Core->GetDevice(), raygenShaderBuffer, &memReqs);
    
    VkMemoryAllocateInfo memAllocInfo = init::memAllocInfo();
    memAllocInfo.pNext = &memAllocFlagsInfo;
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &memAllocInfo, nullptr, &raygenShaderMemory));

    VK_CHECK(vkMapMemory(m_Core->GetDevice(), raygenShaderMemory, 0, memReqs.size, 0, &raygenShaderData));
    
    VkBufferCreateInfo missShaderBufferCI = init::createBufferInfo(shaderGroupHandleAlignedSize * 2, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR);
    VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &missShaderBufferCI, nullptr, &missShaderBuffer));

    vkGetBufferMemoryRequirements(m_Core->GetDevice(), missShaderBuffer, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &memAllocInfo, nullptr, &missShaderMemory));

    VK_CHECK(vkMapMemory(m_Core->GetDevice(), missShaderMemory, 0, memReqs.size, 0, &missShaderData));

    VkBufferCreateInfo cHitShaderBufferCI = init::createBufferInfo(shaderGroupHandleAlignedSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR);
    VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &cHitShaderBufferCI, nullptr, &cHitShaderBuffer));

    vkGetBufferMemoryRequirements(m_Core->GetDevice(), cHitShaderBuffer, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &memAllocInfo, nullptr, &cHitShaderMemory));

    VK_CHECK(vkMapMemory(m_Core->GetDevice(), cHitShaderMemory, 0, memReqs.size, 0, &cHitShaderData));

    VK_CHECK(vkBindBufferMemory(m_Core->GetDevice(), raygenShaderBuffer, raygenShaderMemory, 0));
    VK_CHECK(vkBindBufferMemory(m_Core->GetDevice(), missShaderBuffer, missShaderMemory, 0));
    VK_CHECK(vkBindBufferMemory(m_Core->GetDevice(), cHitShaderBuffer, cHitShaderMemory, 0));

    VkBufferDeviceAddressInfo shaderBindingTableAddress{};
    shaderBindingTableAddress.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
    shaderBindingTableAddress.buffer = raygenShaderBuffer;
    auto raygenShaderBindingTableDeviceAddress = vkGetBufferDeviceAddress(m_Core->GetDevice(), &shaderBindingTableAddress);
    
    shaderBindingTableAddress.buffer = missShaderBuffer;
    auto missShaderBindingTableDeviceAddress = vkGetBufferDeviceAddress(m_Core->GetDevice(), &shaderBindingTableAddress);
    
    shaderBindingTableAddress.buffer = cHitShaderBuffer;
    auto cHitShaderBindingTableDeviceAddress = vkGetBufferDeviceAddress(m_Core->GetDevice(), &shaderBindingTableAddress);
    
    VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegion{};
    stridedDeviceAddressRegion.deviceAddress = raygenShaderBindingTableDeviceAddress;
    stridedDeviceAddressRegion.stride = shaderGroupHandleAlignedSize;
    stridedDeviceAddressRegion.size = shaderGroupHandleAlignedSize;
    raygenShaderDeviceAddressRegion = stridedDeviceAddressRegion;

    stridedDeviceAddressRegion.deviceAddress = cHitShaderBindingTableDeviceAddress;
    cHitShaderDeviceAddressRegion = stridedDeviceAddressRegion;

    stridedDeviceAddressRegion.deviceAddress = missShaderBindingTableDeviceAddress;
    stridedDeviceAddressRegion.size = shaderGroupHandleAlignedSize * 2;
    missShaderDeviceAddressRegion = stridedDeviceAddressRegion;
    
    memcpy(raygenShaderData, shaderGroupData.data(), shaderGroupHandleAlignedSize);
    memcpy(missShaderData, shaderGroupData.data() + shaderGroupHandleAlignedSize, shaderGroupHandleAlignedSize * 2);
    memcpy(cHitShaderData, shaderGroupData.data() + shaderGroupHandleAlignedSize * 3, shaderGroupHandleAlignedSize);
}

void test::RTXShadow::createDescriptorSets()
{
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
    {
        init::descriptorPoolSize(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1),
        init::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1),
        init::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
        init::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2)
    };

    VkDescriptorPoolCreateInfo descriptorPoolCI = init::descriptorPoolCreateInfo();
    descriptorPoolCI.maxSets = descriptorPoolSizes.size();
    descriptorPoolCI.poolSizeCount = descriptorPoolSizes.size();
    descriptorPoolCI.pPoolSizes = descriptorPoolSizes.data();

    VK_CHECK(vkCreateDescriptorPool(m_Core->GetDevice(), &descriptorPoolCI, nullptr, &m_RTXDescriptorPool));

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = init::descriptorSetAllocateInfo();
    descriptorSetAllocInfo.descriptorPool = m_RTXDescriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &m_RTXDescriptorSetLayout;
    
    VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &descriptorSetAllocInfo, &m_RTXDescriptorSet));

    VkWriteDescriptorSetAccelerationStructureKHR writeASDescriptorSet{};
    writeASDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    writeASDescriptorSet.accelerationStructureCount = 1;
    writeASDescriptorSet.pAccelerationStructures = &ptlAS->GetHandle();

    VkWriteDescriptorSet writeAS{};
    writeAS.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeAS.pNext = &writeASDescriptorSet;
    writeAS.dstSet = m_RTXDescriptorSet;
    writeAS.dstBinding = 0;
    writeAS.descriptorCount = 1;
    writeAS.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

    VkDescriptorImageInfo storageImageInfo{};
    storageImageInfo.sampler = VK_NULL_HANDLE;
    storageImageInfo.imageView = m_OutputImage.ImageView;
    storageImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    
    VkDescriptorBufferInfo uniformBufferInfo{};
    uniformBufferInfo.buffer = m_UniformBuffer.Buffer;
    uniformBufferInfo.offset = 0;
    uniformBufferInfo.range = VK_WHOLE_SIZE;

    VkDescriptorBufferInfo vertexBufferInfo{};
    vertexBufferInfo.buffer = scene.vertices.buffer;
    vertexBufferInfo.offset = 0;
    vertexBufferInfo.range = VK_WHOLE_SIZE;

    VkDescriptorBufferInfo indexBufferInfo{};
    indexBufferInfo.buffer = scene.indices.buffer;
    indexBufferInfo.offset = 0;
    indexBufferInfo.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet writeStorageImageInfo{};
    writeStorageImageInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeStorageImageInfo.dstSet = m_RTXDescriptorSet;
    writeStorageImageInfo.dstBinding = 1;
    writeStorageImageInfo.descriptorCount = 1;
    writeStorageImageInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    writeStorageImageInfo.pImageInfo = &storageImageInfo;

    VkWriteDescriptorSet writeUniformBufferInfo{};
    writeUniformBufferInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeUniformBufferInfo.dstSet = m_RTXDescriptorSet;
    writeUniformBufferInfo.dstBinding = 2;
    writeUniformBufferInfo.descriptorCount = 1;
    writeUniformBufferInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeUniformBufferInfo.pBufferInfo = &uniformBufferInfo;

    VkWriteDescriptorSet writeVertexBufferInfo{};
    writeVertexBufferInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeVertexBufferInfo.dstSet = m_RTXDescriptorSet;
    writeVertexBufferInfo.dstBinding = 3;
    writeVertexBufferInfo.descriptorCount = 1;
    writeVertexBufferInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeVertexBufferInfo.pBufferInfo = &vertexBufferInfo;

    VkWriteDescriptorSet writeIndexBufferInfo{};
    writeIndexBufferInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeIndexBufferInfo.dstSet = m_RTXDescriptorSet;
    writeIndexBufferInfo.dstBinding = 4;
    writeIndexBufferInfo.descriptorCount = 1;
    writeIndexBufferInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeIndexBufferInfo.pBufferInfo = &indexBufferInfo;

    std::array<VkWriteDescriptorSet, 5> writeDescriptorSets =
        {
           writeAS, writeStorageImageInfo, writeUniformBufferInfo, writeVertexBufferInfo, writeIndexBufferInfo
        };

    vkUpdateDescriptorSets(m_Core->GetDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, VK_NULL_HANDLE);
}

void test::RTXShadow::buildCommandBuffers()
{
    VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    VkImageSubresourceLayers subresourceLayer = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    uint32_t width = m_Core->swapchain.extent.width;
    uint32_t height = m_Core->swapchain.extent.height;

    VkClearValue clearValue[2];
    clearValue[0].color = { 0.0f };
    clearValue[1].depthStencil = { 1.0f, 0 };
        
    VkRenderPassBeginInfo renderpassBeginInfo = init::renderPassBeginInfo();
    renderpassBeginInfo.renderPass = m_Core->resources.renderPass;
    renderpassBeginInfo.renderArea.offset.x = 0;
    renderpassBeginInfo.renderArea.offset.y = 0;
    renderpassBeginInfo.renderArea.extent.width = width;
    renderpassBeginInfo.renderArea.extent.height = height;
    renderpassBeginInfo.clearValueCount = 2;
    renderpassBeginInfo.pClearValues = clearValue;
    
    //VkCommandBufferBeginInfo cmdBufferBeginInfo = init::cmdBufferBeginInfo();

    auto cmdBuffers = m_Core->resources.drawCmdBuffers;
    for(int i = 0; i < cmdBuffers.size(); i++)
    {
        VkCommandBuffer currentCmdBuffer = cmdBuffers[i];

        VkCommandBufferBeginInfo cmdBufferBeginInfo = init::cmdBufferBeginInfo();
        VK_CHECK(vkBeginCommandBuffer(currentCmdBuffer, &cmdBufferBeginInfo));

        vkCmdBindPipeline(currentCmdBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_RTXPipeline);
        vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_RTXPipelineLayout, 0, 1, &m_RTXDescriptorSet, 0, nullptr);

        VkStridedDeviceAddressRegionKHR dummyStridedDeviceAddress{};
            
        vkPFN::vkCmdTraceRaysKHR(currentCmdBuffer,
            &raygenShaderDeviceAddressRegion, &missShaderDeviceAddressRegion, &cHitShaderDeviceAddressRegion,&dummyStridedDeviceAddress,
            m_Core->swapchain.extent.width,
            m_Core->swapchain.extent.height,
            1);

        renderpassBeginInfo.framebuffer = m_Core->resources.frameBuffers[i];

        // Prepare current swap chain image as transfer destination
        vkTool::setImageLayout(
            currentCmdBuffer,
            m_Core->swapchain.images[i],
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            subresourceRange);

        // Prepare ray tracing output image as transfer source
         vkTool::setImageLayout(
            currentCmdBuffer,
            m_OutputImage.Image,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            subresourceRange);

        VkImageCopy copyRegion{};
        copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        copyRegion.srcOffset = { 0, 0, 0 };
        copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        copyRegion.dstOffset = { 0, 0, 0 };
        copyRegion.extent = { width, height, 1 };
        vkCmdCopyImage(currentCmdBuffer, m_OutputImage.Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_Core->swapchain.images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        // Transition swap chain image back for presentation
         vkTool::setImageLayout(
            currentCmdBuffer,
            m_Core->swapchain.images[i],
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            subresourceRange);

        // Transition ray tracing output image back to general layout
         vkTool::setImageLayout(
            currentCmdBuffer,
            m_OutputImage.Image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_GENERAL,
            subresourceRange);
        
        // vkCmdBeginRenderPass(currentCmdBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        //
        // vkCmdBindPipeline(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PostProcess.Pipeline);
        // vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PostProcess.PipelineLayout, 0, 1, &m_PostProcess.DescriptorSet, 0, nullptr);
        // vkCmdDraw(currentCmdBuffer, 3, 1, 0, 0);
        //
        // vkCmdEndRenderPass(currentCmdBuffer);
        

        VK_CHECK(vkEndCommandBuffer(currentCmdBuffer));
    }
}

void test::RTXShadow::updateUniformBuffers()
{
    UniformBufferData data{};
    
    data.InverseView = glm::inverse(m_Camera->getViewMatrix());
    data.InverseProjection = glm::inverse(m_Camera->getProjectionMatrix());
    data.LightPosition = m_LightPosition;
    
    memcpy(m_UniformBuffer.Data, &data, sizeof(data));
}

void test::RTXShadow::updateLightPosition()
{
    m_LightPosition = glm::vec4(cos(glm::radians(timer * 360.0f)) * 40.0f, -50.0f + sin(glm::radians(timer * 360.0f)) * 20.0f, 25.0f + sin(glm::radians(timer * 360.0f)) * 5.0f, 0.0f);
}
