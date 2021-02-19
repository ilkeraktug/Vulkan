#include "pch.h"
#include "VulkanCore.h"

#include <GLFW\glfw3.h>


VulkanCore::VulkanCore()
{
	createInstance();
}

VulkanCore::~VulkanCore()
{

#ifdef ENABLE_VALIDATION_LAYERS
	destroyDebugUtils();
#endif // ENABLE_VALIDATION_LAYERS

	vkDestroyInstance(m_Instance, nullptr);
}

void VulkanCore::Init()
{
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
	instanceCreateInfo.enabledExtensionCount = glfwExtensionsVector.size();
	instanceCreateInfo.ppEnabledExtensionNames = glfwExtensionsVector.data();
	instanceCreateInfo.enabledLayerCount = 0;
	instanceCreateInfo.ppEnabledLayerNames = nullptr;
	instanceCreateInfo.pNext = nullptr;

#ifdef ENABLE_VALIDATION_LAYERS
	checkValidationSupport();
	if (m_ValidationSupport)
	{
		instanceCreateInfo.enabledLayerCount	= m_ValidationLayers.size();
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