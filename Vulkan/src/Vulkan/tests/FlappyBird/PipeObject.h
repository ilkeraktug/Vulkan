#pragma once

#include "Vulkan/Objects/QuadObj.h"

#include "Vulkan/Renderer/VulkanTexture2D.h"

class PipeObject : public QuadObj
{
public:
	PipeObject() = default;
	PipeObject(const std::string& texturePath, VulkanCore* core);

	virtual ~PipeObject() {};

	std::unique_ptr<VulkanTexture2D> Texture;
private:

};