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

    int succes = glfwInit();
    VK_CORE_ASSERT(succes, "glfwInit()");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    VK_CORE_INFO("{0} extensions supported\n", extensionCount);

    glm::mat4 matrix;
    glm::vec4 vec;
    auto test = matrix * vec;
}

Application::~Application()
{
    glfwDestroyWindow(window);

    glfwTerminate();
}

void Application::Run()
{
    while (!glfwWindowShouldClose(window)) 
    {
        glfwPollEvents();
    }
}
