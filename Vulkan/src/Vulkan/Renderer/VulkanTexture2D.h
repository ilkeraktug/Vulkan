#pragma once

#include "Abstract/Texture.h"

class VulkanTexture2D : public Texture
{
public:
	VulkanTexture2D() = default;
	VulkanTexture2D(const std::string& filepath, VulkanCore* core);
	VulkanTexture2D(void* data, uint32_t width, uint32_t height, VulkanCore* core);

	virtual ~VulkanTexture2D();
private:
	virtual void createImageView() override;
	virtual void createImageSampler() override;
};