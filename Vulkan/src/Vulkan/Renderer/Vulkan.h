#pragma once

#include <vulkan\vulkan.h>
#include <GLFW\glfw3.h>
#include <GLFW\glfw3native.h>

#include "Shader.h"

class Vulkan
{
public:
	Vulkan();
	~Vulkan();
	
	void Init();
	void Shutdown();
	void Run();
private:
	void findDevice();
	bool hasRequiredExtensions();

	void SelectSwapChainObject();
	void createSwapChainImageView();
	void createShaderModules();
	void createRenderer();
	void createFrameBuffer();
	void createCommandPool();
	void createCommandBuffer();
	void createSemaphore();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
private:
	struct QFamilyIndex
	{
		std::optional<uint32_t> Graphics;
		std::optional<uint32_t> Present;

		inline bool hasValue() { return Graphics.has_value() && Present.has_value(); }
	};

	struct SwapChainDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> Present;
	};

	std::vector<const char*> m_DeviceExtensions;

	std::vector<VkImage> m_SwapchainImages;
	std::vector<VkImageView> m_SwapchainImageView;

	std::tuple<VkExtent2D, VkSurfaceFormatKHR, VkPresentModeKHR> m_SwapChainObject;

	VkDebugUtilsMessengerEXT m_DebugMessenger;
	const std::vector<const char*> m_ValidationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};

	VkInstance m_Instance;
	VkPhysicalDevice m_PhysicalDevice;
	VkDevice m_Device;
	VkSurfaceKHR m_Surface;
	VkSwapchainKHR m_SwapChain;
	VkFormat m_Format;
	VkCommandPool m_CommandPool;
	std::vector<VkFramebuffer> m_SwapChainFrameBuffers;
	std::vector<VkCommandBuffer> m_CommandBuffers;

	VkRenderPass m_RenderPass;
	VkPipeline m_GraphicsPipeline;
	VkPipelineLayout m_PipelineLayout;

	VkSemaphore m_ImageAvaible;
	VkSemaphore m_RenderFinished;

	VkQueue m_GraphicsQ;
	VkQueue m_PresentQ;
	QFamilyIndex index;
	SwapChainDetails swapChainDetails;

	std::unique_ptr<Shader> m_VertexShader;
	std::unique_ptr<Shader> m_FragmentShader;
};