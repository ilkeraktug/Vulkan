#include "pch.h"
#include "TestShadowMapping.h"

#define DEPTH_FORMAT VK_FORMAT_D16_UNORM
#define SHADOW_WIDTH  1024
#define SHADOW_HEIGHT 1024

namespace test {

	TestShadowMapping::TestShadowMapping(VulkanCore* core)
	{
		Test::Init(core);
		m_Camera = new PerspectiveCamera(m_Core->swapchain.extent.width, m_Core->swapchain.extent.height, core);
		m_Cube.reset(new CubeObj(core));

		float lightVertices[] = 
		{
			//   Vertex Positions
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.5f,  0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f
		};

		VertexBufferLayout layout = { {"a_Position", ShaderFormat::Float3} };
	
		m_LightVertexBuffer.reset(new VulkanVertexBuffer(lightVertices, sizeof(lightVertices) / sizeof(float), layout, core));
		
		float sceneVertices[] = 
		{
			//   Vertex Positions
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.5f,  0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f
		};

		VertexBufferLayout sceneLayout = { {"a_Position", ShaderFormat::Float3}, {"a_Normal", ShaderFormat::Float3}, {"a_TexCoords", ShaderFormat::Float2} };

		m_SceneVertexBuffer.reset(new VulkanVertexBuffer(sceneVertices, sizeof(sceneVertices) / sizeof(float), sceneLayout, core));

		uint16_t indices[] =
		{
			0, 1, 2,
			2, 3, 0
		};

		m_SceneIndexBuffer.reset(new VulkanIndexBuffer(indices, 6, m_Core));		
		
		/*uint16_t indices[] =
		{
			0, 1, 2,
			2, 3, 0
		};*/

		m_LightIndexBuffer.reset(new VulkanIndexBuffer(indices, 6, m_Core));

		prepareFramebufferAttachment();
		prepareDescriptorPool();
		prepareDescriptorSetLayout();
		prepareUniformBuffers();
		prepareDescriptorSets();
		preparePipelines();
		buildCommandBuffers();

	}

	TestShadowMapping::~TestShadowMapping()
	{
	}

	void TestShadowMapping::OnUpdate(float deltaTime)
	{
	}

	void TestShadowMapping::OnRender()
	{
	}

	void TestShadowMapping::OnImGuiRender()
	{
	}

	void TestShadowMapping::prepareLightRenderPass()
	{

		VkAttachmentDescription attachmentDescription{};
		attachmentDescription.format = DEPTH_FORMAT;
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkAttachmentReference depthAttachment{};
		depthAttachment.attachment = 0;
		depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 0;
		subpassDescription.pDepthStencilAttachment = &depthAttachment;

		std::array<VkSubpassDependency, 2> subpassDependecies;
		subpassDependecies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependecies[0].dstSubpass = 0;
		subpassDependecies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		subpassDependecies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		subpassDependecies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		subpassDependecies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		subpassDependecies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		subpassDependecies[1].srcSubpass = 0;
		subpassDependecies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependecies[1].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		subpassDependecies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		subpassDependecies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		subpassDependecies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		subpassDependecies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassCI = init::renderPassCreateInfo();
		renderPassCI.attachmentCount = 1;
		renderPassCI.pAttachments = &attachmentDescription;
		renderPassCI.subpassCount = 1;
		renderPassCI.pSubpasses = &subpassDescription;
		renderPassCI.dependencyCount = static_cast<uint32_t>(subpassDependecies.size());
		renderPassCI.pDependencies = subpassDependecies.data();

		VK_CHECK(vkCreateRenderPass(m_Core->GetDevice(), &renderPassCI, nullptr, &lightPassView.RenderPass));
	}

	void TestShadowMapping::prepareFramebufferAttachment()
	{
		lightPassView.Width = SHADOW_WIDTH;
		lightPassView.Height = SHADOW_HEIGHT;

		VkImageCreateInfo framebufferImageCI = init::imageCreateInfo();
		framebufferImageCI.extent.width = lightPassView.Width;
		framebufferImageCI.extent.height = lightPassView.Height;
		framebufferImageCI.extent.depth = 1;
		framebufferImageCI.imageType = VK_IMAGE_TYPE_2D;
		framebufferImageCI.format = DEPTH_FORMAT;
		framebufferImageCI.mipLevels = 1;
		framebufferImageCI.arrayLayers = 1;
		framebufferImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		framebufferImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		framebufferImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		framebufferImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		framebufferImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VK_CHECK(vkCreateImage(m_Core->GetDevice(), &framebufferImageCI, nullptr, &lightPassView.FrameBufferAttachment.Image));

		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(m_Core->GetDevice(), lightPassView.FrameBufferAttachment.Image, &memReq);

		VkMemoryAllocateInfo memAllocInfo = init::memAllocInfo();
		memAllocInfo.allocationSize = memReq.size;
		memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &memAllocInfo, nullptr, &lightPassView.FrameBufferAttachment.Memory));
		VK_CHECK(vkBindImageMemory(m_Core->GetDevice(), lightPassView.FrameBufferAttachment.Image, lightPassView.FrameBufferAttachment.Memory, 0));

		VkImageViewCreateInfo framebufferImageViewCI = init::imageViewCreateInfo();
		framebufferImageViewCI.image = lightPassView.FrameBufferAttachment.Image;
		framebufferImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		framebufferImageViewCI.format = DEPTH_FORMAT;
		framebufferImageViewCI.components = {};
		framebufferImageViewCI.subresourceRange = {};
		framebufferImageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		framebufferImageViewCI.subresourceRange.baseMipLevel = 0;
		framebufferImageViewCI.subresourceRange.levelCount = 1;
		framebufferImageViewCI.subresourceRange.baseArrayLayer = 0;
		framebufferImageViewCI.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(m_Core->GetDevice(), &framebufferImageViewCI, nullptr, &lightPassView.FrameBufferAttachment.View));

		VkSamplerCreateInfo framebufferSamplerCI = init::samplerCreateInfo();
		framebufferSamplerCI.magFilter = VK_FILTER_LINEAR;
		framebufferSamplerCI.minFilter = VK_FILTER_LINEAR;
		framebufferSamplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		framebufferSamplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		framebufferSamplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		framebufferSamplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		framebufferSamplerCI.mipLodBias = 0.0f;
		framebufferSamplerCI.anisotropyEnable = VK_FALSE;
		framebufferSamplerCI.maxAnisotropy = 1.0f;
		framebufferSamplerCI.compareEnable = VK_FALSE;
		framebufferSamplerCI.compareOp;
		framebufferSamplerCI.minLod = 0.0f;
		framebufferSamplerCI.maxLod = 1.0f;
		framebufferSamplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE; 
		framebufferSamplerCI.unnormalizedCoordinates = VK_FALSE;

		VK_CHECK(vkCreateSampler(m_Core->GetDevice(), &framebufferSamplerCI, nullptr, &lightPassView.Sampler));


		prepareLightRenderPass();

		VkFramebufferCreateInfo framebufferCI{};
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.renderPass = lightPassView.RenderPass;
		framebufferCI.attachmentCount = 1;
		framebufferCI.pAttachments = &lightPassView.FrameBufferAttachment.View;
		framebufferCI.width = lightPassView.Width;
		framebufferCI.height = lightPassView.Height;
		framebufferCI.layers = 1;

		VK_CHECK(vkCreateFramebuffer(m_Core->GetDevice(), &framebufferCI, nullptr, &lightPassView.Framebuffer));
	}

	void TestShadowMapping::buildCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBufferBI = init::cmdBufferBeginInfo();

		VkClearValue clearValues[2];
		VkViewport viewport{};
		VkRect2D scissor{};

		for (int32_t i = 0; i < m_Core->resources.drawCmdBuffers.size(); i++)
		{

			VK_CHECK(vkBeginCommandBuffer(m_Core->resources.drawCmdBuffers[i], &cmdBufferBI));

			{
				clearValues[0].depthStencil = { 1.0f, 0 };

				VkRenderPassBeginInfo renderPassBI = init::renderPassBeginInfo();
				renderPassBI.renderPass = lightPassView.RenderPass;
				renderPassBI.framebuffer = lightPassView.Framebuffer;
				renderPassBI.renderArea.extent.width = lightPassView.Width;
				renderPassBI.renderArea.extent.height = lightPassView.Height;
				renderPassBI.clearValueCount = 1;
				renderPassBI.pClearValues = clearValues;

				vkCmdBeginRenderPass(m_Core->resources.drawCmdBuffers[i], &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

				viewport.width = static_cast<float>(lightPassView.Width);
				viewport.height = static_cast<float>(lightPassView.Height);
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(m_Core->resources.drawCmdBuffers[i], 0, 1, &viewport);

				scissor.extent.width = lightPassView.Width;
				scissor.extent.height = lightPassView.Height;
				vkCmdSetScissor(m_Core->resources.drawCmdBuffers[i], 0, 1, &scissor);

				vkCmdBindPipeline(m_Core->resources.drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.Light);

				VkDeviceSize offsets[] = { 0 };
				m_Cube->draw(m_Core->resources.drawCmdBuffers[i], m_PipelineLayout, i);

				vkCmdEndRenderPass(m_Core->resources.drawCmdBuffers[i]);
			}
			{
				clearValues[0].color = { 1.0f, 0.5f, 0.31f, 1.0f };
				clearValues[1].depthStencil = { 1.0f, 0 };

				VkRenderPassBeginInfo renderPassBI = init::renderPassBeginInfo();
				renderPassBI.renderPass = m_Core->resources.renderPass;
				renderPassBI.framebuffer = m_Core->resources.frameBuffers[i];
				renderPassBI.renderArea.extent = m_Core->swapchain.extent;
				renderPassBI.renderArea.offset = { 0, 0 };
				renderPassBI.clearValueCount = 2;
				renderPassBI.pClearValues = clearValues;

				vkCmdBeginRenderPass(m_Core->resources.drawCmdBuffers[i], &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

				viewport.width = static_cast<float>(m_Core->swapchain.extent.width);
				viewport.height = static_cast<float>(m_Core->swapchain.extent.height);
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(m_Core->resources.drawCmdBuffers[i], 0, 1, &viewport);

				scissor.extent.width = m_Core->swapchain.extent.width;
				scissor.extent.height = m_Core->swapchain.extent.height;
				vkCmdSetScissor(m_Core->resources.drawCmdBuffers[i], 0, 1, &scissor);

				vkCmdBindPipeline(m_Core->resources.drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.Scene);

				VkDeviceSize offsets[] = { 0 };
				m_Cube->draw(m_Core->resources.drawCmdBuffers[i], m_PipelineLayout, i);

				vkCmdEndRenderPass(m_Core->resources.drawCmdBuffers[i]);
			}
			VK_CHECK(vkEndCommandBuffer(m_Core->resources.drawCmdBuffers[i]));
		}
	}

	void TestShadowMapping::prepareDescriptorPool()
	{
		std::vector<VkDescriptorPoolSize> descriptorPoolSize =
		{
			init::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3),
			init::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3)
		};

		VkDescriptorPoolCreateInfo descriptorPoolCI = init::descriptorPoolCreateInfo();
		descriptorPoolCI.poolSizeCount = descriptorPoolSize.size();
		descriptorPoolCI.pPoolSizes = descriptorPoolSize.data();
		descriptorPoolCI.maxSets = 6;

		VK_CHECK(vkCreateDescriptorPool(m_Core->GetDevice(), &descriptorPoolCI, nullptr, &m_DescriptorPool));
	}

	void TestShadowMapping::prepareDescriptorSetLayout()
	{

		std::vector<VkDescriptorSetLayoutBinding> layoutBindings
		{
			init::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
			init::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
		};

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = init::descriptorSetLayoutCreateInfo();
		descriptorSetLayoutCI.bindingCount = layoutBindings.size();
		descriptorSetLayoutCI.pBindings = layoutBindings.data();

		VK_CHECK(vkCreateDescriptorSetLayout(m_Core->GetDevice(), &descriptorSetLayoutCI, nullptr, &m_DescriptorSetLayout));

		VkPipelineLayoutCreateInfo pipelineLayoutCI = init::pipelineLayoutCreateInfo();
		pipelineLayoutCI.setLayoutCount = 1;
		pipelineLayoutCI.pSetLayouts = &m_DescriptorSetLayout;

		VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCI, nullptr, &m_PipelineLayout));
	}

	void TestShadowMapping::prepareDescriptorSets()
	{
		std::vector<VkWriteDescriptorSet> writeDescriptorSets;

		VkDescriptorImageInfo shadowMapDescriptor{};
		shadowMapDescriptor.sampler = lightPassView.Sampler;
		shadowMapDescriptor.imageView = lightPassView.FrameBufferAttachment.View;
		shadowMapDescriptor.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkDescriptorSetAllocateInfo sceneDescriptorSet = init::descriptorSetAllocateInfo();
		sceneDescriptorSet.descriptorPool = m_DescriptorPool;
		sceneDescriptorSet.descriptorSetCount = 1;
		sceneDescriptorSet.pSetLayouts = &m_DescriptorSetLayout;

		VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &sceneDescriptorSet, &descriptorSets.Light));

		VkWriteDescriptorSet updateDescriptorSet{};
		VkWriteDescriptorSet updateDescriptorSet1{};
		
		updateDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		updateDescriptorSet.dstSet = descriptorSets.Light;
		updateDescriptorSet.dstBinding = 0;
		updateDescriptorSet.descriptorCount = 1;
		updateDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		updateDescriptorSet.pBufferInfo = &uniformBuffer.Light->GetBufferInfo();

		writeDescriptorSets = {updateDescriptorSet};
		
		vkUpdateDescriptorSets(m_Core->GetDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);

		VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &sceneDescriptorSet, &descriptorSets.Scene));

		updateDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		updateDescriptorSet.dstSet = descriptorSets.Scene;
		updateDescriptorSet.dstBinding = 0;
		updateDescriptorSet.descriptorCount = 1;
		updateDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		updateDescriptorSet.pBufferInfo = &uniformBuffer.Scene->GetBufferInfo();

		updateDescriptorSet1.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		updateDescriptorSet1.dstSet = descriptorSets.Scene;
		updateDescriptorSet1.dstBinding = 1;
		updateDescriptorSet1.descriptorCount = 1;
		updateDescriptorSet1.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		updateDescriptorSet1.pImageInfo = &shadowMapDescriptor;

		writeDescriptorSets =
		{
			updateDescriptorSet,
			updateDescriptorSet1
		};

		vkUpdateDescriptorSets(m_Core->GetDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}

	void TestShadowMapping::preparePipelines()
	{
		VkPipelineShaderStageCreateInfo shaderStageInfos[2];
		shaderStageInfos[0] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/shadowmapping/light_shader.vspv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStageInfos[1] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/shadowmapping/light_shader.fspv", VK_SHADER_STAGE_FRAGMENT_BIT);

		VkPipelineVertexInputStateCreateInfo vertexInputState = init::pipelineVertexInputState();
		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = &m_LightVertexBuffer->GetVertexInput();
		vertexInputState.vertexAttributeDescriptionCount = m_LightVertexBuffer->GetVertexAttributes().size();
		vertexInputState.pVertexAttributeDescriptions = m_LightVertexBuffer->GetVertexAttributes().data();

		VkPipelineInputAssemblyStateCreateInfo inputStateCI = init::pipelineInputAssemblyState();
		inputStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineViewportStateCreateInfo viewportStateCI = init::pipelineViewportState();
		viewportStateCI.scissorCount = 1;
		viewportStateCI.viewportCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizationStateCI = init::pipelineRasterizationState();
		rasterizationStateCI.depthClampEnable = VK_FALSE;
		rasterizationStateCI.rasterizerDiscardEnable = VK_FALSE;
		rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationStateCI.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationStateCI.depthBiasEnable = VK_FALSE;
		rasterizationStateCI.depthBiasConstantFactor;
		rasterizationStateCI.depthBiasClamp;
		rasterizationStateCI.depthBiasSlopeFactor;
		rasterizationStateCI.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampleStateCI = init::multiSampleState();
		multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = init::pipelineDepthStencilState();
		depthStencilStateCI.depthTestEnable = VK_TRUE;
		depthStencilStateCI.depthWriteEnable = VK_TRUE;
		depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilStateCI.depthBoundsTestEnable;
		depthStencilStateCI.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.blendEnable = VK_FALSE;
		blendAttachmentState.colorWriteMask = 0xF;

		VkPipelineColorBlendStateCreateInfo colorBlendStateCI = init::pipelineColorBlendState();
		colorBlendStateCI.attachmentCount = 0;

		std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicStateCI = init::dynamicState();
		dynamicStateCI.dynamicStateCount = dynamicStates.size();
		dynamicStateCI.pDynamicStates = dynamicStates.data();

		VkGraphicsPipelineCreateInfo pipelineCI = init::graphicsPipelineCreateInfo();
		pipelineCI.stageCount = 2;
		pipelineCI.pStages = shaderStageInfos;
		pipelineCI.pVertexInputState = &vertexInputState;
		pipelineCI.pInputAssemblyState = &inputStateCI;
		pipelineCI.pViewportState = &viewportStateCI;
		pipelineCI.pRasterizationState = &rasterizationStateCI;
		pipelineCI.pMultisampleState = &multisampleStateCI;
		pipelineCI.pDepthStencilState = &depthStencilStateCI;
		pipelineCI.pColorBlendState = &colorBlendStateCI;
		pipelineCI.pDynamicState = &dynamicStateCI;
		pipelineCI.layout = m_PipelineLayout;
		pipelineCI.renderPass = lightPassView.RenderPass;

		VK_CHECK(vkCreateGraphicsPipelines(m_Core->GetDevice(), 0, 1, &pipelineCI, nullptr, &pipelines.Light));

		shaderStageInfos[0] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/shadowmapping/scene_shader.vspv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStageInfos[1] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/shadowmapping/scene_shader.fspv", VK_SHADER_STAGE_FRAGMENT_BIT);

		colorBlendStateCI.attachmentCount = 1;
		colorBlendStateCI.pAttachments = &blendAttachmentState;

		pipelineCI.renderPass = m_Core->resources.renderPass;

		VK_CHECK(vkCreateGraphicsPipelines(m_Core->GetDevice(), 0, 1, &pipelineCI, nullptr, &pipelines.Scene));
	}

	void TestShadowMapping::prepareUniformBuffers()
	{
		uniformBuffer.Scene.reset(new VulkanUniformBuffer(sizeof(sceneUBO), m_Core));
		uniformBuffer.Light.reset(new VulkanUniformBuffer(sizeof(lightUBO), m_Core));

		updateLight();
		updateUniformBuffers();
		updateUniformBuffersLight();
	}

	void TestShadowMapping::updateLight()
	{
		// Animate the light source
		lightPos.x = cos(glm::radians(Time::deltaTime * 360.0f)) * 40.0f;
		lightPos.y = -50.0f + sin(glm::radians(Time::deltaTime * 360.0f)) * 20.0f;
		lightPos.z = 25.0f + sin(glm::radians(Time::deltaTime * 360.0f)) * 5.0f;
	}

	void TestShadowMapping::updateUniformBuffers()
	{
		sceneUBO.projection = m_Camera->GetProjectionMatrix();
		sceneUBO.view = m_Camera->GetViewMatrix();
		sceneUBO.model = glm::mat4(1.0f);
		sceneUBO.lightPos = lightPos;
		sceneUBO.depthBiasMVP = lightUBO.depthMVP;
		uniformBuffer.Scene->copyData(&sceneUBO, sizeof(sceneUBO));
	}

	void TestShadowMapping::updateUniformBuffersLight()
	{
		// Matrix from light's point of view
		glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, 100.0f);
		glm::mat4 depthViewMatrix = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));
		glm::mat4 depthModelMatrix = glm::mat4(1.0f);

		lightUBO.depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

		//UNIFORM_BUFFER_COPY_DATA(uniformBuffer.Light, lightUBO);
		uniformBuffer.Light->copyData(&lightUBO, sizeof(lightUBO));
	}

	void TestShadowMapping::windowResized()
	{
	}

}
