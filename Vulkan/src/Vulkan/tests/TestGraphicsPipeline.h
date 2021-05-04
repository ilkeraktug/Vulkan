#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Test.h"

#include "Vulkan/Renderer/VulkanShader.h"
#include "Vulkan/Renderer/VulkanVertexBuffer.h"
#include "Vulkan/Renderer/VulkanIndexBuffer.h"
#include "Vulkan/Renderer/VulkanUniformBuffer.h"

#include "Vulkan/Objects/QuadObj.h"

namespace test
{
	class TestGraphicsPipeline : public Test
	{
	public:
		TestGraphicsPipeline() = default;
		TestGraphicsPipeline(VulkanCore* core);
		~TestGraphicsPipeline();

		virtual void OnUpdate(float deltaTime) override;
		virtual void OnRender() override;
		virtual void OnImGuiRender() override;

	private:

		void prepareDescriptorPool();
		void preparePipeline();
		void setCmdBuffers();

		void updateUniformBuffers(float deltaTime);
		virtual void windowResized() override;

	private:
		struct Object
		{
			struct Transform
			{
				glm::mat4 Model;
				glm::mat4 View;
				glm::mat4 Projection;
			} u_Scene;
		
			VkDescriptorSet DescriptorSet;
		} obj;

		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_GraphicsPipeline;
		VkDescriptorPool m_DescriptorPool;
		VkDescriptorSetLayout m_DescriptorSetLayout;

		VulkanVertexBuffer* m_VertexBuffer;
		std::unique_ptr<VulkanIndexBuffer> m_IndexBuffer;
		VulkanUniformBuffer* m_UniformBuffer;

		VulkanUniformBuffer* m_ViewProjBuffer;
		VkDescriptorSet viewProjDescriptorSet;
		VkDescriptorSet debugSet[2];

		glm::mat4 View;
		glm::mat4 Projection;

		std::array<QuadObj*, 5> objs;
	};
}