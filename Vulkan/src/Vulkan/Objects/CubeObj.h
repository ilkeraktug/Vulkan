#pragma once

#include "Drawable.h"

class CubeObj : public Drawable
{
public:
	CubeObj() = default;
	CubeObj(VulkanCore* core);

	virtual ~CubeObj();

	virtual void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int i) override;
	virtual void draw(VkCommandBuffer commandBuffer) override;

	virtual void Rotate(float angle, const glm::vec3& axis);

};