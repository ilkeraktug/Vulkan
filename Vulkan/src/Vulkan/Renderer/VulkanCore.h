#pragma once

#include <vulkan\vulkan.h>
#include "Vulkan\Core.h"

class VulkanCore
{
public:
	VulkanCore();
	~VulkanCore();

	void Init();

	inline VkInstance& GetInstance() { return m_Instance; }
	inline VkPhysicalDevice& GetGPU() { return m_PhysicalDevice; }
	inline VkPhysicalDevice& GetPhysicalDevice() { return m_PhysicalDevice; }
private:
	struct QueueIndices
	{
		std::optional<uint32_t> GraphicsIndex;

		inline bool isCompleted() { return GraphicsIndex.has_value(); }
	};
private:
	void createInstance();
	void selectGPU();
	void createDevice();
private:
	VkInstance m_Instance;
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice m_Device;

	QueueIndices indices;
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