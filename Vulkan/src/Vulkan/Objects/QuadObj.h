#pragma once

#include "Drawable.h"


class QuadObj : public Drawable
{
public:
	QuadObj() {};
	QuadObj(VulkanCore* core);

	virtual void OnUpdate(float deltaTime) override;

	virtual void allocDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout* descriptorLayout) override;

private:
	void updateUniformBuffers();
private:

	VulkanCore* m_Core;
};