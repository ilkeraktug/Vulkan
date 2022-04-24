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

		void OnGameStart();
		void OnGameOver(float deltaTime);
		void OnRestartGame();

		void prepareDescriptorPool();
		void preparePipeline();
		void setCmdBuffers();

		void updateUniformBuffers();
		virtual void windowResized() override;

	private:

		Camera* m_Camera;

		float m_ScreenRight;
		float m_ScreenTop;

		bool gamePaused = true;

		uint16_t m_Score = 0;
		uint16_t m_HighestScore = 0;

		std::array<PipeObject*, 16> m_PipeObjects;
		float m_PipeGap = 0.5f;
		std::unique_ptr<BirdObject> m_Bird;
		std::unique_ptr<Background> m_Background;

		VkDescriptorPool m_DescriptorPool;

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