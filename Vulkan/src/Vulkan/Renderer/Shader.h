#pragma once
#include <vulkan/vulkan.h>

#include "SwapChain.h"

//TODO : Divide shader class to graphics pipeline class!
//TODO : Shader class only needs to handle shader readings and modules.

class Shader
{
public:
	Shader(const std::string& vertexPath, const std::string& fragmentPath);
	~Shader();

	inline const VkPipelineShaderStageCreateInfo* GetShaderStageCreateInfos() const { return m_StageCreateInfos; }

private:
	void setupShaderModule(const std::string& vertexSrc, const std::string& fragmentSrc);
	std::string readFile(const std::string& filepath);
private:

	VkShaderModule m_VertexModule;
	VkShaderModule m_FragmentModule;

	VkPipelineShaderStageCreateInfo m_StageCreateInfos[2];
};