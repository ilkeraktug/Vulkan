#include "pch.h"
#include "Application.h"

#include <GLFW\glfw3.h>
#include <vulkan\vulkan.h>
#include <glm\vec4.hpp>
#include <glm\mat4x4.hpp>

#include "Vulkan\Core.h"
#include "Vulkan\Log.h"

Application::Application()
{
    Log::Init();
    VK_CORE_INFO("Application created!");

    m_Window.reset(new Window());

    m_Vulkan.reset(new Vulkan());
}

Application::~Application()
{
    glfwTerminate();
}

void Application::Run()
{
    while (m_Running) 
    {
        m_Window->OnUpdate();
        m_Vulkan->Run();
    }
}
