#pragma once

#include "VulkanCore.h"
#include "SwapChain.h"
#include "Texture.h"

#include <glm/glm.hpp>


struct Transformation
{
	glm::mat4 Model;
	glm::mat4 View;
	glm::mat4 Projection;
	//glm::mat4 Clip;
};

class UniformBuffer
{
public:
	UniformBuffer() = default;
	UniformBuffer(Transformation& transform, const Texture& texture);
	~UniformBuffer();

	inline const VkDescriptorSetLayout& GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }
	inline const std::vector<VkDescriptorSet> GetDescriptorSets() const { return m_DescriptorSets; }

	void setTransform(const Transformation& transform) { m_Transform = transform; }

	void updateBuffer(uint32_t index);
private:
	void createPool();
	void createSets(const Texture& texture);
	void createLayout();
	void createBuffer(const Transformation& transform);
	void createBuffer(const glm::mat4& matrix);

	uint32_t getMemoryType(uint32_t type, VkMemoryPropertyFlags propertyFlags);
private:
	VkDescriptorPool m_DescriptorPool;
	std::vector<VkDescriptorSet> m_DescriptorSets;

	VkDescriptorSetLayout m_DescriptorSetLayout;
	VkPipelineLayout m_Layout;

	std::vector<VkBuffer> m_Buffers;
	std::vector<VkDeviceMemory> m_BufferMemories;

	Transformation& m_Transform;
};