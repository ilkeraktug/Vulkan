/*
*
* This test is meant for Shadow Mapping.
*
*/

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Test.h"

#include "Vulkan/Renderer/VulkanShader.h"
#include "Vulkan/Renderer/VulkanVertexBuffer.h"
#include "Vulkan/Renderer/VulkanIndexBuffer.h"
#include "Vulkan/Renderer/VulkanUniformBuffer.h"
#include "Vulkan/Renderer/VulkanTexture2D.h"

#include "Vulkan/Renderer/OrthographicCamera.h"
#include "Vulkan/Renderer/PerspectiveCamera.h"

#include "Vulkan/Objects/QuadObj.h"
#include "Vulkan/Objects/CubeObj.h"

#include "Vulkan/ImGui/VulkanUI.h"

#include "Vulkan/Core/Time.h"

namespace test
{
	class TestShadowMapping : public Test
	{
	public:
		TestShadowMapping() = default;
		TestShadowMapping(VulkanCore* core);
		~TestShadowMapping();

		virtual void OnUpdate(float deltaTime) override;
		virtual void OnRender() override;
		virtual void OnImGuiRender() override;

	private:
		void prepareLightRenderPass();
		void prepareFramebufferAttachment();

		void prepareDescriptorPool();
		void prepareDescriptorSetLayout();
		void prepareDescriptorSets();

		void preparePipelines();
		void prepareUniformBuffers();

		void buildCommandBuffers();

		void updateLight();
		void updateUniformBuffers();
		void updateUniformBuffersLight();
		
		virtual void windowResized() override;

	private:
		
		std::unique_ptr<VulkanVertexBuffer> m_SceneVertexBuffer;
		std::unique_ptr<VulkanVertexBuffer> m_LightVertexBuffer;

		std::unique_ptr<VulkanIndexBuffer> m_SceneIndexBuffer;
		std::unique_ptr<VulkanIndexBuffer> m_LightIndexBuffer;

		std::unique_ptr<CubeObj> m_Cube;

		Camera* m_Camera;

		struct
		{
			std::unique_ptr<VulkanUniformBuffer> Scene;
			std::unique_ptr<VulkanUniformBuffer> Light;
		}uniformBuffer;


		struct {
			glm::mat4 projection;
			glm::mat4 view;
			glm::mat4 model;
			glm::mat4 depthBiasMVP;
			glm::vec3 lightPos;
		} sceneUBO;

		glm::vec3 lightPos{};

		struct {
			glm::mat4 depthMVP;
		} lightUBO;

		VkDescriptorSetLayout m_DescriptorSetLayout;
		VkDescriptorPool m_DescriptorPool;

		struct
		{
			VkDescriptorSet Scene;
			VkDescriptorSet Light;
		} descriptorSets;

		struct
		{
			VkPipeline Light;
			VkPipeline Scene;
		} pipelines;

		VkPipelineLayout m_PipelineLayout;

		struct FramebufferAttachment
		{
			VkImage Image;
			VkImageView View;
			VkDeviceMemory Memory;
		};
		
		struct LightViewPass
		{
			uint32_t Width, Height;
			FramebufferAttachment FrameBufferAttachment;
			VkFramebuffer Framebuffer;
			VkRenderPass RenderPass;
			VkSampler Sampler;
			VkDescriptorImageInfo Descriptor;
		} lightPassView;

	};
}