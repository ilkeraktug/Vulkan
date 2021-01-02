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
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;

	VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
	VK_ASSERT((result == VK_SUCCESS), "Can't create instance!");

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
	
	result = vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device);
	VK_ASSERT((result == VK_SUCCESS), "Failed to create Device!");
	vkGetDeviceQueue(m_Device, index.Graphics.value(), 0, &m_GraphicsQ);
	vkGetDeviceQueue(m_Device, index.Present.value(), 0, &m_PresentQ);

	m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VK_ASSERT(hasRequiredExtensions(), "Does not have required extensions!");

	createInfo.enabledExtensionCount = m_DeviceExtensions.size();
	createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

	SelectSwapChainObject();

}
	
void Vulkan::Shutdown()
{
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
	vkDestroyDevice(m_Device, nullptr);
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
		if (f.format == VK_FORMAT_R8G8B8A8_SRGB && f.format == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			format = f;
			break;
		}
		
		format = swapChainDetails.Formats[0];
	}

	for (auto& p : swapChainDetails.Present)
	{
		if (p == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			presentMode = p;
			break;
		}

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
	
	swapChainObject = std::make_tuple(extent, format, presentMode);

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
		swapChainCreateInfo.pQueueFamilyIndices = { index.Graphics.value(), index.Present.value() };
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
		vkCreateSwapchainKHR(m_Device, &swapChainCreateInfo, nullptr, &m_SwapChain), 
		"Failed to create swap chain!");

}
