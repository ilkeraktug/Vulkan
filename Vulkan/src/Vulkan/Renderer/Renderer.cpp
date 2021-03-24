#include "pch.h"
#include "Renderer.h"


Renderer::Renderer(Pipeline& pipeline)
	:m_Pipeline(pipeline)
{
	createSyncObjects();
}

Renderer::~Renderer()
{
	vkQueueWaitIdle(VulkanCore::GetGraphicsQueue());

	for (size_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(VulkanCore::GetDevice(), m_ImageReady.at(i), nullptr);
		vkDestroySemaphore(VulkanCore::GetDevice(), m_RenderFinised.at(i), nullptr);
		vkDestroyFence(VulkanCore::GetDevice(), m_Fences.at(i), nullptr);
	}
}

void Renderer::Run(UniformBuffer& uniformBuffer)
{
	vkWaitForFences(VulkanCore::GetDevice(), 1, &m_Fences.at(currentFrame), VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(VulkanCore::GetDevice(), m_Pipeline.GetSwapchain().GetSwapchainKHR(), UINT64_MAX, m_ImageReady[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (m_SwapchainImagesFences[imageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(VulkanCore::GetDevice(), 1, &m_SwapchainImagesFences[imageIndex], VK_TRUE, UINT64_MAX);
	}

	m_SwapchainImagesFences[imageIndex] = m_Fences[currentFrame];

	const VkPipelineStageFlags stageMask[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	uniformBuffer.updateBuffer(imageIndex);

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = m_Pipeline.GetCommandBuffers().size();
	submitInfo.pCommandBuffers = m_Pipeline.GetCommandBuffers().data();
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_ImageReady[currentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_RenderFinised[currentFrame];
	submitInfo.pWaitDstStageMask = stageMask;

	vkResetFences(VulkanCore::GetDevice(), 1, &m_Fences.at(currentFrame));

	VK_ASSERT(vkQueueSubmit(VulkanCore::GetGraphicsQueue(), 1, &submitInfo, m_Fences.at(currentFrame)) == VK_SUCCESS, "Can't submit queue!");

	VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_RenderFinised[currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &(m_Pipeline.GetSwapchain().GetSwapchainKHR());
	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(VulkanCore::GetPresentationQueue(), &presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
}

void Renderer::createSyncObjects()
{
	m_ImageReady.resize(MAX_FRAME_IN_FLIGHT);
	m_RenderFinised.resize(MAX_FRAME_IN_FLIGHT);
	m_Fences.resize(MAX_FRAME_IN_FLIGHT);
	m_SwapchainImagesFences.resize(m_Pipeline.GetSwapchain().GetImageCount(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	
	VkFenceCreateInfo fenceCreateInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++)
	{
		VK_ASSERT(vkCreateSemaphore(VulkanCore::GetDevice(), &semaphoreCreateInfo, nullptr, &m_ImageReady[i]) == VK_SUCCESS, "Cant vkCreateSemaphore");
		VK_ASSERT(vkCreateSemaphore(VulkanCore::GetDevice(), &semaphoreCreateInfo, nullptr, &m_RenderFinised[i]) == VK_SUCCESS, "Cant vkCreateSemaphore");
		VK_ASSERT(vkCreateFence(VulkanCore::GetDevice(), &fenceCreateInfo, nullptr, &m_Fences[i]) == VK_SUCCESS, "Cant vkCreateFence");
	}

}
