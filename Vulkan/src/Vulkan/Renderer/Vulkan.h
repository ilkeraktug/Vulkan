#pragma once

#include <vulkan\vulkan.h>
#include <GLFW\glfw3.h>

class Vulkan
{
public:
	Vulkan();
	~Vulkan();
	
	void Init();
	void Shutdown();
	void Run();
private:
	void findDevice();
private:
	VkInstance m_Instance;
	VkPhysicalDevice m_Device;

	struct QFamilyIndex
	{
		std::optional<uint32_t> index;
	};
};