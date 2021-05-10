#include "pch.h"
#include "Background.h"

Background::Background(VulkanCore* core)
	:QuadObj(core)
{
	float vertices[] =
	{
		//Vertex Positions,
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.5f,  0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f,
	};

	VertexBufferLayout layout = { {"a_Position", ShaderFormat::Float3} };

	VertexBuffer.reset(new VulkanVertexBuffer(vertices, sizeof(vertices), core));
	VertexBuffer->SetLayout(layout);

	uint16_t indices[] =
	{ 0, 1, 2,
	  2, 3, 0 };

	m_IndexBuffer.reset(new VulkanIndexBuffer(indices, 6, core));

	ModelBuffer.reset(new VulkanUniformBuffer(sizeof(glm::mat4), core));
	ModelBuffer->copyData(&m_ModelMatrix, sizeof(glm::mat4));

	Texture.reset(new VulkanTexture2D("assets/textures/flappyBird/bg.jpeg", core));
}

Background::~Background()
{
}
