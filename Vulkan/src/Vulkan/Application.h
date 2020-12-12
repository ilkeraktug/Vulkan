#pragma once

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
	GLFWwindow* window;

private:
	static Application* s_Application;
};