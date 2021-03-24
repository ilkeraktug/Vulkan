#pragma once

#include <glfw\glfw3.h>

struct WindowProps
{
	std::string Name;
	uint32_t Width, Height;
	bool IsVsync;

	WindowProps(const std::string& name = "Vulkan", uint32_t width = 1280, uint32_t height = 768, bool vsync = true)
		: Name(name), Width(width), Height(height), IsVsync(vsync) {}
};

class Window
{
public:
	Window(const WindowProps& windowProp = WindowProps());
	~Window();

	void OnUpdate();

	inline uint32_t GetWidth() const { return m_WindowData.Width; }
	inline uint32_t GetHeight() const { return m_WindowData.Height; }

	void SetVsync(bool vsync);
	inline bool IsVsync() const { return m_WindowData.IsVsync; }

	inline static void* GetWindow() { return m_Window; }

private:
	void Init(const WindowProps& windowProp);
	void Shutdown();
private:
	static GLFWwindow* m_Window;

	struct WindowData
	{
		std::string Name;
		uint32_t Width, Height;
		bool IsVsync;
	};

	WindowData m_WindowData;
};