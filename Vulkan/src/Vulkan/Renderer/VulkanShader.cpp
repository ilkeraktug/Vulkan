#include "pch.h"
#include "VulkanShader.h"

VkPipelineShaderStageCreateInfo VulkanShader::GetShaderModule(const VkDevice& device, const std::string& shaderPath, VkShaderStageFlagBits stage)
{
	std::string shaderSource   = ReadFile(shaderPath);

	VkShaderModule shaderModule;

	VkShaderModuleCreateInfo shaderModuleCI{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	shaderModuleCI.codeSize = shaderSource.size();
	shaderModuleCI.pCode = reinterpret_cast<const uint32_t*>(shaderSource.data());

	VK_CHECK(vkCreateShaderModule(device, &shaderModuleCI, nullptr, &shaderModule));

	VkPipelineShaderStageCreateInfo shaderStage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	shaderStage.stage = stage;
	shaderStage.module = shaderModule;
	shaderStage.pName = "main";

	return shaderStage;
}

std::string VulkanShader::ReadFile(const std::string& filepath)
{
	std::string source;

	std::ifstream file(filepath, std::ios_base::binary | std::ios_base::ate);

	if (file.is_open())
	{
		uint32_t size = file.tellg();
		source.resize(size);

		file.seekg(0);
		file.read(source.data(), size);

		file.close();
	}
	else
		VK_ERROR("Can not open file {0}", filepath);

	return source;

}
