#pragma once

#include "Core/Window.h"
#include "Vulkan/Renderer/VulkanCore.h"

#include "tests/Test.h"

struct GLFWwindow;

class Application
{
public:
	Application();
	~Application();
	
	void Run();

	inline static Application* GetApplication() { return s_Application; }

	static Application* CreateApplication() { return new Application(); }

private:
	std::unique_ptr<Window> m_Window;
	std::unique_ptr<VulkanCore> m_VulkanCore;

	bool m_Running = true;

	test::Test* m_CurrentTest;
	test::TestMenu* m_TestMenu;
private:
	static Application* s_Application;

};