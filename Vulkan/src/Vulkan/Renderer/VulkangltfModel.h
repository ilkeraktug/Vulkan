#pragma once
/*
#include <stdlib.h>
#include <string>
#include <fstream>
#include <vector>

#include "vulkan/vulkan.h"
#include "VulkanCore.h"

#include <ktx.h>
#include <ktxvulkan.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "tiny_gltf.h"

#include "VulkanCore.h"

namespace vkglTF
{
    enum DescriptorBindingFlags
    {
        ImageBaseColor = BIT(0),
        ImageNormalMap = BIT(1)
    };

    struct Texture
    {
        VulkanCore* core;
        
        VkImage image;
        VkImageView imageView;
        VkDeviceMemory imageMemory;

        VkImageLayout imageLayout;

        uint32_t width, height;
        uint32_t layerCount;
        uint32_t mipLevels;

        VkDescriptorImageInfo descriptor;
        VkSampler imageSampler;

        void updateDescriptor();
        void destroy();
        void fromglTfImage(tinygltf::Image& gltfimage, std::string path, VulkanCore* core, VkQueue copyQueue);
    };
    
    class VulkangltfModel
    {
    public:
    
    };
}
*/