#pragma once

#include "Vulkan\Renderer\VulkanCore.h"

class SwapChain
{
public:
	SwapChain();
	~SwapChain();

	inline const VkExtent2D& GetExtent() const { return m_Extent; }
	inline const VkSurfaceFormatKHR& GetSurfaceFormat() const { return m_SurfaceFormat; }
	inline const VkSwapchainKHR& GetSwapchain() const { return m_Swapchain; }
	inline const VkRenderPass& GetRenderPass() const { return m_RenderPass; }
	inline const std::vector<VkFramebuffer>& GetFramebuffers() const { return m_Framebuffers; }
	inline const uint32_t GetImageCount() const { return m_ImageCount; }

private:
	void selectFormat();
	void selectPresentMode();
	void selectSwapExtent();

	void createSwapchain();
	void createImageViews();
	void createRenderPass();
	void createFramebuffers();

private:
	VkSwapchainKHR m_Swapchain;

	VkSurfaceCapabilitiesKHR m_SurfaceCapabilities;
	VkSurfaceFormatKHR m_SurfaceFormat;
	VkPresentModeKHR m_PresentMode;
	VkExtent2D m_Extent;

	std::vector<VkImage> m_SwapchainImages;
	std::vector<VkImageView> m_SwapchainImageViews;
	uint32_t m_ImageCount = 0;

	std::vector<VkFramebuffer> m_Framebuffers;

	VkRenderPass m_RenderPass;
};