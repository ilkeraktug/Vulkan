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
}
	
void Vulkan::Shutdown()
{
}

void Vulkan::Run()
{

}

