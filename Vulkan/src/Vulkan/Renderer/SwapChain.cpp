#include "pch.h"
#include "SwapChain.h"

#include <GLFW\glfw3.h>
#include "Vulkan\Window.h"

VkSwapchainKHR SwapChain::m_Swapchain;
VkRenderPass SwapChain::m_RenderPass;
std::vector<VkImage> SwapChain::m_SwapchainImages;
std::vector<VkImageView> SwapChain::m_SwapchainImageViews;
uint32_t SwapChain::m_ImageCount = 0;
std::vector<VkFramebuffer> SwapChain::m_Framebuffers;
VkSurfaceCapabilitiesKHR SwapChain::m_SurfaceCapabilities;
VkSurfaceFormatKHR SwapChain::m_SurfaceFormat;
VkPresentModeKHR SwapChain::m_PresentMode;
VkExtent2D SwapChain::m_Extent;

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
		createImageViews();
		createRenderPass();
		createFramebuffers();
	}
}

SwapChain::~SwapChain()
{
	for (const auto& f : m_Framebuffers)
		vkDestroyFramebuffer(VulkanCore::GetDevice(), f, nullptr);

	for (const auto& i : m_SwapchainImageViews)
		vkDestroyImageView(VulkanCore::GetDevice(), i, nullptr);

	vkDestroyRenderPass(VulkanCore::GetDevice(), m_RenderPass, nullptr);
	vkDestroySwapchainKHR(VulkanCore::GetDevice(), m_Swapchain, nullptr);
	vkDestroySurfaceKHR(VulkanCore::GetInstance(), VulkanCore::GetSurface(), nullptr);
}

void SwapChain::recreateSwapchain(uint32_t width, uint32_t height)
{

	vkDeviceWaitIdle(VulkanCore::GetDevice());
	vkDestroySwapchainKHR(VulkanCore::GetDevice(), m_Swapchain, nullptr);
	selectFormat();
	selectPresentMode();
	selectSwapExtent();
	createSwapchain();
	createImageViews();
	createRenderPass();
	createFramebuffers();
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
		/*if (f.format == VK_FORMAT_R8G8B8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			m_SurfaceFormat = f;
			return;
		}*/
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

	vkGetSwapchainImagesKHR(VulkanCore::GetDevice(), m_Swapchain, &m_ImageCount, nullptr);
	m_SwapchainImages.resize(m_ImageCount);
	vkGetSwapchainImagesKHR(VulkanCore::GetDevice(), m_Swapchain, &m_ImageCount, m_SwapchainImages.data());
}

void SwapChain::createImageViews()
{
	m_SwapchainImageViews.resize(m_SwapchainImages.size());

	for (size_t i = 0; i < m_SwapchainImageViews.size(); i++)
	{
		VkImageViewCreateInfo createInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.image = m_SwapchainImages.at(i);
		createInfo.format = m_SurfaceFormat.format;
		createInfo.components = { 
			VK_COMPONENT_SWIZZLE_IDENTITY , //r
			VK_COMPONENT_SWIZZLE_IDENTITY , //g
			VK_COMPONENT_SWIZZLE_IDENTITY , //b
			VK_COMPONENT_SWIZZLE_IDENTITY }; //a
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.layerCount = 1;

		VK_ASSERT(vkCreateImageView(VulkanCore::GetDevice(), &createInfo, nullptr, &m_SwapchainImageViews.at(i)) == VK_SUCCESS, "Cant create swapchain image view!");
	}
}

void SwapChain::createRenderPass()
{
	VkAttachmentDescription attachmentDescription{};
	attachmentDescription.format = m_SurfaceFormat.format;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentReference attachementReference{};
	attachementReference.attachment = 0;
	attachementReference.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	//attachementReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &attachementReference;

	VkSubpassDependency subpassDependency{};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassCreateInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &attachmentDescription;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &subpassDependency;

	VK_ASSERT(vkCreateRenderPass(VulkanCore::GetDevice(), &renderPassCreateInfo, nullptr, &m_RenderPass) == VK_SUCCESS, "Failed to create vkCreateRenderPass");
}

void SwapChain::createFramebuffers()
{
	m_Framebuffers.resize(m_SwapchainImageViews.size());

	for (size_t i = 0; i < m_Framebuffers.size(); i++)
	{
		VkFramebufferCreateInfo frameBufferCrateInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		frameBufferCrateInfo.renderPass = m_RenderPass;
		frameBufferCrateInfo.attachmentCount = 1;
		frameBufferCrateInfo.pAttachments = &m_SwapchainImageViews[i];
		frameBufferCrateInfo.width = m_Extent.width;
		frameBufferCrateInfo.height = m_Extent.height;
		frameBufferCrateInfo.layers = 1;

		VK_ASSERT(vkCreateFramebuffer(VulkanCore::GetDevice(), &frameBufferCrateInfo, nullptr, &m_Framebuffers[i]) == VK_SUCCESS, "Failed to create vkCreateFramebuffer");
	}
}

