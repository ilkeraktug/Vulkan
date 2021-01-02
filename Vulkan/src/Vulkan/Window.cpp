#include "pch.h"
#include "Window.h"

#include "Vulkan\Core.h"

GLFWwindow* Window::m_Window = nullptr;

Window::Window(const WindowProps& windowProp)
{
	Init(windowProp);
}

Window::~Window()
{
	Shutdown();	
}

void Window::OnUpdate()
{
	glfwPollEvents();
}

void Window::SetVsync(bool vsync)
{
	if (vsync)
		glfwSwapInterval(1);
	else
		glfwSwapInterval(0);

	m_WindowData.IsVsync = vsync;
}

void Window::Init(const WindowProps& windowProp)
{
	int succes = glfwInit();
	VK_CORE_ASSERT(succes, "glfwInit()");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_Window = glfwCreateWindow(windowProp.Width, windowProp.Height, windowProp.Name.c_str(), nullptr, nullptr);
	VK_CORE_INFO("Created {0} window, ({1}, {2})",
		windowProp.Name.c_str(),
		windowProp.Width,
		windowProp.Height);
}

void Window::Shutdown()
{
	glfwDestroyWindow(m_Window);
}
