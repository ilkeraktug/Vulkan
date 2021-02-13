#include "pch.h"
#include "Vulkan.h"

#include "Vulkan\Core.h"
#include "Vulkan\Window.h"

#include "Shader.h"

Vulkan::Vulkan()
{
	m_VertexShader.reset(new Shader("assets/shaders/vert.spv"));
	m_FragmentShader.reset(new Shader("assets/shaders/frag.spv"));

	Init();
}

Vulkan::~Vulkan()
{
	Shutdown();
}

void Vulkan::Init()
{

	VkApplicationInfo info{};
	info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	info.pApplicationName = "Vulkan App";
	info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	info.pEngineName = "No Engine";
	info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &info;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> glfwExtensionsVector(glfwExtensions, glfwExtensions + glfwExtensionCount);

	#if ENABLE_VALIDATION_LAYERS
	glfwExtensionsVector.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	#endif // ENABLE_VALIDATION_LAYERS

	createInfo.enabledExtensionCount = glfwExtensionsVector.size();
	createInfo.ppEnabledExtensionNames = glfwExtensionsVector.data();

	createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
	createInfo.enabledLayerCount = m_ValidationLayers.size();
	
#if ENABLE_VALIDATION_LAYERS
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCreateInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugCreateInfo.messageType = 
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugCreateInfo.pfnUserCallback = debugCallback;

	createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)(&debugCreateInfo);
#else
	createInfo.enabledLayerCount = 0;
	createInfo.pNext = nullptr;
#endif // ENABLE_VALIDATION_LAYERS
	


	VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
	VK_ASSERT((result == VK_SUCCESS), "Can't create instance!");

#if ENABLE_VALIDATION_LAYERS
	auto debugCreateFunc = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
	VK_ASSERT(debugCreateFunc(m_Instance, &debugCreateInfo, nullptr, &m_DebugMessenger) == VK_SUCCESS, 
		"Failed to create Debug Messenger!");

#endif // ENABLE_VALIDATION_LAYERS

	uint32_t extensionCount;
	result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	VK_ASSERT((result == VK_SUCCESS), "Can't get extension counts!");
	VK_CORE_TRACE("Extension count {0}", extensionCount);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	for (auto& extesion : extensions)
	{
		VK_CORE_TRACE("Extension {0}", extesion.extensionName);
	}

	result = glfwCreateWindowSurface(m_Instance, static_cast<GLFWwindow*>(Window::GetWindow()), nullptr, &m_Surface);
	VK_ASSERT((result == VK_SUCCESS), "Failed to create Surface!");

	findDevice();

	VkDeviceQueueCreateInfo qCreateInfo = {};
	qCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	qCreateInfo.queueFamilyIndex = index.Graphics.value();
	qCreateInfo.queueCount = 1;
	const float qPriorty = 1.0;
	qCreateInfo.pQueuePriorities = &qPriorty;

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = &qCreateInfo;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledLayerCount = 0;

	m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VK_ASSERT(hasRequiredExtensions(), "Does not have required extensions!");

	deviceCreateInfo.enabledExtensionCount = m_DeviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

	result = vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device);
	VK_ASSERT((result == VK_SUCCESS), "Failed to create Device!");
	vkGetDeviceQueue(m_Device, index.Graphics.value(), 0, &m_GraphicsQ);
	vkGetDeviceQueue(m_Device, index.Present.value(), 0, &m_PresentQ);

	SelectSwapChainObject();
	createSwapChainImageView();
	createRenderer();
	createShaderModules();
	createFrameBuffer();
	createCommandPool();
	createCommandBuffer();
	createSemaphore();
}
	
void Vulkan::Shutdown()
{
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);

	for (auto& frameBuffer : m_SwapChainFrameBuffers)
		vkDestroyFramebuffer(m_Device, frameBuffer, nullptr);

	for (auto& imgView : m_SwapchainImageView)
		vkDestroyImageView(m_Device, imgView, nullptr);
	
	vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
	vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
	vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
	vkDestroySemaphore(m_Device, m_ImageAvaible, nullptr);
	vkDestroySemaphore(m_Device, m_RenderFinished, nullptr);

	vkDestroyDevice(m_Device, nullptr);

#if ENABLE_VALIDATION_LAYERS
	auto destroyDebugMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
	destroyDebugMessenger(m_Instance, m_DebugMessenger, nullptr);
#endif // ENABLE_VALIDATION_LAYERS

	vkDestroyInstance(m_Instance, nullptr);
}

void Vulkan::Run()
{
	uint32_t imageIndex;
	auto a = vkAcquireNextImageKHR(m_Device, m_SwapChain, UINT64_MAX, m_ImageAvaible, VK_NULL_HANDLE, &imageIndex);

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	
	VkSemaphore waitSemaphores[] = { m_ImageAvaible };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex];
	
	VkSemaphore signalSemaphores[] = { m_RenderFinished };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VK_ASSERT(vkQueueSubmit(m_GraphicsQ, 1, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS,
		"Failed to queue submit");

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_SwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(m_PresentQ, &presentInfo);

}

void Vulkan::findDevice()
{
	m_PhysicalDevice = VK_NULL_HANDLE;
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

	VK_ASSERT(deviceCount, "Couldn't find physical device!");

	VkPhysicalDevice* devices = new VkPhysicalDevice[deviceCount];
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices);

	for (uint32_t i = 0; i < deviceCount; i++)
	{
		m_PhysicalDevice = devices[i];

		VkPhysicalDeviceProperties deviceProperty;
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProperty);

		VkPhysicalDeviceFeatures deviceFeature;
		vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &deviceFeature);
		
		uint32_t familyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &familyCount, nullptr);
		std::vector<VkQueueFamilyProperties> qFamilyProperties(familyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &familyCount, qFamilyProperties.data());

		int j = 0;
		for (const auto& property : qFamilyProperties)
		{
			if (property.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				index.Graphics = j;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, j, m_Surface, &presentSupport);
			if (presentSupport)
				index.Present = j;

			if (index.hasValue())
				break;

			j++;
		}

		if (deviceProperty.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && index.Graphics.has_value() && index.Present.has_value())
		{
			VK_INFO("Renderer GPU Name : {0}", deviceProperty.deviceName);
			break;
		}
	}


	delete[] devices;
}

bool Vulkan::hasRequiredExtensions()
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> avaibleExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, avaibleExtensions.data());

	std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

	for (auto& e : avaibleExtensions)
		requiredExtensions.erase(e.extensionName);

	return requiredExtensions.empty();
}

void Vulkan::SelectSwapChainObject()
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &swapChainDetails.Capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, nullptr);
	VK_ASSERT(formatCount, "Format count is 0!");
	swapChainDetails.Formats.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, swapChainDetails.Formats.data());

	uint32_t presentCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &presentCount, nullptr);
	VK_ASSERT(presentCount, "Present count is 0!");
	swapChainDetails.Present.resize(presentCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &presentCount, swapChainDetails.Present.data());

	VK_ASSERT(!swapChainDetails.Formats.empty() && !swapChainDetails.Present.empty(), "Does not have swap chain support!");

	VkExtent2D extent;
	VkSurfaceFormatKHR format;
	VkPresentModeKHR presentMode;
	for (auto& f : swapChainDetails.Formats)
	{
		if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			format = f;
			break;
		}
		else
			format = swapChainDetails.Formats[0];
	}

	for (auto& p : swapChainDetails.Present)
	{
		if (p == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			presentMode = p;
			break;
		}
		else
			presentMode = VK_PRESENT_MODE_FIFO_KHR;
	}

	if (swapChainDetails.Capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		extent = swapChainDetails.Capabilities.currentExtent;
	else
	{
		int width, height;
		glfwGetFramebufferSize(static_cast<GLFWwindow*>(Window::GetWindow()), &width, &height);
		extent = { (uint32_t)width, (uint32_t)height };

		extent.width = std::clamp(extent.width, swapChainDetails.Capabilities.minImageExtent.width, swapChainDetails.Capabilities.maxImageExtent.width);

		extent.height = std::clamp(extent.height, swapChainDetails.Capabilities.minImageExtent.height, swapChainDetails.Capabilities.maxImageExtent.height);
	}
	
	m_SwapChainObject = std::make_tuple(extent, format, presentMode);

	uint32_t imageCount = swapChainDetails.Capabilities.minImageCount + 1;
	if (swapChainDetails.Capabilities.maxImageCount > 0 && imageCount > swapChainDetails.Capabilities.maxImageCount)
		imageCount = swapChainDetails.Capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR swapChainCreateInfo{};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = m_Surface;
	swapChainCreateInfo.minImageCount = imageCount;
	swapChainCreateInfo.imageFormat = format.format;
	swapChainCreateInfo.imageColorSpace = format.colorSpace;
	swapChainCreateInfo.imageExtent = extent;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (index.Graphics.value() != index.Present.value())
	{
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapChainCreateInfo.queueFamilyIndexCount = 2;
		uint32_t familyIndices[] = { index.Graphics.value(), index.Present.value() };
		swapChainCreateInfo.pQueueFamilyIndices = familyIndices;
	}
	else
	{
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfo.queueFamilyIndexCount = 0;
		swapChainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	swapChainCreateInfo.preTransform = swapChainDetails.Capabilities.currentTransform;
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.presentMode = presentMode;
	swapChainCreateInfo.clipped = VK_TRUE;
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
	VK_ASSERT(
		vkCreateSwapchainKHR(m_Device, &swapChainCreateInfo, nullptr, &m_SwapChain) == VK_SUCCESS, 
		"Failed to create swap chain!");

	vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
	m_SwapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapchainImages.data());

	m_Format = format.format;
}

void Vulkan::createSwapChainImageView()
{
	auto [extent, format, present] = m_SwapChainObject;
	m_SwapchainImageView.resize(m_SwapchainImages.size());

	for (int i = 0; i < m_SwapchainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_SwapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format.format;
		
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VK_ASSERT(
			vkCreateImageView(m_Device, &createInfo, nullptr, &m_SwapchainImageView[i]) == VK_SUCCESS,
			"Failed to create Image View!");


	}
}

void Vulkan::createShaderModules()
{
	//TODO :: 
	VkShaderModule vertexShader = m_VertexShader->setupShaderModule(m_Device);
	VkShaderModule fragmentShader = m_FragmentShader->setupShaderModule(m_Device);

	VkPipelineShaderStageCreateInfo vertexShaderInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	vertexShaderInfo.pNext = nullptr;
	vertexShaderInfo.flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
	vertexShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderInfo.module = vertexShader;
	vertexShaderInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentShaderInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	fragmentShaderInfo.pNext = nullptr;
	fragmentShaderInfo.flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
	fragmentShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderInfo.module = fragmentShader;
	fragmentShaderInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderInfo, fragmentShaderInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo inputInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = swapChainDetails.Capabilities.currentExtent.width;
	viewport.height = swapChainDetails.Capabilities.currentExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainDetails.Capabilities.currentExtent;

	VkPipelineViewportStateCreateInfo viewportCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportCreateInfo.viewportCount = 1;
	viewportCreateInfo.pViewports = &viewport;
	viewportCreateInfo.scissorCount = 1;
	viewportCreateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizeCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizeCreateInfo.depthClampEnable = VK_FALSE;
	rasterizeCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizeCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizeCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizeCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizeCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizeCreateInfo.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.sampleShadingEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState blendAttachment{};
	blendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo blendCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	blendCreateInfo.logicOpEnable = VK_FALSE;
	blendCreateInfo.attachmentCount = 1;
	blendCreateInfo.pAttachments = &blendAttachment;

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	//dynamicStateCreateInfo.dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState);
	//dynamicStateCreateInfo.pDynamicStates = dynamicStates;

	VkPipelineLayoutCreateInfo pipelineCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

	VK_ASSERT(vkCreatePipelineLayout(m_Device, &pipelineCreateInfo, nullptr, &m_PipelineLayout) == VK_SUCCESS,
		"Failed to create VkPipeline");

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	graphicsPipelineCreateInfo.stageCount = 2;
	graphicsPipelineCreateInfo.pStages = shaderStages;
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputInfo;
	graphicsPipelineCreateInfo.pViewportState = &viewportCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizeCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampling;
	graphicsPipelineCreateInfo.pColorBlendState = &blendCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	graphicsPipelineCreateInfo.layout = m_PipelineLayout;
	graphicsPipelineCreateInfo.renderPass = m_RenderPass;
	graphicsPipelineCreateInfo.subpass = 0;

	VK_ASSERT(vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &m_GraphicsPipeline) == VK_SUCCESS,
		"Failed to create VkGraphicsPipeline");
}

void Vulkan::createRenderer()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	colorAttachment.format = m_Format;

	VkAttachmentReference colorReference{};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorReference;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassCreateInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colorAttachment;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &dependency;

	VK_ASSERT(vkCreateRenderPass(m_Device, &renderPassCreateInfo, nullptr, &m_RenderPass) == VK_SUCCESS,
		"Failed to create VkRenderPass");
}

void Vulkan::createFrameBuffer()
{
	m_SwapChainFrameBuffers.resize(m_SwapchainImageView.size());

	for (size_t i = 0; i < m_SwapChainFrameBuffers.size(); i++)
	{
		//VkImageView attachment[] = {  };

		VkFramebufferCreateInfo frameBufferCreateInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		frameBufferCreateInfo.attachmentCount = 1;
		frameBufferCreateInfo.pAttachments = &m_SwapchainImageView[i];
		frameBufferCreateInfo.renderPass = m_RenderPass;
		frameBufferCreateInfo.width = swapChainDetails.Capabilities.currentExtent.width;
		frameBufferCreateInfo.height = swapChainDetails.Capabilities.currentExtent.height;
		frameBufferCreateInfo.layers = 1;

		VK_ASSERT(vkCreateFramebuffer(m_Device, &frameBufferCreateInfo, nullptr, &m_SwapChainFrameBuffers[i]) == VK_SUCCESS,
			"Failed to create VkFrameBuffer");
	}
}

void Vulkan::createCommandPool()
{
	VkCommandPoolCreateInfo commandPoolCreateInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	commandPoolCreateInfo.queueFamilyIndex = index.Graphics.value();

	VK_ASSERT(vkCreateCommandPool(m_Device, &commandPoolCreateInfo, nullptr, &m_CommandPool) == VK_SUCCESS,
		"Failed to create VkCommandPool");

}

void Vulkan::createCommandBuffer()
{
	m_CommandBuffers.resize(m_SwapChainFrameBuffers.size());

	VkCommandBufferAllocateInfo commandBufferAllocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	commandBufferAllocInfo.commandBufferCount = m_CommandBuffers.size();
	commandBufferAllocInfo.commandPool = m_CommandPool;
	commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VK_ASSERT(vkAllocateCommandBuffers(m_Device, &commandBufferAllocInfo, m_CommandBuffers.data()) == VK_SUCCESS,
		"Failed to allocate CommandBuffer");

	for (size_t i = 0; i < m_CommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		VK_ASSERT(vkBeginCommandBuffer(m_CommandBuffers[i], &beginInfo) == VK_SUCCESS,
			"Failed to allocate CommandBuffer");

		VkRenderPassBeginInfo renderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderPassBeginInfo.renderPass = m_RenderPass;
		renderPassBeginInfo.framebuffer = m_SwapChainFrameBuffers[i];
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = swapChainDetails.Capabilities.currentExtent;

		VkClearValue clearValue{ 0.4f, 0.2f, 0.3f, 1.0f };
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

		vkCmdDraw(m_CommandBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(m_CommandBuffers[i]);

		VK_ASSERT(vkEndCommandBuffer(m_CommandBuffers[i]) == VK_SUCCESS,
				"Failed to end CommandBuffer");
	}

}

void Vulkan::createSemaphore()
{
	VkSemaphoreCreateInfo semaphoreCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	VK_ASSERT(vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_ImageAvaible) == VK_SUCCESS,
		"Failed to create semaphore image avaible");
	VK_ASSERT(vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_RenderFinished) == VK_SUCCESS,
		"Failed to create semaphore render finished");
}

VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		VK_CORE_WARN("Validation Layer[ERROR]: {0}", pCallbackData->pMessage);
	else if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		VK_CORE_FATAL("Validation Layer[FATAL]: {0}", pCallbackData->pMessage);


	return false;
}
