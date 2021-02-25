#include "pch.h"
#include "SwapChain.h"

#include <GLFW\glfw3.h>
#include "Vulkan\Window.h"

SwapChain::SwapChain()
{
	if (!VulkanCore::GetSwapChainSupport())
	{
		VK_ERROR("Device does not support swap chain extension!");
	}
	else
	{
		selectFormat();
		selectPresentMode();
		selectSwapExtent();
		createSwapchain();
	}
}

SwapChain::~SwapChain()
{
	vkDestroySurfaceKHR(VulkanCore::GetInstance(), VulkanCore::GetSurface(), nullptr);
}

void SwapChain::selectFormat()
{
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanCore::GetGPU(), VulkanCore::GetSurface(), &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanCore::GetGPU(), VulkanCore::GetSurface(), &formatCount, formats.data());

	for (const auto& f : formats)
	{
		if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			m_SurfaceFormat = f;
			return;
		}
	}

	m_SurfaceFormat = formats.at(0);
}

void SwapChain::selectPresentMode()
{
	uint32_t presentCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanCore::GetGPU(), VulkanCore::GetSurface(), &presentCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(presentCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanCore::GetGPU(), VulkanCore::GetSurface(), &presentCount, presentModes.data());

	for (const auto& p : presentModes)
	{
		if (p == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			m_PresentMode = p;
			return;
		}
	}

	m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
}

void SwapChain::selectSwapExtent()
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanCore::GetGPU(), VulkanCore::GetSurface(), &m_SurfaceCapabilities);

	if (m_SurfaceCapabilities.currentExtent.height != 0xFFFFFFFF)
	{
		m_Extent = m_SurfaceCapabilities.currentExtent;
		return;
	}

	int width, height;
	glfwGetFramebufferSize(static_cast<GLFWwindow*>(Window::GetWindow()), &width, &height);

	m_Extent.width = std::clamp(static_cast<uint32_t>(width), m_SurfaceCapabilities.minImageExtent.width, m_SurfaceCapabilities.maxImageExtent.width);
	m_Extent.height = std::clamp(static_cast<uint32_t>(height), m_SurfaceCapabilities.minImageExtent.height, m_SurfaceCapabilities.maxImageExtent.height);
}

void SwapChain::createSwapchain()
{
	uint32_t imageCount = m_SurfaceCapabilities.minImageCount + 1;

	imageCount = std::clamp(imageCount, imageCount, m_SurfaceCapabilities.maxImageCount);

	VkSwapchainCreateInfoKHR swapchainCreateInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	swapchainCreateInfo.surface = VulkanCore::GetSurface();
	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.imageFormat = m_SurfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = m_SurfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = m_Extent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueIndices[] = { VulkanCore::GetQueueIndices().GraphicsIndex.value(), VulkanCore::GetQueueIndices().PresentIndex.value() };

	if (VulkanCore::GetQueueIndices().GraphicsIndex != VulkanCore::GetQueueIndices().PresentIndex)
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueIndices;
	}
	else
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	swapchainCreateInfo.preTransform = m_SurfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = m_PresentMode;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	VK_ASSERT(vkCreateSwapchainKHR(VulkanCore::GetDevice(), &swapchainCreateInfo, nullptr, &m_Swapchain) == VK_SUCCESS, "Failed to create swapchain!");
}