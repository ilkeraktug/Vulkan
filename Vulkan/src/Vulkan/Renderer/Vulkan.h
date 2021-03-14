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

	static void recreateSwapchain(uint32_t width, uint32_t height);
private:
	std::unique_ptr<VulkanCore> m_VulkanCore;

	static std::unique_ptr<SwapChain> m_Swapchain;
	static std::unique_ptr<Pipeline> m_Pipeline;
	static std::unique_ptr<Renderer> m_Renderer;

	static std::unique_ptr<Shader> m_Shader;

};