#include "pch.h"
#include "QuadObj.h"

QuadObj::QuadObj(VulkanCore* core)
	:m_Core(core)
{
	float vertices[] =
	{
		//Vertex Positions,		Colors,				//Tex Coords
		-0.5f, -0.5f, 0.0f,		1.0f, 0.0f, 1.0f,	1.0f, 0.0f,
		 0.5f, -0.5f, 0.0f,		1.0f, 1.0f, 0.0f,	0.0f, 0.0f,
		 0.5f,  0.5f, 0.0f,		1.0f, 1.0f, 1.0f,	0.0f, 1.0f,
		-0.5f,  0.5f, 0.0f,		1.0f, 0.0f, 0.0f,	1.0f, 1.0f
	};

	//m_VertexBuffer = new VulkanVertexBuffer(vertices, sizeof(vertices), { {"a_Position", ShaderFormat::Float3},  {"a_Color", ShaderFormat::Float3}, {"a_TexCoords", ShaderFormat::Float2} }, m_Core);
	VertexBufferLayout layout = { {"a_Position", ShaderFormat::Float3},  {"a_Color", ShaderFormat::Float3}, {"a_TexCoords", ShaderFormat::Float2} };

	m_VertexBuffer.reset(new VulkanVertexBuffer(vertices, sizeof(vertices), m_Core));
	m_VertexBuffer->SetLayout(layout);

	uint16_t indices[] =
	{ 0, 1, 2,
	  2, 3, 0 };

	m_IndexBuffer.reset(new VulkanIndexBuffer(indices, 6, m_Core));

	ModelBuffer.reset(new VulkanUniformBuffer(sizeof(glm::mat4), m_Core));
	ModelBuffer->copyData(&m_ModelMatrix, sizeof(glm::mat4));
}

void QuadObj::OnUpdate(float deltaTime)
{
	m_ModelMatrix = glm::rotate(m_ModelMatrix, glm::radians(90.0f + m_Position.x * 10) * deltaTime, glm::vec3(0.0f, 0.0f, 1.0f));
	//m_ModelMatrix = glm::scale(m_ModelMatrix, m_Position);

	updateUniformBuffers();
}

void QuadObj::allocDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout* descriptorLayout)
{
	VkDescriptorSetAllocateInfo descriptorSetAI = init::descriptorSetAllocateInfo();
	descriptorSetAI.descriptorPool = descriptorPool;
	descriptorSetAI.descriptorSetCount = 1;
	descriptorSetAI.pSetLayouts = descriptorLayout;
	VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &descriptorSetAI, &DescriptorSet));

	VkWriteDescriptorSet writeDescriptor{};
	writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptor.dstSet = DescriptorSet;
	writeDescriptor.dstBinding = 0;
	writeDescriptor.descriptorCount = 1;
	writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptor.pBufferInfo = &ModelBuffer->GetBufferInfo();

	vkUpdateDescriptorSets(m_Core->GetDevice(), 1, &writeDescriptor, 0, nullptr);
}

void QuadObj::updateUniformBuffers()
{
	ModelBuffer->copyData(&m_ModelMatrix, sizeof(glm::mat4));
}
