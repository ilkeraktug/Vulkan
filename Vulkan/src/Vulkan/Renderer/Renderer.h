#pragma once

#include "SwapChain.h"
#include "Pipeline.h"

class Renderer
{
public:
	Renderer(Pipeline& pipeline);
	~Renderer();

	void Run();
private:
	VkSemaphore m_ImageReady;
	VkSemaphore m_RenderFinised;

	Pipeline& m_Pipeline;
};