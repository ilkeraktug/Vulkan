#include "pch.h"
#include "Renderer.h"


Renderer::Renderer(Pipeline& pipeline)
	:m_Pipeline(pipeline)
{
	VkSemaphoreCreateInfo imageReadySemaphoreCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	VK_ASSERT(vkCreateSemaphore(VulkanCore::GetDevice(), &imageReadySemaphoreCreateInfo, nullptr, &m_ImageReady) == VK_SUCCESS, "Cant vkCreateSemaphore");
	VK_ASSERT(vkCreateSemaphore(VulkanCore::GetDevice(), &imageReadySemaphoreCreateInfo, nullptr, &m_RenderFinised) == VK_SUCCESS, "Cant vkCreateSemaphore");
}

Renderer::~Renderer()
{
	vkDestroySemaphore(VulkanCore::GetDevice(), m_ImageReady, nullptr);
	vkDestroySemaphore(VulkanCore::GetDevice(), m_RenderFinised, nullptr);
}

void Renderer::Run()
{
	uint32_t imageIndex;
	vkAcquireNextImageKHR(VulkanCore::GetDevice(), m_Pipeline.GetSwapchain().GetSwapchain(), UINT64_MAX, m_ImageReady, VK_NULL_HANDLE, &imageIndex);

	const VkPipelineStageFlags stageMask[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = m_Pipeline.GetCommandBuffers().size();
	submitInfo.pCommandBuffers = m_Pipeline.GetCommandBuffers().data();
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_ImageReady;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_RenderFinised;
	submitInfo.pWaitDstStageMask = stageMask;

	VK_ASSERT(vkQueueSubmit(VulkanCore::GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS, "Can't submit queue!");

	VkPresentInfoKHR presentInfo{};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_RenderFinised;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &(m_Pipeline.GetSwapchain().GetSwapchain());
	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(VulkanCore::GetPresentationQueue(), &presentInfo);
}
