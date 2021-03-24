#pragma once

#include "SwapChain.h"
#include "Pipeline.h"
#include "UniformBuffer.h"

#define MAX_FRAME_IN_FLIGHT 2

class Renderer
{
public:
	Renderer(Pipeline& pipeline);
	~Renderer();

	void Run(UniformBuffer& uniformBuffer);
private:
	void createSyncObjects();
private:
	std::vector<VkSemaphore> m_ImageReady;
	std::vector<VkSemaphore> m_RenderFinised;
	std::vector<VkFence> m_Fences;
	std::vector<VkFence> m_SwapchainImagesFences;

	size_t currentFrame = 0;

	Pipeline& m_Pipeline;
};