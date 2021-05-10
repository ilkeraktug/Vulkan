#include "pch.h"
#include "TestFlappyBird.h"

namespace test
{
	TestFlappyBird::TestFlappyBird(VulkanCore* core)
	{
		Test::Init(core);
		glfwSetWindowTitle(static_cast<GLFWwindow*>(Window::GetWindow()), "TestFlappyBird");

		float right = m_Core->swapchain.extent.width / 200.0f;
		float top = m_Core->swapchain.extent.height / 200.0f;

		m_Camera = new OrthographicCamera(-right, right, -top, top, core);

		for (int i = 0; i < m_PipeObjects.size(); i++)
		{
			m_PipeObjects[i] = new PipeObject("assets/textures/flappyBird/pipe.png", m_Core);
			m_PipeObjects[i]->Rotate(180.0f * (i % 2));
			m_PipeObjects[i]->SetPosition(0.0f, -3.0f + 6 * (i % 2), 0.0f);
		}

		m_Bird.reset(new BirdObject(m_Core));
		m_Bird->Rotate(180.0f, { 0.0f, 1.0f, 0.0f });

		m_Background.reset(new Background(m_Core));

		prepareDescriptorPool();
		preparePipeline();
		setCmdBuffers();
	}

	TestFlappyBird::~TestFlappyBird()
	{
		vkDestroyDescriptorSetLayout(m_Core->GetDevice(), layout.BackgroundDescriptor, nullptr);
		vkDestroyDescriptorSetLayout(m_Core->GetDevice(), layout.ObjectsDescriptor, nullptr);
		vkDestroyDescriptorPool(m_Core->GetDevice(), m_DescriptorPool, nullptr);

		vkDestroyPipeline(m_Core->GetDevice(), pipelines.Background, nullptr);
		vkDestroyPipeline(m_Core->GetDevice(), pipelines.BirdObject, nullptr);
		vkDestroyPipeline(m_Core->GetDevice(), pipelines.PipeObject, nullptr);
		vkDestroyPipelineLayout(m_Core->GetDevice(), layout.ObjectsPipeline, nullptr);
		vkDestroyPipelineLayout(m_Core->GetDevice(), layout.BackgroundPipeline, nullptr);

		for (int i = 0; i < m_PipeObjects.size(); i++)
		{
			delete m_PipeObjects[i];
		}

		delete m_Camera;
	}

	void TestFlappyBird::OnUpdate(float deltaTime)
	{

		if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_A) == GLFW_PRESS)
		{
			m_CameraPosition.x -= m_CameraMoveSpeed * deltaTime;
		}
		else if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_D) == GLFW_PRESS)
		{
			m_CameraPosition.x += m_CameraMoveSpeed * deltaTime;
		}

		if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_W) == GLFW_PRESS)
		{
			m_CameraPosition.y += m_CameraMoveSpeed * deltaTime;
		}
		else if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_S) == GLFW_PRESS)
		{
			m_CameraPosition.y -= m_CameraMoveSpeed * deltaTime;
		}

		m_Camera->SetPosition(m_CameraPosition);
		
		/*if (glfwGetMouseButton(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
		{
			m_Bird->Translate({ 0.0f, -5.0f * deltaTime, 0.0 });
			m_Bird->SetRotation(0.0f, 180.0f, 45.0f);
		}

		m_Bird->Translate({ 0.5f * deltaTime, 2.0f * deltaTime, 0.0 });
		m_Bird->SetRotation(0.0f, 180.0f, -45.0f);*/

		updateUniformBuffers();
	}

	void TestFlappyBird::OnRender()
	{
		m_Core->BeginScene();

		m_Core->resources.submitInfo.commandBufferCount = 1;
		m_Core->resources.submitInfo.pCommandBuffers = &m_Core->resources.drawCmdBuffers[m_Core->resources.imageIndex];

		//setCmdBuffers();
		VK_CHECK(vkQueueSubmit(m_Core->queue.GraphicsQueue, 1, &m_Core->resources.submitInfo, VK_NULL_HANDLE));

		VkResult err = m_Core->Submit();

		if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
			windowResized();
		else
			VK_CHECK(err);


		//TODO : Fences and Semaphores !
		vkDeviceWaitIdle(m_Core->GetDevice());
	}

	void TestFlappyBird::OnImGuiRender()
	{
	}

	void TestFlappyBird::prepareDescriptorPool()
	{
		std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings =
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

		VK_CHECK(vkCreateDescriptorSetLayout(m_Core->GetDevice(), &descriptorSetLayoutCI, nullptr, &layout.ObjectsDescriptor));

		std::array<VkDescriptorSetLayoutBinding, 2> bgLayoutBindings =
		{
		init::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_VERTEX_BIT, 0),

		init::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT, 1),
		};

		descriptorSetLayoutCI.bindingCount = bgLayoutBindings.size();
		descriptorSetLayoutCI.pBindings = bgLayoutBindings.data();

		VK_CHECK(vkCreateDescriptorSetLayout(m_Core->GetDevice(), &descriptorSetLayoutCI, nullptr, &layout.BackgroundDescriptor));

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

		//Descriptor Set for Pipe OBJs
		for (int i = 0; i < m_PipeObjects.size(); i++)
		{

			VkDescriptorSetAllocateInfo descriptorSetAI = init::descriptorSetAllocateInfo();
			descriptorSetAI.descriptorPool = m_DescriptorPool;
			descriptorSetAI.descriptorSetCount = 1;
			descriptorSetAI.pSetLayouts = &layout.ObjectsDescriptor;
			VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &descriptorSetAI, &m_PipeObjects[i]->DescriptorSet));

			std::array<VkWriteDescriptorSet, 3> writeDescriptors{};
			writeDescriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptors[0].dstSet = m_PipeObjects[i]->DescriptorSet;
			writeDescriptors[0].dstBinding = 0;
			writeDescriptors[0].descriptorCount = 1;
			writeDescriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptors[0].pBufferInfo = &m_PipeObjects[i]->ModelBuffer->GetBufferInfo();

			writeDescriptors[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptors[1].dstSet = m_PipeObjects[i]->DescriptorSet;
			writeDescriptors[1].dstBinding = 1;
			writeDescriptors[1].descriptorCount = 1;
			writeDescriptors[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptors[1].pBufferInfo = &m_Camera->MatricesBuffer->GetBufferInfo();

			writeDescriptors[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptors[2].dstSet = m_PipeObjects[i]->DescriptorSet;
			writeDescriptors[2].dstBinding = 2;
			writeDescriptors[2].descriptorCount = 1;
			writeDescriptors[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptors[2].pImageInfo = &m_PipeObjects[i]->Texture->descriptor;

			vkUpdateDescriptorSets(m_Core->GetDevice(), writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
		}

		//Descriptor Set for Bird OBJ
		{
			VkDescriptorSetAllocateInfo descriptorSetAI = init::descriptorSetAllocateInfo();
			descriptorSetAI.descriptorPool = m_DescriptorPool;
			descriptorSetAI.descriptorSetCount = 1;
			descriptorSetAI.pSetLayouts = &layout.ObjectsDescriptor;
			VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &descriptorSetAI, &m_Bird->DescriptorSet));

			std::array<VkWriteDescriptorSet, 3> writeDescriptors{};
			writeDescriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptors[0].dstSet = m_Bird->DescriptorSet;
			writeDescriptors[0].dstBinding = 0;
			writeDescriptors[0].descriptorCount = 1;
			writeDescriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptors[0].pBufferInfo = &m_Bird->ModelBuffer->GetBufferInfo();

			writeDescriptors[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptors[1].dstSet = m_Bird->DescriptorSet;
			writeDescriptors[1].dstBinding = 1;
			writeDescriptors[1].descriptorCount = 1;
			writeDescriptors[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptors[1].pBufferInfo = &m_Camera->MatricesBuffer->GetBufferInfo();

			writeDescriptors[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptors[2].dstSet = m_Bird->DescriptorSet;
			writeDescriptors[2].dstBinding = 2;
			writeDescriptors[2].descriptorCount = 1;
			writeDescriptors[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptors[2].pImageInfo = &m_Bird->Texture->descriptor;

			vkUpdateDescriptorSets(m_Core->GetDevice(), writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
		}

		//Descriptor Set for Background
		{
			VkDescriptorSetAllocateInfo descriptorSetAI = init::descriptorSetAllocateInfo();
			descriptorSetAI.descriptorPool = m_DescriptorPool;
			descriptorSetAI.descriptorSetCount = 1;
			descriptorSetAI.pSetLayouts = &layout.BackgroundDescriptor;
			VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &descriptorSetAI, &m_Background->DescriptorSet));

			std::array<VkWriteDescriptorSet, 2> writeDescriptors{};
			writeDescriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptors[0].dstSet = m_Background->DescriptorSet;
			writeDescriptors[0].dstBinding = 0;
			writeDescriptors[0].descriptorCount = 1;
			writeDescriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptors[0].pBufferInfo = &m_Background->ModelBuffer->GetBufferInfo();

			writeDescriptors[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptors[1].dstSet = m_Background->DescriptorSet;
			writeDescriptors[1].dstBinding = 1;
			writeDescriptors[1].descriptorCount = 1;
			writeDescriptors[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptors[1].pImageInfo = &m_Background->Texture->descriptor;

			vkUpdateDescriptorSets(m_Core->GetDevice(), writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
		}
	}

	void TestFlappyBird::preparePipeline()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = init::pipelineLayout();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &layout.ObjectsDescriptor;

		VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCreateInfo, nullptr, &layout.ObjectsPipeline));

		pipelineLayoutCreateInfo.pSetLayouts = &layout.BackgroundDescriptor;

		VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCreateInfo, nullptr, &layout.BackgroundPipeline));

		VkPipelineVertexInputStateCreateInfo vertexInputState = init::pipelineVertexInputState();
		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = &m_PipeObjects[0]->VertexBuffer->GetVertexInput();
		vertexInputState.vertexAttributeDescriptionCount = m_PipeObjects[0]->VertexBuffer->GetVertexAttributes().size();
		vertexInputState.pVertexAttributeDescriptions = m_PipeObjects[0]->VertexBuffer->GetVertexAttributes().data();

		VkPipelineShaderStageCreateInfo shaderStages[2];
		shaderStages[0] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/flappyBird/pipeObjectVertex.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/flappyBird/pipeObjectFragment.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

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
		pipelineCreateInfo.layout = layout.ObjectsPipeline;
		pipelineCreateInfo.renderPass = m_Core->resources.renderPass;
		pipelineCreateInfo.subpass = 0;

		VK_CHECK(vkCreateGraphicsPipelines(m_Core->GetDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipelines.PipeObject));

		vkDestroyShaderModule(m_Core->GetDevice(), shaderStages[0].module, nullptr);
		vkDestroyShaderModule(m_Core->GetDevice(), shaderStages[1].module, nullptr);

		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = &m_Bird->VertexBuffer->GetVertexInput();
		vertexInputState.vertexAttributeDescriptionCount = m_Bird->VertexBuffer->GetVertexAttributes().size();
		vertexInputState.pVertexAttributeDescriptions = m_Bird->VertexBuffer->GetVertexAttributes().data();

		shaderStages[0] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/flappyBird/birdObjectVertex.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/flappyBird/birdObjectFragment.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		VK_CHECK(vkCreateGraphicsPipelines(m_Core->GetDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipelines.BirdObject));

		vkDestroyShaderModule(m_Core->GetDevice(), shaderStages[0].module, nullptr);
		vkDestroyShaderModule(m_Core->GetDevice(), shaderStages[1].module, nullptr);

		pipelineCreateInfo.layout = layout.BackgroundPipeline;

		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = &m_Background->VertexBuffer->GetVertexInput();
		vertexInputState.vertexAttributeDescriptionCount = m_Background->VertexBuffer->GetVertexAttributes().size();
		vertexInputState.pVertexAttributeDescriptions = m_Background->VertexBuffer->GetVertexAttributes().data();

		shaderStages[0] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/flappyBird/backgroundVertex.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/flappyBird/backgroundFragment.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		VK_CHECK(vkCreateGraphicsPipelines(m_Core->GetDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipelines.Background));

		vkDestroyShaderModule(m_Core->GetDevice(), shaderStages[0].module, nullptr);
		vkDestroyShaderModule(m_Core->GetDevice(), shaderStages[1].module, nullptr);
	}

	void TestFlappyBird::setCmdBuffers()
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

			vkCmdBindPipeline(m_Core->resources.drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.PipeObject);

			for(int pipeIndex = 0; pipeIndex < m_PipeObjects.size(); pipeIndex++)
				m_PipeObjects[pipeIndex]->draw(m_Core->resources.drawCmdBuffers[i], layout.ObjectsPipeline);

			vkCmdBindPipeline(m_Core->resources.drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.BirdObject);

			m_Bird->draw(m_Core->resources.drawCmdBuffers[i], layout.ObjectsPipeline);

			vkCmdEndRenderPass(m_Core->resources.drawCmdBuffers[i]);

			VK_CHECK(vkEndCommandBuffer(m_Core->resources.drawCmdBuffers[i]));
		}

	}
	void TestFlappyBird::updateUniformBuffers()
	{

	}

	void TestFlappyBird::windowResized()
	{
		m_Core->windowResized();

		vkDestroyPipeline(m_Core->GetDevice(), pipelines.Background, nullptr);
		vkDestroyPipeline(m_Core->GetDevice(), pipelines.BirdObject, nullptr);
		vkDestroyPipeline(m_Core->GetDevice(), pipelines.PipeObject, nullptr);
		vkDestroyPipelineLayout(m_Core->GetDevice(), layout.ObjectsPipeline, nullptr);
		vkDestroyPipelineLayout(m_Core->GetDevice(), layout.BackgroundPipeline, nullptr);

		float right = m_Core->swapchain.extent.width / 200.0f;
		float top = m_Core->swapchain.extent.height / 200.0f;
		dynamic_cast<OrthographicCamera*>(m_Camera)->SetOrthograhic(-right, right, -top, top);

		preparePipeline();
		setCmdBuffers();
	}
}