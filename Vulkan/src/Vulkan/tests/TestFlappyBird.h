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

#include "Vulkan/tests/FlappyBird/PipeObject.h"
#include "Vulkan/tests/FlappyBird/Background.h"
#include "Vulkan/tests/FlappyBird/BirdObject.h"

namespace test {

	class TestFlappyBird : public Test
	{
	public:
		TestFlappyBird() = default;
		TestFlappyBird(VulkanCore * core);
		~TestFlappyBird();

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
		VkDescriptorPool m_DescriptorPool;

		Camera* m_Camera;
		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraMoveSpeed = 5.0f;

		std::array<PipeObject*, 5> m_PipeObjects;
		std::unique_ptr<BirdObject> m_Bird;
		std::unique_ptr<Background> m_Background;

		struct
		{
			VkPipelineLayout ObjectsPipeline;
			VkPipelineLayout BackgroundPipeline;

			VkDescriptorSetLayout ObjectsDescriptor;
			VkDescriptorSetLayout BackgroundDescriptor;

		} layout;

		struct
		{
			VkPipeline PipeObject;
			VkPipeline BirdObject;
			VkPipeline Background;
		} pipelines;
	};
}