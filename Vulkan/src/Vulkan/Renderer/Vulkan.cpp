#include "pch.h"
#include "Vulkan.h"

#include "Vulkan\Core.h"

Vulkan::Vulkan()
{
	Init();
}

Vulkan::~Vulkan()
{
}

void Vulkan::Init()
{
	VkApplicationInfo info{};
	info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	info.pApplicationName = "Vulkan App";
	info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	info.pEngineName = "No Engine";
	info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &info;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;

	VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
	VK_ASSERT((result == VK_SUCCESS), "Can't create instance!");

	uint32_t extensionCount;
	result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	VK_ASSERT((result == VK_SUCCESS), "Can't get extension counts!");
	VK_CORE_TRACE("Extension count {0}", extensionCount);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	for (auto& extesion : extensions)
	{
		VK_INFO("Extension {0}", extesion.extensionName);
	}

	findDevice();
}

void Vulkan::Shutdown()
{
	vkDestroyInstance(m_Instance, nullptr);
}

void Vulkan::Run()
{

}

void Vulkan::findDevice()
{
	QFamilyIndex index;

	m_Device = VK_NULL_HANDLE;
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

	VK_ASSERT(deviceCount, "Couldn't find physical device!");

	VkPhysicalDevice* devices = new VkPhysicalDevice[deviceCount];
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices);

	for (int i = 0; i < deviceCount; i++)
	{
		m_Device = devices[i];

		VkPhysicalDeviceProperties deviceProperty;
		vkGetPhysicalDeviceProperties(m_Device, &deviceProperty);

		VkPhysicalDeviceFeatures deviceFeature;
		vkGetPhysicalDeviceFeatures(m_Device, &deviceFeature);
		
		uint32_t familyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(m_Device, &familyCount, nullptr);
		std::vector<VkQueueFamilyProperties> qFamilyProperties(familyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_Device, &familyCount, qFamilyProperties.data());

		int j = 0;
		for (const auto& property : qFamilyProperties)
		{
			if (property.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				index.index = j;
				break;
			}
			j++;
		}

		if (deviceProperty.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeature.geometryShader && index.index.has_value())
		{
			m_Device = devices[i];
			VK_INFO("\nRenderer GPU Name : {0}", deviceProperty.deviceName);
		}
	}


	delete[] devices;
}
