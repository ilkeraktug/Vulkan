#include "pch.h"
#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(Transformation& transform, const Texture& texture)
	:m_Transform(transform)
{
	createBuffer(m_Transform);
	createLayout();
	createPool();
	createSets(texture);
}

UniformBuffer::~UniformBuffer()
{
}

void UniformBuffer::updateBuffer(uint32_t index)
{
	void* data;
	vkMapMemory(VulkanCore::GetDevice(), m_BufferMemories[index], 0, sizeof(Transformation), 0, &data);
	memcpy(data, &m_Transform, sizeof(m_Transform));
	vkUnmapMemory(VulkanCore::GetDevice(), m_BufferMemories[index]);
}

void UniformBuffer::createPool()
{
	VkDescriptorPoolSize descriptorPoolSizes[2];
	descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSizes[0].descriptorCount = SwapChain::GetImageCount();
	descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorPoolSizes[1].descriptorCount = SwapChain::GetImageCount();

	VkDescriptorPoolCreateInfo poolCreateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	poolCreateInfo.poolSizeCount = 2;
	poolCreateInfo.pPoolSizes = descriptorPoolSizes;
	poolCreateInfo.maxSets = SwapChain::GetImageCount();
	
	VK_ASSERT(vkCreateDescriptorPool(VulkanCore::GetDevice(), &poolCreateInfo, nullptr, &m_DescriptorPool) == VK_SUCCESS, "Cant vkCreateDescriptorSetLayout!");
	
}

void UniformBuffer::createSets(const Texture& texture)
{
	std::vector<VkDescriptorSetLayout> layouts(SwapChain::GetImageCount(), m_DescriptorSetLayout);
	
	VkDescriptorSetAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocateInfo.descriptorPool = m_DescriptorPool;
	allocateInfo.descriptorSetCount = layouts.size();
	allocateInfo.pSetLayouts = layouts.data();

	m_DescriptorSets.resize(SwapChain::GetImageCount());
	VK_ASSERT(vkAllocateDescriptorSets(VulkanCore::GetDevice(), &allocateInfo, m_DescriptorSets.data()) == VK_SUCCESS, "Cant vkAllocateDescriptorSets!");

	for (size_t i = 0; i < m_DescriptorSets.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_Buffers.at(i);
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(Transformation);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = texture.GetImageView();
		imageInfo.sampler = texture.GetSampler();

		VkWriteDescriptorSet writeDescriptors[2] = {};
		writeDescriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptors[0].pBufferInfo = &bufferInfo;
		writeDescriptors[0].dstSet = m_DescriptorSets.at(i);
		writeDescriptors[0].dstBinding = 0;
		writeDescriptors[0].dstArrayElement = 0;
		writeDescriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptors[0].descriptorCount = 1;

		writeDescriptors[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptors[1].pImageInfo = &imageInfo;
		writeDescriptors[1].dstSet = m_DescriptorSets.at(i);
		writeDescriptors[1].dstBinding = 1;
		writeDescriptors[1].dstArrayElement = 0;
		writeDescriptors[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptors[1].descriptorCount = 1;

		vkUpdateDescriptorSets(VulkanCore::GetDevice(), 2, writeDescriptors, 0, nullptr);
	}
}

void UniformBuffer::createLayout()
{
	VkDescriptorSetLayoutBinding settingLayout{};
	settingLayout.binding = 0;
	settingLayout.descriptorCount = 1;
	settingLayout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	settingLayout.pImmutableSamplers = nullptr;
	settingLayout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayout{};
	samplerLayout.binding = 1;
	samplerLayout.descriptorCount = 1;
	samplerLayout.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayout.pImmutableSamplers = nullptr;
	samplerLayout.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding bindings[] = { settingLayout, samplerLayout };

	VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	setLayoutCreateInfo.bindingCount = 2;
	setLayoutCreateInfo.pBindings = bindings;

	VK_ASSERT(vkCreateDescriptorSetLayout(VulkanCore::GetDevice(), &setLayoutCreateInfo, nullptr, &m_DescriptorSetLayout) == VK_SUCCESS, "Cant vkCreateDescriptorSetLayout!");
}

void UniformBuffer::createBuffer(const Transformation& transform)
{
	auto size = sizeof(transform);
	
	m_Buffers.resize(SwapChain::GetImageCount());
	m_BufferMemories.resize(SwapChain::GetImageCount());

	for (size_t i = 0; i < m_Buffers.size(); i++)
	{
		VkBufferCreateInfo bufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_ASSERT(vkCreateBuffer(VulkanCore::GetDevice(), &bufferCreateInfo, nullptr, &m_Buffers[i]) == VK_SUCCESS, "Cant vkCreateBuffer!");

		VkMemoryRequirements bufferMemoryRequriment;
		vkGetBufferMemoryRequirements(VulkanCore::GetDevice(), m_Buffers[i], &bufferMemoryRequriment);

		VkMemoryAllocateInfo bufferMemoryAllocate{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		bufferMemoryAllocate.allocationSize = bufferMemoryRequriment.size;
		bufferMemoryAllocate.memoryTypeIndex = getMemoryType(bufferMemoryRequriment.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VK_ASSERT(vkAllocateMemory(VulkanCore::GetDevice(), &bufferMemoryAllocate, nullptr, &m_BufferMemories[i]) == VK_SUCCESS, "Cant vkAllocateMemory!");

		vkBindBufferMemory(VulkanCore::GetDevice(), m_Buffers[i], m_BufferMemories[i], 0);
	}

}

void UniformBuffer::createBuffer(const glm::mat4& matrix)
{
	auto size = sizeof(matrix);

	m_Buffers.resize(SwapChain::GetImageCount());
	m_BufferMemories.resize(SwapChain::GetImageCount());

	for (size_t i = 0; i < m_Buffers.size(); i++)
	{
		VkBufferCreateInfo bufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_ASSERT(vkCreateBuffer(VulkanCore::GetDevice(), &bufferCreateInfo, nullptr, &m_Buffers[i]) == VK_SUCCESS, "Cant vkCreateBuffer!");

		VkMemoryRequirements bufferMemoryRequriment;
		vkGetBufferMemoryRequirements(VulkanCore::GetDevice(), m_Buffers[i], &bufferMemoryRequriment);

		VkMemoryAllocateInfo bufferMemoryAllocate{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		bufferMemoryAllocate.allocationSize = bufferMemoryRequriment.size;
		bufferMemoryAllocate.memoryTypeIndex = getMemoryType(bufferMemoryRequriment.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VK_ASSERT(vkAllocateMemory(VulkanCore::GetDevice(), &bufferMemoryAllocate, nullptr, &m_BufferMemories[i]) == VK_SUCCESS, "Cant vkAllocateMemory!");

		vkBindBufferMemory(VulkanCore::GetDevice(), m_Buffers[i], m_BufferMemories[i], 0);
	}
}

uint32_t UniformBuffer::getMemoryType(uint32_t type, VkMemoryPropertyFlags propertyFlags)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(VulkanCore::GetPhysicalDevice(), &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((type & (1 << i)) && ((memoryProperties.memoryTypes[i].propertyFlags & propertyFlags)))
			return i;
	}

	VK_ASSERT(false, "Cant find required memory flags!");
}