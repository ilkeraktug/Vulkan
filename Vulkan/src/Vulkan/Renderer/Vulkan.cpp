#include "pch.h"
#include "Vulkan.h"

#include "Vulkan\Core.h"
#include "Vulkan\Window.h"
#include "VertexBuffer.h"

#include "Shader.h"

std::unique_ptr<SwapChain> Vulkan::m_Swapchain;
std::unique_ptr<Pipeline> Vulkan::m_Pipeline;
std::unique_ptr<Renderer> Vulkan::m_Renderer;
std::unique_ptr<Shader>	Vulkan::m_Shader;
std::unique_ptr<VertexBuffer> Vulkan::m_VertexBuffer;

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
	float indices[] = 
	{ 
		//Vertex Positions,		Colors
		-0.5f,  0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
		 0.5f,  0.0f, 0.0f,		1.0f, 1.0f, 0.0f,
		 0.0f, -0.5f, 0.0f,		1.0f, 0.0f, 1.0f
	};
	BufferLayout layout = { {"a_Position", ShaderType::Float3}, {"a_Color", ShaderType::Float3 } };
	m_VertexBuffer.reset(new VertexBuffer(indices, sizeof(indices), layout));
	m_Pipeline.reset(new Pipeline(*m_Swapchain, *m_Shader, *m_VertexBuffer));
	m_Renderer.reset(new Renderer(*m_Pipeline));
}
	
void Vulkan::Shutdown()
{
}

void Vulkan::Run()
{
	m_Renderer->Run();
}

void Vulkan::recreateSwapchain(uint32_t width, uint32_t height)
{
	m_Swapchain->recreateSwapchain(width, height);
	m_Pipeline.reset(new Pipeline(*m_Swapchain, *m_Shader, *m_VertexBuffer));
	m_Renderer.reset(new Renderer(*m_Pipeline));
}


