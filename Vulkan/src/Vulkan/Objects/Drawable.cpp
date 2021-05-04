#include "pch.h"
#include "Drawable.h"

void Drawable::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{

	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_VertexBuffer->GetBuffer(), offsets);
	vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->GetBuffer(), 0, m_IndexBuffer->GetIndexType());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &DescriptorSet, 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, m_IndexBuffer->GetCount(), 1, 0, 0, 0);
}

void Drawable::SetPosition(glm::vec3 position)
{
	m_Position = position;
}

void Drawable::SetPosition(float* position)
{
	m_Position.x = position[0];
	m_Position.y = position[1];
	m_Position.z = position[2];
}

void Drawable::SetPosition(float x, float y, float z)
{
	m_Position.x = x;
	m_Position.y = y;
	m_Position.z = z;
}

void Drawable::SetPosition(float xyz)
{
	m_Position.x = xyz;
	m_Position.y = xyz;
	m_Position.z = xyz;
}
