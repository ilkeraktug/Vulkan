#include "pch.h"
#include "Shader.h"

#include "Vulkan\Core.h"

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
	setupShaderModule(readFile(vertexPath), readFile(fragmentPath));
}

Shader::~Shader()
{
	vkDestroyShaderModule(VulkanCore::GetDevice(), m_VertexModule, nullptr);
	vkDestroyShaderModule(VulkanCore::GetDevice(), m_FragmentModule, nullptr);
}

void Shader::setupShaderModule(const std::string& vertexSrc, const std::string& fragmentSrc)
{
	VkShaderModuleCreateInfo vertexCreateInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	vertexCreateInfo.codeSize = vertexSrc.size();
	vertexCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertexSrc.c_str());

	VkShaderModuleCreateInfo fragmentCreateInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	fragmentCreateInfo.codeSize = fragmentSrc.size();
	fragmentCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentSrc.c_str());

	VK_ASSERT(vkCreateShaderModule(VulkanCore::GetDevice(), &vertexCreateInfo, nullptr, &m_VertexModule) == VK_SUCCESS, "Failed to create vertexShaderModule");
	VK_ASSERT(vkCreateShaderModule(VulkanCore::GetDevice(), &fragmentCreateInfo, nullptr, &m_FragmentModule) == VK_SUCCESS, "Failed to create fragmentShaderModule");

	VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageCreateInfo.module = m_VertexModule;
	vertexShaderStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageCreateInfo.module = m_FragmentModule;
	fragmentShaderStageCreateInfo.pName = "main";

	m_StageCreateInfos[0] = vertexShaderStageCreateInfo;
	m_StageCreateInfos[1] = fragmentShaderStageCreateInfo;
}

std::string Shader::readFile(const std::string& filepath)
{
	std::ifstream file(filepath, std::ios::in | std::ios::binary);
	std::string buffer;

	if (!file.is_open())
	{
		VK_CORE_ERROR("Cant open file {0}", filepath);
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
			VK_CORE_ERROR("Cant read file {0}", filepath);
		}
	}

	return buffer;
}
