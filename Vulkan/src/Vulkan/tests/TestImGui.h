#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Test.h"

#include "Vulkan/Renderer/VulkanShader.h"
#include "Vulkan/Renderer/VulkanVertexBuffer.h"
#include "Vulkan/Renderer/VulkanIndexBuffer.h"
#include "Vulkan/Renderer/VulkanUniformBuffer.h"
#include "Vulkan/Renderer/VulkanTexture2D.h"

#include "Vulkan/ImGui/VulkanUI.h"

#include "Vulkan/Renderer/OrthographicCamera.h"
#include "Vulkan/Renderer/PerspectiveCamera.h"

#include "Vulkan/Objects/QuadObj.h"
#include "Vulkan/Objects/CubeObj.h"

//
#include "Vulkan/tests/FlappyBird/PipeObject.h"

namespace test
{
	class TestImGui : public Test
	{
	public:
		TestImGui() = default;
		TestImGui(VulkanCore* core);
		~TestImGui();

		virtual void OnUpdate(float deltaTime) override;
		virtual void OnRender() override;
		virtual void OnImGuiRender() override;

	private:

		void prepareDescriptorPool();
		void preparePipeline();
		void setCmdBuffers();

		void updateUniformBuffers();
		virtual void windowResized() override;

	private:
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_GraphicsPipeline;
		VkDescriptorPool m_DescriptorPool;
		VkDescriptorSetLayout m_DescriptorSetLayout;

		std::unique_ptr<VulkanVertexBuffer> m_VertexBuffer;
		std::unique_ptr<VulkanIndexBuffer> m_IndexBuffer;
		std::unique_ptr<VulkanTexture2D> m_Texture;

		VkDescriptorSet viewProjDescriptorSet;

		Camera* m_Camera;

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraMoveSpeed = 5.0f;

		std::array<Drawable*, 1> objs;

		VulkanUI* UI;
	};
}