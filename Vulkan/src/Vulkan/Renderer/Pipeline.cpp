#include "pch.h"
#include "Pipeline.h"

Pipeline::Pipeline(SwapChain& swapchain, const Shader& shader, VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, UniformBuffer& uniformBuffer)
	:m_Swapchain(swapchain), m_VertexBuffer(vertexBuffer), m_IndexBuffer(indexBuffer), m_UniformBuffer(uniformBuffer)
{
	createPipeline(shader);
	createCommandPool();
}

Pipeline::~Pipeline()
{
	vkQueueWaitIdle(VulkanCore::GetGraphicsQueue());
	vkDestroyCommandPool(VulkanCore::GetDevice(), m_CommandPool, nullptr);
	vkDestroyPipeline(VulkanCore::GetDevice(), m_GraphicsPipeline, VK_NULL_HANDLE);
	vkDestroyPipelineLayout(VulkanCore::GetDevice(), m_PipelineLayout, nullptr);
}

void Pipeline::createPipeline(const Shader& shader)
{
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.pVertexBindingDescriptions = &m_VertexBuffer.GetVertexBinding();
	vertexInputCreateInfo.vertexAttributeDescriptionCount = m_VertexBuffer.GetVertexAttribute().size();
	vertexInputCreateInfo.pVertexAttributeDescriptions = m_VertexBuffer.GetVertexAttribute().data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.width = m_Swapchain.GetExtent().width;
	viewport.height = m_Swapchain.GetExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport.x = 0;
	viewport.y = 0;

	VkRect2D scissor{};
	scissor.extent = m_Swapchain.GetExtent();
	scissor.offset = { 0, 0 };

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationCreateInfo.depthClampEnable = VK_FALSE;
	rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationCreateInfo.cullMode = VK_CULL_MODE_NONE;
	rasterizationCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizationCreateInfo.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendCreateInfo.attachmentCount = 1;
	colorBlendCreateInfo.pAttachments = &colorBlendAttachment;

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicStateCreateInfo.dynamicStateCount = 0;
	dynamicStateCreateInfo.pDynamicStates = nullptr;

	VkPipelineLayoutCreateInfo pipelineLayoutCrateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutCrateInfo.setLayoutCount = 1;
	pipelineLayoutCrateInfo.pSetLayouts = &m_UniformBuffer.GetDescriptorSetLayout();

	VK_ASSERT(vkCreatePipelineLayout(VulkanCore::GetDevice(), &pipelineLayoutCrateInfo, nullptr, &m_PipelineLayout) == VK_SUCCESS, "Failed to create vkCreatePipelineLayout");

	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	graphicsPipelineCreateInfo.stageCount = 2;
	graphicsPipelineCreateInfo.pStages = shader.GetShaderStageCreateInfos();
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = nullptr;
	graphicsPipelineCreateInfo.layout = m_PipelineLayout;
	graphicsPipelineCreateInfo.renderPass = m_Swapchain.GetRenderPass();
	graphicsPipelineCreateInfo.subpass = 0;

	VK_ASSERT(vkCreateGraphicsPipelines(VulkanCore::GetDevice(), VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &m_GraphicsPipeline) == VK_SUCCESS, "Failed to create vkCreateGraphicsPipelines");
}

void Pipeline::createCommandPool()
{
	VkCommandPoolCreateInfo commandPoolCreateInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = VulkanCore::GetQueueIndices().GraphicsIndex.value();

	VK_ASSERT(vkCreateCommandPool(VulkanCore::GetDevice(), &commandPoolCreateInfo, nullptr, &m_CommandPool) == VK_SUCCESS, "Failed to create vkCreateCommandPool");

	m_CommandBuffers.resize(m_Swapchain.GetImageCount());

	VkCommandBufferAllocateInfo commandBufferAllocate{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	commandBufferAllocate.commandPool = m_CommandPool;
	commandBufferAllocate.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocate.commandBufferCount = m_CommandBuffers.size();

	vkAllocateCommandBuffers(VulkanCore::GetDevice(), &commandBufferAllocate, m_CommandBuffers.data());

	for (size_t i = 0; i < m_CommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo commandBuffer{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		//commandBuffer.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		vkBeginCommandBuffer(m_CommandBuffers.at(i), &commandBuffer);

		VkClearValue clearScreen[2];
		clearScreen[0].color = { 1.0f, 0.5f, 0.31f, 1.0f };
		clearScreen[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBegin{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderPassBegin.renderPass = m_Swapchain.GetRenderPass();
		renderPassBegin.clearValueCount = 2;
		renderPassBegin.pClearValues = clearScreen;
		renderPassBegin.renderArea.extent = m_Swapchain.GetExtent();
		renderPassBegin.renderArea.offset = { 0, 0 };
		renderPassBegin.framebuffer = m_Swapchain.GetFramebuffers().at(i);


		vkCmdBeginRenderPass(m_CommandBuffers.at(i), &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(m_CommandBuffers.at(i), VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(m_CommandBuffers.at(i), 0, 1, &m_VertexBuffer.GetBuffer(), offsets);
		vkCmdBindIndexBuffer(m_CommandBuffers.at(i), m_IndexBuffer.GetBuffer(), 0, m_IndexBuffer.GetType());
		vkCmdBindDescriptorSets(m_CommandBuffers.at(i), VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_UniformBuffer.GetDescriptorSets().at(i), 0, 0);

		vkCmdDrawIndexed(m_CommandBuffers.at(i), m_IndexBuffer.GetCount(), 1, 0, 0, 0);

		vkCmdEndRenderPass(m_CommandBuffers.at(i));
		VK_ASSERT(vkEndCommandBuffer(m_CommandBuffers.at(i)) == VK_SUCCESS, "Cant end command buffer");
	}

}
