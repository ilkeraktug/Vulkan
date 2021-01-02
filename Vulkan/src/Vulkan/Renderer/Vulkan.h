#pragma once

#include <vulkan\vulkan.h>
#include <GLFW\glfw3.h>
#include <GLFW\glfw3native.h>

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
	bool hasRequiredExtensions();

	void SelectSwapChainObject();
private:
	struct QFamilyIndex
	{
		std::optional<uint32_t> Graphics;
		std::optional<uint32_t> Present;

		inline bool hasValue() { return Graphics.has_value() && Present.has_value(); }
	};

	struct SwapChainDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> Present;
	};

	std::vector<const char*> m_DeviceExtensions;
	std::tuple<VkExtent2D, VkSurfaceFormatKHR, VkPresentModeKHR> swapChainObject;

	VkInstance m_Instance;
	VkPhysicalDevice m_PhysicalDevice;
	VkDevice m_Device;
	VkSurfaceKHR m_Surface;
	VkSwapchainKHR m_SwapChain;

	VkQueue m_GraphicsQ;
	VkQueue m_PresentQ;
	QFamilyIndex index;
	SwapChainDetails swapChainDetails;
};