#include "pch.h"
#include "QuadObj.h"

QuadObj::QuadObj(VulkanCore* core)
{
	float vertices[] =
	{
		//Vertex Positions,		Colors,				//Tex Coords
		-0.5f, -0.5f, 0.0f,		1.0f, 0.0f, 1.0f,	1.0f, 0.0f,
		 0.5f, -0.5f, 0.0f,		1.0f, 1.0f, 0.0f,	0.0f, 0.0f,
		 0.5f,  0.5f, 0.0f,		1.0f, 1.0f, 1.0f,	0.0f, 1.0f,
		-0.5f,  0.5f, 0.0f,		1.0f, 0.0f, 0.0f,	1.0f, 1.0f
	};

	VertexBufferLayout layout = { {"a_Position", ShaderFormat::Float3},  {"a_Color", ShaderFormat::Float3}, {"a_TexCoords", ShaderFormat::Float2} };

	VertexBuffer.reset(new VulkanVertexBuffer(vertices, sizeof(vertices), core));
	VertexBuffer->SetLayout(layout);

	uint16_t indices[] =
	{ 0, 1, 2,
	  2, 3, 0 };

	m_IndexBuffer.reset(new VulkanIndexBuffer(indices, 6, core));

	ModelBuffer.reset(new VulkanUniformBuffer(sizeof(glm::mat4), core));
	ModelBuffer->copyData(&m_ModelMatrix, sizeof(glm::mat4));
}

QuadObj::~QuadObj()
{
}

