#pragma once

#include "stb_image.h"

#include "VulkanCore.h"

class Texture
{
public:
	Texture() = default;
	Texture(const std::string& filepath);
	~Texture();

	inline const VkImageView& GetImageView() const { return m_ImageView; }
	inline const VkSampler& GetSampler() const { return m_Sampler; }

private:
	void createBuffer(stbi_uc* pixels);
	void createImage();
	void createTransition(VkImageLayout oldLayout, VkImageLayout newLayout);
	void createImageView();
	void createSampler();

	uint32_t getMemoryType(uint32_t type, VkMemoryPropertyFlags propertyFlags);
private:
	uint32_t m_Width, m_Height, m_Channel;
	VkDeviceSize m_Size;

	VkImage m_Image;
	VkDeviceMemory m_ImageMemory;
	VkImageView m_ImageView;
	VkSampler m_Sampler;


	VkImageSubresourceRange m_SubresourceRange;
};