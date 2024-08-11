#include "pch.h"
#include "DescriptorSetBuilder.h"

#include <vulkan/vulkan_core.h>

DescriptorSetBuilder* DescriptorSetBuilder::s_Instance = new DescriptorSetBuilder();

DescriptorSetBuilder& DescriptorSetBuilder::Begin(VkDevice Device)
{
    m_Device = Device;

    m_PoolSizes.clear();
    m_LayoutBindings.clear();

    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::AddDescriptorPoolSize(VkDescriptorType type, uint32_t count)
{
    m_PoolSizes.push_back({type, count});

    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::CreateDescriptorPool()
{
    return CreateDescriptorPool(m_PoolSizes);
}

DescriptorSetBuilder& DescriptorSetBuilder::CreateDescriptorPool(const std::vector<struct VkDescriptorPoolSize>& poolSize)
{
    VkDescriptorPoolCreateInfo poolCI{};
    poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCI.maxSets = 100;
    poolCI.poolSizeCount = static_cast<uint32_t>(poolSize.size());
    poolCI.pPoolSizes = poolSize.data();

    VK_CHECK(vkCreateDescriptorPool(m_Device, &poolCI, nullptr, &m_DescriptorPool));

    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::AddDescriptorLayoutBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags shaderStage)
{
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = type;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = shaderStage;

    m_LayoutBindings.push_back(layoutBinding);
    
    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::CreateDescriptorLayout()
{
    CreateDescriptorLayout(m_LayoutBindings);
    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::CreateDescriptorLayout(const std::vector<VkDescriptorSetLayoutBinding>& layoutBinding)
{
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
    descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(layoutBinding.size());
    descriptorSetLayoutCI.pBindings = layoutBinding.data();
    
     VK_CHECK(vkCreateDescriptorSetLayout(m_Device, &descriptorSetLayoutCI, nullptr, &m_DescriptorSetLayout));
    
    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::AllocateDescriptorSet(VkDescriptorSet& outDescriptorSet)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_DescriptorSetLayout;

    VkDescriptorSet& descriptorSet = m_DescriptorSets.emplace_back();
    outDescriptorSet = descriptorSet;
    
    VK_CHECK(vkAllocateDescriptorSets(m_Device, &allocInfo, &descriptorSet));

    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::AddImageInfo(uint32_t binding, VkImageView imageView, VkImageLayout layout, VkSampler sampler /*= VK_NULL_HANDLE*/)
{
    VkDescriptorImageInfo& imageInfo = m_ImageInfos.emplace_back();
    imageInfo.sampler = sampler;
    imageInfo.imageView = imageView;
    imageInfo.imageLayout = layout;

    VkWriteDescriptorSet set{};
    set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    set.dstSet = *m_DescriptorSets.end();
    set.dstBinding = binding;
    set.descriptorCount = 1;
    set.descriptorType = sampler == VK_NULL_HANDLE ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    set.pImageInfo = &imageInfo;

    m_WriteDescriptorSets.push_back(set);

    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::AddBufferInfo(uint32_t binding, VkBuffer buffer, VkDeviceSize offset /*= 0*/, VkDeviceSize range /*= VK_WHOLE_SIZE*/)
{
    VkDescriptorBufferInfo& bufferInfo = m_BufferInfos.emplace_back();
    bufferInfo.buffer = buffer;
    bufferInfo.offset = offset;
    bufferInfo.range = range;

    VkWriteDescriptorSet set{};
    set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    set.dstSet = *m_DescriptorSets.end();
    set.dstBinding = binding;
    set.descriptorCount = 1;
    set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    set.pBufferInfo = &bufferInfo;

    m_WriteDescriptorSets.push_back(set);

    return *this;
}


DescriptorSetBuilder& DescriptorSetBuilder::UpdateDescriptorSet()
{
    vkUpdateDescriptorSets(m_Device, m_WriteDescriptorSets.size(), m_WriteDescriptorSets.data(), 0, nullptr);

    return *this;
}
