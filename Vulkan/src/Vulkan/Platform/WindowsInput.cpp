#include "pch.h"
#include "WindowsInput.h"

#include "glfw/glfw3.h"
#include "Vulkan/Core/Window.h"
#include "Vulkan/Core/Events/Event.h"
#include "Vulkan/Renderer/PerspectiveCamera.h"

Input* Input::s_Instance = new WindowsInput();

WindowsInput::WindowsInput()
{
}

WindowsInput::~WindowsInput()
{
}

bool WindowsInput::IsKeyPressedImpl(int keycode) const
{
    GLFWwindow* Window = static_cast<GLFWwindow*>(Window::GetWindow());
    return glfwGetKey(Window, keycode) == GLFW_PRESS;
}

bool WindowsInput::IsMouseButtonPressedImpl(int button) const
{
    GLFWwindow* Window = static_cast<GLFWwindow*>(Window::GetWindow());
    return glfwGetMouseButton(Window, button) == GLFW_PRESS;
}

bool WindowsInput::IsKeyReleasedImpl(int keycode) const
{
    GLFWwindow* Window = static_cast<GLFWwindow*>(Window::GetWindow());
    return glfwGetKey(Window, keycode) == GLFW_RELEASE;
}

bool WindowsInput::IsMouseButtonReleasedImpl(int button) const
{
    GLFWwindow* Window = static_cast<GLFWwindow*>(Window::GetWindow());
    return glfwGetMouseButton(Window, button) == GLFW_RELEASE;
}

bool WindowsInput::IsCtrlPressedImpl() const
{
    GLFWwindow* Window = static_cast<GLFWwindow*>(Window::GetWindow());
    return glfwGetKey(Window, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(Window, GLFW_KEY_RIGHT_CONTROL) > 0;

}

bool WindowsInput::IsAltPressedImpl() const
{
    GLFWwindow* Window = static_cast<GLFWwindow*>(Window::GetWindow());
    return glfwGetKey(Window, GLFW_KEY_LEFT_ALT) || glfwGetKey(Window, GLFW_KEY_RIGHT_ALT) > 0;
}

bool WindowsInput::IsShiftPressedImpl() const
{
    GLFWwindow* Window = static_cast<GLFWwindow*>(Window::GetWindow());
    return glfwGetKey(Window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(Window, GLFW_KEY_RIGHT_SHIFT) > 0;
}

std::pair<double, double> WindowsInput::GetMousePositionImp() const
{
    GLFWwindow* Window = static_cast<GLFWwindow*>(Window::GetWindow());
    double xPos;
    double yPos;
    
    glfwGetCursorPos(Window, &xPos, &yPos);

    return { xPos, yPos };
}

void WindowsInput::SetCursorHiddenImp(bool Hidden) const
{
    GLFWwindow* Window = static_cast<GLFWwindow*>(Window::GetWindow());
    glfwSetInputMode(Window, GLFW_CURSOR, Hidden ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

bool WindowsInput::OnEvent(Event& e)
{
    EventDispatcher dispatcher(e);

    dispatcher.Dispatch<MouseScrollEvent>(std::bind(OnMouseScrollEvent, std::placeholders::_1));
    dispatcher.Dispatch<MouseScrollEvent>(std::bind(PerspectiveCamera::OnMouseScrollEvent, std::placeholders::_1));

    return true;
}

bool WindowsInput::OnMouseScrollEvent(MouseScrollEvent& e)
{
    return true;
}
