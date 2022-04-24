#include "pch.h"
#include "VulkanUI.h"

VulkanUI::VulkanUI(VulkanCore* core)
	:m_Core(core)
{
	ImGui::CreateContext();

	//this initializes imgui for SDL
	ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(Window::GetWindow()), true);
	// Color scheme
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
	style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	// Dimensions
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 1.0f;
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

	m_BufferLayout = { {"a_Position", ShaderFormat::Float2},  {"a_TexCoords", ShaderFormat::Float2}, {"a_Color", ShaderFormat::Float4} };

	prepareFont();
	prepareDescriptor();
	createRenderPass();
	preparePipeline();
}

void VulkanUI::OnUpdate()
{
	ImDrawData* imDrawData = ImGui::GetDrawData();

	if (!imDrawData)
		return;

	if (imDrawData->TotalVtxCount == 0 || imDrawData->TotalIdxCount == 0)
		return;

	std::vector<uint16_t> imIndexBuffer;
	imIndexBuffer.resize(imDrawData->TotalIdxCount);

	VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
	VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

	int vertexOffset = 0;
	int indexOffset = 0;

	VkBufferCreateInfo vertexBufferCI = init::createBufferInfo(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &vertexBufferCI, nullptr, &vertexBuffer));

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(m_Core->GetDevice(), vertexBuffer, &memReqs);

	VkMemoryAllocateInfo memAllocInfo = init::memAllocInfo();
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReqs.memoryTypeBits, (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

	VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &memAllocInfo, nullptr, &vertexBufferMemory));

	vkBindBufferMemory(m_Core->GetDevice(), vertexBuffer, vertexBufferMemory, 0);

	void* data;
	vkMapMemory(m_Core->GetDevice(), vertexBufferMemory, 0, vertexBufferSize, 0, &data);
	ImDrawVert* vtxDst = (ImDrawVert*)data;





	VkBufferCreateInfo indexBufferCI = init::createBufferInfo(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &indexBufferCI, nullptr, &indexBuffer));

	vkGetBufferMemoryRequirements(m_Core->GetDevice(), indexBuffer, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReqs.memoryTypeBits, (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

	VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &memAllocInfo, nullptr, &indexBufferMemory));

	vkBindBufferMemory(m_Core->GetDevice(), indexBuffer, indexBufferMemory, 0);

	void* indexData;
	vkMapMemory(m_Core->GetDevice(), indexBufferMemory, 0, indexBufferSize, 0, &indexData);
	ImDrawIdx* idxDst = (ImDrawIdx*)indexData;

	for (int n = 0; n < imDrawData->CmdListsCount; n++) {
		const ImDrawList* cmd_list = imDrawData->CmdLists[n];

		memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vtxDst += cmd_list->VtxBuffer.Size;
		idxDst += cmd_list->IdxBuffer.Size;

		vertexOffset += cmd_list->VtxBuffer.Size;
		indexOffset += cmd_list->IdxBuffer.Size;
	}

	vkUnmapMemory(m_Core->GetDevice(), vertexBufferMemory);
	vkUnmapMemory(m_Core->GetDevice(), indexBufferMemory);

	if (m_IndexCount != imDrawData->TotalIdxCount || firstTimeIndex)
	{
		//m_IndexBuffer.reset(new VulkanIndexBuffer(imIndexBuffer.data(), imIndexBuffer.size(), m_Core));
		firstTimeIndex = false;
	}

	m_VertexCount = imDrawData->TotalVtxCount;
	m_IndexCount = imDrawData->TotalIdxCount;
}

void VulkanUI::draw(VkCommandBuffer commandBuffer)
{
	ImDrawData* imDrawData = ImGui::GetDrawData();
	int32_t vertexOffset = 0;
	int32_t indexOffset = 0;

	if ((!imDrawData) || (imDrawData->CmdListsCount == 0)) {
		return;
	}

	ImGuiIO& io = ImGui::GetIO();

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, NULL);

	pushConstant.Scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
	pushConstant.Translate = glm::vec2(-1.0f);
	vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantBlock), &pushConstant);

	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

	for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
	{
		const ImDrawList* cmd_list = imDrawData->CmdLists[i];
		for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
			VkRect2D scissorRect;
			scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
			scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
			scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
			scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
			//vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
			vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
			indexOffset += pcmd->ElemCount;
		}
		vertexOffset += cmd_list->VtxBuffer.Size;
	}

}

void VulkanUI::prepareFont()
{
	ImGuiIO& io = ImGui::GetIO();

	unsigned char* fontData;
	int width, height;

	io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f);
	io.Fonts->GetTexDataAsRGBA32(&fontData, &width, &height);

	Texture.reset(new VulkanTexture2D((void*)fontData, width, height, m_Core));
}

void VulkanUI::prepareDescriptor()
{

	std::array<VkDescriptorSetLayoutBinding, 1> descriptorBinding = {
		init::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)
	};

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = init::descriptorSetLayoutCreateInfo();
	descriptorSetLayoutCI.bindingCount = descriptorBinding.size();
	descriptorSetLayoutCI.pBindings = descriptorBinding.data();

	VK_CHECK(vkCreateDescriptorSetLayout(m_Core->GetDevice(), &descriptorSetLayoutCI, nullptr, &m_DescriptorSetLayout));

	std::array<VkDescriptorPoolSize, 1> descriptorPoolSize = {
		init::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
	};

	VkDescriptorPoolCreateInfo descpritorPoolCI = init::descriptorPoolCreateInfo();
	descpritorPoolCI.poolSizeCount = descriptorPoolSize.size();
	descpritorPoolCI.pPoolSizes = descriptorPoolSize.data();
	descpritorPoolCI.maxSets = descriptorPoolSize.size() + 1;

	VK_CHECK(vkCreateDescriptorPool(m_Core->GetDevice(), &descpritorPoolCI, nullptr, &m_DescriptorPool));

	VkDescriptorSetAllocateInfo descriptorSetAI = init::descriptorSetAllocateInfo();
	descriptorSetAI.descriptorPool = m_DescriptorPool;
	descriptorSetAI.descriptorSetCount = 1;
	descriptorSetAI.pSetLayouts = &m_DescriptorSetLayout;

	for (int i = 0; i < 3; i++)
	{
		VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &descriptorSetAI, &m_DescriptorSet));

		VkWriteDescriptorSet writeDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		writeDescriptorSet.dstSet = m_DescriptorSet;
		writeDescriptorSet.dstBinding = 0;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet.pImageInfo = &Texture->descriptors[i];

		vkUpdateDescriptorSets(m_Core->GetDevice(), 1, &writeDescriptorSet, 0, nullptr);
	}
}

void VulkanUI::createRenderPass()
{

	VkAttachmentDescription attachment{};
	attachment.format = m_Core->swapchain.colorFormat;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachment{};
	colorAttachment.attachment = 0;
	colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachment;

	VkRenderPassCreateInfo renderPassCI = init::renderPassCreateInfo();
	renderPassCI.attachmentCount = 1;
	renderPassCI.pAttachments = &attachment;
	renderPassCI.subpassCount = 1;
	renderPassCI.pSubpasses = &subpass;

	VK_CHECK(vkCreateRenderPass(m_Core->GetDevice(), &renderPassCI, nullptr, &m_RenderPass));
}

void VulkanUI::preparePipeline()
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.size = sizeof(PushConstantBlock);
	pushConstantRange.offset = 0;

	VkPipelineLayoutCreateInfo pipelineLayoutCI = init::pipelineLayoutCreateInfo();
	pipelineLayoutCI.setLayoutCount = 1;
	pipelineLayoutCI.pSetLayouts = &m_DescriptorSetLayout;
	pipelineLayoutCI.pushConstantRangeCount = 1;
	pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;

	VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCI, nullptr, &m_PipelineLayout));

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
		VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/ui/uioverlayVert.spv", VK_SHADER_STAGE_VERTEX_BIT),
		VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/ui/uioverlayFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	VkVertexInputBindingDescription vertexInputBinding{};
	vertexInputBinding.binding = 0;
	vertexInputBinding.stride = m_BufferLayout.GetStride();
	vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputAttributeDescription> m_VertexAttributes;
	m_VertexAttributes.clear();
	m_VertexAttributes.resize(m_BufferLayout.GetElements().size());

	for (size_t i = 0; i < m_VertexAttributes.size(); i++)
	{
		m_VertexAttributes[i].binding = 0;
		m_VertexAttributes[i].location = i;
		m_VertexAttributes[i].format = m_BufferLayout.GetElements().at(i).VkFormat;
		m_VertexAttributes[i].offset = m_BufferLayout.GetElements().at(i).Offset;
	}

	VkPipelineVertexInputStateCreateInfo vertexInputState = init::pipelineVertexInputState();
	vertexInputState.vertexBindingDescriptionCount = 1;
	vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
	vertexInputState.vertexAttributeDescriptionCount = m_VertexAttributes.size();
	vertexInputState.pVertexAttributeDescriptions = m_VertexAttributes.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = init::pipelineInputAssemblyState();
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = init::pipelineRasterizationState();
	rasterizationStateInfo.depthClampEnable = VK_FALSE;
	rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationStateInfo.cullMode = 0;
	rasterizationStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationStateInfo.depthBiasEnable = VK_FALSE;
	rasterizationStateInfo.lineWidth = 1.0f;

	VkPipelineColorBlendAttachmentState blendAttachment{};
	blendAttachment.blendEnable = VK_FALSE;
	blendAttachment.colorWriteMask = 0xF;

	VkPipelineColorBlendStateCreateInfo blendState = init::pipelineColorBlendState();
	blendState.logicOpEnable = VK_FALSE;
	blendState.attachmentCount = 1;
	blendState.pAttachments = &blendAttachment;

	VkPipelineDepthStencilStateCreateInfo depthStencilState = init::pipelineDepthStencilState();
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.depthWriteEnable = VK_FALSE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilState.stencilTestEnable = VK_FALSE;

	VkViewport viewPort{ 0, 0, m_Core->swapchain.extent.width, m_Core->swapchain.extent.height, 0.0f, 1.0f };

	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = m_Core->swapchain.extent;

	VkPipelineViewportStateCreateInfo viewportState = init::pipelineViewportState();
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewPort;

	VkPipelineMultisampleStateCreateInfo multiSampleState = init::multiSampleState();
	multiSampleState.sampleShadingEnable = VK_FALSE;
	multiSampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	std::array<VkDynamicState, 2> dynamicStates = {
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState = init::dynamicState();
	dynamicState.dynamicStateCount = 0;

	VkGraphicsPipelineCreateInfo graphicsPipelineCI = init::graphicsPipelineCreateInfo();
	graphicsPipelineCI.stageCount = shaderStages.size();
	graphicsPipelineCI.pStages = shaderStages.data();
	graphicsPipelineCI.pVertexInputState = &vertexInputState;
	graphicsPipelineCI.pInputAssemblyState = &inputAssembly;
	graphicsPipelineCI.pRasterizationState = &rasterizationStateInfo;
	graphicsPipelineCI.pViewportState = &viewportState;
	graphicsPipelineCI.pMultisampleState = &multiSampleState;
	graphicsPipelineCI.pDepthStencilState = &depthStencilState;
	graphicsPipelineCI.pColorBlendState = &blendState;
	graphicsPipelineCI.pDynamicState = &dynamicState;
	graphicsPipelineCI.layout = m_PipelineLayout;
	//graphicsPipelineCI.renderPass = m_RenderPass;
	graphicsPipelineCI.renderPass = m_Core->resources.renderPass;
	graphicsPipelineCI.subpass = 0;

	VK_CHECK(vkCreateGraphicsPipelines(m_Core->GetDevice(), nullptr, 1, &graphicsPipelineCI, nullptr, &m_GraphicsPipeline));
}

