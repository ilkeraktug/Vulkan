#pragma once

#include <vulkan\vulkan.h>
#include <GLFW\glfw3.h>
#include <GLFW\glfw3native.h>

#include "Shader.h"
#include "Vulkan\Renderer\VulkanCore.h"
#include "SwapChain.h"
#include "Pipeline.h"
#include "Renderer.h"

class Vulkan
{
public:
	Vulkan();
	~Vulkan();
	
	void Init();
	void Shutdown();
	void Run();
private:
	std::unique_ptr<VulkanCore> m_VulkanCore;
	std::unique_ptr<SwapChain> m_Swapchain;
	std::unique_ptr<Pipeline> m_Pipeline;
	std::unique_ptr<Renderer> m_Renderer;

	std::unique_ptr<Shader> m_Shader;

};