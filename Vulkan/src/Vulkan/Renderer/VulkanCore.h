#pragma once

#include <vulkan/vulkan.h>
#ifdef PLATFORM_WINDOWS
#include <vulkan/vulkan_win32.h>
#endif

#include "Vulkan/Core/Core.h"
#include "Vulkan/Core/Window.h"
#include "Vulkan/Core/init.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

class VulkanCore
{
public:
	VulkanCore();
	VulkanCore(std::vector<const char*> enabledDeviceExtensions);
	~VulkanCore();

	void enableDeviceExtension(const std::vector<const char*>& extensionList);

	const bool supportedInstanceExtension(const std::string& extensionName) const;
	const bool supportedDeviceExtension(const std::string& extensionName) const;
	const uint32_t getQueueIndex(VkQueueFlagBits queueFlag) const;
	const uint32_t getMemoryType(uint32_t typeBit, VkMemoryPropertyFlags memoryFlag) const;

	void SetVsync(bool enable) { swapchain.isVsync = enable; }

	VkDevice GetDevice() { return m_Device; }
	const VkDevice GetDevice() const { return m_Device; }

	const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const { return m_PhysicalDeviceProperties; }

	VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin);
	VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin);
	VkCommandBuffer createCopyCommandBuffer(VkCommandBufferLevel level, bool begin);

	struct
	{
		uint32_t Graphics = UINT32_MAX;
		uint32_t Transfer = UINT32_MAX;
		uint32_t Compute = UINT32_MAX;
	}queueIndex;

	struct
	{
		VkQueue GraphicsQueue;
		VkQueue TransferQueue;
		VkQueue ComputeQueue;
	}queue;

	struct
	{
		VkSurfaceKHR surface;
		VkFormat colorFormat;
		VkColorSpaceKHR colorSpace;
		VkSwapchainKHR swapchainKHR;

		VkExtent2D extent;
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;
		bool isVsync = false;

		uint32_t queueIndex;
	}swapchain;

	struct
	{
		VkCommandPool commandPool;
		VkCommandPool copyCommandPool;
		std::vector<VkCommandBuffer> drawCmdBuffers;
		std::vector<VkFramebuffer> frameBuffers;

		VkRenderPass renderPass;
		VkSubmitInfo submitInfo;
		VkPipelineStageFlags waitDstMask[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		uint32_t imageIndex;

	}resources;
	
	struct
	{
		VkImage image;
		VkImageView view;
		VkDeviceMemory memory;

		VkFormat format = VK_FORMAT_D24_UNORM_S8_UINT;
	}depthStencil;

	struct
	{
		//VkFence fences[2];
		VkSemaphore imageRendered;
		VkSemaphore imageAvaible;
	} sync;


	struct
	{
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
		VkPipelineCache pipelineCache   = VK_NULL_HANDLE;
		VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
	}imgui;

	void BeginScene();
	//VkResult acquireNextImage(uint32_t* imageIndex);
	VkResult Submit();

	void windowResized();
private:
	void createInstance();
	void createPhysicalDevice();
	void createLogicalDevice();

	void createSwapchain();
	void createDepthStencilBuffer();
	void createCommandBuffers();
	void createRenderPass();
	void createFrameBuffer();

	void createSyncObjs();

	void initImGui();

	void checkEnabledDeviceFeatures(std::vector<const char*> enabledDeviceExtensions);

#ifdef ENABLE_VALIDATION_LAYERS
	void createDebugMessenger();
#endif //ENABLE_VALIDATION_LAYERS

private:
	VkBool32 m_EnableValidation = false;

#ifdef ENABLE_VALIDATION_LAYERS
	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger;
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger;
	VkDebugUtilsMessengerEXT m_DebugMessenger;
#endif //ENABLE_VALIDATION_LAYERS

	VkInstance m_Instance;
	std::vector<std::string> m_SupportedInstanceExtension;

	VkPhysicalDevice m_PhysicalDevice;
	VkPhysicalDeviceFeatures m_PhysicalDeviceFeatures;
	VkPhysicalDeviceProperties m_PhysicalDeviceProperties;
	VkPhysicalDeviceMemoryProperties m_PhysicalDeviceMemoryProperties;

	std::vector<VkQueueFamilyProperties> m_QueueProperties;

	VkDevice m_Device;
	std::vector<std::string> m_SupportedDeviceExtension;
	std::vector<const char*> m_EnabledDeviceExtension;
	VkPhysicalDeviceFeatures m_EnabledDeviceFeatures{};
};