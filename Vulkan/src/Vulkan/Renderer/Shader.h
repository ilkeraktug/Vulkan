#pragma once
#include <vulkan/vulkan.h>

class Shader
{
public:
	Shader(const std::string& filepath)
		:m_FileName(filepath) {}
	VkShaderModule setupShaderModule(VkDevice& m_Device);
private:
	std::string readFile(const std::string& fileName);

	std::string m_FileName;
	VkDevice m_Device;
};