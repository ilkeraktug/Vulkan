#include "pch.h"
#include "RTXReflection.h"

#include "Vulkan/Core/tools.h"
#include "Vulkan/Renderer/glTFModel.h"
#include "Vulkan/Renderer/PerspectiveCamera.h"
#include "Vulkan/Renderer/VulkanFunctions.h"
#include "Vulkan/Renderer/VulkanShader.h"
#include "Vulkan/tests/TestShadow.h"

namespace test
{
    template<class T>
    static void VulkanZeroStruct(T& vulkanStruct, uint32_t StructureType)
    {
        memset(&vulkanStruct, 0, sizeof(T));
        *(uint32_t*)&vulkanStruct.sType = StructureType;
    }
    
    RTXReflection::RTXReflection(VulkanCore* core)
    {
        Init(core);
        m_Camera = std::make_unique<PerspectiveCamera>(m_Core->swapchain.extent.width, m_Core->swapchain.extent.height, core);
        m_Camera->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
        m_Camera->setPosition(glm::vec3(0.0f, 0.5f, -2.0f));
        runBatchFile();
        
        prepareStructureProperties();
        createBottomLevelAccelerationStructure();
        createTopLevelAccelerationStructure();
        
        createStorageImage();
        createUniformBuffer();
        createRayTracingPipeline();
        createShaderBindingTable();
        createDescriptorSets();
        createPostProcessPipeline();
        buildCommandBuffers();
        initSuccess = true;

    }

    RTXReflection::~RTXReflection()
    {
    }
    void RTXReflection::OnUpdate(float deltaTime)
    {
        m_Camera->OnUpdate(deltaTime);
        // VK_INFO("CameraPos X({0}), Y({1}), Z({2})", m_Camera->getPosition().x, m_Camera->getPosition().y, m_Camera->getPosition().z);
        timer += timerSpeed * deltaTime;
        if (timer > 1.0f)
        {
            timer -= 1.0f;
        }

        if(m_UniformBuffer.data)
        {
            updateUniformBuffers();
        }
    }

    void RTXReflection::OnRender()
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

    void RTXReflection::OnImGuiRender()
    {
    }

    void RTXReflection::windowResized()
    {
        m_Core->windowResized();
    }

    void RTXReflection::runBatchFile()
    {
        //std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingreflection & for %i in (*.vert) do (%VULKAN_SDK%/Bin/glslc.exe %i -o %~ni.vspv)");
        //std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingreflection & for %i in (*.frag) do (%VULKAN_SDK%/Bin/glslc.exe %i -o %~ni.fspv)");
        //std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingreflection & for %i in (*.rchit) do (%VULKAN_SDK%/Bin/glslc.exe %i --target-env=vulkan1.2 -o %~ni.rchit.spv)");
        //std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingreflection & for %i in (*.rmiss) do (%VULKAN_SDK%/Bin/glslc.exe %i --target-env=vulkan1.2 -o %~ni.rmiss.spv)");
        //std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingreflection & for %i in (*.rgen) do (%VULKAN_SDK%/Bin/glslc.exe %i --target-env=vulkan1.2 -o %~ni.rgen.spv)");

        std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingreflection & for /r %i in (*.vert) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tvs_6_1 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo C:%~pi%~ni.vspv)");
        std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingreflection & for /r %i in (*.frag) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tps_6_1 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo C:%~pi%~ni.fspv)");
        std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingreflection & for %i in (*.rchit) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tlib_6_3 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo %~ni.rchit.spv)");
        std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingreflection & for %i in (*.rmiss) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tlib_6_3 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo %~ni.rmiss.spv)");
        std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingreflection & for %i in (*.rgen) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tlib_6_3 -Emain   -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing  -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo %~ni.rgen.spv)");
    }

    uint32_t RTXReflection::getAlignedSize(uint32_t size, uint32_t alignment)
    {
        return (size + alignment - 1) & ~(alignment - 1);
    }

    void RTXReflection::prepareStructureProperties()
    {
        m_RayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

        VkPhysicalDeviceProperties2 deviceProperties{};
        deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProperties.pNext = &m_RayTracingPipelineProperties;

        m_AccelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
        VkPhysicalDeviceFeatures2 deviceFeatures{};
        deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures.pNext = &m_AccelerationStructureProperties;
        
       // m_RayTracingPipelineProperties.pNext = &m_AccelerationStructureProperties;

        vkGetPhysicalDeviceProperties2(m_Core->GetPhysicalDevice(), &deviceProperties);
        vkGetPhysicalDeviceFeatures2(m_Core->GetPhysicalDevice(), &deviceFeatures);
    }

    void RTXReflection::createBottomLevelAccelerationStructure()
    {
        vkglTF::memoryPropertyFlags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::FlipY | vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors;

        scene.loadFromFile("C:/dev/Vulkan/Vulkan/assets/models/reflection_scene.gltf", m_Core, m_Core->queue.TransferQueue, glTFLoadingFlags);

        VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
        VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};

        vertexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(*m_Core, scene.vertices.buffer);
        indexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(*m_Core, scene.indices.buffer);

        uint32_t numTriangles = scene.indices.count / 3;
        uint32_t maxVertex = scene.vertices.count;

        VkAccelerationStructureGeometryKHR asGeometry;
        VulkanZeroStruct(asGeometry, VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR);
        asGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
        asGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        asGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        asGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        asGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
        asGeometry.geometry.triangles.vertexStride = sizeof(vkglTF::Vertex);
        asGeometry.geometry.triangles.maxVertex = maxVertex;
        asGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
        asGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;

        VkAccelerationStructureBuildGeometryInfoKHR asGeometryBuildInfo{};
        asGeometryBuildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        asGeometryBuildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        asGeometryBuildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        //asGeometryBuildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        asGeometryBuildInfo.geometryCount = 1;
        asGeometryBuildInfo.pGeometries = &asGeometry;

        VkAccelerationStructureBuildSizesInfoKHR accelerationBufferSizeInfo{};
        accelerationBufferSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        
        vkPFN::vkGetAccelerationStructureBuildSizesKHR(*m_Core, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &asGeometryBuildInfo, &numTriangles, &accelerationBufferSizeInfo);

        createAccelerationStructure(blAs, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, accelerationBufferSizeInfo);

        ScratchBuffer scratchBuffer = createScratchBuffer(accelerationBufferSizeInfo.buildScratchSize);


        VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
        accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        accelerationBuildGeometryInfo.dstAccelerationStructure = blAs.Handle;
        accelerationBuildGeometryInfo.geometryCount = 1;
        accelerationBuildGeometryInfo.pGeometries = &asGeometry;
        accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.DeviceAddress;

        VkAccelerationStructureBuildRangeInfoKHR accelerationBufferBuildRangeInfo{};
        accelerationBufferBuildRangeInfo.primitiveCount = numTriangles;
        accelerationBufferBuildRangeInfo.primitiveOffset = 0;
        accelerationBufferBuildRangeInfo.firstVertex = 0;
        accelerationBufferBuildRangeInfo.transformOffset = 0;

        std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationBufferBuildRangeInfo };

        VkCommandBuffer cmdBuffer = m_Core->createComputeCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        vkPFN::vkCmdBuildAccelerationStructuresKHR(cmdBuffer, 1, &accelerationBuildGeometryInfo, accelerationBuildStructureRangeInfos.data());
        m_Core->flushComputeCommandBuffer(cmdBuffer, m_Core->queue.ComputeQueue);

        deleteScratchBuffer(scratchBuffer);
    }

    void RTXReflection::createBottomLevelAccelerationStructureWithTransform()
    {
        vkglTF::memoryPropertyFlags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::FlipY | vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors;

        scene.loadFromFile("C:/dev/Vulkan/Vulkan/assets/models/reflection_scene.gltf", m_Core, m_Core->queue.TransferQueue, glTFLoadingFlags);

        VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
        VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};

        vertexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(*m_Core, scene.vertices.buffer);
        indexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(*m_Core, scene.indices.buffer);

        uint32_t numTriangles = scene.indices.count / 3;
        uint32_t maxVertex = scene.vertices.count;

        VkAccelerationStructureGeometryKHR asGeometry;
        VulkanZeroStruct(asGeometry, VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR);
        asGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
        asGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        asGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        asGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        asGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
        asGeometry.geometry.triangles.vertexStride = sizeof(vkglTF::Vertex);
        asGeometry.geometry.triangles.maxVertex = maxVertex;
        asGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
        asGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;

        VkAccelerationStructureBuildGeometryInfoKHR asGeometryBuildInfo{};
        asGeometryBuildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        asGeometryBuildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        asGeometryBuildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        //asGeometryBuildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        asGeometryBuildInfo.geometryCount = 1;
        asGeometryBuildInfo.pGeometries = &asGeometry;

        VkAccelerationStructureBuildSizesInfoKHR accelerationBufferSizeInfo{};
        accelerationBufferSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        
        vkPFN::vkGetAccelerationStructureBuildSizesKHR(*m_Core, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &asGeometryBuildInfo, &numTriangles, &accelerationBufferSizeInfo);

        createAccelerationStructure(blAs, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, accelerationBufferSizeInfo);

        ScratchBuffer scratchBuffer = createScratchBuffer(accelerationBufferSizeInfo.buildScratchSize);

        VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
        accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        accelerationBuildGeometryInfo.dstAccelerationStructure = blAs.Handle;
        accelerationBuildGeometryInfo.geometryCount = 1;
        accelerationBuildGeometryInfo.pGeometries = &asGeometry;
        accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.DeviceAddress;

        VkAccelerationStructureBuildRangeInfoKHR accelerationBufferBuildRangeInfo{};
        accelerationBufferBuildRangeInfo.primitiveCount = numTriangles;
        accelerationBufferBuildRangeInfo.primitiveOffset = 0;
        accelerationBufferBuildRangeInfo.firstVertex = 0;
        accelerationBufferBuildRangeInfo.transformOffset = 0;

        std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationBufferBuildRangeInfo };

        VkCommandBuffer cmdBuffer = m_Core->createComputeCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        vkPFN::vkCmdBuildAccelerationStructuresKHR(cmdBuffer, 1, &accelerationBuildGeometryInfo, accelerationBuildStructureRangeInfos.data());
        m_Core->flushComputeCommandBuffer(cmdBuffer, m_Core->queue.ComputeQueue);

        deleteScratchBuffer(scratchBuffer);
    }

    void RTXReflection::createTopLevelAccelerationStructure()
    {
        VkTransformMatrixKHR transformMatrix = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f };

        VkTransformMatrixKHR transformMatrixDifferent = {
            1.0f, 10.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 10.0f, 0.0f,
            10.0f, 0.0f, 1.0f, 0.0f };

        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 position = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 0.0f));

        glm::mat4 transform = glm::transpose(rotation * position);

        memcpy(&transformMatrixDifferent, &transform, sizeof(decltype(transformMatrixDifferent)));
        
        std::vector<VkAccelerationStructureInstanceKHR> asInstances;
        VkAccelerationStructureInstanceKHR asInstance{};
        asInstance.transform = transformMatrix;
        asInstance.instanceCustomIndex = 0;
        asInstance.mask = 0xFF;
        asInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        asInstance.instanceShaderBindingTableRecordOffset = 0;
        asInstance.accelerationStructureReference = blAs.DeviceAddress;

        asInstances.push_back(asInstance);
        asInstance.transform = transformMatrixDifferent;
        asInstance.instanceCustomIndex = 0;
        asInstances.push_back(asInstance);
        
        VkBuffer instanceBuffer;
        VkDeviceMemory instanceBufferMemory;
        
        VkBufferCreateInfo instanceBufferCI = init::createBufferInfo(
            sizeof(VkAccelerationStructureInstanceKHR) * asInstances.size(),
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

        VK_CHECK(vkCreateBuffer(*m_Core, &instanceBufferCI, nullptr, &instanceBuffer));
        
        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(*m_Core, instanceBuffer, &memReqs);

        VkMemoryAllocateFlagsInfo memFlagsInfo{};
        memFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        memFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

        VkMemoryAllocateInfo memAllocInfo = init::memAllocInfo();
        memAllocInfo.pNext = &memFlagsInfo;
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        
        VK_CHECK(vkAllocateMemory(*m_Core, &memAllocInfo, nullptr, &instanceBufferMemory));

        void* instanceBufferData = nullptr;
        
        vkMapMemory(*m_Core, instanceBufferMemory, 0, sizeof(VkAccelerationStructureInstanceKHR) * asInstances.size(), 0, &instanceBufferData);

        memcpy(instanceBufferData, asInstances.data(), sizeof(VkAccelerationStructureInstanceKHR) * asInstances.size());

        vkUnmapMemory(*m_Core, instanceBufferMemory);
        
        VK_CHECK(vkBindBufferMemory(*m_Core, instanceBuffer, instanceBufferMemory, 0));

        VkBufferDeviceAddressInfo instanceBufferDeviceAddressInfo{};
        instanceBufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        instanceBufferDeviceAddressInfo.buffer = instanceBuffer;

        VkDeviceOrHostAddressConstKHR instanceBufferDeviceAddress{};
        instanceBufferDeviceAddress.deviceAddress = vkPFN::vkGetBufferDeviceAddressKHR(*m_Core, &instanceBufferDeviceAddressInfo);
        
        VkAccelerationStructureGeometryKHR asGeometryInfo{};
        asGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        asGeometryInfo.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
        asGeometryInfo.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        asGeometryInfo.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        asGeometryInfo.geometry.instances.arrayOfPointers = VK_FALSE;
        asGeometryInfo.geometry.instances.data = instanceBufferDeviceAddress;
        
        VkAccelerationStructureBuildGeometryInfoKHR asBuildGeometryInfo{};
        asBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        asBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        asBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        asBuildGeometryInfo.geometryCount = 1;
        asBuildGeometryInfo.pGeometries = &asGeometryInfo;

        VkAccelerationStructureBuildSizesInfoKHR asBuildSizeInfo{};
        asBuildSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

        uint32_t numTriangles = 2;
        
        vkPFN::vkGetAccelerationStructureBuildSizesKHR(*m_Core, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &asBuildGeometryInfo, &numTriangles, &asBuildSizeInfo);

        createAccelerationStructure(tlAs, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, asBuildSizeInfo);
        
        ScratchBuffer scratchBuffer = createScratchBuffer(asBuildSizeInfo.buildScratchSize);

        VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
        accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        accelerationStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        accelerationStructureBuildGeometryInfo.dstAccelerationStructure = tlAs.Handle;
        accelerationStructureBuildGeometryInfo.geometryCount = 1;
        accelerationStructureBuildGeometryInfo.pGeometries = &asGeometryInfo;
        accelerationStructureBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.DeviceAddress;

        VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
        accelerationStructureBuildRangeInfo.primitiveCount = 2;
        accelerationStructureBuildRangeInfo.primitiveOffset = 0;
        accelerationStructureBuildRangeInfo.firstVertex = 0;
        accelerationStructureBuildRangeInfo.transformOffset = 0;
        
        
        std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

        VkCommandBuffer commandBuffer = m_Core->createComputeCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        vkPFN::vkCmdBuildAccelerationStructuresKHR(
            commandBuffer,
            1,
            &accelerationStructureBuildGeometryInfo,
            accelerationBuildStructureRangeInfos.data());
        m_Core->flushComputeCommandBuffer(commandBuffer, m_Core->queue.ComputeQueue);

        deleteScratchBuffer(scratchBuffer);

        vkFreeMemory(*m_Core, instanceBufferMemory, nullptr);
        vkDestroyBuffer(*m_Core, instanceBuffer, nullptr);

        VkAccessFlagBits;
        VkPipelineStageFlagBits;
        VkSubpassDependency;
        
    }

    void RTXReflection::createStorageImage()
    {
        VkImageCreateInfo imageCI = init::imageCreateInfo();
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.format = m_Core->swapchain.colorFormat;
        imageCI.extent.width = m_Core->swapchain.extent.width;
        imageCI.extent.height = m_Core->swapchain.extent.height;
        imageCI.extent.depth = 1;
        imageCI.mipLevels = 1;
        imageCI.arrayLayers = 1;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VK_CHECK(vkCreateImage(*m_Core, &imageCI, nullptr, &m_StorageImage.Image));

        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(*m_Core, m_StorageImage.Image, &memReqs);
        
        VkMemoryAllocateInfo memAllocInfo = init::memAllocInfo();
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VK_CHECK(vkAllocateMemory(*m_Core, &memAllocInfo, nullptr, &m_StorageImage.Memory));
        VK_CHECK(vkBindImageMemory(*m_Core, m_StorageImage.Image, m_StorageImage.Memory, 0));

        VkImageViewCreateInfo imageViewCI = init::imageViewCreateInfo();
        imageViewCI.image = m_StorageImage.Image;
        imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCI.format = m_Core->swapchain.colorFormat;
        imageViewCI.subresourceRange = {};
        imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCI.subresourceRange.baseMipLevel = 0;
        imageViewCI.subresourceRange.levelCount = 1;
        imageViewCI.subresourceRange.baseArrayLayer = 0;
        imageViewCI.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(*m_Core, &imageViewCI, nullptr, &m_StorageImage.ImageView));

        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = 0;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.image = m_StorageImage.Image;
        imageMemoryBarrier.subresourceRange = {};
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.levelCount = 1;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.layerCount = 1;

        VkCommandBuffer cmdBuffer = m_Core->createCopyCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        vkCmdPipelineBarrier(cmdBuffer,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_DEPENDENCY_BY_REGION_BIT,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
        m_Core->flushCopyCommandBuffer(cmdBuffer, m_Core->queue.TransferQueue);
    }

    void RTXReflection::createUniformBuffer()
    {
        VkBufferCreateInfo bufferCI = init::createBufferInfo(sizeof(m_UniformData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

        VK_CHECK(vkCreateBuffer(*m_Core, &bufferCI, nullptr, &m_UniformBuffer.Buffer));

        VkMemoryRequirements memReqs{};
        vkGetBufferMemoryRequirements(*m_Core, m_UniformBuffer.Buffer, &memReqs);

        VkMemoryAllocateInfo memAllocInfo = init::memAllocInfo();
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK(vkAllocateMemory(*m_Core, &memAllocInfo, nullptr, &m_UniformBuffer.Memory));

        vkBindBufferMemory(*m_Core, m_UniformBuffer.Buffer, m_UniformBuffer.Memory, 0);
        vkMapMemory(*m_Core, m_UniformBuffer.Memory, 0, sizeof(m_UniformData), 0, &m_UniformBuffer.data);

        updateUniformBuffers();
    }

    void RTXReflection::createRayTracingPipeline()
    {
        std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = 0;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        descriptorSetLayoutBindings.push_back(layoutBinding);
        
        layoutBinding.binding = 1;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        descriptorSetLayoutBindings.push_back(layoutBinding);

        layoutBinding.binding = 2;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;
        descriptorSetLayoutBindings.push_back(layoutBinding);

        layoutBinding.binding = 3;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        descriptorSetLayoutBindings.push_back(layoutBinding);

        layoutBinding.binding = 4;
        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        descriptorSetLayoutBindings.push_back(layoutBinding);

        VkDescriptorSetLayoutCreateInfo layoutCI = init::descriptorSetLayoutCreateInfo();
        layoutCI.bindingCount = descriptorSetLayoutBindings.size();
        layoutCI.pBindings = descriptorSetLayoutBindings.data();
        
        VK_CHECK(vkCreateDescriptorSetLayout(*m_Core, &layoutCI, nullptr, &rtxDescriptorSetLayout));

        VkPipelineLayoutCreateInfo pipelineLayoutCI = init::pipelineLayoutCreateInfo();
        pipelineLayoutCI.setLayoutCount = 1;
        pipelineLayoutCI.pSetLayouts = &rtxDescriptorSetLayout;
        
        VK_CHECK(vkCreatePipelineLayout(*m_Core, &pipelineLayoutCI, nullptr, &rtxPipelineLayout));

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

        uint32_t maxRecursion = 16;
        

        {
            VkSpecializationMapEntry specializationMapEntry{};
            specializationMapEntry.size = sizeof(maxRecursion);
            specializationMapEntry.constantID = 0;
            specializationMapEntry.offset = 0;
        
            VkSpecializationInfo specializationInfo{};
            specializationInfo.mapEntryCount = 1;
            specializationInfo.pMapEntries = &specializationMapEntry;
            specializationInfo.dataSize = specializationMapEntry.size;
            specializationInfo.pData = &maxRecursion;
            
            VkPipelineShaderStageCreateInfo shaderStageCI = VulkanShader::GetShaderModule(*m_Core, "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingreflection/raygen.rgen.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR);
           
            shaderStageCI.pSpecializationInfo = &specializationInfo;

            shaderStages.push_back(shaderStageCI);

            VkRayTracingShaderGroupCreateInfoKHR shaderGroupCI{};
            shaderGroupCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            shaderGroupCI.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            shaderGroupCI.generalShader = shaderStages.size() - 1;
            shaderGroupCI.closestHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroupCI.anyHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroupCI.intersectionShader = VK_SHADER_UNUSED_KHR;

            shaderGroups.push_back(shaderGroupCI);
        }
        
        {
             VkPipelineShaderStageCreateInfo shaderStageCI = VulkanShader::GetShaderModule(*m_Core, "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingreflection/miss.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR);

            shaderStages.push_back(shaderStageCI);

            VkRayTracingShaderGroupCreateInfoKHR shaderGroupCI{};
            shaderGroupCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            shaderGroupCI.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            shaderGroupCI.generalShader = shaderStages.size() - 1;
            shaderGroupCI.closestHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroupCI.anyHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroupCI.intersectionShader = VK_SHADER_UNUSED_KHR;
            shaderGroups.push_back(shaderGroupCI);
        }

        {
             VkPipelineShaderStageCreateInfo shaderStageCI = VulkanShader::GetShaderModule(*m_Core, "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingreflection/closesthit.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);

            shaderStages.push_back(shaderStageCI);

            VkRayTracingShaderGroupCreateInfoKHR shaderGroupCI{};
            shaderGroupCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            shaderGroupCI.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
            shaderGroupCI.generalShader = VK_SHADER_UNUSED_KHR;
            shaderGroupCI.closestHitShader = shaderStages.size() - 1;
            shaderGroupCI.anyHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroupCI.intersectionShader = VK_SHADER_UNUSED_KHR;
            shaderGroups.push_back(shaderGroupCI);
        }

        VkRayTracingPipelineCreateInfoKHR rtxPipelineCI{};
        rtxPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        rtxPipelineCI.stageCount = shaderStages.size();
        rtxPipelineCI.pStages = shaderStages.data();
        rtxPipelineCI.groupCount = shaderGroups.size();
        rtxPipelineCI.pGroups = shaderGroups.data();
        rtxPipelineCI.maxPipelineRayRecursionDepth = maxRecursion;
        rtxPipelineCI.layout = rtxPipelineLayout;
        VK_CHECK(vkPFN::vkCreateRayTracingPipelinesKHR(*m_Core, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rtxPipelineCI, nullptr, &rtxPipeline));
    }

    void RTXReflection::createPostProcessPipeline()
    {
        VkDescriptorSetLayoutBinding finalImageTextureBinding{};
        finalImageTextureBinding.binding = 0;
        finalImageTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        finalImageTextureBinding.descriptorCount = 1;
        finalImageTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        
        std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBinding =
        {
            finalImageTextureBinding        
        };

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = init::descriptorSetLayoutCreateInfo();
        descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBinding.size());
        descriptorSetLayoutCI.pBindings = descriptorSetLayoutBinding.data();
        
        VK_CHECK(vkCreateDescriptorSetLayout(*m_Core, &descriptorSetLayoutCI, nullptr, &m_PostProcessDescriptorSetLayout));

        VkDescriptorPoolSize finalImageDescriptorPoolSize{};
        finalImageDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        finalImageDescriptorPoolSize.descriptorCount = 1;
        
        VkDescriptorPoolCreateInfo descriptorPoolCI = init::descriptorPoolCreateInfo();
        descriptorPoolCI.maxSets = 1;
        descriptorPoolCI.poolSizeCount = 1;
        descriptorPoolCI.pPoolSizes = &finalImageDescriptorPoolSize;

        VK_CHECK(vkCreateDescriptorPool(*m_Core, &descriptorPoolCI, nullptr, &m_PostProcessDescriptorPool));
        
        VkDescriptorSetAllocateInfo descriptorSetAllocInfo = init::descriptorSetAllocateInfo();
        descriptorSetAllocInfo.descriptorPool = m_PostProcessDescriptorPool;
        descriptorSetAllocInfo.descriptorSetCount = 1;
        descriptorSetAllocInfo.pSetLayouts = &m_PostProcessDescriptorSetLayout;
        
        VK_CHECK(vkAllocateDescriptorSets(*m_Core, &descriptorSetAllocInfo, &m_PostProcessDescriptorSet));

        VkSamplerCreateInfo samplerCI = init::samplerCreateInfo();
        samplerCI.magFilter = VK_FILTER_LINEAR;
        samplerCI.minFilter = VK_FILTER_LINEAR;
        samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        samplerCI.mipLodBias = 0;
        samplerCI.anisotropyEnable = VK_FALSE;
        samplerCI.compareEnable = VK_FALSE;
        samplerCI.minLod = 0;
        samplerCI.maxLod = 0;
        samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        
        VK_CHECK(vkCreateSampler(*m_Core, &samplerCI, nullptr, &m_StorageImage.Sampler));
        
        VkDescriptorImageInfo finalImageDescriptorInfo{};
        finalImageDescriptorInfo.sampler = m_StorageImage.Sampler;
        finalImageDescriptorInfo.imageView = m_StorageImage.ImageView;
        finalImageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        
        VkWriteDescriptorSet finalImageDescriptorWrite{};
        finalImageDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        finalImageDescriptorWrite.dstSet = m_PostProcessDescriptorSet;
        finalImageDescriptorWrite.dstBinding = 0;
        finalImageDescriptorWrite.descriptorCount = 1;
        finalImageDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        finalImageDescriptorWrite.pImageInfo = &finalImageDescriptorInfo;
        
        vkUpdateDescriptorSets(*m_Core, 1, &finalImageDescriptorWrite, 0, VK_NULL_HANDLE);
        
        VkPipelineLayoutCreateInfo pipelineLayoutCI = init::pipelineLayoutCreateInfo();
        pipelineLayoutCI.setLayoutCount = 1;
        pipelineLayoutCI.pSetLayouts = &m_PostProcessDescriptorSetLayout;
        pipelineLayoutCI.pushConstantRangeCount = 0;
        pipelineLayoutCI.pPushConstantRanges = VK_NULL_HANDLE;
        
        VK_CHECK(vkCreatePipelineLayout(*m_Core, &pipelineLayoutCI, nullptr, &m_PostProcessPipelineLayout));
        
        // std::vector shaderStages =
        //     {
        //         VulkanShader::GetShaderModule(*m_Core, "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingreflection/postprocess/postprocess.vspv", VK_SHADER_STAGE_VERTEX_BIT),
        //         VulkanShader::GetShaderModule(*m_Core, "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingreflection/postprocess/postprocess.fspv", VK_SHADER_STAGE_FRAGMENT_BIT)
        //     };

        VkPipelineShaderStageCreateInfo shaderStages[2];

        shaderStages[0] = VulkanShader::GetShaderModule(*m_Core, "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingreflection/postprocess/postprocess.vspv", VK_SHADER_STAGE_VERTEX_BIT);
        shaderStages[1] = VulkanShader::GetShaderModule(*m_Core, "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingreflection/postprocess/postprocess.fspv", VK_SHADER_STAGE_FRAGMENT_BIT);

        VkPipelineVertexInputStateCreateInfo emptyVertexInputStateCI = init::pipelineVertexInputState();
        emptyVertexInputStateCI.vertexBindingDescriptionCount = 0;
        emptyVertexInputStateCI.pVertexBindingDescriptions = nullptr;
        emptyVertexInputStateCI.vertexAttributeDescriptionCount = 0;
        emptyVertexInputStateCI.pVertexAttributeDescriptions = nullptr;
        
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = init::pipelineInputAssemblyState();
        inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;

        VkViewport VkViewport{};
        VkViewport.x = 0.0f;
        VkViewport.y = 0.0f;
        VkViewport.width = static_cast<float>(m_Core->swapchain.extent.width);
        VkViewport.height = static_cast<float>(m_Core->swapchain.extent.height);
        VkViewport.minDepth = 0.0f;
        VkViewport.maxDepth = 1.0f;

        VkRect2D scissor{ {0, 0}, {m_Core->swapchain.extent.width, m_Core->swapchain.extent.height} };
        
        VkPipelineViewportStateCreateInfo viewportStateCI = init::pipelineViewportState();
        viewportStateCI.viewportCount = 1;
        viewportStateCI.pViewports = &VkViewport;
        viewportStateCI.scissorCount = 1;
        viewportStateCI.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizationStateCI = init::pipelineRasterizationState();
        rasterizationStateCI.depthClampEnable = VK_FALSE;
        rasterizationStateCI.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationStateCI.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationStateCI.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationStateCI.depthBiasEnable = VK_FALSE;
        rasterizationStateCI.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisampleStateCI = init::multiSampleState();
        multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleStateCI.sampleShadingEnable = VK_FALSE;
        multisampleStateCI.alphaToCoverageEnable = VK_FALSE; 
        multisampleStateCI.alphaToOneEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depthStencilCI = init::pipelineDepthStencilState();
        depthStencilCI.depthTestEnable = VK_FALSE;
        depthStencilCI.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilCI.depthBoundsTestEnable = VK_FALSE;
        depthStencilCI.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
        colorBlendAttachmentState.blendEnable = VK_FALSE;
        colorBlendAttachmentState.colorWriteMask = 0xF;
        
        VkPipelineColorBlendStateCreateInfo colorBlendStateCI = init::pipelineColorBlendState();
        colorBlendStateCI.logicOpEnable = VK_FALSE;
        colorBlendStateCI.attachmentCount = 1;
        colorBlendStateCI.pAttachments = &colorBlendAttachmentState;

        VkPipelineDynamicStateCreateInfo dynamicState = init::dynamicState();
        dynamicState.dynamicStateCount = 0;
        
        VkGraphicsPipelineCreateInfo pipelineCI = init::pipelineCreateInfo();
        pipelineCI.stageCount = 2;//shaderStages.size();
        pipelineCI.pStages = shaderStages;//shaderStages.data();
        pipelineCI.pVertexInputState = &emptyVertexInputStateCI;
        pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
        pipelineCI.pViewportState = &viewportStateCI;
        pipelineCI.pRasterizationState = &rasterizationStateCI;
        pipelineCI.pMultisampleState = &multisampleStateCI;
        pipelineCI.pDepthStencilState = &depthStencilCI;
        pipelineCI.pColorBlendState = &colorBlendStateCI;
        pipelineCI.pDynamicState = &dynamicState;
        pipelineCI.layout = m_PostProcessPipelineLayout;
        pipelineCI.renderPass = m_Core->resources.renderPass;
        pipelineCI.subpass = 0;

       VK_CHECK(vkCreateGraphicsPipelines(*m_Core, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &m_PostProcessGraphicsPipeline));
    }

    void RTXReflection::createShaderBindingTable()
    {
        uint32_t handleSize = m_RayTracingPipelineProperties.shaderGroupHandleSize;
        uint32_t handleAlignmnet = m_RayTracingPipelineProperties.shaderGroupHandleAlignment;
        uint32_t alignedSize = getAlignedSize(handleSize, handleAlignmnet);

        uint32_t groupCount = shaderGroups.size();
        uint32_t sbtSize = groupCount * alignedSize;

        std::vector<uint8_t> sbtData(sbtSize);
        
        VK_CHECK(vkPFN::vkGetRayTracingShaderGroupHandlesKHR(*m_Core, rtxPipeline, 0, groupCount, sbtSize, sbtData.data()));

        VkBufferCreateInfo bufferCI = init::createBufferInfo(alignedSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

        VK_CHECK(vkCreateBuffer(*m_Core, &bufferCI, nullptr, &m_Raygen.Buffer));
        VK_CHECK(vkCreateBuffer(*m_Core, &bufferCI, nullptr, &m_Miss.Buffer));
        VK_CHECK(vkCreateBuffer(*m_Core, &bufferCI, nullptr, &m_Hit.Buffer));

        VkMemoryRequirements memReqs{};
        vkGetBufferMemoryRequirements(*m_Core, m_Raygen.Buffer, &memReqs);

        VkMemoryAllocateFlagsInfo memAllocFlags{};
        memAllocFlags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        memAllocFlags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        
        VkMemoryAllocateInfo memAllocInfo = init::memAllocInfo();
        memAllocInfo.pNext = &memAllocFlags;
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK(vkAllocateMemory(*m_Core, &memAllocInfo, nullptr, &m_Raygen.Memory));
        VK_CHECK(vkBindBufferMemory(*m_Core, m_Raygen.Buffer, m_Raygen.Memory, 0));
        VK_CHECK(vkMapMemory(*m_Core, m_Raygen.Memory, 0, memReqs.size, 0, &m_Raygen.data));

        vkGetBufferMemoryRequirements(*m_Core, m_Miss.Buffer, &memReqs);
        
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK(vkAllocateMemory(*m_Core, &memAllocInfo, nullptr, &m_Miss.Memory));
        VK_CHECK(vkBindBufferMemory(*m_Core, m_Miss.Buffer, m_Miss.Memory, 0));
        VK_CHECK(vkMapMemory(*m_Core, m_Miss.Memory, 0, memReqs.size, 0, &m_Miss.data));

        vkGetBufferMemoryRequirements(*m_Core, m_Hit.Buffer, &memReqs);
        
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK(vkAllocateMemory(*m_Core, &memAllocInfo, nullptr, &m_Hit.Memory));
        VK_CHECK(vkBindBufferMemory(*m_Core, m_Hit.Buffer, m_Hit.Memory, 0));
        VK_CHECK(vkMapMemory(*m_Core, m_Hit.Memory, 0, memReqs.size, 0, &m_Hit.data));
        
        VkStridedDeviceAddressRegionKHR stridedDeviceAddress{};
        stridedDeviceAddress.deviceAddress = getBufferDeviceAddress(*m_Core, m_Raygen.Buffer);
        stridedDeviceAddress.stride = alignedSize;
        stridedDeviceAddress.size = alignedSize;
        m_Raygen.StridedDeviceAdress = stridedDeviceAddress;

        stridedDeviceAddress.deviceAddress = getBufferDeviceAddress(*m_Core, m_Miss.Buffer);
        m_Miss.StridedDeviceAdress = stridedDeviceAddress;

        stridedDeviceAddress.deviceAddress = getBufferDeviceAddress(*m_Core, m_Hit.Buffer);
        m_Hit.StridedDeviceAdress = stridedDeviceAddress;

        memcpy(m_Raygen.data, sbtData.data(), alignedSize);
        memcpy(m_Miss.data, sbtData.data() + alignedSize, alignedSize);
        memcpy(m_Hit.data, sbtData.data() + alignedSize * 2, alignedSize);
        
    }

    void RTXReflection::createDescriptorSets()
    {
        std::vector<VkDescriptorPoolSize> descriptorPoolSize =
            {
                { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1},
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2}
            };

        VkDescriptorPoolCreateInfo descriptorPoolCI = init::descriptorPoolCreateInfo();
        descriptorPoolCI.maxSets = descriptorPoolSize.size();
        descriptorPoolCI.poolSizeCount = descriptorPoolSize.size();
        descriptorPoolCI.pPoolSizes = descriptorPoolSize.data();

        
        VK_CHECK(vkCreateDescriptorPool(*m_Core, &descriptorPoolCI, nullptr, &descriptorPool));
        
        VkDescriptorSetAllocateInfo descriptorSetAllocInfo = init::descriptorSetAllocateInfo();
        descriptorSetAllocInfo.descriptorPool = descriptorPool;
        descriptorSetAllocInfo.descriptorSetCount = 1;
        descriptorSetAllocInfo.pSetLayouts = &rtxDescriptorSetLayout;
        VK_CHECK(vkAllocateDescriptorSets(*m_Core, &descriptorSetAllocInfo, &descriptorSet));

        VkWriteDescriptorSetAccelerationStructureKHR writeDescriptorAS{};
        writeDescriptorAS.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
        writeDescriptorAS.accelerationStructureCount = 1;
        writeDescriptorAS.pAccelerationStructures = &tlAs.Handle;

        VkWriteDescriptorSet asWriteDescriptorSet{};
        asWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        asWriteDescriptorSet.pNext = &writeDescriptorAS;
        asWriteDescriptorSet.dstSet = descriptorSet;
        asWriteDescriptorSet.dstBinding = 0;
        asWriteDescriptorSet.descriptorCount = 1;
        asWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        
        VkDescriptorImageInfo storageImageDescriptorInfo{};
        storageImageDescriptorInfo.sampler = VK_NULL_HANDLE;
        storageImageDescriptorInfo.imageView = m_StorageImage.ImageView;
        storageImageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkDescriptorBufferInfo uniformBufferDescriptorInfo{};
        uniformBufferDescriptorInfo.buffer = m_UniformBuffer.Buffer;
        uniformBufferDescriptorInfo.offset = 0;
        uniformBufferDescriptorInfo.range = VK_WHOLE_SIZE;
        
        VkDescriptorBufferInfo vertexBufferDescriptorInfo{};
        vertexBufferDescriptorInfo.buffer = scene.vertices.buffer;
        vertexBufferDescriptorInfo.offset = 0;
        vertexBufferDescriptorInfo.range = VK_WHOLE_SIZE;

        VkDescriptorBufferInfo indexBufferDescriptorInfo{};
        indexBufferDescriptorInfo.buffer = scene.indices.buffer;
        indexBufferDescriptorInfo.offset = 0;
        indexBufferDescriptorInfo.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet writeStorageImageDescriptorSet{};
        writeStorageImageDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeStorageImageDescriptorSet.dstSet = descriptorSet;
        writeStorageImageDescriptorSet.dstBinding = 1;
        writeStorageImageDescriptorSet.descriptorCount = 1;
        writeStorageImageDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        writeStorageImageDescriptorSet.pImageInfo = &storageImageDescriptorInfo;

        VkWriteDescriptorSet writeUniformDescriptorSet{};
        writeUniformDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeUniformDescriptorSet.dstSet = descriptorSet;
        writeUniformDescriptorSet.dstBinding = 2;
        writeUniformDescriptorSet.descriptorCount = 1;
        writeUniformDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeUniformDescriptorSet.pBufferInfo = &uniformBufferDescriptorInfo;
        
        VkWriteDescriptorSet writeVertexDescriptorSet{};
        writeVertexDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeVertexDescriptorSet.dstSet = descriptorSet;
        writeVertexDescriptorSet.dstBinding = 3;
        writeVertexDescriptorSet.descriptorCount = 1;
        writeVertexDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeVertexDescriptorSet.pBufferInfo = &vertexBufferDescriptorInfo;

        VkWriteDescriptorSet writeIndexDescriptorSet{};
        writeIndexDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeIndexDescriptorSet.dstSet = descriptorSet;
        writeIndexDescriptorSet.dstBinding = 4;
        writeIndexDescriptorSet.descriptorCount = 1;
        writeIndexDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeIndexDescriptorSet.pBufferInfo = &indexBufferDescriptorInfo;

        std::vector<VkWriteDescriptorSet> writeDescriptorSets = { asWriteDescriptorSet, writeStorageImageDescriptorSet, writeUniformDescriptorSet, writeVertexDescriptorSet, writeIndexDescriptorSet };
        
        vkUpdateDescriptorSets(*m_Core, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, VK_NULL_HANDLE);
    }

    void RTXReflection::buildCommandBuffers()
    {
        VkCommandBufferBeginInfo cmdBufInfo = init::commandBufferBeginInfo();

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
        
        auto cmdBuffers = m_Core->resources.drawCmdBuffers;
        for(int i = 0; i < cmdBuffers.size(); i++)
        {
            VkCommandBuffer currentCmdBuffer = cmdBuffers[i];

            VkCommandBufferBeginInfo cmdBufferBeginInfo = init::cmdBufferBeginInfo();
            VK_CHECK(vkBeginCommandBuffer(currentCmdBuffer, &cmdBufferBeginInfo));

            vkCmdBindPipeline(currentCmdBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rtxPipeline);
            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rtxPipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

            VkStridedDeviceAddressRegionKHR dummyStridedDeviceAddress{};
            
            vkPFN::vkCmdTraceRaysKHR(currentCmdBuffer,
                &m_Raygen.StridedDeviceAdress, &m_Miss.StridedDeviceAdress, &m_Hit.StridedDeviceAdress,
                &dummyStridedDeviceAddress,
                m_Core->swapchain.extent.width,
                m_Core->swapchain.extent.height,
                1);

            renderpassBeginInfo.framebuffer = m_Core->resources.frameBuffers[i];
            vkCmdBeginRenderPass(currentCmdBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            
            vkCmdBindPipeline(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PostProcessGraphicsPipeline);
            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PostProcessPipelineLayout, 0, 1, &m_PostProcessDescriptorSet, 0, nullptr);
            vkCmdDraw(currentCmdBuffer, 3, 1, 0, 0);

            vkCmdEndRenderPass(currentCmdBuffer);
            
    //         vkTool::setImageLayout(
    // m_Core->resources.drawCmdBuffers[i],
    // m_Core->swapchain.images[i],
    // VK_IMAGE_LAYOUT_UNDEFINED,
    // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    // subresourceRange);
    //
    //         // Prepare ray tracing output image as transfer source
    //         vkTool::setImageLayout(
    //             m_Core->resources.drawCmdBuffers[i],
    //             m_StorageImage.Image,
    //             VK_IMAGE_LAYOUT_GENERAL,
    //             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    //             subresourceRange);
    //
    //         VkImageCopy copyRegion{};
    //         copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    //         copyRegion.srcOffset = { 0, 0, 0 };
    //         copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    //         copyRegion.dstOffset = { 0, 0, 0 };
    //         copyRegion.extent = { width, height, 1 };
    //         vkCmdCopyImage(m_Core->resources.drawCmdBuffers[i], m_StorageImage.Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_Core->swapchain.images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
    //
    //         // Transition swap chain image back for presentation
    //         vkTool::setImageLayout(
    //             m_Core->resources.drawCmdBuffers[i],
    //             m_Core->swapchain.images[i],
    //             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //             VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    //             subresourceRange);
    //
    //         // Transition ray tracing output image back to general layout
    //         vkTool::setImageLayout(
    //             m_Core->resources.drawCmdBuffers[i],
    //             m_StorageImage.Image,
    //             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    //             VK_IMAGE_LAYOUT_GENERAL,
    //             subresourceRange);
            // VkImageMemoryBarrier storageImageMemBarrier{};
            // storageImageMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            // storageImageMemBarrier.srcAccessMask = 0;
            // storageImageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            // storageImageMemBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            // storageImageMemBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            // storageImageMemBarrier.image = m_StorageImage.Image;
            // storageImageMemBarrier.subresourceRange = subresourceRange;
            //
            // vkCmdPipelineBarrier(currentCmdBuffer,
            //     VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
            //     0, nullptr,
            //     0, nullptr,
            //     1, &storageImageMemBarrier);
            //
            // VkImageMemoryBarrier swapchainImageMemBarrier{};
            // swapchainImageMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            // swapchainImageMemBarrier.srcAccessMask = 0;
            // swapchainImageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            // swapchainImageMemBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            // swapchainImageMemBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            // swapchainImageMemBarrier.image = m_Core->swapchain.images[i];
            // swapchainImageMemBarrier.subresourceRange = subresourceRange;
            //
            // vkCmdPipelineBarrier(currentCmdBuffer,
            //     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
            //     0, nullptr,
            //     0, nullptr,
            //     1, &swapchainImageMemBarrier);
            //
            // VkImageCopy imageCopy{};
            // imageCopy.srcSubresource = subresourceLayer;
            // imageCopy.srcOffset = { 0, 0, 0 };
            // imageCopy.dstSubresource = subresourceLayer;
            // imageCopy.dstOffset = { 0, 0, 0 };
            // imageCopy.extent = { m_Core->swapchain.extent.width, m_Core->swapchain.extent.height, 1};
            //
            // vkCmdCopyImage(currentCmdBuffer,
            //     m_StorageImage.Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            //     m_Core->swapchain.images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            //     1, &imageCopy);
            //
            // storageImageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            // storageImageMemBarrier.dstAccessMask = 0;
            // storageImageMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            // storageImageMemBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            // storageImageMemBarrier.image = m_StorageImage.Image;
            // storageImageMemBarrier.subresourceRange = subresourceRange;
            //
            // vkCmdPipelineBarrier(currentCmdBuffer,
            //     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT,
            //     0, nullptr,
            //     0, nullptr,
            //     1, &storageImageMemBarrier);
            //
            // swapchainImageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            // swapchainImageMemBarrier.dstAccessMask = 0;
            // swapchainImageMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            // swapchainImageMemBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            // swapchainImageMemBarrier.image = m_Core->swapchain.images[i];
            // swapchainImageMemBarrier.subresourceRange = subresourceRange;
            //
            // vkCmdPipelineBarrier(currentCmdBuffer,
            //     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT,
            //     0, nullptr,
            //     0, nullptr,
            //     1, &swapchainImageMemBarrier);

            VK_CHECK(vkEndCommandBuffer(currentCmdBuffer));
        }
    }
    
    void RTXReflection::createAccelerationStructure(AccelerationStruct& as, VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
    {
        VkBufferCreateInfo bufferCI = init::createBufferInfo(
            buildSizeInfo.accelerationStructureSize,
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
            );
        VK_CHECK(vkCreateBuffer(*m_Core, &bufferCI, nullptr, &as.Buffer));

        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(*m_Core, as.Buffer, &memReqs);

        VkMemoryAllocateFlagsInfo memAllocateFlagsInfo{};
        memAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        memAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        
        VkMemoryAllocateInfo memAllocInfo = init::memAllocInfo();
        memAllocInfo.pNext = &memAllocateFlagsInfo;
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        
        VK_CHECK(vkAllocateMemory(*m_Core, &memAllocInfo, nullptr, &as.Memory));

        VK_CHECK(vkBindBufferMemory(*m_Core, as.Buffer, as.Memory, 0));

        
        VkAccelerationStructureCreateInfoKHR asCreateInfo{};
        asCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        asCreateInfo.buffer = as.Buffer;
        asCreateInfo.offset = 0;
        asCreateInfo.size = buildSizeInfo.accelerationStructureSize;
        asCreateInfo.type = type;
        //asCreateInfo.deviceAddress = as.DeviceAddress;
        
        VK_CHECK(vkPFN::vkCreateAccelerationStructureKHR(*m_Core, &asCreateInfo, nullptr, &as.Handle));

        VkAccelerationStructureDeviceAddressInfoKHR asDeviceAddressInfo{};
        asDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
        asDeviceAddressInfo.accelerationStructure = as.Handle;
        as.DeviceAddress = vkPFN::vkGetAccelerationStructureDeviceAddressKHR(*m_Core, &asDeviceAddressInfo);
    }

    RTXReflection::ScratchBuffer RTXReflection::createScratchBuffer(VkDeviceSize size)
    {
        ScratchBuffer scratchBuffer{};

        VkBufferCreateInfo bufferCI = init::createBufferInfo(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
        VK_CHECK(vkCreateBuffer(*m_Core, &bufferCI, nullptr, &scratchBuffer.Buffer));
        
        VkMemoryRequirements bufferMemReqs{};
        vkGetBufferMemoryRequirements(*m_Core, scratchBuffer.Buffer, &bufferMemReqs);

        VkMemoryAllocateFlagsInfo memAllocFlags{};
        memAllocFlags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        memAllocFlags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        
        VkMemoryAllocateInfo memAllocInfo = init::memAllocInfo();
        memAllocInfo.pNext = &memAllocFlags;
        memAllocInfo.allocationSize = size;
        memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(bufferMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        
        VK_CHECK(vkAllocateMemory(*m_Core, &memAllocInfo, nullptr, &scratchBuffer.Memory));
        VK_CHECK(vkBindBufferMemory(*m_Core, scratchBuffer.Buffer, scratchBuffer.Memory, 0));

        VkBufferDeviceAddressInfo deviceAddressInfo{};
        deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        deviceAddressInfo.buffer = scratchBuffer.Buffer;
        
        scratchBuffer.DeviceAddress = vkPFN::vkGetBufferDeviceAddressKHR(*m_Core, &deviceAddressInfo);;

        return scratchBuffer;
    }

    void RTXReflection::deleteScratchBuffer(ScratchBuffer& scratchBuffer)
    {
        if(scratchBuffer.Buffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(*m_Core, scratchBuffer.Buffer, nullptr);
        }

        if(scratchBuffer.Memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(*m_Core, scratchBuffer.Memory, nullptr);
        }

        if(scratchBuffer.DeviceAddress)
        {
            scratchBuffer.DeviceAddress = 0;
        }
    }

    void RTXReflection::updateUniformBuffers()
    {
        updateLightPosition();
        m_UniformData.projInverse = glm::inverse(m_Camera->getProjectionMatrix());
        m_UniformData.viewInverse = glm::inverse(m_Camera->getViewMatrix());
        m_UniformData.lightPos = glm::vec4(cos(glm::radians(timer * 360.0f)) * 40.0f, -20.0f + sin(glm::radians(timer * 360.0f)) * 20.0f, 25.0f + sin(glm::radians(timer * 360.0f)) * 5.0f, 0.0f);
        // Pass the vertex size to the shader for unpacking vertices
        m_UniformData.vertexSize = sizeof(vkglTF::Vertex);
        memcpy(m_UniformBuffer.data, &m_UniformData, sizeof(m_UniformData));
    }

    void RTXReflection::updateLightPosition()
    {
         m_UniformData.lightPos.x = cos(glm::radians(timer * 360.0f)) * 40.0f;
         m_UniformData.lightPos.y = -50.0f + sin(glm::radians(timer * 360.0f)) * 20.0f;
         m_UniformData.lightPos.z = 25.0f + sin(glm::radians(timer * 360.0f)) * 5.0f;
    }

    VkDeviceAddress RTXReflection::getBufferDeviceAddress(VkDevice device, VkBuffer buffer)
    {
        VkBufferDeviceAddressInfo bufferDeviceAddressInfo{};
        bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        bufferDeviceAddressInfo.buffer = buffer;

        return vkPFN::vkGetBufferDeviceAddressKHR(device, &bufferDeviceAddressInfo);
    }
    
}
