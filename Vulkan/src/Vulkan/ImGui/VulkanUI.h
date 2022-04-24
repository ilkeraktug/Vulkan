#pragma once

#include <glm/glm.hpp>
#include <imgui.h>

#include "Vulkan/Renderer/VulkanVertexBuffer.h"
#include "Vulkan/Renderer/VulkanIndexBuffer.h"
#include "Vulkan/Renderer/VulkanTexture2D.h"
#include "Vulkan/Renderer/VulkanShader.h"

class VulkanUI
{
public:
	VulkanUI() = default;
	VulkanUI(VulkanCore* core);

	std::unique_ptr< VulkanTexture2D> Texture;

	void OnUpdate();
	void draw(VkCommandBuffer commandBuffer);
private:

	void prepareFont();
	void prepareDescriptor();
	void createRenderPass();
	void preparePipeline();
private:
	struct PushConstantBlock
	{
		glm::vec2 Scale;
		glm::vec2 Translate;
	}pushConstant;

	VulkanCore* m_Core;

	std::unique_ptr<VulkanVertexBuffer> m_VertexBuffer;
	int m_VertexCount;
	VkBuffer vertexBuffer;
	VkBuffer indexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkDeviceMemory indexBufferMemory;
	std::unique_ptr<VulkanIndexBuffer> m_IndexBuffer;
	int m_IndexCount;
	bool firstTimeVertex = true;
	bool firstTimeIndex = true;

	VkDescriptorPool m_DescriptorPool;
	VkDescriptorSetLayout m_DescriptorSetLayout;
	VkDescriptorSet m_DescriptorSet;

	VkRenderPass m_RenderPass;

	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_GraphicsPipeline;

	VertexBufferLayout m_BufferLayout;
	std::string fontPath = "assets/fonts/Roboto-Medium.ttf";
};