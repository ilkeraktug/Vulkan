#pragma once

#include "Vulkan/Objects/QuadObj.h"

#include "Vulkan/Renderer/VulkanTexture2D.h"

class Background : public QuadObj
{
public:
	Background() = default;
	Background(VulkanCore* core);

	virtual ~Background();

	glm::mat4 GetModelMatrix() { return m_ModelMatrix; }

	std::unique_ptr<VulkanTexture2D> Texture;
private:

};
