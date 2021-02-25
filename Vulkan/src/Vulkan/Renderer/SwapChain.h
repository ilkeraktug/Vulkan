#pragma once

#include "Vulkan\Renderer\VulkanCore.h"

class SwapChain
{
public:
	SwapChain();
	~SwapChain();

private:
	void selectFormat();
	void selectPresentMode();
	void selectSwapExtent();

	void createSwapchain();

private:
	VkSurfaceCapabilitiesKHR m_SurfaceCapabilities;
	VkSurfaceFormatKHR m_SurfaceFormat;
	VkPresentModeKHR m_PresentMode;
	VkExtent2D m_Extent;

	VkSwapchainKHR m_Swapchain;
};