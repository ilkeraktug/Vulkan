#include "pch.h"
#include "Shader.h"

#include "Vulkan\Core.h"


VkShaderModule Shader::setupShaderModule(VkDevice& m_Device)
{
	VkShaderModule shaderModule;

	std::string source = readFile(m_FileName);

	VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	createInfo.codeSize = source.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(source.c_str());

	if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		VK_CORE_WARN("Cant create shader module at {0}", m_FileName);

	return shaderModule;
}

std::string Shader::readFile(const std::string& fileName)
{
	std::ifstream file(fileName, std::ios::in | std::ios::binary);
	std::string buffer;

	if (!file.is_open())
	{
		VK_CORE_ERROR("Cant open file {0}", fileName);
	}
	else
	{
		file.seekg(0, std::ios::end);
		size_t size = file.tellg();

		if (size != -1)
		{
			buffer.resize(size);
			file.seekg(0, std::ios::beg);
			file.read(&buffer[0], size);
		}
		else
		{
			VK_CORE_ERROR("Cant read file {0}", fileName);
		}
	}

	return buffer;
}
