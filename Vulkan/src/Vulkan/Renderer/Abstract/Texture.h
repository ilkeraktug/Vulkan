#pragma once

#include "Vulkan/Core/init.h"
#include "Vulkan/Renderer/VulkanCore.h"

class Texture
{
public:

	virtual ~Texture()
	{
		vkDestroySampler(m_Core->GetDevice(), m_ImageSampler, nullptr);
		vkDestroyImageView(m_Core->GetDevice(), m_ImageView, nullptr);
		vkFreeMemory(m_Core->GetDevice(), m_ImageMemory, nullptr);
		vkDestroyImage(m_Core->GetDevice(), m_Image, nullptr);
	}

	void Init(VulkanCore* core) { m_Core = core; }

	VkDescriptorImageInfo descriptor;
protected:
	virtual void createImageView() = 0;
	virtual void createImageSampler() = 0;

	void updateDescriptor()
	{
		descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptor.imageView = m_ImageView;
		descriptor.sampler = m_ImageSampler;
	}
protected:

	VkImage m_Image;
	VkDeviceMemory m_ImageMemory;
	VkDeviceSize m_Size;
	VkImageView m_ImageView;
	VkFormat m_Format = VK_FORMAT_R8G8B8A8_UNORM;
	VkSampler m_ImageSampler;

	uint32_t m_Width, m_Height, m_Channel;

	VulkanCore* m_Core;
private:
};