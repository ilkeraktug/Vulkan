#pragma once

#include "Vulkan\Renderer\VulkanCore.h"

class SwapChain
{
public:
	SwapChain();
	~SwapChain();

	inline const VkExtent2D& GetExtent() const { return m_Extent; }
	inline const VkSurfaceFormatKHR& GetSurfaceFormat() const { return m_SurfaceFormat; }
	inline const VkSwapchainKHR& GetSwapchainKHR() const { return m_Swapchain; }
	inline const VkRenderPass& GetRenderPass() const { return m_RenderPass; }
	inline const std::vector<VkFramebuffer>& GetFramebuffers() const { return m_Framebuffers; }
	static inline const uint32_t GetImageCount() { return m_ImageCount; }

	static void recreateSwapchain(uint32_t width, uint32_t height);

private:
	static void selectFormat();
	static void selectPresentMode();
	static void selectSwapExtent();

	static void createSwapchain();
	static void createImageViews();
	static void createRenderPass();
	static void createFramebuffers();

private:
	uint32_t m_Width, m_Heigth;

	static VkSwapchainKHR m_Swapchain;

	static VkSurfaceCapabilitiesKHR m_SurfaceCapabilities;
	static VkSurfaceFormatKHR m_SurfaceFormat;
	static VkPresentModeKHR m_PresentMode;
	static VkExtent2D m_Extent;

	static std::vector<VkImage> m_SwapchainImages;
	static std::vector<VkImageView> m_SwapchainImageViews;
	static uint32_t m_ImageCount;

	static std::vector<VkFramebuffer> m_Framebuffers;

	static VkRenderPass m_RenderPass;
};