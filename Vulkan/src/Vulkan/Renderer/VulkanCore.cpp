#include "pch.h"
#include "VulkanCore.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


VulkanCore::VulkanCore()
{
	createInstance();
	createPhysicalDevice();
	createLogicalDevice();
	createSwapchain();
	createDepthStencilBuffer();
	createCommandBuffers();
	createRenderPass();
	createFrameBuffer();

	createSyncObjs();

	initImGui();
}

VulkanCore::VulkanCore(std::vector<const char*> enabledDeviceExtensions)
{
	createInstance();
	createPhysicalDevice();
	checkEnabledDeviceFeatures(enabledDeviceExtensions);
	createLogicalDevice();
	createSwapchain();
	createDepthStencilBuffer();
	createCommandBuffers();
	createRenderPass();
	createFrameBuffer();

	createSyncObjs();
	initImGui();
}

VulkanCore::~VulkanCore()
{
#ifdef ENABLE_VALIDATION_LAYERS
	if (m_DebugMessenger != VK_NULL_HANDLE)
		vkDestroyDebugUtilsMessenger(m_Instance, m_DebugMessenger, nullptr);
#endif //ENABLE_VALIDATION_LAYERS

	vkDestroyDescriptorPool(m_Device, imgui.descriptorPool, nullptr);

	vkDestroySemaphore(m_Device, sync.imageRendered, nullptr);
	vkDestroySemaphore(m_Device, sync.imageAvaible, nullptr);

	for (auto& f : resources.frameBuffers)
	{
		vkDestroyFramebuffer(m_Device, f, nullptr);
	}

	if (depthStencil.memory)
	{
		vkDestroyImageView(m_Device, depthStencil.view, nullptr);
		vkDestroyImage(m_Device, depthStencil.image, nullptr);
		vkFreeMemory(m_Device, depthStencil.memory, nullptr);
	}

	vkDestroyCommandPool(m_Device, resources.commandPool, nullptr);
	vkDestroyCommandPool(m_Device, resources.copyCommandPool, nullptr);
	vkDestroyRenderPass(m_Device, resources.renderPass, nullptr);

	for (size_t i = 0; i < swapchain.imageViews.size(); i++)
	{
		vkDestroyImageView(m_Device, swapchain.imageViews[i], nullptr);
	}
	vkDestroySwapchainKHR(m_Device, swapchain.swapchainKHR, nullptr);
	vkDestroySurfaceKHR(m_Instance, swapchain.surface, nullptr);

	vkDestroyDevice(m_Device, nullptr);
	vkDestroyInstance(m_Instance, nullptr);
}

void VulkanCore::enableDeviceExtension(const std::vector<const char*>& extensionList)
{
	for (int i = 0; i < extensionList.size(); i++)
		if (supportedDeviceExtension(extensionList.at(i)))
			m_EnabledDeviceExtension.push_back(extensionList.at(i));
}

const bool VulkanCore::supportedInstanceExtension(const std::string& extensionName) const
{
	return std::find(m_SupportedInstanceExtension.begin(), m_SupportedInstanceExtension.end(), extensionName) != m_SupportedInstanceExtension.end();
}

const bool VulkanCore::supportedDeviceExtension(const std::string& extensionName) const
{
	return std::find(m_SupportedDeviceExtension.begin(), m_SupportedDeviceExtension.end(), extensionName) != m_SupportedDeviceExtension.end();
}

const uint32_t VulkanCore::getQueueIndex(VkQueueFlagBits queueFlag) const
{
	
	if (queueFlag & VK_QUEUE_COMPUTE_BIT)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(m_QueueProperties.size()); i++)
		{
			if ((m_QueueProperties.at(i).queueFlags & queueFlag) && ((m_QueueProperties.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				return i;
		}
	}

	if (queueFlag & VK_QUEUE_TRANSFER_BIT)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(m_QueueProperties.size()); i++)
		{
			if ((m_QueueProperties.at(i).queueFlags & queueFlag) && ((m_QueueProperties.at(i).queueFlags & VK_QUEUE_COMPUTE_BIT) == 0) && ((m_QueueProperties.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				return i;
		}
	}
	
	for (uint32_t i = 0; i < static_cast<uint32_t>(m_QueueProperties.size()); i++)
	{
		if (m_QueueProperties.at(i).queueFlags & queueFlag)
			return i;
	}

	VK_ASSERT(false, "Could not find queue family index!");

	return UINT64_MAX;
}

const uint32_t VulkanCore::getMemoryType(uint32_t typeBit, VkMemoryPropertyFlags memoryFlag) const
{
	
	for (uint32_t i = 0; i < m_PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((typeBit & 1) == 1)
		{
			if (m_PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memoryFlag)
				return i;
		}

		typeBit >>= 1;
	}

	VK_ASSERT(false, "Could not find required memory type!");
	
	return UINT64_MAX;
}

VkCommandBuffer VulkanCore::createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin)
{
	VkCommandBuffer cmdBuffer;

	VkCommandBufferAllocateInfo cmdBufferAI = init::commandBufferAllocateInfo();
	cmdBufferAI.commandBufferCount = 1;
	cmdBufferAI.commandPool = pool;
	cmdBufferAI.level = level;

	VK_CHECK(vkAllocateCommandBuffers(m_Device, &cmdBufferAI, &cmdBuffer));

	if (begin)
	{
		VkCommandBufferBeginInfo beginInfo = init::cmdBufferBeginInfo();
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
	}

	return cmdBuffer;
}

VkCommandBuffer VulkanCore::createCommandBuffer(VkCommandBufferLevel level, bool begin)
{
	VkCommandBuffer cmdBuffer;

	VkCommandBufferAllocateInfo cmdBufferAI = init::commandBufferAllocateInfo();
	cmdBufferAI.commandBufferCount = 1;
	cmdBufferAI.commandPool = resources.commandPool;
	cmdBufferAI.level = level;

	VK_CHECK(vkAllocateCommandBuffers(m_Device, &cmdBufferAI, &cmdBuffer));

	if (begin)
	{
		VkCommandBufferBeginInfo beginInfo = init::cmdBufferBeginInfo();
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
	}

	return cmdBuffer;
}

VkCommandBuffer VulkanCore::createCopyCommandBuffer(VkCommandBufferLevel level, bool begin)
{
	VkCommandBuffer cmdBuffer;

	VkCommandBufferAllocateInfo cmdBufferAI = init::commandBufferAllocateInfo();
	cmdBufferAI.commandBufferCount = 1;
	cmdBufferAI.commandPool = resources.copyCommandPool;
	cmdBufferAI.level = level;

	VK_CHECK(vkAllocateCommandBuffers(m_Device, &cmdBufferAI, &cmdBuffer));

	if (begin)
	{
		VkCommandBufferBeginInfo beginInfo = init::cmdBufferBeginInfo();
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK(vkBeginCommandBuffer(cmdBuffer, &beginInfo));
	}

	return cmdBuffer;
}

void VulkanCore::BeginScene()
{
	VkResult err = vkAcquireNextImageKHR(m_Device, swapchain.swapchainKHR, UINT64_MAX, sync.imageAvaible, VK_NULL_HANDLE, &resources.imageIndex);

	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
		windowResized();
	else
		VK_CHECK(err);
}

VkResult VulkanCore::Submit()
{
	VkPresentInfoKHR presentInfo = init::presentInfo();
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &sync.imageRendered;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain.swapchainKHR;
	presentInfo.pImageIndices = &resources.imageIndex;

	VkResult err = vkQueuePresentKHR(queue.GraphicsQueue, &presentInfo);

	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
		windowResized();
	else
		VK_CHECK(err);

	return err;
}

void VulkanCore::createInstance()
{
	VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.pApplicationName = "Vulkan Application";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Vulkan Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };
#ifdef PLATFORM_WINDOWS
	instanceExtensions.push_back("VK_KHR_win32_surface");
#endif //PLATFORM_WINDOWS
#ifdef ENABLE_VALIDATION_LAYERS
	instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // ENABLE_VALIDATION_LAYERS

	uint32_t extensionCount;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	if (extensionCount > 0)
	{
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		for (auto& extension : extensions)
		{
			m_SupportedInstanceExtension.push_back(extension.extensionName);
		}
	}

	VkInstanceCreateInfo instanceCreateInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	instanceCreateInfo.enabledLayerCount = 0;

#ifdef ENABLE_VALIDATION_LAYERS
	const char* validationLayer = "VK_LAYER_KHRONOS_validation";
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	if (layerCount > 0)
	{
		std::vector<VkLayerProperties> layerProperties(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

		for (auto& layer : layerProperties)
		{
			if (strcmp(layer.layerName, validationLayer) == 0)
			{
				m_EnableValidation = true;
				instanceCreateInfo.enabledLayerCount = 1;
				instanceCreateInfo.ppEnabledLayerNames = &validationLayer;
				break;
			}
		}
	}
	if (!m_EnableValidation)
		VK_CORE_ERROR("Can't enable validation error");
#endif // ENABLE_VALIDATION_LAYERS

	VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance));

#ifdef ENABLE_VALIDATION_LAYERS
	createDebugMessenger();
#endif // ENABLE_VALIDATION_LAYERS

}

void VulkanCore::createPhysicalDevice()
{
	uint32_t gpuCount;
	vkEnumeratePhysicalDevices(m_Instance, &gpuCount, nullptr);

	if (gpuCount > 0)
	{
		std::vector<VkPhysicalDevice> gpuDevices(gpuCount);
		vkEnumeratePhysicalDevices(m_Instance, &gpuCount, gpuDevices.data());

		for (auto& gpu : gpuDevices)
		{
			VkPhysicalDeviceProperties gpuProperty;
			vkGetPhysicalDeviceProperties(gpu, &gpuProperty);

			if (gpuProperty.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				m_PhysicalDevice = gpu;
				break;
			}
		}
	}
	else
	{
		VK_ASSERT(false, "Could not find any GPU!");
	}

	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_PhysicalDeviceProperties);
	vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_PhysicalDeviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_PhysicalDeviceMemoryProperties);

	m_EnabledDeviceFeatures.samplerAnisotropy = VK_TRUE;

	uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueCount, nullptr);
	if (queueCount > 0)
	{
		m_QueueProperties.resize(queueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueCount, m_QueueProperties.data());
	}

	uint32_t extCount;
	vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extCount, nullptr);

	if (extCount > 0)
	{
		uint32_t index = 0;
		m_SupportedDeviceExtension.resize(extCount);
		std::vector<VkExtensionProperties> extensionProperties(extCount);
		vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extCount, extensionProperties.data());

		for (size_t i = 0; i < extensionProperties.size(); i++)
		{
			m_SupportedDeviceExtension[i] = extensionProperties[i].extensionName;
		}
	}

}

void VulkanCore::createLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfo;
	const float queuePriorty = 1.0f;

	VkDeviceQueueCreateInfo graphicsQueueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueIndex.Graphics = getQueueIndex(VK_QUEUE_GRAPHICS_BIT);	
	graphicsQueueCreateInfo.queueFamilyIndex = queueIndex.Graphics;
	graphicsQueueCreateInfo.queueCount = 1;
	graphicsQueueCreateInfo.pQueuePriorities = &queuePriorty;
	deviceQueueCreateInfo.push_back(graphicsQueueCreateInfo);

	VkDeviceQueueCreateInfo transferQueueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueIndex.Transfer = getQueueIndex(VK_QUEUE_TRANSFER_BIT);
	transferQueueCreateInfo.queueFamilyIndex = queueIndex.Transfer;
	transferQueueCreateInfo.queueCount = 1;
	transferQueueCreateInfo.pQueuePriorities = &queuePriorty;
	deviceQueueCreateInfo.push_back(transferQueueCreateInfo);

	queueIndex.Compute = getQueueIndex(VK_QUEUE_COMPUTE_BIT);

	if (queueIndex.Graphics != queueIndex.Compute)
	{
		VkDeviceQueueCreateInfo computeQueueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		computeQueueCreateInfo.queueFamilyIndex = queueIndex.Compute;
		computeQueueCreateInfo.queueCount = 1;
		computeQueueCreateInfo.pQueuePriorities = &queuePriorty;
		deviceQueueCreateInfo.push_back(computeQueueCreateInfo);
	}

	/*if(supportedDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
		m_EnabledDeviceExtension.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);*/

	VkDeviceCreateInfo deviceCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	deviceCreateInfo.queueCreateInfoCount = (uint32_t)deviceQueueCreateInfo.size();
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfo.data();
	deviceCreateInfo.enabledExtensionCount = (uint32_t)m_EnabledDeviceExtension.size();
	deviceCreateInfo.ppEnabledExtensionNames = m_EnabledDeviceExtension.data();
	deviceCreateInfo.pEnabledFeatures = &m_EnabledDeviceFeatures;

	VK_CHECK(vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device));

	vkGetDeviceQueue(m_Device, queueIndex.Graphics, 0, &queue.GraphicsQueue);
	vkGetDeviceQueue(m_Device, queueIndex.Transfer, 0, &queue.TransferQueue);
	vkGetDeviceQueue(m_Device, queueIndex.Compute, 0, &queue.ComputeQueue);
}

void VulkanCore::createSwapchain()
{
	static bool surfaceSet = false;
	if(!surfaceSet)
		VK_CHECK(glfwCreateWindowSurface(m_Instance, static_cast<GLFWwindow*>(Window::GetWindow()), nullptr, &swapchain.surface));

	surfaceSet = true;

	uint32_t graphicsQueue = UINT32_MAX;
	uint32_t presentQueue  = UINT32_MAX;

	for (uint32_t i = 0; i < m_QueueProperties.size(); i++)
	{
		VkBool32 isSupported;
		VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, swapchain.surface, &isSupported));
		
		if (m_QueueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			if (graphicsQueue == UINT32_MAX)
				graphicsQueue = i;

			if (isSupported && (presentQueue == UINT32_MAX))
				presentQueue = i;
		}

	}

	if (graphicsQueue == presentQueue)
		swapchain.queueIndex = graphicsQueue;
	else
		VK_ERROR("Present and graphics queues are different!");


	uint32_t surfaceFormatCount;
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, swapchain.surface, &surfaceFormatCount, nullptr));
	VK_ASSERT(surfaceFormatCount, "Surface format count is 0!");
	std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, swapchain.surface, &surfaceFormatCount, surfaceFormats.data()));

	for (auto& surfaceFormat : surfaceFormats)
	{
		if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB)
		{
			swapchain.colorFormat = surfaceFormat.format;
			swapchain.colorSpace  = surfaceFormat.colorSpace;
			break;
		}
		else
		{
			swapchain.colorFormat = surfaceFormats[0].format;
			swapchain.colorSpace  = surfaceFormats[0].colorSpace;
		}
	}

	VkSurfaceCapabilitiesKHR surfaceCap;
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, swapchain.surface, &surfaceCap));

	

	if (surfaceCap.currentExtent.width == 0xFFFFFFFF)
	{
		int width, height;
		glfwGetFramebufferSize(static_cast<GLFWwindow*>(Window::GetWindow()), &width, &height);

		swapchain.extent.width  = (uint32_t)width;
		swapchain.extent.height = (uint32_t)height;
	}
	else
	{
		swapchain.extent = surfaceCap.currentExtent;
		swapchain.extent.width  = surfaceCap.currentExtent.width;
		swapchain.extent.height = surfaceCap.currentExtent.height;
	}

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

	uint32_t presentModeCount;
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, swapchain.surface, &presentModeCount, nullptr));

	if (presentModeCount > 0)
	{
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, swapchain.surface, &presentModeCount, presentModes.data()));

		if (!swapchain.isVsync)
		{
			for (auto& mode : presentModes)
			{
				if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}
				if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
					presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}

	}

	uint32_t desiredNumberofSwapchainImages = surfaceCap.minImageCount + 1;

	if ((desiredNumberofSwapchainImages > surfaceCap.maxImageCount) && (surfaceCap.maxImageCount > 0))
		desiredNumberofSwapchainImages = surfaceCap.maxImageCount;

	VkSwapchainCreateInfoKHR swapchainCreateInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	swapchainCreateInfo.surface = swapchain.surface;
	swapchainCreateInfo.minImageCount = desiredNumberofSwapchainImages;
	swapchainCreateInfo.imageFormat = swapchain.colorFormat;
	swapchainCreateInfo.imageColorSpace = swapchain.colorSpace;
	swapchainCreateInfo.imageExtent = swapchain.extent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 1;
	swapchainCreateInfo.pQueueFamilyIndices = &swapchain.queueIndex;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	if (surfaceCap.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	else
		swapchainCreateInfo.preTransform = surfaceCap.currentTransform;

	if (surfaceCap.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	if (surfaceCap.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
		swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	VK_CHECK(vkCreateSwapchainKHR(m_Device, &swapchainCreateInfo, nullptr, &swapchain.swapchainKHR));

	uint32_t swapchainImageCount;
	VK_CHECK(vkGetSwapchainImagesKHR(m_Device, swapchain.swapchainKHR, &swapchainImageCount, nullptr));
	swapchain.images.resize(swapchainImageCount);
	VK_CHECK(vkGetSwapchainImagesKHR(m_Device, swapchain.swapchainKHR, &swapchainImageCount, swapchain.images.data()));
	swapchain.imageViews.resize(swapchainImageCount);

	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		VkImageViewCreateInfo imageViewCI{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		imageViewCI.image = swapchain.images[i];
		imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCI.format = swapchain.colorFormat;
		imageViewCI.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
		imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCI.subresourceRange.layerCount = 1;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.levelCount = 1;
		imageViewCI.subresourceRange.baseMipLevel = 0;

		VK_CHECK(vkCreateImageView(m_Device, &imageViewCI, nullptr, &swapchain.imageViews[i]));
	}
}

void VulkanCore::createDepthStencilBuffer()
{
	VkImageCreateInfo imageCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = depthStencil.format;
	imageCreateInfo.extent = { swapchain.extent.width, swapchain.extent.height, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VK_CHECK(vkCreateImage(m_Device, &imageCreateInfo, nullptr, &depthStencil.image));

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(m_Device, depthStencil.image, &memReqs);

	VkMemoryAllocateInfo memAllocInfo = init::memAllocInfo();
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_CHECK(vkAllocateMemory(m_Device, &memAllocInfo, nullptr, &depthStencil.memory));
	VK_CHECK(vkBindImageMemory(m_Device, depthStencil.image, depthStencil.memory, 0));

	VkImageViewCreateInfo imageViewCI{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	imageViewCI.image = depthStencil.image;
	imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCI.format = VK_FORMAT_D24_UNORM_S8_UINT;
	imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	imageViewCI.subresourceRange.layerCount = 1;
	imageViewCI.subresourceRange.baseArrayLayer = 0;
	imageViewCI.subresourceRange.levelCount = 1;
	imageViewCI.subresourceRange.baseMipLevel = 0;

	VK_CHECK(vkCreateImageView(m_Device, &imageViewCI, nullptr, &depthStencil.view));
}

void VulkanCore::createCommandBuffers()
{
	VkCommandPoolCreateInfo commandPoolCI = init::commandPoolCreateInfo();
	commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCI.queueFamilyIndex = swapchain.queueIndex;

	VK_CHECK(vkCreateCommandPool(m_Device, &commandPoolCI, nullptr, &resources.commandPool));

	VkCommandPoolCreateInfo copyCommandPoolCI = init::commandPoolCreateInfo();
	copyCommandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	copyCommandPoolCI.queueFamilyIndex = queueIndex.Transfer;

	VK_CHECK(vkCreateCommandPool(m_Device, &copyCommandPoolCI, nullptr, &resources.copyCommandPool));

	resources.drawCmdBuffers.resize(swapchain.images.size());

	VkCommandBufferAllocateInfo cmdBufferAlloc = init::commandBufferAllocateInfo();
	cmdBufferAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAlloc.commandPool = resources.commandPool;
	cmdBufferAlloc.commandBufferCount = resources.drawCmdBuffers.size();

	VK_CHECK(vkAllocateCommandBuffers(m_Device, &cmdBufferAlloc, resources.drawCmdBuffers.data()));

}

void VulkanCore::createRenderPass()
{
	VkAttachmentDescription descriptions[2] = {};

	descriptions[0].format = swapchain.colorFormat;
	descriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
	descriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	descriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	descriptions[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	descriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	descriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	descriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	descriptions[1].format = depthStencil.format;
	descriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
	descriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	descriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	descriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	descriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	descriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	descriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference{};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	
	
	VkAttachmentReference depthReference{};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorReference;
	subpass.pDepthStencilAttachment = &depthReference;

	/*VkSubpassDependency subpassDependency{};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask  = 0;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;*/

	VkSubpassDependency subpassDependencies[2] = {};

	// Transition from final to initial (VK_SUBPASS_EXTERNAL refers to all commands executed outside of the actual renderpass)
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Transition from initial to final
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassCI{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassCI.attachmentCount = 2;
	renderPassCI.pAttachments = descriptions;
	renderPassCI.subpassCount = 1;
	renderPassCI.pSubpasses = &subpass;
	renderPassCI.dependencyCount = 2;
	renderPassCI.pDependencies = subpassDependencies;

	VK_CHECK(vkCreateRenderPass(m_Device, &renderPassCI, nullptr, &resources.renderPass));
}

void VulkanCore::createFrameBuffer()
{
	resources.frameBuffers.resize(swapchain.images.size());

	VkImageView attachments[2];

	attachments[1] = depthStencil.view;

	VkFramebufferCreateInfo frameBufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	frameBufferInfo.renderPass = resources.renderPass;
	frameBufferInfo.attachmentCount = 2;
	frameBufferInfo.pAttachments = attachments;
	frameBufferInfo.width = swapchain.extent.width;
	frameBufferInfo.height = swapchain.extent.height;
	frameBufferInfo.layers = 1;

	for (uint32_t i = 0; i < resources.frameBuffers.size(); i++)
	{
		attachments[0] = swapchain.imageViews[i];

		VK_CHECK(vkCreateFramebuffer(m_Device, &frameBufferInfo, nullptr, &resources.frameBuffers[i]));
	}
}

void VulkanCore::createSyncObjs()
{
	VkSemaphoreCreateInfo semaphoreCreateInfo = init::semaphoreCreateInfo();

	VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &sync.imageAvaible));
	VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &sync.imageRendered));

	resources.submitInfo = init::submitInfo();
	resources.submitInfo.waitSemaphoreCount = 1;
	resources.submitInfo.pWaitSemaphores = &sync.imageAvaible;
	resources.submitInfo.pWaitDstStageMask = resources.waitDstMask;
	resources.submitInfo.signalSemaphoreCount = 1;
	resources.submitInfo.pSignalSemaphores = &sync.imageRendered;

}


void VulkanCore::checkEnabledDeviceFeatures(std::vector<const char*> enabledDeviceExtensions)
{
	for (size_t i = 0; i < enabledDeviceExtensions.size(); i++)
	{
		if (supportedDeviceExtension(enabledDeviceExtensions[i]))
		{
			m_EnabledDeviceExtension.push_back(enabledDeviceExtensions[i]);
		}
	}
}

void VulkanCore::initImGui()
{
		//1: create descriptor pool for IMGUI
	// the size of the pool is very oversize, but it's copied from imgui demo itself.
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	VkDescriptorPool imguiPool;
	VK_CHECK(vkCreateDescriptorPool(m_Device, &pool_info, nullptr, &imgui.descriptorPool));


	// 2: initialize imgui library

	//this initializes the core structures of imgui
	ImGui::CreateContext();

	//this initializes imgui for SDL
	ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(Window::GetWindow()), true);

	//this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = m_Instance;
	init_info.PhysicalDevice = m_PhysicalDevice;
	init_info.Device = m_Device;
	init_info.Queue = queue.GraphicsQueue;
	init_info.DescriptorPool = imgui.descriptorPool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;

	ImGui_ImplVulkan_Init(&init_info, resources.renderPass);

	VkCommandBufferAllocateInfo cmdAllocInfo = init::commandBufferAllocateInfo();
	cmdAllocInfo.commandBufferCount = 1;
	cmdAllocInfo.commandPool = resources.commandPool;
	cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	vkAllocateCommandBuffers(m_Device, &cmdAllocInfo, &imgui.cmdBuffer);

	VkCommandBufferBeginInfo cmdInfo = init::commandBufferBeginInfo();
	cmdInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	
	vkBeginCommandBuffer(imgui.cmdBuffer, &cmdInfo);

	//execute a gpu command to upload imgui font textures
	ImGui_ImplVulkan_CreateFontsTexture(imgui.cmdBuffer);

	VK_CHECK(vkEndCommandBuffer(imgui.cmdBuffer));

	VkSubmitInfo end_info = {};
	end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	end_info.commandBufferCount = 1;
	end_info.pCommandBuffers = &imgui.cmdBuffer;
	VK_CHECK(vkQueueSubmit(queue.GraphicsQueue, 1, &end_info, VK_NULL_HANDLE));

	VK_CHECK(vkDeviceWaitIdle(m_Device));
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

#ifdef ENABLE_VALIDATION_LAYERS

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		VK_CORE_INFO(pCallbackData->pMessage); return VK_FALSE;
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		VK_CORE_WARN(pCallbackData->pMessage);  return VK_FALSE;
	}
	else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		VK_CORE_ERROR(pCallbackData->pMessage); return VK_FALSE;
	}

	return VK_FALSE;
}

void VulkanCore::createDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
	debugUtilsCreateInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugUtilsCreateInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugUtilsCreateInfo.pfnUserCallback = debugCallback;

	vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
	vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");

	VK_CHECK(vkCreateDebugUtilsMessenger(m_Instance, &debugUtilsCreateInfo, nullptr, &m_DebugMessenger));
}

#endif //ENABLE_VALIDATION_LAYERS

void VulkanCore::windowResized()
{
	vkQueueWaitIdle(queue.GraphicsQueue);

	for (auto& f : resources.frameBuffers)
	{
		vkDestroyFramebuffer(m_Device, f, nullptr);
	}

	//vkFreeCommandBuffers(m_Device, resources.commandPool, resources.drawCmdBuffers.size(), resources.drawCmdBuffers.data());

	if (depthStencil.memory)
	{
		vkDestroyImageView(m_Device, depthStencil.view, nullptr);
		vkDestroyImage(m_Device, depthStencil.image, nullptr);
		vkFreeMemory(m_Device, depthStencil.memory, nullptr);
	}

	vkDestroyRenderPass(m_Device, resources.renderPass, nullptr);

	for (size_t i = 0; i < swapchain.imageViews.size(); i++)
	{
		vkDestroyImageView(m_Device, swapchain.imageViews[i], nullptr);
	}
	vkDestroySwapchainKHR(m_Device, swapchain.swapchainKHR, nullptr);

	createSwapchain();
	createDepthStencilBuffer();
	//createCommandBuffers();
	createRenderPass();
	createFrameBuffer();
}
