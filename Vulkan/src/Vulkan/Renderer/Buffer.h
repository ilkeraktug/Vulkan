#pragma once

#include "Vulkan\Core.h"

enum VkFormat;

enum class ShaderType
{
	None = -1, Float, Float2, Float3, Float4
};

static VkFormat ShaderTypeToVkFormat(ShaderType type)
{
	switch (type)
	{
	case ShaderType::None:		return VK_FORMAT_UNDEFINED;
	case ShaderType::Float:		return VK_FORMAT_R32_SFLOAT;
	case ShaderType::Float2:	return VK_FORMAT_R32G32_SFLOAT;
	case ShaderType::Float3:	return VK_FORMAT_R32G32B32_SFLOAT;
	case ShaderType::Float4:	return VK_FORMAT_R32G32B32A32_SFLOAT;
	}

	return VK_FORMAT_UNDEFINED;
}

static uint32_t ShaderDataTypeSize(ShaderType type)
{
	switch (type)
	{
	case ShaderType::Float:		return 4;
	case ShaderType::Float2:	return 4 * 2;
	case ShaderType::Float3:	return 4 * 3;
	case ShaderType::Float4:	return 4 * 4;
	}

	VK_ASSERT(false, "Unknown ShaderDataType!");
}

struct BufferElement
{
	std::string Name;
	ShaderType Type;
	uint32_t Offset;
	uint32_t Size;

	BufferElement() = default;
	BufferElement(const std::string& name, ShaderType type)
	: Name(name), Type(type), Offset(0), Size(ShaderDataTypeSize(type))	{}
};

class BufferLayout
{
public:
	BufferLayout() = default;
	BufferLayout(const std::initializer_list<BufferElement>& list)
		:m_Elements(list)
	{
		CalculateStrideAndOffset();
	}

	inline const uint32_t GetStride() const { return m_Stride; }
	inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

private:
	void CalculateStrideAndOffset()
	{
		uint32_t offset = 0;
		
		for (auto& element : m_Elements)
		{
			element.Offset = offset;
			offset += element.Size;
			m_Stride += element.Size;
		}
	}
private:
	std::vector<BufferElement> m_Elements;
	uint32_t m_Stride = 0;
};