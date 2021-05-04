#include "pch.h"
#include "TestGraphicsPipeline.h"

namespace test
{
	TestGraphicsPipeline::TestGraphicsPipeline(VulkanCore* core)
	{
		Test::Init(core);
		glfwSetWindowTitle(static_cast<GLFWwindow*>(Window::GetWindow()), "TestGraphicsPipeline");

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

		m_VertexBuffer = new VulkanVertexBuffer(vertices, sizeof(vertices), m_Core);
		m_VertexBuffer->SetLayout(layout);

		uint16_t indices[] =
		{ 0, 1, 2,
		  2, 3, 0};

		m_IndexBuffer.reset(new VulkanIndexBuffer(indices, 6, m_Core));

		obj.u_Scene.Model = glm::mat4(1.0f);
		obj.u_Scene.View = glm::mat4(1.0f);
		obj.u_Scene.Projection = glm::mat4(1.0f);

		m_UniformBuffer = new VulkanUniformBuffer(sizeof(Object::Transform), m_Core);
		m_UniformBuffer->copyData(&obj.u_Scene, sizeof(Object::Transform));

		m_ViewProjBuffer = new VulkanUniformBuffer(sizeof(glm::mat4), m_Core);
		glm::mat4 a = glm::mat4(1.0f);
		m_ViewProjBuffer->copyData(&a, sizeof(glm::mat4));

		for (int i = 0; i < objs.size(); i++)
		{
			objs[i] = new QuadObj(m_Core);
			objs[i]->SetPosition(i);
		}
		prepareDescriptorPool();
		preparePipeline();
		setCmdBuffers();
	}

	TestGraphicsPipeline::~TestGraphicsPipeline()
	{
		vkDestroyDescriptorSetLayout(m_Core->GetDevice(), m_DescriptorSetLayout, nullptr);
		vkDestroyDescriptorPool(m_Core->GetDevice(), m_DescriptorPool, nullptr);
		vkDestroyPipeline(m_Core->GetDevice(), m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_Core->GetDevice(), m_PipelineLayout, nullptr);

		delete m_VertexBuffer;
		delete m_UniformBuffer;
	}

	void TestGraphicsPipeline::OnUpdate(float deltaTime)
	{
		updateUniformBuffers(deltaTime);

		objs[0]->OnUpdate(deltaTime);
		objs[1]->OnUpdate(deltaTime);
		objs[2]->OnUpdate(deltaTime);
		objs[3]->OnUpdate(deltaTime);
		objs[4]->OnUpdate(deltaTime);
	}

	void TestGraphicsPipeline::OnRender()
	{
		m_Core->BeginScene();

		m_Core->resources.submitInfo.commandBufferCount = 1;
		m_Core->resources.submitInfo.pCommandBuffers = &m_Core->resources.drawCmdBuffers[m_Core->resources.imageIndex];

		VK_CHECK(vkQueueSubmit(m_Core->queue.GraphicsQueue, 1, &m_Core->resources.submitInfo, VK_NULL_HANDLE));

		VkResult err = m_Core->Submit();

		if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
			windowResized();
		else
			VK_CHECK(err);

		vkDeviceWaitIdle(m_Core->GetDevice());
	}

	void TestGraphicsPipeline::OnImGuiRender()
	{
	}

	void TestGraphicsPipeline::prepareDescriptorPool()
	{
		VkDescriptorSetLayoutBinding layoutBinding[2];

		layoutBinding[0].binding = 0;
		layoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBinding[0].descriptorCount = 1;	
		
		layoutBinding[1].binding = 1;
		layoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBinding[1].descriptorCount = 1;

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = init::descriptorSetLayoutCreateInfo();
		descriptorSetLayoutCI.bindingCount = 2;
		descriptorSetLayoutCI.pBindings = layoutBinding;

		VK_CHECK(vkCreateDescriptorSetLayout(m_Core->GetDevice(), &descriptorSetLayoutCI, nullptr, &m_DescriptorSetLayout));

		VkDescriptorPoolSize poolsize{};
		poolsize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolsize.descriptorCount = 100;

		VkDescriptorPoolCreateInfo descriptorPoolCI = init::descriptorPoolCreateInfo();
		descriptorPoolCI.poolSizeCount = 1;
		descriptorPoolCI.pPoolSizes = &poolsize;
		descriptorPoolCI.maxSets = 100;

		VK_CHECK(vkCreateDescriptorPool(m_Core->GetDevice(), &descriptorPoolCI, nullptr, &m_DescriptorPool));

		for (int i = 0; i < objs.size(); i++)
		{

			VkDescriptorSetAllocateInfo descriptorSetAI = init::descriptorSetAllocateInfo();
			descriptorSetAI.descriptorPool = m_DescriptorPool;
			descriptorSetAI.descriptorSetCount = 1;
			descriptorSetAI.pSetLayouts = &m_DescriptorSetLayout;
			VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &descriptorSetAI, &objs[i]->DescriptorSet));

			std::array<VkWriteDescriptorSet, 2> writeDescriptors{};
			writeDescriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptors[0].dstSet = objs[i]->DescriptorSet;
			writeDescriptors[0].dstBinding = 0;
			writeDescriptors[0].descriptorCount = 1;
			writeDescriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptors[0].pBufferInfo = &objs[i]->ModelBuffer->GetBufferInfo();

			writeDescriptors[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptors[1].dstSet = objs[i]->DescriptorSet;
			writeDescriptors[1].dstBinding = 1;
			writeDescriptors[1].descriptorCount = 1;
			writeDescriptors[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptors[1].pBufferInfo = &m_ViewProjBuffer->GetBufferInfo();

			vkUpdateDescriptorSets(m_Core->GetDevice(), writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
		}
	}

	void TestGraphicsPipeline::preparePipeline()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = init::pipelineLayout();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout;

		VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout));

		VkPipelineVertexInputStateCreateInfo vertexInputState = init::pipelineVertexInputState();
		vertexInputState.vertexBindingDescriptionCount   = 1;
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
		rasterizationStateInfo.cullMode = VK_CULL_MODE_NONE;
		rasterizationStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;;
		rasterizationStateInfo.depthBiasEnable = VK_FALSE;
		rasterizationStateInfo.lineWidth = 1.0f;

		VkPipelineColorBlendAttachmentState blendAttachment{};
		blendAttachment.colorWriteMask = 0xF;
			//VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo blendState = init::pipelineColorBlendState();
		blendState.logicOpEnable = VK_FALSE;
		blendState.attachmentCount = 1;
		blendState.pAttachments = &blendAttachment;

		VkPipelineDepthStencilStateCreateInfo depthStencilState = init::pipelineDepthStencilState();
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.depthWriteEnable = VK_FALSE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilState.stencilTestEnable = VK_FALSE;

		VkViewport viewPort{0, 0, m_Core->swapchain.extent.width, m_Core->swapchain.extent.height, 0.0f, 1.0f};

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
	void TestGraphicsPipeline::setCmdBuffers()
	{
		VkCommandBufferBeginInfo cmdBufferBI = init::cmdBufferBeginInfo();

		VkClearValue clearValues[2];
		clearValues[0].color = { 0.0f, 1.0f, 0.0f, 1.0f };
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

			vkCmdBindPipeline(m_Core->resources.drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

			/*VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(m_Core->resources.drawCmdBuffers[i], 0, 1, &m_VertexBuffer->GetBuffer(), offsets);
			vkCmdBindIndexBuffer(m_Core->resources.drawCmdBuffers[i], m_IndexBuffer->GetBuffer(), 0, m_IndexBuffer->GetIndexType());
			vkCmdBindDescriptorSets(m_Core->resources.drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &obj.DescriptorSet, 0, nullptr);

			vkCmdDrawIndexed(m_Core->resources.drawCmdBuffers[i], m_IndexBuffer->GetCount(), 1, 0, 0, 0);*/
	

			objs[0]->draw(m_Core->resources.drawCmdBuffers[i], m_PipelineLayout);
			objs[1]->draw(m_Core->resources.drawCmdBuffers[i], m_PipelineLayout);
			objs[2]->draw(m_Core->resources.drawCmdBuffers[i], m_PipelineLayout);
			objs[3]->draw(m_Core->resources.drawCmdBuffers[i], m_PipelineLayout);
			objs[4]->draw(m_Core->resources.drawCmdBuffers[i], m_PipelineLayout);

			//quad1->draw(m_Core->resources.drawCmdBuffers[i], m_PipelineLayout);

			vkCmdEndRenderPass(m_Core->resources.drawCmdBuffers[i]);

			VK_CHECK(vkEndCommandBuffer(m_Core->resources.drawCmdBuffers[i]));
		}

	}
	void TestGraphicsPipeline::updateUniformBuffers(float deltaTime)
	{
		glm::mat4 a = glm::mat4(1.0f);
		m_ViewProjBuffer->copyData(&a, sizeof(glm::mat4));

		//m_ViewProjBuffer->copyData(&data, sizeof(glm::mat4));
	}

	void TestGraphicsPipeline::windowResized()
	{
		m_Core->windowResized();

		vkDestroyPipeline(m_Core->GetDevice(), m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_Core->GetDevice(), m_PipelineLayout, nullptr);

		preparePipeline();
		setCmdBuffers();
	}
}