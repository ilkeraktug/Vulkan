#include "pch.h"
#include "Application.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "Vulkan/Core/Core.h"
#include "Vulkan/Core/Log.h"
#include "Core/Time.h"

#include "tests/TestGraphicsPipeline.h"
#include "tests/TestFlappyBird.h"
#include "tests/TestImGui.h"
#include "tests/TestShadowMapping.h"

Application::Application()
{
    //TODO : Test Menu
    m_TestMenu = nullptr;

    Log::Init();
    VK_CORE_INFO("Application created!");

    std::vector<const char*> enableExtension = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    m_Window.reset(new Window());

    m_VulkanCore.reset(new VulkanCore(enableExtension));

    m_CurrentTest = new test::TestShadowMapping(m_VulkanCore.get());
    
   //m_TestMenu = new test::TestMenu(m_CurrentTest);
   //m_TestMenu->PushMenu<test::TestFlappyBird>("TestFlappyBird");
   //m_TestMenu->PushMenu<test::TestGraphicsPipeline>("TestGraphicsPipeline");
}

Application::~Application()
{
    glfwTerminate();

    if (m_TestMenu)
    {
        delete m_TestMenu;
    }

    if (m_CurrentTest)
    {
        delete m_CurrentTest;
    }
}

void Application::Run()
{
    while (!glfwWindowShouldClose(static_cast<GLFWwindow*>(m_Window->GetWindow())))
    {
        Time::OnUpdate();

        m_Window->OnUpdate();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();
		
        if (m_CurrentTest)
        {
            m_CurrentTest->OnUpdate(Time::deltaTime);
            m_CurrentTest->OnImGuiRender();

            m_CurrentTest->OnRender();
        }
        
        ImGui::EndFrame();
    }
}
