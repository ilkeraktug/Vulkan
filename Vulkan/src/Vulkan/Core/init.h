#pragma once

#include <vulkan\vulkan.h>
#include "Core.h"

namespace init 
{
	inline VkCommandPoolCreateInfo commandPoolCreateInfo()
	{
		VkCommandPoolCreateInfo commandPoolCI{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };

		return commandPoolCI;
	}

	inline VkCommandBufferAllocateInfo commandBufferAllocateInfo()
	{
		VkCommandBufferAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		return allocateInfo;
	}

	inline VkCommandBufferBeginInfo commandBufferBeginInfo()
	{
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		return beginInfo;
	}

	inline VkRenderPassBeginInfo renderPassBeginInfo()
	{
		VkRenderPassBeginInfo beginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		return beginInfo;
	}


	inline VkImageCreateInfo imageCreateInfo()
	{
		VkImageCreateInfo imageCI{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		return imageCI;
	}

	inline VkMemoryAllocateInfo memAllocInfo()
	{
		VkMemoryAllocateInfo memAllocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		return memAllocInfo;
	}	
	
	inline VkSemaphoreCreateInfo semaphoreCreateInfo()
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		return semaphoreCreateInfo;
	}	
	
	inline VkSubmitInfo submitInfo()
	{
		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		return submitInfo;
	}	
	
	inline VkPresentInfoKHR presentInfo()
	{
		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		return presentInfo;
	}	
		
	inline VkPipelineLayoutCreateInfo pipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		return pipelineLayoutCreateInfo;
	}	
	
	inline VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyState()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		return inputAssembly;
	}
		
	inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputState()
	{
		VkPipelineVertexInputStateCreateInfo vertexInput{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		return vertexInput;
	}	
	
	inline VkPipelineRasterizationStateCreateInfo pipelineRasterizationState()
	{
		VkPipelineRasterizationStateCreateInfo rasterizationInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		return rasterizationInfo;
	}	
	
	inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendState()
	{
		VkPipelineColorBlendStateCreateInfo blendStateInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		return blendStateInfo;
	}	
	
	inline VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilState()
	{
		VkPipelineDepthStencilStateCreateInfo depthStencilState{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		return depthStencilState;
	}	
	
	inline VkGraphicsPipelineCreateInfo pipelineCreateInfo()
	{
		VkGraphicsPipelineCreateInfo pipelineCreateInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		return pipelineCreateInfo;
	}	
	
	inline VkPipelineViewportStateCreateInfo pipelineViewportState()
	{
		VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		return viewportState;
	}	
	
	inline VkPipelineMultisampleStateCreateInfo multiSampleState()
	{
		VkPipelineMultisampleStateCreateInfo multiSampleState{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		return multiSampleState;
	}	
	
	inline VkPipelineDynamicStateCreateInfo dynamicState()
	{
		VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		return dynamicState;
	}	
	
	inline VkCommandBufferBeginInfo cmdBufferBeginInfo()
	{
		VkCommandBufferBeginInfo cmdBufferBI{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		return cmdBufferBI;
	}	

	inline VkBufferCreateInfo createBufferInfo(uint32_t size, VkBufferUsageFlags usage)
	{
		VkBufferCreateInfo bufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = usage;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		
		return bufferCreateInfo;
	}

	inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo()
	{
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		
		return descriptorSetLayoutCI;
	}	
	
	inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo()
	{
		VkDescriptorPoolCreateInfo descriptorPoolCI{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		
		return descriptorPoolCI;
	}	
	
	inline VkDescriptorSetAllocateInfo descriptorSetAllocateInfo()
	{
		VkDescriptorSetAllocateInfo descriptorSetAI{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		
		return descriptorSetAI;
	}
}