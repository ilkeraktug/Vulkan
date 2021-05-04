#pragma once
#include "Abstract/Shader.h"

#include <vulkan/vulkan.h>

class VulkanShader : public Shader
{
public:
	static VkPipelineShaderStageCreateInfo GetShaderModule(const VkDevice& device, const std::string& shaderPath, VkShaderStageFlagBits stage);
private:
	static std::string ReadFile(const std::string& filepath);
};