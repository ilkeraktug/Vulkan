#pragma once

#include <vulkan\vulkan.h>
#include <GLFW\glfw3.h>
#include <GLFW\glfw3native.h>

#include "Shader.h"

class Vulkan
{
public:
	Vulkan();
	~Vulkan();
	
	void Init();
	void Shutdown();
	void Run();
private:
	void createInstance();
private:
	VkInstance m_Instance;
};