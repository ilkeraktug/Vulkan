#pragma once

#include "pch.h"

#include "Vulkan/Renderer/VulkanCore.h"

namespace test {
	class Test
	{
	public:
		Test() {};
		virtual ~Test() {};

		void Init(VulkanCore* core) { m_Core = core; };

		virtual void windowResized() { 		VK_CORE_INFO("Test::windowResized"); };

		virtual void OnUpdate(float deltaTime) = 0;
		virtual void OnRender() = 0;
		virtual void OnImGuiRender() = 0;
	protected:
		VulkanCore* m_Core;
	};

	class TestMenu : public Test
	{
	public:
		TestMenu(Test*& currentTest)
			:m_CurrentTest(currentTest) {};
		virtual ~TestMenu() = default;

		template<typename T>
		void PushMenu(const std::string& name)
		{
			m_Tests.push_back(std::make_pair(name, [this]()
			{
				return new T(m_Core);
			}));
		}

		virtual void OnUpdate(float deltaTime) override;
		virtual void OnRender() override;
		virtual void OnImGuiRender() override;

	private:
		Test*& m_CurrentTest;
		std::vector<std::pair<std::string, std::function<Test*()>>> m_Tests;

		bool isOpen = true;
	};
}