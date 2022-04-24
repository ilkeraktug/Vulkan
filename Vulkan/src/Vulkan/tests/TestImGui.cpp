#include "pch.h"
#include "TestImGui.h"

namespace test
{
	TestImGui::TestImGui(VulkanCore* core)
	{
		Test::Init(core);
		glfwSetWindowTitle(static_cast<GLFWwindow*>(Window::GetWindow()), "TestImGui");

		float right = m_Core->swapchain.extent.width / 200.0f;
		float top = m_Core->swapchain.extent.height / 200.0f;
		m_Camera = new OrthographicCamera(-right, right, -top, top, core);

		float vertices[] =
		{
			//   Vertex Positions,		Colors,				Tex Coords
				-0.5f, -0.5f, 0.0f,		1.0f, 0.0f, 1.0f,	1.0f, 0.0f,
				 0.5f, -0.5f, 0.0f,		1.0f, 1.0f, 0.0f,	0.0f, 0.0f,
				 0.5f,  0.5f, 0.0f,		1.0f, 1.0f, 1.0f,	0.0f, 1.0f,
				-0.5f,  0.5f, 0.0f,		1.0f, 0.0f, 0.0f,	1.0f, 1.0f
		};

		VertexBufferLayout layout = { {"a_Position", ShaderFormat::Float3},  {"a_Color", ShaderFormat::Float3}, {"a_TexCoords", ShaderFormat::Float2} };

		m_VertexBuffer.reset(new VulkanVertexBuffer(vertices, sizeof(vertices), m_Core));
		m_VertexBuffer->SetLayout(layout);

		uint16_t indices[] =
		{ 0, 1, 2,
		  2, 3, 0
		};

		m_IndexBuffer.reset(new VulkanIndexBuffer(indices, 6, m_Core));

		for (int i = 0; i < objs.size(); i++)
		{
			objs[i] = new QuadObj(m_Core);
		}

		objs[0]->SetScale(5.0f, 5.0f, 0.0f);
		objs[0]->SetRotation(0.0f, 0.0f, 45.0f);
		m_Texture.reset(new VulkanTexture2D("assets/textures/face.jpg", m_Core));

		UI = new VulkanUI(core);

		prepareDescriptorPool();
		preparePipeline();

		UI->OnUpdate();
		setCmdBuffers();
	}

	TestImGui::~TestImGui()
	{
		vkDestroyDescriptorSetLayout(m_Core->GetDevice(), m_DescriptorSetLayout, nullptr);
		vkDestroyDescriptorPool(m_Core->GetDevice(), m_DescriptorPool, nullptr);
		vkDestroyPipeline(m_Core->GetDevice(), m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_Core->GetDevice(), m_PipelineLayout, nullptr);

		for (int i = 0; i < objs.size(); i++)
		{
			delete objs[i];
		}

		delete m_Camera;
	}

	void TestImGui::OnUpdate(float deltaTime)
	{
		updateUniformBuffers();
	}

	void TestImGui::OnRender()
	{
		m_Core->BeginScene();

		m_Core->resources.submitInfo.commandBufferCount = 1;
		m_Core->resources.submitInfo.pCommandBuffers = &m_Core->resources.drawCmdBuffers[m_Core->resources.imageIndex];

		setCmdBuffers();
		VK_CHECK(vkQueueSubmit(m_Core->queue.GraphicsQueue, 1, &m_Core->resources.submitInfo, VK_NULL_HANDLE));

		VkResult err = m_Core->Submit();

		if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
			windowResized();
		else
			VK_CHECK(err);


		//TODO : Fences and Semaphores !
		vkDeviceWaitIdle(m_Core->GetDevice());
	}

	void TestImGui::OnImGuiRender()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		ImGui::Text("Fps : %f", ImGui::GetIO().Framerate);

		//ImGui::EndFrame();
		//ImGui::PopStyleVar();
		ImGui::Render();

		UI->OnUpdate();

	}

	void TestImGui::prepareDescriptorPool()
	{
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings =
		{
		init::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_VERTEX_BIT, 0),

		init::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_VERTEX_BIT, 1),

		init::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT, 2),
		};

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = init::descriptorSetLayoutCreateInfo();
		descriptorSetLayoutCI.bindingCount = layoutBindings.size();
		descriptorSetLayoutCI.pBindings = layoutBindings.data();

		VK_CHECK(vkCreateDescriptorSetLayout(m_Core->GetDevice(), &descriptorSetLayoutCI, nullptr, &m_DescriptorSetLayout));

		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			init::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100),
			init::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100)
		};

		VkDescriptorPoolCreateInfo descriptorPoolCI = init::descriptorPoolCreateInfo();
		descriptorPoolCI.poolSizeCount = poolSizes.size();
		descriptorPoolCI.pPoolSizes = poolSizes.data();
		descriptorPoolCI.maxSets = 100;

		VK_CHECK(vkCreateDescriptorPool(m_Core->GetDevice(), &descriptorPoolCI, nullptr, &m_DescriptorPool));

		for (int i = 0; i < objs.size(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				VkDescriptorSetAllocateInfo descriptorSetAI = init::descriptorSetAllocateInfo();
				descriptorSetAI.descriptorPool = m_DescriptorPool;
				descriptorSetAI.descriptorSetCount = 1;
				descriptorSetAI.pSetLayouts = &m_DescriptorSetLayout;
				VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &descriptorSetAI, &objs[i]->DescriptorSets[j]));

				std::array<VkWriteDescriptorSet, 3> writeDescriptors{};
				writeDescriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptors[0].dstSet = objs[i]->DescriptorSets[j];
				writeDescriptors[0].dstBinding = 0;
				writeDescriptors[0].descriptorCount = 1;
				writeDescriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescriptors[0].pBufferInfo = &objs[i]->ModelBuffer->GetBufferInfo();

				writeDescriptors[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptors[1].dstSet = objs[i]->DescriptorSets[j];
				writeDescriptors[1].dstBinding = 1;
				writeDescriptors[1].descriptorCount = 1;
				writeDescriptors[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescriptors[1].pBufferInfo = &m_Camera->MatricesBuffer->GetBufferInfo();

				writeDescriptors[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptors[2].dstSet = objs[i]->DescriptorSets[j];
				writeDescriptors[2].dstBinding = 2;
				writeDescriptors[2].descriptorCount = 1;
				writeDescriptors[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeDescriptors[2].pImageInfo = &m_Texture->descriptors[j];

				vkUpdateDescriptorSets(m_Core->GetDevice(), writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
			}
		}
	}

	void TestImGui::preparePipeline()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = init::pipelineLayoutCreateInfo();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout;

		VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout));

		VkPipelineVertexInputStateCreateInfo vertexInputState = init::pipelineVertexInputState();
		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = &m_VertexBuffer->GetVertexInput();
		vertexInputState.vertexAttributeDescriptionCount = m_VertexBuffer->GetVertexAttributes().size();
		vertexInputState.pVertexAttributeDescriptions = m_VertexBuffer->GetVertexAttributes().data();

		VkPipelineShaderStageCreateInfo shaderStages[2];
		shaderStages[0] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/graphicsPipeline/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/graphicsPipeline/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = init::pipelineInputAssemblyState();
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = init::pipelineRasterizationState();
		rasterizationStateInfo.depthClampEnable = VK_FALSE;
		rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationStateInfo.cullMode = 0;
		rasterizationStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationStateInfo.depthBiasEnable = VK_FALSE;
		rasterizationStateInfo.lineWidth = 1.0f;

		VkPipelineColorBlendAttachmentState blendAttachment{};
		blendAttachment.blendEnable = VK_TRUE;
		blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		blendAttachment.colorWriteMask = 0xF;
		//VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo blendState = init::pipelineColorBlendState();
		blendState.logicOpEnable = VK_FALSE;
		blendState.attachmentCount = 1;
		blendState.pAttachments = &blendAttachment;

		VkPipelineDepthStencilStateCreateInfo depthStencilState = init::pipelineDepthStencilState();
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.depthWriteEnable = VK_TRUE;
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

		VkPipelineDynamicStateCreateInfo dynamicState = init::dynamicState();
		dynamicState.dynamicStateCount = 0;

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = init::pipelineCreateInfo();
		pipelineCreateInfo.stageCount = 2;
		pipelineCreateInfo.pStages = shaderStages;
		pipelineCreateInfo.pVertexInputState = &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationStateInfo;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pMultisampleState = &multiSampleState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pColorBlendState = &blendState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.layout = m_PipelineLayout;
		pipelineCreateInfo.renderPass = m_Core->resources.renderPass;
		pipelineCreateInfo.subpass = 0;

		VK_CHECK(vkCreateGraphicsPipelines(m_Core->GetDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_GraphicsPipeline));

		vkDestroyShaderModule(m_Core->GetDevice(), shaderStages[0].module, nullptr);
		vkDestroyShaderModule(m_Core->GetDevice(), shaderStages[1].module, nullptr);

	}

	void TestImGui::setCmdBuffers()
	{
		VkCommandBufferBeginInfo cmdBufferBI = init::cmdBufferBeginInfo();

		VkClearValue clearValues[2];
		clearValues[0].color = { 0.3f, 0.5f, 0.8f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBI = init::renderPassBeginInfo();
		renderPassBI.clearValueCount = 2;
		renderPassBI.pClearValues = clearValues;
		renderPassBI.renderArea.extent = m_Core->swapchain.extent;
		renderPassBI.renderArea.offset = { 0, 0 };
		renderPassBI.renderPass = m_Core->resources.renderPass;

		for (uint32_t i = 0; i < m_Core->resources.drawCmdBuffers.size(); i++)
		{
			VK_CHECK(vkBeginCommandBuffer(m_Core->resources.drawCmdBuffers[i], &cmdBufferBI));

			renderPassBI.framebuffer = m_Core->resources.frameBuffers[i];
			vkCmdBeginRenderPass(m_Core->resources.drawCmdBuffers[i], &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

			UI->draw(m_Core->resources.drawCmdBuffers[i]);

			vkCmdBindPipeline(m_Core->resources.drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

			/*for(int k = 0; k < objs.size(); k++)
				objs[k]->draw(m_Core->resources.drawCmdBuffers[i], m_PipelineLayout);*/

			objs[0]->draw(m_Core->resources.drawCmdBuffers[i], m_PipelineLayout, i);

			vkCmdEndRenderPass(m_Core->resources.drawCmdBuffers[i]);

			VK_CHECK(vkEndCommandBuffer(m_Core->resources.drawCmdBuffers[i]));
		}

	}
	void TestImGui::updateUniformBuffers()
	{

	}

	void TestImGui::windowResized()
	{
		m_Core->windowResized();

		vkDestroyPipeline(m_Core->GetDevice(), m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_Core->GetDevice(), m_PipelineLayout, nullptr);

		float right = m_Core->swapchain.extent.width / 200.0f;
		float top = m_Core->swapchain.extent.height / 200.0f;
		dynamic_cast<OrthographicCamera*>(m_Camera)->SetOrthograhic(-right, right, -top, top);

		preparePipeline();
		setCmdBuffers();
	}
}