#pragma once
#include "SwapChain.h"
#include "Shader.h"

class Pipeline
{
public:
	Pipeline(SwapChain& swapchain, const Shader& shader)
		: m_Swapchain(swapchain)
	{
		createPipeline(shader);
		createCommandPool();
	}

	SwapChain& GetSwapchain() { return m_Swapchain; }
	const std::vector< VkCommandBuffer>& GetCommandBuffers() { return m_CommandBuffers; }
private:
	void createPipeline(const Shader& shader);
	void createCommandPool();
private:
	SwapChain& m_Swapchain;

	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_GraphicsPipeline;

	VkCommandPool m_CommandPool;
	std::vector<VkCommandBuffer> m_CommandBuffers;
};