#pragma once
#include "SwapChain.h"
#include "Shader.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

class Pipeline
{
public:
	Pipeline(SwapChain& swapchain, const Shader& shader, VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer);
	~Pipeline();

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

	VertexBuffer& m_VertexBuffer;
	IndexBuffer& m_IndexBuffer;
};