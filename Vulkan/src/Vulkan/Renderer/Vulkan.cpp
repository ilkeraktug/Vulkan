#include "pch.h"
#include "Vulkan.h"

#include "Vulkan\Core.h"
#include "Vulkan\Window.h"
#include "VertexBuffer.h"

#include "Shader.h"
#include "UniformBuffer.h"

#include <glm/gtc/matrix_transform.hpp>

std::unique_ptr<SwapChain> Vulkan::m_Swapchain;
std::unique_ptr<Pipeline> Vulkan::m_Pipeline;
std::unique_ptr<Renderer> Vulkan::m_Renderer;
std::unique_ptr<Shader>	Vulkan::m_Shader;
std::unique_ptr<VertexBuffer> Vulkan::m_VertexBuffer;
std::unique_ptr<IndexBuffer> Vulkan::m_IndexBuffer;
std::unique_ptr<UniformBuffer> Vulkan::m_UniformBuffer;
std::unique_ptr<Texture> Vulkan::m_Texture;
std::unique_ptr<Model> Vulkan::m_Model;
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

	float vertices[] =
	{ 
	/*	//Vertex Positions,		Colors,				//Tex Coords
		-0.5f, -0.5f, 0.0f,		1.0f, 0.0f, 1.0f,	1.0f, 0.0f,
		 0.5f, -0.5f, 0.0f,		1.0f, 1.0f, 0.0f,	0.0f, 0.0f,
		 0.5f,  0.5f, 0.0f,		1.0f, 1.0f, 1.0f,	0.0f, 1.0f,
		-0.5f,  0.5f, 0.0f,		1.0f, 0.0f, 0.0f,	1.0f, 1.0f*/
		-0.5f, -0.5f,  0.0f,	1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
		 0.5f, -0.5f,  0.0f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
		 0.5f,  0.5f,  0.0f,	0.0f, 0.0f, 1.0f,	1.0f, 1.0f,
		-0.5f,  0.5f,  0.0f,	1.0f, 1.0f, 1.0f,	0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	1.0f, 0.0f, 0.0f,	0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,	0.0f, 0.0f, 1.0f,	1.0f, 1.0f,
	    -0.5f,  0.5f, -0.5f,	1.0f, 1.0f, 1.0f,	0.0f, 1.0f
	};

	m_Model.reset(new Model("assets/objects/vRoom/viking_room.obj", "assets/objects/vRoom/viking_room.png"));

	BufferLayout layout = { {"a_Position", ShaderType::Float3}, {"a_Color", ShaderType::Float3 }, {"a_TexCoords", ShaderType::Float2} };

	m_VertexBuffer.reset(new VertexBuffer(m_Model->GetVertex(), m_Model->GetVertex().size() * sizeof(Vertex), layout));
	
	uint16_t indices[] =
	{
		/*0, 1, 2,
		2, 3, 0*/
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};

	transform.Model = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f));
	transform.View = glm::lookAt(m_CameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	transform.Projection = glm::perspective(glm::radians(45.0f), (float)m_Swapchain->GetExtent().width / (float)m_Swapchain->GetExtent().height, 0.1f, 10.0f);

	m_Texture.reset(m_Model->GetTexture());
	m_UniformBuffer.reset(new UniformBuffer(transform, *m_Texture));

	m_IndexBuffer.reset(new IndexBuffer((uint32_t*)&m_Model->GetIndex().at(0), m_Model->GetIndex().size()));
	m_Pipeline.reset(new Pipeline(*m_Swapchain, *m_Shader, *m_VertexBuffer, *m_IndexBuffer, *m_UniformBuffer));
	m_Renderer.reset(new Renderer(*m_Pipeline));

}
	
void Vulkan::Shutdown()
{
}

void Vulkan::Run()
{
	float time = glfwGetTime();
	currentTime = time - lastTime;
	lastTime = time;

	transform.Model = glm::rotate(transform.Model, glm::radians(90.0f) * currentTime, glm::vec3(0.0f, 0.0f, 1.0f));
	transform.View = glm::lookAt(m_CameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_W) == GLFW_PRESS)
	{
		m_CameraPos.y -= 5.0f * currentTime;
	}
	else if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_S) == GLFW_PRESS)
	{
		m_CameraPos.y += 5.0f * currentTime;
	}

	if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_E) == GLFW_PRESS)
	{
		m_CameraPos.x += 5.0f * currentTime;
	}
	else if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_Q) == GLFW_PRESS)
	{
		m_CameraPos.x -= 5.0f * currentTime;
	}

	if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_A) == GLFW_PRESS)
	{
		m_CameraPos.z += 5.0f * currentTime;
	}
	else if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_D) == GLFW_PRESS)
	{
		m_CameraPos.z -= 5.0f * currentTime;
	}

	if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		m_CameraPos = glm::vec3(0.0f);
	}

	m_Renderer->Run(*m_UniformBuffer);

}

void Vulkan::recreateSwapchain(uint32_t width, uint32_t height)
{
	m_Swapchain->recreateSwapchain(width, height);
	m_Pipeline.reset(new Pipeline(*m_Swapchain, *m_Shader, *m_VertexBuffer, *m_IndexBuffer, *m_UniformBuffer));
	m_Renderer.reset(new Renderer(*m_Pipeline));
}


