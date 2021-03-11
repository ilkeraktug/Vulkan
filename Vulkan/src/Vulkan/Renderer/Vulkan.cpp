#include "pch.h"
#include "Vulkan.h"

#include "Vulkan\Core.h"
#include "Vulkan\Window.h"

#include "Shader.h"

Vulkan::Vulkan()
{
	Init();
}

Vulkan::~Vulkan()
{
	Shutdown();
}

void Vulkan::Init()
{
	m_VulkanCore.reset(new VulkanCore);
	m_Swapchain.reset(new SwapChain);
	m_Shader.reset(new Shader("assets/shaders/vert.spv", "assets/shaders/frag.spv"));
	m_Pipeline.reset(new Pipeline(*m_Swapchain, *m_Shader));
	m_Renderer.reset(new Renderer(*m_Pipeline));
}
	
void Vulkan::Shutdown()
{

}

void Vulkan::Run()
{
	m_Renderer->Run();
}

