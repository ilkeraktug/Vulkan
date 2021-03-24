#include "pch.h"

#include "Texture.h"
#include "SwapChain.h"

Texture::Texture(const std::string& filepath)
{
	int width, height, channel;

	stbi_uc* data = stbi_load(filepath.c_str(), &width, &height, &channel, STBI_rgb_alpha);

	if (!data)
	{
		VK_ERROR("Cant load file {0}", filepath);
		VK_ASSERT(false, "..");
	}

	m_Width = width;
	m_Height = height;
	m_Channel = channel;

	m_Size = m_Width * m_Height * 4;

	m_SubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	m_SubresourceRange.baseArrayLayer = 0;
	m_SubresourceRange.baseMipLevel = 0;
	m_SubresourceRange.layerCount = 1;
	m_SubresourceRange.levelCount = 1;

	createBuffer(data);
	stbi_image_free(data);

	createImageView();
	createSampler();
}

Texture::~Texture()
{
	vkDestroySampler(VulkanCore::GetDevice(), m_Sampler, nullptr);
	vkDestroyImageView(VulkanCore::GetDevice(), m_ImageView, nullptr);
	vkDestroyImage(VulkanCore::GetDevice(), m_Image, nullptr);

	vkFreeMemory(VulkanCore::GetDevice(), m_ImageMemory, nullptr);
}

void Texture::createBuffer(stbi_uc* pixels)
{
	VkBuffer stageBuffer;
	VkDeviceMemory stageMemory;

	VkBufferCreateInfo stageBufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	stageBufferCreateInfo.size = m_Size;
	stageBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stageBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_ASSERT(vkCreateBuffer(VulkanCore::GetDevice(), &stageBufferCreateInfo, nullptr, &stageBuffer) == VK_SUCCESS, "Cant vkCreateBuffer!");

	VkMemoryRequirements memoryRequirement;
	vkGetBufferMemoryRequirements(VulkanCore::GetDevice(), stageBuffer, &memoryRequirement);

	VkMemoryAllocateInfo memoryAllocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	memoryAllocateInfo.allocationSize = memoryRequirement.size;
	memoryAllocateInfo.memoryTypeIndex = getMemoryType(memoryRequirement.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VK_ASSERT(vkAllocateMemory(VulkanCore::GetDevice(), &memoryAllocateInfo, nullptr, &stageMemory) == VK_SUCCESS, "Cant vkAllocateMemory!");

	vkBindBufferMemory(VulkanCore::GetDevice(), stageBuffer, stageMemory, 0);

	void* data;
	vkMapMemory(VulkanCore::GetDevice(), stageMemory, 0, m_Size, 0, &data);
	memcpy(data, pixels, m_Size);
	vkUnmapMemory(VulkanCore::GetDevice(), stageMemory);

	createImage();
	createTransition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkCommandBuffer commandBuffer = VulkanCore::BeginSingleCommandBuffer();

	VkBufferImageCopy imageCopyRegion{};
	imageCopyRegion.bufferOffset = 0;
	imageCopyRegion.bufferRowLength = 0;
	imageCopyRegion.bufferImageHeight = 0;
	imageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageCopyRegion.imageSubresource.mipLevel = 0;
	imageCopyRegion.imageSubresource.baseArrayLayer = 0;
	imageCopyRegion.imageSubresource.layerCount = 1;
	imageCopyRegion.imageOffset = { 0, 0, 0 };
	imageCopyRegion.imageExtent = { m_Width, m_Height, 1 };

	vkCmdCopyBufferToImage(commandBuffer, stageBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);

	VulkanCore::EndSingleCommandBuffer(commandBuffer);

	createTransition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(VulkanCore::GetDevice(), stageBuffer, nullptr);
	vkFreeMemory(VulkanCore::GetDevice(), stageMemory, nullptr);
}

void Texture::createImage()
{
	VkImageCreateInfo imageCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageCreateInfo.extent = { m_Width, m_Height, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	
	VK_ASSERT(vkCreateImage(VulkanCore::GetDevice(), &imageCreateInfo, nullptr, &m_Image) == VK_SUCCESS, "Cant vkCreateImage!");

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(VulkanCore::GetDevice(), m_Image, &memoryRequirements);

	VkMemoryAllocateInfo imageMemoryAllocateMemory{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	imageMemoryAllocateMemory.allocationSize = memoryRequirements.size;
	imageMemoryAllocateMemory.memoryTypeIndex = getMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_ASSERT(vkAllocateMemory(VulkanCore::GetDevice(), &imageMemoryAllocateMemory, nullptr, &m_ImageMemory) == VK_SUCCESS, "Cant vkAllocateMemory!");

	vkBindImageMemory(VulkanCore::GetDevice(), m_Image, m_ImageMemory, 0);
}

void Texture::createTransition(VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	VkImageMemoryBarrier imageMemoryBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.srcQueueFamilyIndex = VulkanCore::GetQueueIndices().TransferIndex.value();
	imageMemoryBarrier.dstQueueFamilyIndex = VulkanCore::GetQueueIndices().TransferIndex.value();
	imageMemoryBarrier.image = m_Image;
	imageMemoryBarrier.subresourceRange = m_SubresourceRange;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}


	imageMemoryBarrier.srcAccessMask = 0;
	imageMemoryBarrier.dstAccessMask = 0;

	VkCommandBuffer commandBuffer = VulkanCore::BeginSingleCommandBuffer();
	
	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

	VulkanCore::EndSingleCommandBuffer(commandBuffer);
}

void Texture::createImageView()
{
	VkImageViewCreateInfo imageViewCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	imageViewCreateInfo.subresourceRange = m_SubresourceRange;
	imageViewCreateInfo.image = m_Image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;

	VK_ASSERT(vkCreateImageView(VulkanCore::GetDevice(), &imageViewCreateInfo, nullptr, &m_ImageView) == VK_SUCCESS, "Cant vkCreateImageView!");
}

void Texture::createSampler()
{
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(VulkanCore::GetPhysicalDevice(), &properties);

	VkSamplerCreateInfo samplerCreateInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;
	samplerCreateInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;

	VK_ASSERT(vkCreateSampler(VulkanCore::GetDevice(), &samplerCreateInfo, nullptr, &m_Sampler) == VK_SUCCESS, "Cant vkCreateImageView!");

}

uint32_t Texture::getMemoryType(uint32_t type, VkMemoryPropertyFlags propertyFlags)
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
