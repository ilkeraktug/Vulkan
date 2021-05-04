#include "pch.h"
#include "Application.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "Vulkan/Core/Core.h"
#include "Vulkan/Core/Log.h"
#include "Core/Time.h"



Application::Application()
{
    Log::Init();
    VK_CORE_INFO("Application created!");

    std::vector<const char*> enableExtension;

    m_Window.reset(new Window());

    m_VulkanCore.reset(new VulkanCore());

    m_CurrentTest = new test::TestGraphicsPipeline(m_VulkanCore.get());
    m_TestMenu = new test::TestMenu(m_CurrentTest);
}

Application::~Application()
{
    glfwTerminate();


    delete m_TestMenu;
    delete m_CurrentTest;
}

void Application::Run()
{
    while (!glfwWindowShouldClose(static_cast<GLFWwindow*>(m_Window->GetWindow())))
    {
        Time::OnUpdate();

        m_Window->OnUpdate();

        if (m_CurrentTest)
        {
            m_CurrentTest->OnUpdate(Time::deltaTime);
            m_CurrentTest->OnImGuiRender();

            m_CurrentTest->OnRender();


           /* ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Select one of tests");
            if (m_CurrentTest != m_TestMenu && ImGui::Button("<--"))
            {
                delete m_CurrentTest;
                m_CurrentTest = m_TestMenu;
            }

            ImGui::End();

            m_CurrentTest->OnImGuiRender();*/

        }
    }
}
