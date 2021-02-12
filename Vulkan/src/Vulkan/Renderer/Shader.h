#pragma once
#include <vulkan/vulkan.h>

class Shader
{
public:
	static VkShaderModule setupShaderModule(const std::string& filepath, VkDevice device);
private:
	static std::string readFile(const std::string& fileName);
};