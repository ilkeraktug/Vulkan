#include "pch.h"
#include "VulkanCore.h"

#include "Vulkan\Window.h"

#include <GLFW\glfw3.h>

VkInstance VulkanCore::m_Instance;
VkPhysicalDevice VulkanCore::m_PhysicalDevice = VK_NULL_HANDLE;
VkDevice VulkanCore::m_Device;
VkSurfaceKHR VulkanCore::m_Surface;
bool VulkanCore::m_SwapChainSupport = false;
QueueIndices VulkanCore::m_QueueIndices;

VulkanCore::VulkanCore()
{
	Init();
}

VulkanCore::~VulkanCore()
{

#ifdef ENABLE_VALIDATION_LAYERS
	destroyDebugUtils();
#endif // ENABLE_VALIDATION_LAYERS

	vkDestroyDevice(m_Device, nullptr);
	vkDestroyInstance(m_Instance, nullptr);
}

void VulkanCore::Init()
{
	createInstance();
	createSurface();
	selectGPU();
	createDevice();
	checkSwapChainSupport();
}


void VulkanCore::createInstance()
{
	VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.pApplicationName	= "VkApp";
	appInfo.applicationVersion	= VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName			= "VkEngine";
	appInfo.engineVersion		= VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion			= VK_MAKE_VERSION(1, 0, 0);

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> glfwExtensionsVector(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef ENABLE_VALIDATION_LAYERS
	glfwExtensionsVector.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // ENABLE_VALIDATION_LAYERS

	VkInstanceCreateInfo instanceCreateInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = (uint32_t)glfwExtensionsVector.size();
	instanceCreateInfo.ppEnabledExtensionNames = glfwExtensionsVector.data();
	instanceCreateInfo.enabledLayerCount = 0;
	instanceCreateInfo.ppEnabledLayerNames = nullptr;
	instanceCreateInfo.pNext = nullptr;

#ifdef ENABLE_VALIDATION_LAYERS
	checkValidationSupport();
	if (m_ValidationSupport)
	{
		instanceCreateInfo.enabledLayerCount	= (uint32_t)m_ValidationLayers.size();
		instanceCreateInfo.ppEnabledLayerNames	= m_ValidationLayers.data();
		instanceCreateInfo.pNext				= &createDebugMessenger();
	}
	else
		VK_CORE_WARN("Validation layers is not supported");
#endif // ENABLE_VALIDATION_LAYERS
	
	VK_ASSERT(vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance) == VK_SUCCESS, "Failed to create vkInstance");

#ifdef ENABLE_VALIDATION_LAYERS
	auto debugCreateFunc = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
	debugCreateFunc(m_Instance, &createDebugMessenger(), nullptr, &m_DebugMessenger);
#endif // ENABLE_VALIDATION_LAYERS
}

void VulkanCore::selectGPU()
{
	VkPhysicalDeviceProperties gpuProperties;

	uint32_t gpuCount;
	vkEnumeratePhysicalDevices(m_Instance, &gpuCount, nullptr);
	std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
	vkEnumeratePhysicalDevices(m_Instance, &gpuCount, physicalDevices.data());
	
	VK_ASSERT(gpuCount, "Couldnt find any GPU supports Vulkan API!");


	//TODO:: Build more advanced GPU selection system.
	for (size_t i = 0; i < physicalDevices.size(); i++)
	{
		vkGetPhysicalDeviceProperties(physicalDevices.at(i), &gpuProperties);
		if (gpuProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			m_PhysicalDevice = physicalDevices.at(i);
			VK_INFO("Device Name : {0}", gpuProperties.deviceName);
			break;
		}
	}

	VK_ASSERT(m_PhysicalDevice != VK_NULL_HANDLE, "No GPU found!");

	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilyProperties.data());

	for (size_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		if (queueFamilyProperties.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			m_QueueIndices.GraphicsIndex = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, m_Surface, &presentSupport);
		if (presentSupport)
			m_QueueIndices.PresentIndex = i;

		if (m_QueueIndices.isCompleted())
			break;

	}
	
}

void VulkanCore::createDevice()
{
	std::vector<uint32_t> queueIndices = { m_QueueIndices.GraphicsIndex.value(), m_QueueIndices.PresentIndex.value() };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	float priorty = 1.0f;
	for (const auto& i : queueIndices)
	{
		VkDeviceQueueCreateInfo deviceQueueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		deviceQueueCreateInfo.queueFamilyIndex = i;
		deviceQueueCreateInfo.queueCount = 1;
		deviceQueueCreateInfo.pQueuePriorities = &priorty;
		queueCreateInfos.push_back(deviceQueueCreateInfo);
	}

	VkPhysicalDeviceFeatures physicalDeviceFeatures{};

	VkDeviceCreateInfo deviceCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

	deviceCreateInfo.enabledExtensionCount = m_DeviceExtension.size();
	deviceCreateInfo.ppEnabledExtensionNames = m_DeviceExtension.data();

	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
#ifdef ENABLE_VALIDATION_LAYERS
	deviceCreateInfo.enabledLayerCount = (uint32_t)m_ValidationLayers.size();
	deviceCreateInfo.ppEnabledLayerNames = m_ValidationLayers.data();
#endif // ENABLE_VALIDATION_LAYERS

	VK_ASSERT(vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device) == VK_SUCCESS, 
		"Failed to create vkDevice!");

	vkGetDeviceQueue(m_Device, m_QueueIndices.GraphicsIndex.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_Device, m_QueueIndices.PresentIndex.value(), 0, &m_PresentQueue);
}

void VulkanCore::createSurface()
{
	//TODO: Use own functions to create surface.
	VK_ASSERT(glfwCreateWindowSurface(m_Instance, static_cast<GLFWwindow*>(Window::GetWindow()), nullptr, &m_Surface) == VK_SUCCESS, "Failed to create window surface!");
}

void VulkanCore::checkSwapChainSupport()
{
	uint32_t propertyCount = 0;
	vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &propertyCount, nullptr);
	std::vector<VkExtensionProperties> deviceExtensions(propertyCount);
	vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &propertyCount, deviceExtensions.data());

	int count = 0;
	for (size_t i = 0; i < deviceExtensions.size(); i++)
	{
		for (size_t j = 0; j < m_DeviceExtension.size(); j++)
		{
			int debug = strcmp(deviceExtensions.at(i).extensionName, m_DeviceExtension.at(j));
			if (!debug) //strcmp returns 0 if both strings are equal!
				count++;
		}
	}

	if (count == m_DeviceExtension.size())
		m_SwapChainSupport = true;
}


#ifdef ENABLE_VALIDATION_LAYERS

void VulkanCore::checkValidationSupport()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> layerProperties(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());
	
	for (const auto& layer : m_ValidationLayers)
	{
		for (const auto& prop : layerProperties)
		{
			if (strcmp(layer, prop.layerName))
			{
				m_ValidationSupport = true;
				break;
			}

		}
	}
}

VkDebugUtilsMessengerCreateInfoEXT VulkanCore::createDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };

	debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
											   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	debugMessengerCreateInfo.messageType	 = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |																			 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
											   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	debugMessengerCreateInfo.pfnUserCallback = VulkanCore::debugCallback;

	return debugMessengerCreateInfo;
}

void VulkanCore::destroyDebugUtils()
{
	auto debugDestroyFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
	debugDestroyFunc(m_Instance, m_DebugMessenger, nullptr);
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanCore::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		VK_CORE_WARN(pCallbackData->pMessage);
	else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		VK_CORE_FATAL(pCallbackData->pMessage);

	return VK_FALSE;
}


#endif // ENABLE_VALIDATION_LAYERS