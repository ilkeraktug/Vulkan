#include "pch.h"
#include "Window.h"

#include "Vulkan/Core/Core.h"

GLFWwindow* Window::m_Window = nullptr;

static void GLFWErrorCallback(int error_code, const char* description)
{
	VK_CORE_ERROR("GLFW Error Code ({0}) : {1}", error_code, description);
}

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

	glfwSetErrorCallback(GLFWErrorCallback);
	
	glfwSetWindowUserPointer(m_Window, &m_WindowData);

	glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
	{
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		data.Width = width;
		data.Height = height;

	});

	glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xScroll, double yScroll)
	{
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		MouseScrollEvent e(xScroll, yScroll);
		//VK_INFO("{0}", e.ToString());
		for(auto& callback : data.callbacks)
		{
			callback(e);
		}
	
	});
}

void Window::Shutdown()
{
	glfwDestroyWindow(m_Window);
}
