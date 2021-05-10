#pragma once

#include "Vulkan/Objects/QuadObj.h"

#include "Vulkan/Renderer/VulkanTexture2D.h"

class BirdObject : public QuadObj
{
public:
	BirdObject() = default;
	BirdObject(VulkanCore* core);

	virtual ~BirdObject();

	std::unique_ptr<VulkanTexture2D> Texture;
private:

};