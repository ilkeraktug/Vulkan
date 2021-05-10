#include "pch.h"
#include "VulkanTexture2D.h"

#include <stb_image.h>

VulkanTexture2D::VulkanTexture2D(const std::string& filepath, VulkanCore* core)
{
	Init(core);

	int width, height, channel;

	stbi_uc* textureData = stbi_load(filepath.c_str(), &width, &height, &channel, STBI_rgb_alpha);

	VK_ASSERT(width > 0, "Cant load texture at : {0}", filepath.c_str());

	m_Width = static_cast<uint32_t>(width);
	m_Height = static_cast<uint32_t>(height);
	m_Channel = static_cast<uint32_t>(channel);

	m_Size = m_Width * m_Height * 4;

	VkBuffer stagingBuffer;
	VkDeviceMemory stageBufferMemory;

	VkBufferCreateInfo stageBufferCI = init::createBufferInfo(m_Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	stageBufferCI.size = m_Size;
	stageBufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stageBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK(vkCreateBuffer(m_Core->GetDevice(), &stageBufferCI, nullptr, &stagingBuffer));

	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(m_Core->GetDevice(), stagingBuffer, &memReq);

	VkMemoryAllocateInfo memAllocInfo = init::memAllocInfo();
	memAllocInfo.allocationSize = memReq.size;
	memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &memAllocInfo, nullptr, &stageBufferMemory));

	VK_CHECK(vkBindBufferMemory(m_Core->GetDevice(), stagingBuffer, stageBufferMemory, 0));

	void* data;
	VK_CHECK(vkMapMemory(m_Core->GetDevice(), stageBufferMemory, 0, m_Size, 0, &data));
	memcpy(data, textureData, m_Size);
	vkUnmapMemory(m_Core->GetDevice(), stageBufferMemory);

	stbi_image_free(textureData);

	VkImageCreateInfo imageCI = init::imageCreateInfo();
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.format = m_Format;
	imageCI.extent = { m_Width, m_Height, 1 };
	imageCI.mipLevels = 1;
	imageCI.arrayLayers = 1;
	imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VK_CHECK(vkCreateImage(m_Core->GetDevice(), &imageCI, nullptr, &m_Image));

	vkGetImageMemoryRequirements(m_Core->GetDevice(), m_Image, &memReq);
	memAllocInfo.allocationSize = memReq.size;
	memAllocInfo.memoryTypeIndex = m_Core->getMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_CHECK(vkAllocateMemory(m_Core->GetDevice(), &memAllocInfo, nullptr, &m_ImageMemory));
	VK_CHECK(vkBindImageMemory(m_Core->GetDevice(), m_Image, m_ImageMemory, 0));

	VkCommandBuffer copyCommandBuffer = m_Core->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	VkImageSubresourceRange imageSubresource{};
	imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSubresource.baseMipLevel = 0;
	imageSubresource.levelCount = 1;
	imageSubresource.baseArrayLayer = 0;
	imageSubresource.layerCount = 1;

	VkImageMemoryBarrier imageMemoryBarrier = init::imageMemoryBarrier();
	imageMemoryBarrier.srcAccessMask = 0;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarrier.image = m_Image;
	imageMemoryBarrier.subresourceRange = imageSubresource;

	vkCmdPipelineBarrier(copyCommandBuffer,
		VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
		0, 
		0, nullptr, 
		0, nullptr, 
		1, &imageMemoryBarrier);

	VkBufferImageCopy bufferCopyRegion{};
	bufferCopyRegion.bufferOffset = 0;
	bufferCopyRegion.imageExtent = { m_Width, m_Height, 1 };
	bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
	bufferCopyRegion.imageSubresource.layerCount = 1;
	bufferCopyRegion.imageSubresource.mipLevel = 0;

	vkCmdCopyBufferToImage(copyCommandBuffer, 
		stagingBuffer, m_Image, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
		1, &bufferCopyRegion);

	imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkCmdPipelineBarrier(copyCommandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier);

	VK_CHECK(vkEndCommandBuffer(copyCommandBuffer));

	VkSubmitInfo submitInfo = init::submitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCommandBuffer;

	VK_CHECK(vkQueueSubmit(m_Core->queue.GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
	vkQueueWaitIdle(m_Core->queue.GraphicsQueue);

	vkDestroyBuffer(m_Core->GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(m_Core->GetDevice(), stageBufferMemory, nullptr);
	vkFreeCommandBuffers(m_Core->GetDevice(), m_Core->resources.commandPool, 1, &copyCommandBuffer);

	createImageView();
	createImageSampler();
	updateDescriptor();
}

VulkanTexture2D::~VulkanTexture2D()
{
}

void VulkanTexture2D::createImageView()
{
	VkImageViewCreateInfo imageViewCI = init::imageViewCreateInfo();
	imageViewCI.image = m_Image;
	imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCI.format = m_Format;
	imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCI.subresourceRange.levelCount = 1;
	imageViewCI.subresourceRange.baseMipLevel = 0;
	imageViewCI.subresourceRange.layerCount = 1;
	imageViewCI.subresourceRange.baseArrayLayer = 0;

	VK_CHECK(vkCreateImageView(m_Core->GetDevice(), &imageViewCI, nullptr, &m_ImageView));
}

void VulkanTexture2D::createImageSampler()
{
	VkSamplerCreateInfo samplerCreateInfo = init::samplerCreateInfo();
	samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
	samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;
	samplerCreateInfo.maxAnisotropy = m_Core->GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;

	VK_CHECK(vkCreateSampler(m_Core->GetDevice(), &samplerCreateInfo, nullptr, &m_ImageSampler));
}
