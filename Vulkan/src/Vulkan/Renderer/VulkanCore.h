#pragma once

#include <vulkan\vulkan.h>
#include "Vulkan\Core.h"

class VulkanCore
{
public:
	VulkanCore();
	~VulkanCore();

	void Init();

	VkInstance& GetInstance() { return m_Instance; }

private:
	void createInstance();
private:
	VkInstance m_Instance;


	#ifdef ENABLE_VALIDATION_LAYERS
private:
	void checkValidationSupport();
	VkDebugUtilsMessengerCreateInfoEXT createDebugMessenger();

	void destroyDebugUtils();

	std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
	bool m_ValidationSupport = false;
	VkDebugUtilsMessengerCreateInfoEXT m_DebugMessengerCreateInfo;
	VkDebugUtilsMessengerEXT m_DebugMessenger;

	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanCore::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	#endif // ENABLE_VALIDATION_LAYERS
};