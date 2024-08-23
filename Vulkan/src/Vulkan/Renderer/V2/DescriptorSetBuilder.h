#pragma once
#include "VulkanBufferDeviceAddress.h"

class DescriptorSetBuilder;

class DescriptorSetBuilderProject
{
    DescriptorSetBuilderProject(const DescriptorSetBuilder& builder);
    
private:
    VkDevice m_Device = VK_NULL_HANDLE;

    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;

    std::vector<VkDescriptorSet> m_DescriptorSets;
    std::vector<VkWriteDescriptorSet> m_WriteDescriptorSets;

    std::vector<struct VkDescriptorPoolSize> m_PoolSizes;
    std::vector<struct VkDescriptorSetLayoutBinding> m_LayoutBindings;
    
    std::vector<std::shared_ptr<VkDescriptorImageInfo>> m_ImageInfos;
    std::vector<std::shared_ptr<VkDescriptorBufferInfo>> m_BufferInfos;
};

class DescriptorSetBuilder
{
public:
    
    static DescriptorSetBuilder& Get()
    {
        if(s_Instance == nullptr)
        {
            s_Instance = new DescriptorSetBuilder();
        }

        return *s_Instance;
    }
    
    DescriptorSetBuilder() = default;

    DescriptorSetBuilder& Begin(VkDevice Device);
    
    DescriptorSetBuilder& AddDescriptorPoolSize(VkDescriptorType type, uint32_t count);
    DescriptorSetBuilder& CreateDescriptorPool();
    DescriptorSetBuilder& CreateDescriptorPool(VkDescriptorPool& pool);
    DescriptorSetBuilder& CreateDescriptorPool(const std::vector<struct VkDescriptorPoolSize>& poolSize);
    DescriptorSetBuilder& CreateDescriptorPool(const std::vector<struct VkDescriptorPoolSize>& poolSize, VkDescriptorPool& pool);

    DescriptorSetBuilder& AddDescriptorLayoutBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags shaderStage);
    DescriptorSetBuilder& ResetDescriptorLayoutBinding();
    DescriptorSetBuilder& CreateDescriptorLayout(VkDescriptorSetLayout& outLayout);
    DescriptorSetBuilder& CreateDescriptorLayout(std::vector<VkDescriptorSetLayout>& outLayout);
    DescriptorSetBuilder& CreateDescriptorLayout(const std::vector<VkDescriptorSetLayoutBinding>& layoutBinding, VkDescriptorSetLayout& outLayout);
    DescriptorSetBuilder& CreateDescriptorLayout(const std::vector<VkDescriptorSetLayoutBinding>& layoutBinding, std::vector<VkDescriptorSetLayout>& outLayout);
    
    DescriptorSetBuilder& AllocateDescriptorSet(VkDescriptorSet& outDescriptorSet);
    DescriptorSetBuilder& AllocateDescriptorSet(uint32_t set, VkDescriptorSet& outDescriptorSet);

    DescriptorSetBuilder& AddImageInfo(uint32_t binding, VkDescriptorImageInfo* imageInfo);
    DescriptorSetBuilder& AddImageInfo(uint32_t binding, VkImageView imageView, VkImageLayout layout, VkSampler sampler = VK_NULL_HANDLE);
    DescriptorSetBuilder& AddBufferInfo(uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorType bufferType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    DescriptorSetBuilder& AddBufferInfo(uint32_t binding, VkBuffer buffer, VkDescriptorType bufferType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE);
    DescriptorSetBuilder& UpdateDescriptorSet();
    
    DescriptorSetBuilder& ResetDescriptorWrite();

    static VkDescriptorSet AllocateDescriptorSet(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout* layout);

private:
    VkDevice m_Device = VK_NULL_HANDLE;

    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;

    std::vector<VkDescriptorSet> m_DescriptorSets;
    std::vector<VkWriteDescriptorSet> m_WriteDescriptorSets;

    std::vector<struct VkDescriptorPoolSize> m_PoolSizes;
    std::vector<struct VkDescriptorSetLayoutBinding> m_LayoutBindings;
    
    std::vector<std::shared_ptr<VkDescriptorImageInfo>> m_ImageInfos;
    std::vector<std::shared_ptr<VkDescriptorBufferInfo>> m_BufferInfos;

    static DescriptorSetBuilder* s_Instance;
};
