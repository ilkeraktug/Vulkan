#pragma once

#include "Window.h"
#include "Vulkan\Renderer\Vulkan.h"

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
	std::unique_ptr<Vulkan> m_Vulkan;

	bool m_Running = true;

private:
	static Application* s_Application;
};