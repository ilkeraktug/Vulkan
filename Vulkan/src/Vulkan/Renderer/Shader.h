#pragma once
#include <vulkan/vulkan.h>

#include "SwapChain.h"
#include "VertexBuffer.h"

class Shader
{
public:
	Shader(const std::string& vertexPath, const std::string& fragmentPath);
	~Shader();

	inline const VkPipelineShaderStageCreateInfo* GetShaderStageCreateInfos() const { return m_StageCreateInfos; }

private:
	void setupShaderModule(const std::string& vertexPath, const std::string& fragmentPath);
	std::string readFile(const std::string& filepath);
private:

	VkShaderModule m_VertexModule;
	VkShaderModule m_FragmentModule;

	VkPipelineShaderStageCreateInfo m_StageCreateInfos[2];
};