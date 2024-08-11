#pragma once
#include "VulkanBufferDeviceAddress.h"

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
    DescriptorSetBuilder& CreateDescriptorPool(const std::vector<struct VkDescriptorPoolSize>& poolSize);

    DescriptorSetBuilder& AddDescriptorLayoutBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags shaderStage);
    DescriptorSetBuilder& CreateDescriptorLayout();
    DescriptorSetBuilder& CreateDescriptorLayout(const std::vector<VkDescriptorSetLayoutBinding>& layoutBinding);
    DescriptorSetBuilder& AllocateDescriptorSet(VkDescriptorSet& outDescriptorSet);

    DescriptorSetBuilder& AddImageInfo(uint32_t binding, VkImageView imageView, VkImageLayout layout, VkSampler sampler = VK_NULL_HANDLE);
    DescriptorSetBuilder& AddBufferInfo(uint32_t binding, VkBuffer buffer, VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE);
    DescriptorSetBuilder& UpdateDescriptorSet();

private:
    VkDevice m_Device = VK_NULL_HANDLE;

    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;

    std::vector<VkDescriptorSet> m_DescriptorSets;
    std::vector<VkWriteDescriptorSet> m_WriteDescriptorSets;

    std::vector<struct VkDescriptorPoolSize> m_PoolSizes;
    std::vector<struct VkDescriptorSetLayoutBinding> m_LayoutBindings;
    
    std::vector<VkDescriptorImageInfo> m_ImageInfos;
    std::vector<VkDescriptorBufferInfo> m_BufferInfos;

    static DescriptorSetBuilder* s_Instance;
};
