#include "pch.h"
#include "TestShadow.h"

#include "Vulkan/Core/LogHelper.h"
#include "Vulkan/Core/Time.h"
#include "Vulkan/Objects/CubeObj.h"
#include "Vulkan/Renderer/PerspectiveCamera.h"
#include "Vulkan/Renderer/VulkanShader.h"

namespace test
{

	double TestShadow::cameraSpeed = 1.0f;
	
	TestShadow::TestShadow(VulkanCore* core)
	{
		Test::Init(core);
    
		m_Camera = new PerspectiveCamera(m_Core->swapchain.extent.width, m_Core->swapchain.extent.height, core);
		m_Cube.reset(new CubeObj(core));

		float vertices[] = {
			//Positon        //TexCoords	//Color						//Normal             
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,  1.0f,  
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,  1.0f,  
		 1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,  1.0f,  
		 1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,  1.0f,  
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,  1.0f,  
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,  1.0f,  
 
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,  0.0f,  0.0f,  1.0f,  
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  1.0f, 1.0f, 0.0f,  0.0f,  0.0f,  1.0f,  
		 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 0.0f,  0.0f,  0.0f,  1.0f,  
		 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 0.0f,  0.0f,  0.0f,  1.0f,  
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  1.0f, 1.0f, 0.0f,  0.0f,  0.0f,  1.0f,  
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,  0.0f,  0.0f,  1.0f,  
 
		-1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,  
		-1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,  
		-1.0f, -1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,  
		-1.0f, -1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,  
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,  
		-1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,  

		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  1.0f,  0.0f,  0.0f,  
		 1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  0.0f, 0.0f, 1.0f,  1.0f,  0.0f,  0.0f,  
		 1.0f, -1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,  1.0f,  0.0f,  0.0f,  
		 1.0f, -1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,  1.0f,  0.0f,  0.0f,  
		 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,  1.0f,  0.0f,  0.0f,  
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  1.0f,  0.0f,  0.0f,  

		-1.0f, -1.0f, -1.0f,  0.0f,  1.0f,  1.0f, 0.0f, 0.0f,  0.0f,  1.0f,  0.0f,  
		 1.0f, -1.0f, -1.0f,  1.0f,  1.0f,  1.0f, 0.0f, 0.0f,  0.0f,  1.0f,  0.0f,  
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  1.0f, 0.0f, 0.0f,  0.0f,  1.0f,  0.0f,  
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  1.0f, 0.0f, 0.0f,  0.0f,  1.0f,  0.0f,  
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,  0.0f,  1.0f,  0.0f,  
		-1.0f, -1.0f, -1.0f,  0.0f,  1.0f,  1.0f, 0.0f, 0.0f,  0.0f,  1.0f,  0.0f,  

		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  
		 1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  
		};

		float Offscreenvertices[] = {
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
 
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
 
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,

		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,

		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f
		};
		
		VertexBufferLayout layout = { {"a_Position", ShaderFormat::Float3}, {"a_TexCoords", ShaderFormat::Float2}, {"a_Color", ShaderFormat::Float3}, {"a_Normal", ShaderFormat::Float3} };
		VertexBufferLayout Offscreenlayout = { {"a_Position", ShaderFormat::Float3}};

		VertexBuffer.reset(new VulkanVertexBuffer(vertices, sizeof(vertices), core));
		VertexBuffer->SetLayout(layout);

		OffscreenVertexBuffer.reset(new VulkanVertexBuffer(Offscreenvertices, sizeof(Offscreenvertices), core));
		OffscreenVertexBuffer->SetLayout(Offscreenlayout);
		runBatchFile();
    
		prepareOffscreenFramebuffer();
		prepareUniformBuffers();
		setupDescriptorSetLayout();
		preparePipelines();
		setupDescriptorPool();
		setupDescriptorSets();
		buildCommandBuffers();
		initSuccess = true;

		glfwSetScrollCallback(static_cast<GLFWwindow*>(Window::GetWindow()), &TestShadow::onMouseScrollMoved);
	}

	TestShadow::~TestShadow()
	{
	}

	void TestShadow::OnUpdate(float deltaTime)
	{

		if ((glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
			glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS))
		{
			keys.shift = true;
		}
		if ((glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE ||
				glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_RIGHT_SHIFT) == GLFW_RELEASE))
		{
			keys.shift = false;
		}
		
		if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_W) == GLFW_PRESS)
		{
			glm::vec3 cameraCurrentPosition = m_Camera->getPosition();
			m_Camera->setPosition(cameraCurrentPosition + m_Camera->getForwardVector() * deltaTime * (float)cameraSpeed);
		}
		else if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_S) == GLFW_PRESS)
		{
			glm::vec3 cameraCurrentPosition = m_Camera->getPosition();
			m_Camera->setPosition(cameraCurrentPosition + -m_Camera->getForwardVector() * deltaTime * (float)cameraSpeed);
		}
		if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_A) == GLFW_PRESS)
		{
			glm::vec3 cameraCurrentPosition = m_Camera->getPosition();
			m_Camera->setPosition(cameraCurrentPosition + -m_Camera->getRightVector() * deltaTime * (float)cameraSpeed);
		}
		else if (glfwGetKey(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_KEY_D) == GLFW_PRESS)
		{
			glm::vec3 cameraCurrentPosition = m_Camera->getPosition();
			m_Camera->setPosition(cameraCurrentPosition + m_Camera->getRightVector() * deltaTime * (float)cameraSpeed);
		}
		
		if (glfwGetMouseButton(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
		{
			mouseButtons.left = true;
		}
		if (glfwGetMouseButton(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
		{
			mouseButtons.right = true;
		}
		if (glfwGetMouseButton(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_MOUSE_BUTTON_3) == GLFW_PRESS)
		{
			mouseButtons.middle = true;
		}

		if (glfwGetMouseButton(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE)
		{
			mouseButtons.left = false;
		}
		if (glfwGetMouseButton(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE)
		{
			mouseButtons.right = false;
		}
		if (glfwGetMouseButton(static_cast<GLFWwindow*>(Window::GetWindow()), GLFW_MOUSE_BUTTON_3) == GLFW_RELEASE)
		{
			mouseButtons.middle = false;
		}

		//VK_INFO("Mouse1{0}, Mouse2{1}, Mouse3{2}", mouseButtons.left, mouseButtons.middle, mouseButtons.right);

		double x, y;
		glfwGetCursorPos(static_cast<GLFWwindow*>(Window::GetWindow()), &x, &y);
		
		double dx = mousePos.x - x;
		double dy = mousePos.y - y;

		mousePos.x = x;
		mousePos.y = y;

		if(mouseButtons.left)
		{
			m_Camera->addRotation(glm::vec3( dy, -dx, 0));
		}
		if (mouseButtons.middle)
		{
			m_Camera->addPosition(glm::vec3(-dx * 0.005f, -dy * 0.005f, 0.0f));
		}

		timer += timerSpeed * deltaTime;
		if (timer > 1.0)
		{
			timer -= 1.0f;
		}
		
		updateLight();
		updateUniformBufferOffscreen();
		updateUniformBuffers();
	}

	void TestShadow::OnRender()
	{
		if(!initSuccess)
		{
			return;
		}
		
		m_Core->BeginScene();

		m_Core->resources.submitInfo.commandBufferCount = 1;
		m_Core->resources.submitInfo.pCommandBuffers = &m_Core->resources.drawCmdBuffers[m_Core->resources.imageIndex];

		buildCommandBuffers();

		VK_CHECK(vkQueueSubmit(m_Core->queue.GraphicsQueue, 1, &m_Core->resources.submitInfo, VK_NULL_HANDLE));

		VkResult err = m_Core->Submit();
		
		//TODO : Fences and Semaphores !
		vkDeviceWaitIdle(m_Core->GetDevice());
	}

	void TestShadow::OnImGuiRender()
	{
		ImGui::Begin("SCORES", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Score : ");
		ImGui::SameLine();
		ImGui::Checkbox("Display Debug", &displayDebug);
		ImGui::End();
	}

	void TestShadow::runBatchFile()
	{
		std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\shadowmapping & for %i in (*.vert) do (%VULKAN_SDK%/Bin/glslc.exe %i -o %~ni.vspv)");
		std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\shadowmapping & for %i in (*.frag) do (%VULKAN_SDK%/Bin/glslc.exe %i -o %~ni.fspv)");
	}

	void TestShadow::prepareOffscreenFramebuffer()
	{
		VkDevice device = m_Core->GetDevice();
		offscreenPass.width = SHADOWMAP_DIM;
		offscreenPass.height = SHADOWMAP_DIM;

		VkImageCreateInfo offscreenImageCI = init::imageCreateInfo();
		offscreenImageCI.imageType = VK_IMAGE_TYPE_2D;
		offscreenImageCI.format = DEPTH_FORMAT;
		offscreenImageCI.extent.height = offscreenPass.width;
		offscreenImageCI.extent.width = offscreenPass.height;
		offscreenImageCI.extent.depth = 1;
		offscreenImageCI.mipLevels = 1;
		offscreenImageCI.arrayLayers = 1;
		offscreenImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		offscreenImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		offscreenImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    
		VK_CHECK(vkCreateImage(device, &offscreenImageCI, nullptr, &offscreenPass.depth.Image));

		VkMemoryRequirements offscreenImageMemReq;
		vkGetImageMemoryRequirements(device, offscreenPass.depth.Image, &offscreenImageMemReq);
		VkMemoryAllocateInfo imageMemAllocInfo = init::memAllocInfo();
		imageMemAllocInfo.allocationSize = offscreenImageMemReq.size;
		imageMemAllocInfo.memoryTypeIndex = m_Core->getMemoryType(offscreenImageMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK(vkAllocateMemory(device, &imageMemAllocInfo, nullptr, &offscreenPass.depth.Memory));
		VK_CHECK(vkBindImageMemory(device, offscreenPass.depth.Image, offscreenPass.depth.Memory, 0));

		VkImageViewCreateInfo offscreenImageViewCI = init::imageViewCreateInfo();
		offscreenImageViewCI.image = offscreenPass.depth.Image;
		offscreenImageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		offscreenImageViewCI.format = DEPTH_FORMAT;
		offscreenImageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		offscreenImageViewCI.subresourceRange.levelCount = 1;
		offscreenImageViewCI.subresourceRange.layerCount = 1;
		offscreenImageViewCI.subresourceRange.baseMipLevel = 0;
		offscreenImageViewCI.subresourceRange.baseArrayLayer = 0;
    
		VK_CHECK(vkCreateImageView(device, &offscreenImageViewCI, nullptr, &offscreenPass.depth.View));

		VkFilter shadowMapFilter = VK_FILTER_NEAREST;

		VkSamplerCreateInfo offscreenImageSamplerCI = init::samplerCreateInfo();
		offscreenImageSamplerCI.magFilter = shadowMapFilter;
		offscreenImageSamplerCI.minFilter = shadowMapFilter;
		offscreenImageSamplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		offscreenImageSamplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		offscreenImageSamplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		offscreenImageSamplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		offscreenImageSamplerCI.mipLodBias = 0.0f;
		offscreenImageSamplerCI.maxAnisotropy = 1.0f;
		offscreenImageSamplerCI.minLod = 0.0f;
		offscreenImageSamplerCI.maxLod = 1.0f;
		offscreenImageSamplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		vkCreateSampler(device, &offscreenImageSamplerCI, nullptr, &offscreenPass.depthSampler);

		prepareOffscreenRenderpass();

		VkFramebufferCreateInfo offscreenFramebufferCI{};
		offscreenFramebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		offscreenFramebufferCI.attachmentCount = 1;
		offscreenFramebufferCI.pAttachments = &offscreenPass.depth.View;
		offscreenFramebufferCI.renderPass = offscreenPass.renderPass;
		offscreenFramebufferCI.width = offscreenPass.width;
		offscreenFramebufferCI.height = offscreenPass.height;
		offscreenFramebufferCI.layers = 1;

		VK_CHECK(vkCreateFramebuffer(device, &offscreenFramebufferCI, nullptr, &offscreenPass.frameBuffer));
	}

	void TestShadow::prepareOffscreenRenderpass()
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

		VkAttachmentReference depthReference{};
		depthReference.attachment = 0;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;
		subpass.pDepthStencilAttachment = &depthReference;

		std::array<VkSubpassDependency, 2> subpassDependencies;
		subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependencies[0].dstSubpass = 0;
		subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		subpassDependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		subpassDependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		subpassDependencies[1].srcSubpass = 0;
		subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		subpassDependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		subpassDependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassCreateInfo = init::renderPassCreateInfo();
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &attachmentDescription;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		renderPassCreateInfo.dependencyCount = subpassDependencies.size();
		renderPassCreateInfo.pDependencies = subpassDependencies.data();

		VK_CHECK(vkCreateRenderPass(m_Core->GetDevice(), &renderPassCreateInfo, nullptr, &offscreenPass.renderPass));
	}

	void TestShadow::prepareUniformBuffers()
	{
		m_UniformBuffers.SceneBuffer.reset(new VulkanUniformBuffer(sizeof(uboVSscene), m_Core));
		m_UniformBuffers.ILKER.reset(new VulkanUniformBuffer(sizeof(uboVSscene), m_Core));
		
		m_UniformBuffers.OffscreenBuffer.reset(new VulkanUniformBuffer(sizeof(uboOffscreenVS), m_Core));

		updateLight();
		updateUniformBufferOffscreen();
		updateUniformBuffers();
	}

	void TestShadow::setupDescriptorSetLayout()
	{
		// Shared pipeline layout for all pipelines used in this sample
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			// Binding 0 : Vertex shader uniform buffer
			init::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0),
			// Binding 1 : Fragment shader image sampler (shadow map)
			init::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_SHADER_STAGE_FRAGMENT_BIT, 1)
		};
		
		VkDescriptorSetLayoutCreateInfo descriptorLayout = init::descriptorSetLayoutCreateInfo();
		descriptorLayout.bindingCount = setLayoutBindings.size();
		descriptorLayout.pBindings = setLayoutBindings.data();
		VK_CHECK(vkCreateDescriptorSetLayout(m_Core->GetDevice(), &descriptorLayout, nullptr, &m_DescriptorSetLayout));
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = init::pipelineLayoutCreateInfo();
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout;
		VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));
	}

	void TestShadow::preparePipelines()
	{
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		VK_CHECK(vkCreatePipelineCache(m_Core->GetDevice(), &pipelineCacheCreateInfo, nullptr, &m_PipelineCache));
		
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = init::pipelineInputAssemblyState();
		inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterizationStateCI = init::pipelineRasterizationState();
		rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationStateCI.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationStateCI.depthClampEnable = VK_FALSE;
		rasterizationStateCI.lineWidth = 1.0f;

		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.colorWriteMask = 0xf;
		blendAttachmentState.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendStateCI = init::pipelineColorBlendState();
		colorBlendStateCI.attachmentCount = 1;
		colorBlendStateCI.pAttachments = &blendAttachmentState;

		VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = init::pipelineDepthStencilState();
		depthStencilStateCI.depthTestEnable = VK_TRUE;
		depthStencilStateCI.depthWriteEnable = VK_TRUE;
		depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilStateCI.back.compareOp = VK_COMPARE_OP_ALWAYS;

		VkPipelineViewportStateCreateInfo viewportStateCI = init::pipelineViewportState();
		viewportStateCI.viewportCount = 1;
		viewportStateCI.scissorCount = 1;

		VkPipelineMultisampleStateCreateInfo multisampleStateCI = init::multiSampleState();
		multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicStateCI = init::dynamicState();
		dynamicStateCI.dynamicStateCount = dynamicStateEnables.size();
		dynamicStateCI.pDynamicStates = dynamicStateEnables.data();
    
		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

		VkGraphicsPipelineCreateInfo pipelineCI = init::graphicsPipelineCreateInfo();
		pipelineCI.layout = pipelineLayout;
		pipelineCI.renderPass = m_Core->resources.renderPass;
		pipelineCI.stageCount = shaderStages.size();
		pipelineCI.pStages = shaderStages.data();
		pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
		pipelineCI.pViewportState = &viewportStateCI;
		pipelineCI.pRasterizationState = &rasterizationStateCI;
		pipelineCI.pMultisampleState = &multisampleStateCI;
		pipelineCI.pDepthStencilState = &depthStencilStateCI;
		pipelineCI.pColorBlendState = &colorBlendStateCI;
		pipelineCI.pDynamicState = &dynamicStateCI;

		rasterizationStateCI.cullMode = VK_CULL_MODE_FRONT_BIT;
		shaderStages[0] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/shadowmapping/quad.vspv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/shadowmapping/quad.fspv", VK_SHADER_STAGE_FRAGMENT_BIT);

		VkPipelineVertexInputStateCreateInfo emptyVertexInputStateCI = init::pipelineVertexInputState();
		pipelineCI.pVertexInputState = &emptyVertexInputStateCI;

		VK_CHECK(vkCreateGraphicsPipelines(m_Core->GetDevice(), m_PipelineCache, 1, &pipelineCI, nullptr, &m_Pipelines.Debug));

		// Scene rendering with shadows applied

		VkPipelineVertexInputStateCreateInfo vertexInputState = init::pipelineVertexInputState();
		vertexInputState.vertexBindingDescriptionCount   = 1;
		vertexInputState.pVertexBindingDescriptions = &VertexBuffer->GetVertexInputRef();
		vertexInputState.vertexAttributeDescriptionCount = VertexBuffer->GetVertexAttributes().size();
		vertexInputState.pVertexAttributeDescriptions = VertexBuffer->GetVertexAttributes().data();

	
		pipelineCI.pVertexInputState = &vertexInputState;
		rasterizationStateCI.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
		shaderStages[0] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/shadowmapping/scene.vspv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/shadowmapping/scene.fspv", VK_SHADER_STAGE_FRAGMENT_BIT);
		// Use specialization constants to select between horizontal and vertical blur
		//uint32_t enablePCF = 0;
		//VkSpecializationMapEntry specializationMapEntry = init::specializationMapEntry(0, 0, sizeof(uint32_t));
		//VkSpecializationInfo specializationInfo = init::specializationInfo(1, &specializationMapEntry, sizeof(uint32_t), &enablePCF);
		//shaderStages[1].pSpecializationInfo = &specializationInfo;
		// No filtering
		rasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
		VK_CHECK(vkCreateGraphicsPipelines(m_Core->GetDevice(), m_PipelineCache, 1, &pipelineCI, nullptr, &m_Pipelines.SceneShadow));

		// Offscreen pipeline (vertex shader only)
		shaderStages[0] = VulkanShader::GetShaderModule(m_Core->GetDevice(), "assets/shaders/shadowmapping/offscreen.vspv", VK_SHADER_STAGE_VERTEX_BIT);
		pipelineCI.stageCount = 1;

		vertexInputState.vertexBindingDescriptionCount   = 1;
		vertexInputState.pVertexBindingDescriptions = &OffscreenVertexBuffer->GetVertexInputRef();
		vertexInputState.vertexAttributeDescriptionCount = OffscreenVertexBuffer->GetVertexAttributes().size();
		vertexInputState.pVertexAttributeDescriptions = OffscreenVertexBuffer->GetVertexAttributes().data();

		
		// No blend attachment states (no color attachments used)
		colorBlendStateCI.attachmentCount = 0;
		// Cull front faces
		depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		// Enable depth bias
		rasterizationStateCI.depthBiasEnable = VK_TRUE;
		rasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
		
		// Add depth bias to dynamic state, so we can change it at runtime
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
		dynamicStateCI = init::dynamicState();
		dynamicStateCI.dynamicStateCount = dynamicStateEnables.size();
		dynamicStateCI.pDynamicStates = dynamicStateEnables.data();

		pipelineCI.renderPass = offscreenPass.renderPass;
		VK_CHECK(vkCreateGraphicsPipelines(m_Core->GetDevice(), m_PipelineCache, 1, &pipelineCI, nullptr, &m_Pipelines.Offscreen));
	}

	void TestShadow::setupDescriptorPool()
	{
		uint32_t descriptorSetSize = 4;
		std::vector<VkDescriptorPoolSize> poolSizes = {
			init::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorSetSize),
			init::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorSetSize),
		};

		VkDescriptorPoolCreateInfo descriptorPoolInfo = init::descriptorPoolCreateInfo();
		descriptorPoolInfo.poolSizeCount = poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = descriptorSetSize;
		VK_CHECK(vkCreateDescriptorPool(m_Core->GetDevice(), &descriptorPoolInfo, nullptr, &m_DescriptorPool));
	}

	void TestShadow::setupDescriptorSets()
	{
		VkDescriptorImageInfo shadowMapDescriptor;
		shadowMapDescriptor.sampler = offscreenPass.depthSampler;
		shadowMapDescriptor.imageView = offscreenPass.depth.View;
		shadowMapDescriptor.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkDescriptorSetAllocateInfo allocInfo = init::descriptorSetAllocateInfo();
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_DescriptorSetLayout;

		VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &allocInfo, &descriptorSets.debug));

		std::array<VkWriteDescriptorSet, 2> writeDescriptorSetsILKER{};
		writeDescriptorSetsILKER[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSetsILKER[0].dstSet = descriptorSets.debug;
		writeDescriptorSetsILKER[0].dstBinding = 0;
		writeDescriptorSetsILKER[0].descriptorCount = 1;
		writeDescriptorSetsILKER[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSetsILKER[0].pBufferInfo = &m_UniformBuffers.SceneBuffer->GetBufferInfoRef();

		writeDescriptorSetsILKER[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSetsILKER[1].dstSet = descriptorSets.debug;
		writeDescriptorSetsILKER[1].dstBinding = 1;
		writeDescriptorSetsILKER[1].descriptorCount = 1;
		writeDescriptorSetsILKER[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSetsILKER[1].pImageInfo = &shadowMapDescriptor;

		vkUpdateDescriptorSets(m_Core->GetDevice(), writeDescriptorSetsILKER.size(), writeDescriptorSetsILKER.data(), 0, nullptr);

		
		VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &allocInfo, &descriptorSets.offscreen));
		VkWriteDescriptorSet writeDescriptorSetOffscreen{};
		writeDescriptorSetOffscreen.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSetOffscreen.dstSet = descriptorSets.offscreen;
		writeDescriptorSetOffscreen.dstBinding = 0;
		writeDescriptorSetOffscreen.descriptorCount = 1;
		writeDescriptorSetOffscreen.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSetOffscreen.pBufferInfo = &m_UniformBuffers.OffscreenBuffer->GetBufferInfoRef();
		vkUpdateDescriptorSets(m_Core->GetDevice(), 1, &writeDescriptorSetOffscreen, 0, nullptr);
		
		VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &allocInfo, &descriptorSets.scene));
		std::array<VkWriteDescriptorSet, 2> writeDescriptorSets{};
		writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[0].dstSet = descriptorSets.scene;
		writeDescriptorSets[0].dstBinding = 0;
		writeDescriptorSets[0].descriptorCount = 1;
		writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[0].pBufferInfo = &m_UniformBuffers.SceneBuffer->GetBufferInfoRef();

		writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[1].dstSet = descriptorSets.scene;
		writeDescriptorSets[1].dstBinding = 1;
		writeDescriptorSets[1].descriptorCount = 1;
		writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[1].pImageInfo = &shadowMapDescriptor;
		vkUpdateDescriptorSets(m_Core->GetDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		
		VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &allocInfo, &descriptorSets.ILKER));
		writeDescriptorSets[0].dstSet = descriptorSets.ILKER;
		writeDescriptorSets[0].pBufferInfo = &m_UniformBuffers.ILKER->GetBufferInfoRef();
		
		writeDescriptorSets[1].dstSet = descriptorSets.ILKER;
		vkUpdateDescriptorSets(m_Core->GetDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}

	void TestShadow::buildCommandBuffers()
	{
		if(initSuccess)
		{
			ImGui::Render();
		}

		VkCommandBufferBeginInfo cmdBufInfo = init::commandBufferBeginInfo();

		VkClearValue clearValues[2];
		VkViewport viewport{};
		VkRect2D scissor{};
		VkDeviceSize offsets[] = { 0 };

		auto& resources = m_Core->resources;
		auto& drawCmdBuffers = resources.drawCmdBuffers;
		
		for(int i = 0; i < drawCmdBuffers.size(); i++)
		{
			VK_CHECK(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

			{
				clearValues[0].depthStencil = { 1.0f, 0 };

				VkRenderPassBeginInfo renderPassBeginInfo = init::renderPassBeginInfo();
				renderPassBeginInfo.renderPass = offscreenPass.renderPass;
				renderPassBeginInfo.framebuffer = offscreenPass.frameBuffer;
				renderPassBeginInfo.renderArea.extent.width = offscreenPass.width;
				renderPassBeginInfo.renderArea.extent.height = offscreenPass.height;
				renderPassBeginInfo.clearValueCount = 1;
				renderPassBeginInfo.pClearValues = clearValues;

				vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

				viewport.width = (float)offscreenPass.width;
				viewport.height = (float)offscreenPass.height;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				
				vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

				scissor.extent.width = offscreenPass.width;
				scissor.extent.height = offscreenPass.height;
				scissor.offset.x = 0;
				scissor.offset.y = 0;
				
				vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

				// Set depth bias (aka "Polygon offset")
				// Required to avoid shadow mapping artifacts
				vkCmdSetDepthBias(
					drawCmdBuffers[i],
					depthBiasConstant,
					0.0f,
					depthBiasSlope);

				vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.Offscreen);
				vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets.offscreen, 0, nullptr);
				//m_Cube->draw(drawCmdBuffers[i]);
				//scenes[sceneIndex].draw(drawCmdBuffers[i]);
				//vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, &OffscreenVertexBuffer->GetBuffer(), offsets);

				//vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets.scene, 0, nullptr);
				vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, &OffscreenVertexBuffer->GetBuffer(), offsets);
				vkCmdDraw(drawCmdBuffers[i], 36, 1, 0, 0);

				//vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets.ILKER, 0, nullptr);
				vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, &OffscreenVertexBuffer->GetBuffer(), offsets);
				vkCmdDraw(drawCmdBuffers[i], 36, 1, 0, 0);
				
				vkCmdEndRenderPass(drawCmdBuffers[i]);
			}

			/*
				Note: Explicit synchronization is not required between the render pass, as this is done implicit via sub pass dependencies
			*/

			/*
				Second pass: Scene rendering with applied shadow map
			*/

			{
				clearValues[0].color = defaultClearColor;
				clearValues[1].depthStencil = { 1.0f, 0 };

				VkRenderPassBeginInfo renderPassBeginInfo = init::renderPassBeginInfo();
				renderPassBeginInfo.renderPass = resources.renderPass;
				renderPassBeginInfo.framebuffer = resources.frameBuffers[i];
				// TODO : ILKER
				uint32_t width = m_Core->swapchain.extent.width;
				uint32_t height =  m_Core->swapchain.extent.height;
				renderPassBeginInfo.renderArea.extent.width = width;
				renderPassBeginInfo.renderArea.extent.height = height;
				renderPassBeginInfo.clearValueCount = 2;
				renderPassBeginInfo.pClearValues = clearValues;

				vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

				viewport.width = (float)width;
				viewport.height = (float)height;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				
				vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

				scissor.extent.width = width;
				scissor.extent.height = height;
				scissor.offset.x = 0;
				scissor.offset.y = 0;
				
				vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

				// Visualize shadow map
				if (displayDebug) {
					vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets.debug, 0, nullptr);
					vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.Debug);
					vkCmdDraw(drawCmdBuffers[i], 3, 1, 0, 0);
				}
				else
				{
					vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets.scene, 0, nullptr);
					vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.SceneShadow);
					vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, &VertexBuffer->GetBuffer(), offsets);

					vkCmdDraw(drawCmdBuffers[i], 36, 1, 0, 0);

					
					vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets.ILKER, 0, nullptr);
					vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.SceneShadow);
					vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, &VertexBuffer->GetBuffer(), offsets);
					vkCmdDraw(drawCmdBuffers[i], 36, 1, 0, 0);
				}
				// 3D scene


				//drawUI(drawCmdBuffers[i]);
				if(initSuccess)
				{
					ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_Core->resources.drawCmdBuffers[i]);
				}
				
				vkCmdEndRenderPass(drawCmdBuffers[i]);
			}

			VK_CHECK(vkEndCommandBuffer(drawCmdBuffers[i]));
		}
	}

	void TestShadow::windowResized()
	{
	}

	void TestShadow::updateLight()
	{
		lightPos.x = cos(glm::radians(timer * 360.0f)) * 40.0f;
		lightPos.y = -50.0f + sin(glm::radians(timer * 360.0f)) * 20.0f;
		lightPos.z = 25.0f + sin(glm::radians(timer * 360.0f)) * 5.0f;
		
		//lightPos.x = 0;//-30+cos(glm::radians(deltaTime * 360.0f)) * 400.0f;
		//lightPos.y = 0;//5 + sin(glm::radians(deltaTime * 360.0f)) * 200.0f;
		//lightPos.z = -5 + sin(glm::radians(deltaTime * 360.0f)) * 5.0f;
	}

	void TestShadow::updateUniformBufferOffscreen()
	{
		// Matrix from light's point of view
		glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(lightFOV), 1.0f, zNear, zFar);
		glm::mat4 depthViewMatrix = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));
		glm::mat4 depthModelMatrix = glm::mat4(1.0f);

		uboOffscreenVS.depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
		if(m_UniformBuffers.OffscreenBuffer)
		{
			m_UniformBuffers.OffscreenBuffer->copyData(&uboOffscreenVS, sizeof(uboOffscreenVS));
		}
	}

	void TestShadow::updateUniformBuffers()
	{
		uboVSscene.projection = m_Camera->getProjectionMatrix();
		uboVSscene.view = m_Camera->getViewMatrix();
		uboVSscene.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
		uboVSscene.model = glm::rotate(uboVSscene.model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		uboVSscene.model = glm::rotate(uboVSscene.model, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		uboVSscene.lightPos = glm::vec4(lightPos, 1.0f);
		uboVSscene.depthBiasMVP = uboOffscreenVS.depthMVP;
		uboVSscene.zNear = zNear;
		uboVSscene.zFar = zFar;

		uboVSILKER.projection = m_Camera->getProjectionMatrix();
		uboVSILKER.view = m_Camera->getViewMatrix();
		glm::mat4 translate_ILKER = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, -5.0f));
		glm::mat4 rotate_ILKER = glm::mat4(1.0f);
		auto a = glm::mat4(1.0f)[0];
		rotate_ILKER = glm::rotate(rotate_ILKER, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		rotate_ILKER = glm::rotate(rotate_ILKER, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		uboVSILKER.model = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 5.0f));
		uboVSILKER.model = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		uboVSILKER.model = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		
		uboVSILKER.model = translate_ILKER * rotate_ILKER;
		
		uboVSILKER.lightPos = glm::vec4(lightPos, 1.0f);
		uboVSILKER.depthBiasMVP = uboOffscreenVS.depthMVP;
		uboVSILKER.zNear = zNear;
		uboVSILKER.zFar = zFar;
		
		if(m_UniformBuffers.SceneBuffer)
		{
			m_UniformBuffers.SceneBuffer->copyData(&uboVSscene, sizeof(uboVSscene));
		}
		if(m_UniformBuffers.ILKER)
		{
			m_UniformBuffers.ILKER->copyData(&uboVSILKER, sizeof(uboVSILKER));
		}
	}

	void TestShadow::onMouseScrollMoved(GLFWwindow* window, double dX, double dY)
	{
		cameraSpeed = std::clamp(cameraSpeed + dY, 1.0, 10.0);
	}
}
