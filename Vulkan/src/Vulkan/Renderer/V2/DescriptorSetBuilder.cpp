#include "pch.h"
#include "DescriptorSetBuilder.h"

#include <vulkan/vulkan_core.h>

DescriptorSetBuilder* DescriptorSetBuilder::s_Instance = new DescriptorSetBuilder();

DescriptorSetBuilder& DescriptorSetBuilder::Begin(VkDevice Device)
{
    m_Device = Device;

    m_DescriptorSets.clear();
    m_WriteDescriptorSets.clear();
    m_PoolSizes.clear();
    m_LayoutBindings.clear();
    m_ImageInfos.clear();
    m_BufferInfos.clear();

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

DescriptorSetBuilder& DescriptorSetBuilder::CreateDescriptorPool(VkDescriptorPool& pool)
{
    return CreateDescriptorPool(m_PoolSizes, pool);
}

DescriptorSetBuilder& DescriptorSetBuilder::CreateDescriptorPool(const std::vector<VkDescriptorPoolSize>& poolSize)
{
    VkDescriptorPoolCreateInfo poolCI{};
    poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCI.maxSets = 100;
    poolCI.poolSizeCount = static_cast<uint32_t>(poolSize.size());
    poolCI.pPoolSizes = poolSize.data();

    VK_CHECK(vkCreateDescriptorPool(m_Device, &poolCI, nullptr, &m_DescriptorPool));

    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::CreateDescriptorPool(const std::vector<struct VkDescriptorPoolSize>& poolSize, VkDescriptorPool& pool)
{
    VkDescriptorPoolCreateInfo poolCI{};
    poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCI.maxSets = 100;
    poolCI.poolSizeCount = static_cast<uint32_t>(poolSize.size());
    poolCI.pPoolSizes = poolSize.data();

    VK_CHECK(vkCreateDescriptorPool(m_Device, &poolCI, nullptr, &m_DescriptorPool));

    pool = m_DescriptorPool;

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

DescriptorSetBuilder& DescriptorSetBuilder::ResetDescriptorLayoutBinding()
{
    m_LayoutBindings.clear();
    
    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::CreateDescriptorLayout(VkDescriptorSetLayout& outLayout)
{
    CreateDescriptorLayout(m_LayoutBindings, outLayout);
    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::CreateDescriptorLayout(std::vector<VkDescriptorSetLayout>& outLayout)
{
    VkDescriptorSetLayout layout;
    CreateDescriptorLayout(m_LayoutBindings, layout);
    outLayout.push_back(layout);
    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::CreateDescriptorLayout(const std::vector<VkDescriptorSetLayoutBinding>& layoutBinding, VkDescriptorSetLayout& outLayout)
{
    VkDescriptorSetLayout& layout = m_DescriptorSetLayouts.emplace_back();
    
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
    descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(layoutBinding.size());
    descriptorSetLayoutCI.pBindings = layoutBinding.data();
    
     VK_CHECK(vkCreateDescriptorSetLayout(m_Device, &descriptorSetLayoutCI, nullptr, &layout));

    outLayout = layout;
    
    
    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::CreateDescriptorLayout(const std::vector<VkDescriptorSetLayoutBinding>& layoutBinding, std::vector<VkDescriptorSetLayout>& outLayout)
{
    VkDescriptorSetLayout& layout = m_DescriptorSetLayouts.emplace_back();
    
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
    descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(layoutBinding.size());
    descriptorSetLayoutCI.pBindings = layoutBinding.data();
    
    VK_CHECK(vkCreateDescriptorSetLayout(m_Device, &descriptorSetLayoutCI, nullptr, &layout));

    outLayout.push_back(layout);
    
    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::AllocateDescriptorSet(VkDescriptorSet& outDescriptorSet)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_DescriptorSetLayouts.back();

    VkDescriptorSet& descriptorSet = m_DescriptorSets.emplace_back();
    VK_CHECK(vkAllocateDescriptorSets(m_Device, &allocInfo, &descriptorSet));
    
    outDescriptorSet = descriptorSet;

    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::AllocateDescriptorSet(uint32_t set, VkDescriptorSet& outDescriptorSet)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_DescriptorSetLayouts[set];

    VkDescriptorSet& descriptorSet = m_DescriptorSets.emplace_back();
    VK_CHECK(vkAllocateDescriptorSets(m_Device, &allocInfo, &descriptorSet));
    
    outDescriptorSet = descriptorSet;

    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::AddImageInfo(uint32_t binding, VkDescriptorImageInfo* imageInfo)
{
    if(imageInfo)
    {
        AddImageInfo(binding, imageInfo->imageView, imageInfo->imageLayout, imageInfo->sampler);
    }

    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::AddImageInfo(uint32_t binding, VkImageView imageView, VkImageLayout layout, VkSampler sampler /*= VK_NULL_HANDLE*/)
{
    std::shared_ptr imageInfo = m_ImageInfos.emplace_back(std::make_shared<VkDescriptorImageInfo>());
    imageInfo->sampler = sampler;
    imageInfo->imageView = imageView;
    imageInfo->imageLayout = layout;

    VkWriteDescriptorSet set{};
    set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    set.dstSet = m_DescriptorSets[m_DescriptorSets.size() - 1];
    set.dstBinding = binding;
    set.descriptorCount = 1;
    set.descriptorType = sampler == VK_NULL_HANDLE ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    set.pImageInfo = m_ImageInfos[m_ImageInfos.size() - 1].get();

    m_WriteDescriptorSets.push_back(set);

    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::AddBufferInfo(uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorType bufferType /*= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER*/)
{
    if(bufferInfo)
    {
        AddBufferInfo(binding, bufferInfo->buffer, bufferType, bufferInfo->offset, bufferInfo->range);
    }

    return *this;
}

DescriptorSetBuilder& DescriptorSetBuilder::AddBufferInfo(uint32_t binding, VkBuffer buffer, VkDescriptorType bufferType /*= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER*/, VkDeviceSize offset /*= 0*/, VkDeviceSize range /*= VK_WHOLE_SIZE*/)
{
    std::shared_ptr bufferInfo = m_BufferInfos.emplace_back(std::make_shared<VkDescriptorBufferInfo>());
    bufferInfo->buffer = buffer;
    bufferInfo->offset = offset;
    bufferInfo->range = range;

    VkWriteDescriptorSet set{};
    set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    set.dstSet = m_DescriptorSets[m_DescriptorSets.size() - 1];
    set.dstBinding = binding;
    set.descriptorCount = 1;
    set.descriptorType = bufferType;
    set.pBufferInfo = m_BufferInfos[m_BufferInfos.size() - 1].get();

    m_WriteDescriptorSets.push_back(set);

    return *this;
}


DescriptorSetBuilder& DescriptorSetBuilder::UpdateDescriptorSet()
{
    vkUpdateDescriptorSets(m_Device, m_WriteDescriptorSets.size(), m_WriteDescriptorSets.data(), 0, nullptr);

    return ResetDescriptorWrite();
}

DescriptorSetBuilder& DescriptorSetBuilder::ResetDescriptorWrite()
{
    m_WriteDescriptorSets.clear();

    return *this;
}

VkDescriptorSet DescriptorSetBuilder::AllocateDescriptorSet(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout* layout)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layout;

    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

    return descriptorSet;
}
