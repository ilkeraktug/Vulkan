#include "pch.h"
#include "CubeObj.h"

CubeObj::CubeObj(VulkanCore* core)
{
    float vertices[] = {
        //Positon             //Normal             //TexCoords
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-1.0f, -1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 1.0f, -1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-1.0f, -1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-1.0f, -1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

	VertexBufferLayout layout = { {"a_Position", ShaderFormat::Float3},  {"a_Normal", ShaderFormat::Float3}, {"a_TexCoords", ShaderFormat::Float2} };

	VertexBuffer.reset(new VulkanVertexBuffer(vertices, sizeof(vertices), core));
	VertexBuffer->SetLayout(layout);

	ModelBuffer.reset(new VulkanUniformBuffer(sizeof(glm::mat4), core));
	ModelBuffer->copyData(&m_ModelMatrix, sizeof(glm::mat4));
}

CubeObj::~CubeObj()
{
}

void CubeObj::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &VertexBuffer->GetBuffer(), offsets);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &DescriptorSet, 0, nullptr);

	vkCmdDraw(commandBuffer, 36, 1, 0, 0);
}

void CubeObj::Rotate(float angle, const glm::vec3& axis)
{
    m_RotationMatrix = glm::rotate(m_RotationMatrix, glm::radians(angle), axis);

    updateUniformBuffers();
}

