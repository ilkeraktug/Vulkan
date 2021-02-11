#include "pch.h"
#include "Vulkan.h"

#include "Vulkan\Core.h"
#include "Vulkan\Window.h"

Vulkan::Vulkan()
{
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

}
	
void Vulkan::Shutdown()
{
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
	for (auto& imgView : m_SwapchainImageView)
		vkDestroyImageView(m_Device, imgView, nullptr);

	vkDestroyDevice(m_Device, nullptr);

#if ENABLE_VALIDATION_LAYERS
	auto destroyDebugMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
	destroyDebugMessenger(m_Instance, m_DebugMessenger, nullptr);
#endif // ENABLE_VALIDATION_LAYERS

	vkDestroyInstance(m_Instance, nullptr);
}

void Vulkan::Run()
{

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

VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		VK_CORE_TRACE("Validation Layer[TRACE]: {0}", pCallbackData->pMessage);
	else if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		VK_CORE_WARN("Validation Layer[ERROR]: {0}", pCallbackData->pMessage);
	else
		VK_CORE_FATAL("Validation Layer[FATAL]: {0}", pCallbackData->pMessage);


	return false;
}
