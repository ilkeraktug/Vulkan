#include "pch.h"
#include "VulkanImage2D.h"

namespace V2
{
    VulkanImage2D::VulkanImage2D()
    {
        imageType = VK_IMAGE_TYPE_2D;
        viewType = VK_IMAGE_VIEW_TYPE_2D;
    }
}
