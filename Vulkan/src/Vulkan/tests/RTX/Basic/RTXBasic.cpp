#include "pch.h"
#include "RTXBasic.h"

#include <variant>

#include "Vulkan/Core/tools.h"
#include "Vulkan/Renderer/PerspectiveCamera.h"
#include "Vulkan/Renderer/VulkanShader.h"

namespace test
{
	double RTXBasic::cameraSpeed = 1.0;

	static int ilkerGet() { return 5; }
	
    RTXBasic::RTXBasic(VulkanCore* core)
    {
	    Init(core);
    	m_Camera = new PerspectiveCamera(m_Core->swapchain.extent.width, m_Core->swapchain.extent.height, core);
    	//m_Camera->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    	runBatchFile();
    	prepareStructureProperties();
    	createBottomLevelAccelerationStructure();
    	createTopLevelAccelerationStructure();
        
    	createStorageImage();
    	createUniformBuffer();
    	createRayTracingPipeline();
    	createShaderBindingTable();
    	createDescriptorSets();
    	buildCommandBuffers();
    	initSuccess = true;
    	
    	glfwSetScrollCallback(static_cast<GLFWwindow*>(Window::GetWindow()), &RTXBasic::onMouseScrollMoved);
    }

    RTXBasic::~RTXBasic()
    {
    }

    void RTXBasic::OnUpdate(float deltaTime)
    {
		if ((glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
			glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS))
		{
			keys.shift = true;
		}
		if ((glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE ||
				glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_RIGHT_SHIFT) == GLFW_RELEASE))
		{
			keys.shift = false;
		}
		
		if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_W) == GLFW_PRESS)
		{
			glm::vec3 cameraCurrentPosition = m_Camera->getPosition();
			m_Camera->setPosition(cameraCurrentPosition + m_Camera->getForwardVector() * deltaTime * (float)cameraSpeed);
		}
		else if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_S) == GLFW_PRESS)
		{
			glm::vec3 cameraCurrentPosition = m_Camera->getPosition();
			m_Camera->setPosition(cameraCurrentPosition + -m_Camera->getForwardVector() * deltaTime * (float)cameraSpeed);
		}
		if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_A) == GLFW_PRESS)
		{
			glm::vec3 cameraCurrentPosition = m_Camera->getPosition();
			m_Camera->setPosition(cameraCurrentPosition + -m_Camera->getRightVector() * deltaTime * (float)cameraSpeed);
		}
		else if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_D) == GLFW_PRESS)
		{
			glm::vec3 cameraCurrentPosition = m_Camera->getPosition();
			m_Camera->setPosition(cameraCurrentPosition + m_Camera->getRightVector() * deltaTime * (float)cameraSpeed);
		}
		
		if (glfwGetMouseButton(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
		{
			mouseButtons.left = true;
		}
		if (glfwGetMouseButton(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
		{
			mouseButtons.right = true;
		}
		if (glfwGetMouseButton(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_MOUSE_BUTTON_3) == GLFW_PRESS)
		{
			mouseButtons.middle = true;
		}

		if (glfwGetMouseButton(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE)
		{
			mouseButtons.left = false;
		}
		if (glfwGetMouseButton(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE)
		{
			mouseButtons.right = false;
		}
		if (glfwGetMouseButton(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_MOUSE_BUTTON_3) == GLFW_RELEASE)
		{
			mouseButtons.middle = false;
		}

		//VK_INFO("Mouse1{0}, Mouse2{1}, Mouse3{2}", mouseButtons.left, mouseButtons.middle, mouseButtons.right);

		double x, y;
		glfwGetCursorPos(static_cast<GLFWwindow*>(Window::GetWindow()), &x, &y);
		
		double dx = mousePos.x - x;
		double dy = mousePos.y - y;

		mousePos.x = x;
		mousePos.y = y;

		if(mouseButtons.left)
		{
			m_Camera->addRotation(glm::vec3( dy, -dx, 0));
		}
		if (mouseButtons.middle)
		{
			m_Camera->addPosition(glm::vec3(-dx * 0.005f, -dy * 0.005f, 0.0f));
		}
    	
		updateUniformBuffers();
    }

    void RTXBasic::OnRender()
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

    void RTXBasic::OnImGuiRender()
    {
    }

    void RTXBasic::windowResized()
    {
    }

	void RTXBasic::runBatchFile()
    {
    	std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingbasic & for %i in (*.vert) do (%VULKAN_SDK%/Bin/glslc.exe %i -o %~ni.vspv)");
    	std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingbasic & for %i in (*.frag) do (%VULKAN_SDK%/Bin/glslc.exe %i -o %~ni.fspv)");
    	std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingbasic & for %i in (*.rchit) do (%VULKAN_SDK%/Bin/glslc.exe %i --target-env=vulkan1.2 -o %~ni.rchit.spv)");
    	std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingbasic & for %i in (*.rmiss) do (%VULKAN_SDK%/Bin/glslc.exe %i --target-env=vulkan1.2 -o %~ni.rmiss.spv)");
    	std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingbasic & for %i in (*.rgen) do (%VULKAN_SDK%/Bin/glslc.exe %i --target-env=vulkan1.2 -o %~ni.rgen.spv)");
    }

    void RTXBasic::prepareStructureProperties()
    {
        rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
        
        VkPhysicalDeviceProperties2 deviceProperties2{};
        deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProperties2.pNext = &rayTracingPipelineProperties;

        vkGetPhysicalDeviceProperties2(m_Core->GetPhysicalDevice(), &deviceProperties2);

        accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        VkPhysicalDeviceFeatures2 deviceFeatures2{};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = &accelerationStructureFeatures;
        vkGetPhysicalDeviceFeatures2(m_Core->GetPhysicalDevice(), &deviceFeatures2);
        
        vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(m_Core->GetDevice(), "vkGetBufferDeviceAddressKHR"));
        vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(m_Core->GetDevice(), "vkCmdBuildAccelerationStructuresKHR"));
        vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(m_Core->GetDevice(), "vkBuildAccelerationStructuresKHR"));
        vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(m_Core->GetDevice(), "vkCreateAccelerationStructureKHR"));
        vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(m_Core->GetDevice(), "vkDestroyAccelerationStructureKHR"));
        vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(m_Core->GetDevice(), "vkGetAccelerationStructureBuildSizesKHR"));
        vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(m_Core->GetDevice(), "vkGetAccelerationStructureDeviceAddressKHR"));
        vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(m_Core->GetDevice(), "vkCmdTraceRaysKHR"));
        vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(m_Core->GetDevice(), "vkGetRayTracingShaderGroupHandlesKHR"));
        vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(m_Core->GetDevice(), "vkCreateRayTracingPipelinesKHR"));
    }

    void RTXBasic::createBottomLevelAccelerationStructure()
    {
        // Setup vertices for a single triangle
        struct Vertex {
            float pos[3];
        };
        std::vector<Vertex> vertices = {
            { {  1.0f,  1.0f, 0.0f } },
            { { -1.0f,  1.0f, 0.0f } },
            { {  0.0f, -1.0f, 0.0f } }
        };

        // Setup indices
        std::vector<uint32_t> indices = { 0, 1, 2 };
        std::vector<uint32_t> indices2 = { 1, 1, 0 };
        indexCount = static_cast<uint32_t>(indices.size());
        indexCount2 = static_cast<uint32_t>(indices2.size());

        // Setup identity transform matrix
        VkTransformMatrixKHR transformMatrix = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f
        };

        VK_CHECK(m_Core->createBuffer(
        	VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    &vertexBuffer,
    vertices.size() * sizeof(Vertex),
    vertices.data()));
        // Index buffer
        VK_CHECK(m_Core->createBuffer(
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &indexBuffer,
            indices.size() * sizeof(uint32_t),
            indices.data()));
        // Transform buffer
        VK_CHECK(m_Core->createBuffer(
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &transformBuffer,
            sizeof(VkTransformMatrixKHR),
            &transformMatrix));

    	VK_CHECK(m_Core->createBuffer(
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&indexBuffer2,
			indices.size() * sizeof(uint32_t),
			indices.data()));
    	
        VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
        VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
        VkDeviceOrHostAddressConstKHR indexBuffer2DeviceAddress{};
        VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

        vertexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(vertexBuffer.buffer);
        indexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(indexBuffer.buffer);
        indexBuffer2DeviceAddress.deviceAddress = getBufferDeviceAddress(indexBuffer2.buffer);
        transformBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(transformBuffer.buffer);

        VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
        accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
        accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
        accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
        accelerationStructureGeometry.geometry.triangles.maxVertex = 3;
        accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
        accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
        accelerationStructureGeometry.geometry.triangles.transformData = transformBufferDeviceAddress;

    	VkAccelerationStructureGeometryKHR accelerationStructureGeometry2{};
    	accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    	accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
    	accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    	accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    	accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    	accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
    	accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
    	accelerationStructureGeometry.geometry.triangles.maxVertex = 3;
    	accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    	accelerationStructureGeometry.geometry.triangles.indexData = indexBuffer2DeviceAddress;
    	accelerationStructureGeometry.geometry.triangles.transformData = transformBufferDeviceAddress;

    	VkAccelerationStructureGeometryKHR geometries[2] = {accelerationStructureGeometry, accelerationStructureGeometry2};
    	
        VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometry{};
        accelerationStructureBuildGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        accelerationStructureBuildGeometry.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        accelerationStructureBuildGeometry.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        accelerationStructureBuildGeometry.geometryCount = 1;
        accelerationStructureBuildGeometry.pGeometries = geometries;

        const uint32_t numTriangles = 10;
        VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
        accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        vkGetAccelerationStructureBuildSizesKHR(m_Core->GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &accelerationStructureBuildGeometry, &numTriangles, &accelerationStructureBuildSizesInfo);

        createAccelerationStructureBuffer(bottomLevelAS, accelerationStructureBuildSizesInfo);

        VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
        accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        accelerationStructureCreateInfo.buffer = bottomLevelAS.buffer;
        accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
        accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        VK_CHECK(vkCreateAccelerationStructureKHR(m_Core->GetDevice(), &accelerationStructureCreateInfo, nullptr, &bottomLevelAS.handle));

        RayTracingScratchBuffer scratchBuffer = createScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

        VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
        accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        accelerationBuildGeometryInfo.dstAccelerationStructure = bottomLevelAS.handle;
        accelerationBuildGeometryInfo.geometryCount = 1;
        accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
        accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

        VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
        accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
        accelerationStructureBuildRangeInfo.primitiveOffset = 0;
        accelerationStructureBuildRangeInfo.firstVertex = 0;
        accelerationStructureBuildRangeInfo.transformOffset = 0;
        std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

        VkCommandBuffer commandBuffer = m_Core->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &accelerationBuildGeometryInfo, accelerationBuildStructureRangeInfos.data());
        m_Core->flushCommandBuffer(commandBuffer, m_Core->queue.GraphicsQueue);

        VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
        accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
        accelerationDeviceAddressInfo.accelerationStructure = bottomLevelAS.handle;

        bottomLevelAS.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(*m_Core, &accelerationDeviceAddressInfo);

        deleteScratchBuffer(scratchBuffer);
    }

    void RTXBasic::createTopLevelAccelerationStructure()
    {
        VkTransformMatrixKHR transformMatrix = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f };

        VkAccelerationStructureInstanceKHR instance{};
        instance.transform = transformMatrix;
        instance.instanceCustomIndex = 0;
        instance.mask = 0xFF;
        instance.instanceShaderBindingTableRecordOffset = 0;
        instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        instance.accelerationStructureReference = bottomLevelAS.deviceAddress;

        VulkanBuffer accelerationInstanceBuffer;
        m_Core->createBuffer(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             &accelerationInstanceBuffer,
                             sizeof(VkAccelerationStructureInstanceKHR),
                             &instance);

        VkDeviceOrHostAddressConstKHR instanceDeviceAddressConst{};
        instanceDeviceAddressConst.deviceAddress = getBufferDeviceAddress(accelerationInstanceBuffer.buffer);

        VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
        accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
        accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
        accelerationStructureGeometry.geometry.instances.data = instanceDeviceAddressConst;

        VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
        accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        accelerationStructureBuildGeometryInfo.geometryCount = 1;
        accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

        uint32_t primitiveCount = 1;

        VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
        accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        vkGetAccelerationStructureBuildSizesKHR(*m_Core, 
                                                VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                                                &accelerationStructureBuildGeometryInfo,
                                                &primitiveCount,
                                                &accelerationStructureBuildSizesInfo);

        createAccelerationStructureBuffer(topLevelAS, accelerationStructureBuildSizesInfo);

        VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
        accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        accelerationStructureCreateInfo.buffer = topLevelAS.buffer;
        accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
        accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        vkCreateAccelerationStructureKHR(*m_Core, &accelerationStructureCreateInfo, nullptr, &topLevelAS.handle);

        RayTracingScratchBuffer scratchBuffer = createScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);
    	
    	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
		accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		accelerationBuildGeometryInfo.dstAccelerationStructure = topLevelAS.handle;
		accelerationBuildGeometryInfo.geometryCount = 1;
		accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
		accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

		VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
		accelerationStructureBuildRangeInfo.primitiveCount = 1;
		accelerationStructureBuildRangeInfo.primitiveOffset = 0;
		accelerationStructureBuildRangeInfo.firstVertex = 0;
		accelerationStructureBuildRangeInfo.transformOffset = 0;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

		// Build the acceleration structure on the device via a one-time command buffer submission
		// Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds
		VkCommandBuffer commandBuffer = m_Core->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		vkCmdBuildAccelerationStructuresKHR(
			commandBuffer,
			1,
			&accelerationBuildGeometryInfo,
			accelerationBuildStructureRangeInfos.data());
		m_Core->flushCommandBuffer(commandBuffer, m_Core->queue.GraphicsQueue);

		VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
		accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		accelerationDeviceAddressInfo.accelerationStructure = topLevelAS.handle;
		topLevelAS.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(*m_Core, &accelerationDeviceAddressInfo);

		deleteScratchBuffer(scratchBuffer);
		accelerationInstanceBuffer.destroy();
    }

    void RTXBasic::createStorageImage()
    {
    	VkImageCreateInfo image = init::imageCreateInfo();
    	image.imageType = VK_IMAGE_TYPE_2D;
    	image.format = m_Core->swapchain.colorFormat;
    	image.extent.width = m_Core->swapchain.extent.width;
    	image.extent.height = m_Core->swapchain.extent.height;
    	image.extent.depth = 1;
    	image.mipLevels = 1;
    	image.arrayLayers = 1;
    	image.samples = VK_SAMPLE_COUNT_1_BIT;
    	image.tiling = VK_IMAGE_TILING_OPTIMAL;
    	image.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    	image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    	VK_CHECK(vkCreateImage(*m_Core, &image, nullptr, &storageImage.image));

    	VkMemoryRequirements memReqs;
    	vkGetImageMemoryRequirements(*m_Core, storageImage.image, &memReqs);
    	VkMemoryAllocateInfo memoryAllocateInfo = init::memAllocInfo();
    	memoryAllocateInfo.allocationSize = memReqs.size;
    	memoryAllocateInfo.memoryTypeIndex = m_Core->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    	VK_CHECK(vkAllocateMemory(*m_Core, &memoryAllocateInfo, nullptr, &storageImage.memory));
    	VK_CHECK(vkBindImageMemory(*m_Core, storageImage.image, storageImage.memory, 0));

    	VkImageViewCreateInfo colorImageView = init::imageViewCreateInfo();
    	colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    	colorImageView.format = m_Core->swapchain.colorFormat;
    	colorImageView.subresourceRange = {};
    	colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    	colorImageView.subresourceRange.baseMipLevel = 0;
    	colorImageView.subresourceRange.levelCount = 1;
    	colorImageView.subresourceRange.baseArrayLayer = 0;
    	colorImageView.subresourceRange.layerCount = 1;
    	colorImageView.image = storageImage.image;
    	VK_CHECK(vkCreateImageView(*m_Core, &colorImageView, nullptr, &storageImage.view));

    	VkImageSubresourceRange imageSubresource{};
    	imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    	imageSubresource.baseMipLevel = 0;
    	imageSubresource.levelCount = 1;
    	imageSubresource.baseArrayLayer = 0;
    	imageSubresource.layerCount = 1;
    	
    	VkImageMemoryBarrier imageMemoryBarrier{};
    	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    	imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE;
    	imageMemoryBarrier.dstAccessMask = VK_ACCESS_NONE;
    	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    	imageMemoryBarrier.image = storageImage.image;
    	imageMemoryBarrier.subresourceRange = imageSubresource;

    	VkCommandBuffer cmdBuffer = m_Core->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    	vkCmdPipelineBarrier(cmdBuffer,
    		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    		0,
    		0, nullptr,
    		0, nullptr,
    		1, &imageMemoryBarrier);

    	m_Core->flushCommandBuffer(cmdBuffer, m_Core->queue.GraphicsQueue);
    	
    }

    void RTXBasic::createUniformBuffer()
    {
    	VK_CHECK(m_Core->createBuffer(
    		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&ubo,
			sizeof(uniformData),
			&uniformData));
    	
    	VK_CHECK(ubo.map());

    	updateUniformBuffers();
    }

    void RTXBasic::createRayTracingPipeline()
    {
    	VkDescriptorSetLayoutBinding accelerationStructureLayoutBinding{};
    	accelerationStructureLayoutBinding.binding = 0;
    	accelerationStructureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    	accelerationStructureLayoutBinding.descriptorCount = 1;
    	accelerationStructureLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    	VkDescriptorSetLayoutBinding resultImageLayoutBinding{};
    	resultImageLayoutBinding.binding = 1;
    	resultImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    	resultImageLayoutBinding.descriptorCount = 1;
    	resultImageLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    	VkDescriptorSetLayoutBinding uniformBufferLayoutBinding{};
    	uniformBufferLayoutBinding.binding = 2;
    	uniformBufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    	uniformBufferLayoutBinding.descriptorCount = 1;
    	uniformBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    	std::vector<VkDescriptorSetLayoutBinding> bindings({
			accelerationStructureLayoutBinding,
			resultImageLayoutBinding,
    		uniformBufferLayoutBinding
			});

    	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = init::descriptorSetLayoutCreateInfo();
    	descriptorSetLayoutCreateInfo.bindingCount = bindings.size();
    	descriptorSetLayoutCreateInfo.pBindings = bindings.data();

    	VK_CHECK(vkCreateDescriptorSetLayout(*m_Core, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout));

    	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = init::pipelineLayoutCreateInfo();
    	pipelineLayoutCreateInfo.setLayoutCount = 1;
    	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
    	VK_CHECK(vkCreatePipelineLayout(*m_Core, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		TCHAR workDirectory[MAX_PATH] = { 0 };
		GetModuleFileName( NULL, workDirectory, MAX_PATH );
		char* cwd = _getcwd( 0, 0 ) ; // **** microsoft specific ****
		std::string working_directory(cwd) ;
		std::free(cwd) ;
		
		std::cout << "Work Directory " << workDirectory;
    	// Ray generation group
    	{
    		shaderStages.emplace_back(VulkanShader::GetShaderModule(*m_Core, "assets/shaders/raytracingbasic/raygen.rgen.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR));

    		VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
    		shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    		shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    		shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
    		shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
    		shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
    		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
    		shaderGroups.push_back(shaderGroup);
    	}

    	// Miss group
    	{
    		shaderStages.emplace_back(VulkanShader::GetShaderModule(*m_Core, "assets/shaders/raytracingbasic/miss.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR));
    		VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
    		shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    		shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    		shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
    		shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
    		shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
    		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
    		shaderGroups.push_back(shaderGroup);
    	}
    	
    	// Closest hit group
    	{
    		shaderStages.emplace_back(VulkanShader::GetShaderModule(*m_Core, "assets/shaders/raytracingbasic/closesthit.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR));
		    
    		VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
    		shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    		shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    		shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
    		shaderGroup.closestHitShader = static_cast<uint32_t>(shaderStages.size()) - 1;
    		shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
    		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
    		shaderGroups.push_back(shaderGroup);
    	}

    	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI{};
    	rayTracingPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    	rayTracingPipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
    	rayTracingPipelineCI.pStages = shaderStages.data();
    	rayTracingPipelineCI.groupCount = static_cast<uint32_t>(shaderGroups.size());
    	rayTracingPipelineCI.pGroups = shaderGroups.data();
    	rayTracingPipelineCI.maxPipelineRayRecursionDepth = 1;
    	rayTracingPipelineCI.layout = pipelineLayout;
    	VK_CHECK(vkCreateRayTracingPipelinesKHR(*m_Core, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &pipeline));
    }

    void RTXBasic::createShaderBindingTable()
    {
    	const uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
    	const uint32_t handleSizeAligned = alignedSize(rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);
    	const uint32_t groupCount = static_cast<uint32_t>(shaderGroups.size());
    	const uint32_t sbtSize = groupCount * handleSizeAligned;
    	
    	std::vector<uint8_t> shaderHandleStorage(sbtSize);
    	VK_CHECK(vkGetRayTracingShaderGroupHandlesKHR(*m_Core, pipeline, 0, groupCount, sbtSize, shaderHandleStorage.data()));

    	const VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    	const VkMemoryPropertyFlags memoryUsageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    	VK_CHECK(m_Core->createBuffer(bufferUsageFlags, memoryUsageFlags, &raygenShaderBindingTable, handleSize));
    	VK_CHECK(m_Core->createBuffer(bufferUsageFlags, memoryUsageFlags, &missShaderBindingTable, handleSize));
    	VK_CHECK(m_Core->createBuffer(bufferUsageFlags, memoryUsageFlags, &hitShaderBindingTable, handleSize));

    	// Copy handles
    	raygenShaderBindingTable.map();
    	missShaderBindingTable.map();
    	hitShaderBindingTable.map();
    	memcpy(raygenShaderBindingTable.mapped, shaderHandleStorage.data(), handleSize);
    	memcpy(missShaderBindingTable.mapped, shaderHandleStorage.data() + handleSizeAligned, handleSize);
    	memcpy(hitShaderBindingTable.mapped, shaderHandleStorage.data() + handleSizeAligned * 2, handleSize);
    }

    void RTXBasic::createDescriptorSets()
    {
    	std::vector<VkDescriptorPoolSize> poolSizes = {
			{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 }
		};
    	
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = init::descriptorPoolCreateInfo();
    	descriptorPoolCreateInfo.maxSets = poolSizes.size();
    	descriptorPoolCreateInfo.poolSizeCount = poolSizes.size();
    	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
		VK_CHECK(vkCreateDescriptorPool(*m_Core, &descriptorPoolCreateInfo, nullptr, &descriptorPool));

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = init::descriptorSetAllocateInfo();
    	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
    	descriptorSetAllocateInfo.descriptorSetCount = 1;
    	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
		VK_CHECK(vkAllocateDescriptorSets(*m_Core, &descriptorSetAllocateInfo, &descriptorSet));

		VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo{};
		descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
		descriptorAccelerationStructureInfo.pAccelerationStructures = &topLevelAS.handle;

		VkWriteDescriptorSet accelerationStructureWrite{};
		accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// The specialized acceleration structure descriptor has to be chained
		accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo;
		accelerationStructureWrite.dstSet = descriptorSet;
		accelerationStructureWrite.dstBinding = 0;
		accelerationStructureWrite.descriptorCount = 1;
		accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

		VkDescriptorImageInfo storageImageDescriptor{};
		storageImageDescriptor.imageView = storageImage.view;
		storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkWriteDescriptorSet resultImageWrite{};
    	resultImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    	resultImageWrite.dstSet = descriptorSet;
    	resultImageWrite.dstBinding = 1;
    	resultImageWrite.descriptorCount = 1;
    	resultImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    	resultImageWrite.pImageInfo = &storageImageDescriptor;

		VkDescriptorBufferInfo uniformBufferDescriptorInfo{};
    	uniformBufferDescriptorInfo.buffer = ubo.buffer;
    	uniformBufferDescriptorInfo.offset = 0;
    	uniformBufferDescriptorInfo.range = ubo.size;
    	
		VkWriteDescriptorSet uniformBufferWrite{};
    	uniformBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    	uniformBufferWrite.dstSet = descriptorSet;
    	uniformBufferWrite.dstBinding = 2;
    	uniformBufferWrite.descriptorCount = 1;
    	uniformBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    	uniformBufferWrite.pBufferInfo = &uniformBufferDescriptorInfo;
    	
		std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			accelerationStructureWrite,
			resultImageWrite,
			uniformBufferWrite
		};
		vkUpdateDescriptorSets(*m_Core, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, VK_NULL_HANDLE);
    }

    void RTXBasic::buildCommandBuffers()
    {
    	
    	VkCommandBufferBeginInfo cmdBufInfo = init::commandBufferBeginInfo();

		VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    	uint32_t width = m_Core->swapchain.extent.width;
    	uint32_t height = m_Core->swapchain.extent.height;
    	
		for (int32_t i = 0; i < m_Core->resources.drawCmdBuffers.size(); ++i)
		{
			VK_CHECK(vkBeginCommandBuffer(m_Core->resources.drawCmdBuffers[i], &cmdBufInfo));

			/*
				Setup the buffer regions pointing to the shaders in our shader binding table
			*/

			const uint32_t handleSizeAligned = alignedSize(rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);

			VkStridedDeviceAddressRegionKHR raygenShaderSbtEntry{};
			raygenShaderSbtEntry.deviceAddress = getBufferDeviceAddress(raygenShaderBindingTable.buffer);
			raygenShaderSbtEntry.stride = handleSizeAligned;
			raygenShaderSbtEntry.size = handleSizeAligned;

			VkStridedDeviceAddressRegionKHR missShaderSbtEntry{};
			missShaderSbtEntry.deviceAddress = getBufferDeviceAddress(missShaderBindingTable.buffer);
			missShaderSbtEntry.stride = handleSizeAligned;
			missShaderSbtEntry.size = handleSizeAligned;

			VkStridedDeviceAddressRegionKHR hitShaderSbtEntry{};
			hitShaderSbtEntry.deviceAddress = getBufferDeviceAddress(hitShaderBindingTable.buffer);
			hitShaderSbtEntry.stride = handleSizeAligned;
			hitShaderSbtEntry.size = handleSizeAligned;

			VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};

			/*
				Dispatch the ray tracing commands
			*/
			vkCmdBindPipeline(m_Core->resources.drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
			vkCmdBindDescriptorSets(m_Core->resources.drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineLayout, 0, 1, &descriptorSet, 0, 0);

			vkCmdTraceRaysKHR(
				m_Core->resources.drawCmdBuffers[i],
				&raygenShaderSbtEntry,
				&missShaderSbtEntry,
				&hitShaderSbtEntry,
				&callableShaderSbtEntry,
				width,
				height,
				1);

			/*
				Copy ray tracing output to swap chain image
			*/

			// Prepare current swap chain image as transfer destination
			vkTool::setImageLayout(
				m_Core->resources.drawCmdBuffers[i],
				m_Core->swapchain.images[i],
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				subresourceRange);

			// Prepare ray tracing output image as transfer source
			vkTool::setImageLayout(
				m_Core->resources.drawCmdBuffers[i],
				storageImage.image,
				VK_IMAGE_LAYOUT_GENERAL,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				subresourceRange);

			VkImageCopy copyRegion{};
			copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			copyRegion.srcOffset = { 0, 0, 0 };
			copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			copyRegion.dstOffset = { 0, 0, 0 };
			copyRegion.extent = { width, height, 1 };
			vkCmdCopyImage(m_Core->resources.drawCmdBuffers[i], storageImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_Core->swapchain.images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			// Transition swap chain image back for presentation
			vkTool::setImageLayout(
				m_Core->resources.drawCmdBuffers[i],
				m_Core->swapchain.images[i],
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				subresourceRange);

			// Transition ray tracing output image back to general layout
			vkTool::setImageLayout(
				m_Core->resources.drawCmdBuffers[i],
				storageImage.image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_IMAGE_LAYOUT_GENERAL,
				subresourceRange);

			VK_CHECK(vkEndCommandBuffer(m_Core->resources.drawCmdBuffers[i]));
		}
    }

    void RTXBasic::updateUniformBuffers()
    {
    	uniformData.projInverse = glm::inverse(m_Camera->getProjectionMatrix());
    	uniformData.viewInverse = glm::inverse(m_Camera->getViewMatrix());
    	memcpy(ubo.mapped, &uniformData, sizeof(uniformData));
    }

    void RTXBasic::onMouseScrollMoved(GLFWwindow* window, double dX, double dY)
    {
	    cameraSpeed = std::clamp(cameraSpeed + dY, 1.0, 10.0);
    }
}
