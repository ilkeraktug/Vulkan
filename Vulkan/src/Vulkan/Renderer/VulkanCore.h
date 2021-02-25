#pragma once

#include <vulkan\vulkan.h>
#include "Vulkan\Core.h"


struct QueueIndices
{
	std::optional<uint32_t> GraphicsIndex;
	std::optional<uint32_t> PresentIndex;

	inline bool isCompleted() { return GraphicsIndex.has_value() && PresentIndex.has_value(); }
};

class VulkanCore
{
public:
	VulkanCore();
	~VulkanCore();

	void Init();
	inline static VkInstance& GetInstance() { return m_Instance; }
	inline static VkPhysicalDevice& GetGPU() { return m_PhysicalDevice; }
	inline static VkPhysicalDevice& GetPhysicalDevice() { return m_PhysicalDevice; }
	inline static VkDevice& GetDevice() { return m_Device; }
	inline static VkSurfaceKHR& GetSurface() { return m_Surface; }
	
	inline static bool GetSwapChainSupport() { return m_SwapChainSupport; }
	
	inline static QueueIndices& GetQueueIndices() { return m_QueueIndices; }
private:
	void createInstance();
	void selectGPU();
	void createDevice();
	void createSurface();
	void checkSwapChainSupport();
private:
	static VkInstance m_Instance;
	static VkPhysicalDevice m_PhysicalDevice;
	static VkDevice m_Device;
	static VkSurfaceKHR m_Surface;

	static bool m_SwapChainSupport;
	static QueueIndices m_QueueIndices;
	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;

	const std::vector<const char*> m_DeviceExtension = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef ENABLE_VALIDATION_LAYERS
private:
	void checkValidationSupport();
	VkDebugUtilsMessengerCreateInfoEXT createDebugMessenger();

	void destroyDebugUtils();

	const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
	bool m_ValidationSupport = false;
	VkDebugUtilsMessengerCreateInfoEXT m_DebugMessengerCreateInfo;
	VkDebugUtilsMessengerEXT m_DebugMessenger;

	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanCore::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

#endif // ENABLE_VALIDATION_LAYERS
};