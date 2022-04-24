#pragma once

#include "Vulkan/Objects/QuadObj.h"

#include "Vulkan/Renderer/VulkanTexture2D.h"

class PipeObject : public QuadObj
{
public:
	PipeObject() = default;
	PipeObject(const std::string& texturePath, VulkanCore* core);

	virtual ~PipeObject() {};
public:

	std::unique_ptr<VulkanTexture2D> Texture;

	float HeightScale = 3.0f;
	float PipeGap = 0.05f;

	float worldPosX;
	float worldPosY;
	float worldScale;

private:

};